/**
 * @file    cli.cpp
 * @author  Samuel Martel
 * @date    2024-04-08
 * @brief
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <a href=https://www.gnu.org/licenses/>https://www.gnu.org/licenses/</a>.
 */
#include "cli.h"

#include "built-ins/built-ins.h"

#include <cstring>
#include <utility>

CLI::CLI(CDC_DeviceInfo* device) : m_device(device), m_streamBuff(xStreamBufferCreate(s_streamBuffSize, 1))
{
    Logging::Logger::setLevel(s_level);
    CDC_SetOnReceived(m_device, &onReceive, this);

    configASSERT(m_streamBuff != nullptr);

    xTaskCreate(&cliTask, "CLI_Task", s_taskStackSize, this, configMAX_PRIORITIES - 1, &m_task);
    configASSERT(m_task != nullptr);

    for (auto&& command : cli::s_builtInCommands) {
        FreeRTOS_CLIRegisterCommand(&command);
    }
}

CLI::~CLI()
{
    if (m_isTaskRunning && m_task != nullptr) {
        // Ask the worker to shut down; it will delete itself.
        m_shouldTaskRun = false;
        // Increase its priority to ours +1 so that it can stop sooner.
        vTaskPrioritySet(m_task, uxTaskPriorityGet(nullptr) + 1);
        while (m_isTaskRunning) {
            portYIELD();
        }

        // We can now assume that the worker will not be using the message buffer, and that producers will not try
        // to take the semaphore and write to the message buffer.
        vStreamBufferDelete(m_streamBuff);
    }
}

void CLI::onReceive(CDC_DeviceInfo* device, void* userData, const uint8_t* data, size_t len)
{
    auto& that = *reinterpret_cast<CLI*>(userData);
    xStreamBufferSendFromISR(that.m_streamBuff, data, len, nullptr);
}

[[noreturn]] void CLI::cliTask(void* args)
{
    configASSERT(args != nullptr);

    auto& that = *reinterpret_cast<CLI*>(args);

    static constexpr size_t cmdBufferSize = 128;
    char*                   cmdBuffer     = new char[cmdBufferSize];
    size_t                  writeIndex    = 0;

    char* outBuff = FreeRTOS_CLIGetOutputBuffer();

    that.m_isTaskRunning = true;
    vTaskPrioritySet(nullptr, s_taskPriority);

    LOGI(s_tag, "Started");

    constexpr const char s_shell[] = "> ";
    CDC_Transmit_FS(that.m_device, reinterpret_cast<const uint8_t*>(s_shell), sizeof(s_shell));

    while (that.m_shouldTaskRun) {
        char received[16] = {};
        if (size_t read = xStreamBufferReceive(that.m_streamBuff, &received, sizeof(received), pdMS_TO_TICKS(100));
            read != 0) {

            for (size_t i = 0; i < read; i++) {
                if (received[i] == '\n' || received[i] == '\r') {
                    CDC_Queue(that.m_device, reinterpret_cast<const uint8_t*>("\n"), 1);
                    LOGI(s_tag, "Received command: (%d bytes) %s", writeIndex, cmdBuffer);
                    // Dispatch the command.
                    BaseType_t moreToCome = pdFALSE;
                    do {
                        moreToCome = FreeRTOS_CLIProcessCommand(cmdBuffer, outBuff, configCOMMAND_INT_MAX_OUTPUT_SIZE);
                        size_t lenOutBuff = std::strlen(outBuff);

                        if (lenOutBuff != 0) {
                            CDC_Queue(that.m_device, reinterpret_cast<const uint8_t*>(outBuff), lenOutBuff);
                            std::memset(outBuff, '\0', lenOutBuff);
                        }
                    } while (moreToCome != pdFALSE);

                    std::memset(cmdBuffer, '\0', writeIndex);
                    writeIndex = 0;
                    CDC_Queue(that.m_device, reinterpret_cast<const uint8_t*>(s_shell), sizeof(s_shell));
                }
                else if (received[i] == '\b') {
                    // Backspace was pressed. Erase the last character in the string, if any.
                    if (writeIndex > 0) {
                        writeIndex--;
                        cmdBuffer[writeIndex]              = '\0';
                        constexpr const char s_eraseLast[] = "\b \b";
                        CDC_Queue(that.m_device, reinterpret_cast<const uint8_t*>(s_eraseLast), sizeof(s_eraseLast));
                    }
                }
                else {
                    // Add the character to the string, if possible. Otherwise, make bing
                    if (writeIndex < cmdBufferSize - 1) {
                        cmdBuffer[writeIndex] = received[i];
                        writeIndex++;
                        CDC_Queue(that.m_device, reinterpret_cast<const uint8_t*>(&received[i]), 1);
                    }
                    else {
                        // Only thing possible when full is to press enter or erase characters.
                        static constexpr const char s_bing[] = "\a";
                        CDC_Queue(that.m_device, reinterpret_cast<const uint8_t*>(s_bing), sizeof(s_bing));
                    }
                }
            }
            CDC_SendQueue(that.m_device);
        }
    }

    delete[] cmdBuffer;

    that.m_isTaskRunning = false;
    vTaskDelete(nullptr);
    std::unreachable();
}
