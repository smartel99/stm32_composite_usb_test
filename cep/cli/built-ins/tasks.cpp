/**
 * @file    tasks.cpp
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
#include "tasks.h"

#include <FreeRTOS.h>
#include <task.h>

#include <cstdio>

namespace cli {
namespace {
TaskStatus_t* tasks        = nullptr;
size_t        atTask       = 0;
size_t        taskCount    = 0;
uint32_t      totalRuntime = 0;

char statusToChar(eTaskState state)
{
    switch (state) {
        case eRunning: return 'X';
        case eReady: return 'R';
        case eBlocked: return 'B';
        case eSuspended: return 'S';
        case eDeleted: return 'D';
        case eInvalid:
        default: return 'U';
    }
}
}    // namespace
BaseType_t tasksCommand(char* writeBuffer, size_t writeBufferLen, const char* commandStr)
{
    char*  writeBufferPtr = writeBuffer;
    size_t at             = 0;
    size_t written        = 0;
    if (tasks == nullptr) {
        taskCount = uxTaskGetNumberOfTasks();
        tasks     = new TaskStatus_t[taskCount];
        taskCount = uxTaskGetSystemState(tasks, taskCount, &totalRuntime);

        written = std::snprintf(
          writeBufferPtr,
          writeBufferLen - written,
          "Task          State  Priority  Stack\t#\r\n************************************************\r\n");
        at += written;
        writeBufferPtr += written;
    }

    // We need 40 characters per tasks.
    static constexpr size_t s_charsPerTasks = 51;
    while ((at + s_charsPerTasks) < writeBufferLen && atTask < taskCount) {
        auto& task = tasks[atTask];
        written    = std::snprintf(writeBufferPtr,
                                writeBufferLen - at,
                                "%-15s %c\t%lu\t%u\t%lu\r\n",
                                task.pcTaskName,
                                statusToChar(task.eCurrentState),
                                task.uxCurrentPriority,
                                task.usStackHighWaterMark,
                                task.xTaskNumber);
        at += written;
        writeBufferPtr += written;
        atTask++;
    }

    if (atTask == taskCount) {
        delete[] tasks;
        tasks     = nullptr;
        atTask    = 0;
        taskCount = 0;
        return pdFALSE;
    }
    return pdTRUE;
}
}    // namespace cli
