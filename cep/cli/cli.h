/**
 * @file    cli.h
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


#ifndef CEP_CLI_CLI_H
#define CEP_CLI_CLI_H

#include "usbd_cdc_if.h"

#include <logging/logger.h>

#include <FreeRTOS.h>
#include <FreeRTOS_CLI.h>
#include <stream_buffer.h>
#include <task.h>

#include <cstddef>
#include <cstdint>

class CLI {
public:
    CLI(CDC_DeviceInfo* device);
    CLI(const CLI&)            = delete;
    CLI(CLI&&)                 = delete;
    CLI& operator=(const CLI&) = delete;
    CLI& operator=(CLI&&)      = delete;
    ~CLI();

private:
    static void onReceive(CDC_DeviceInfo* device, void* userData, const uint8_t* data, size_t len);

    [[noreturn]] static void cliTask(void* args);

private:
    static constexpr const char*    s_tag   = "CLI";
    static constexpr Logging::Level s_level = Logging::Level::debug;

    CDC_DeviceInfo* m_device;

    StreamBufferHandle_t    m_streamBuff     = nullptr;
    static constexpr size_t s_streamBuffSize = 128;

    TaskHandle_t                 m_task          = nullptr;
    static constexpr size_t      s_taskStackSize = 512;
    static constexpr UBaseType_t s_taskPriority  = 5;
    volatile bool                m_isTaskRunning = false;
    volatile bool                m_shouldTaskRun = true;
};

#endif    // CEP_CLI_CLI_H
