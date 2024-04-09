/**
 * @file    tasks.h
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


#ifndef CEP_CLI_BUILT_INS_TASKS_H
#define CEP_CLI_BUILT_INS_TASKS_H
#include <FreeRTOS.h>
#include <FreeRTOS_CLI.h>

#include <cstddef>

namespace cli {
BaseType_t tasksCommand(char* writeBuffer, size_t writeBufferLen, const char* commandStr);

constexpr CLI_Command_Definition_t s_tasks = {
  "tasks", /* The command string to type. */
  "\r\ntasks:\r\n Displays a table showing the state of each FreeRTOS task\r\n\r\n",
  tasksCommand, /* The function to run. */
  0             /* No parameters are expected. */
};
}    // namespace cli
#endif    // CEP_CLI_BUILT_INS_TASKS_H
