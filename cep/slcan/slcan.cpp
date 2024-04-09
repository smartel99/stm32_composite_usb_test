/**
 * @file    slcan.cpp
 * @author  Samuel Martel
 * @date    2024-04-09
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
#include "slcan.h"

#include <logging/logger.h>

#include <FreeRTOS.h>

#include <cctype>

namespace SlCan {
namespace {
constexpr const char* s_tag = "SlCan";
}
Packet::Packet(const uint8_t* data, size_t len)
{
    configASSERT(data != nullptr);
    configASSERT(len >= 2);    // Needs at least two characters: command and \r

    command = commandFromChar(data[0]);
    if (command == Command::Invalid) {
        LOGE(s_tag, "Invalid command ID %#02x", data[0]);
        return;
    }

    data++;
    len--;                                   // Remove the command.
    if (data[len - 1] == '\r') { len--; }    // Remove the terminator, if present.

    uint8_t workBuff[s_mtu];
    // Convert from ASCII.
    for (size_t i = 0; i < len; i++) {
        if (data[i] >= 'a' && data[i] <= 'f') { workBuff[i] = data[i] - 'a' + 10; }
        else if (data[i] >= 'A' && data[i] <= 'F') {
            workBuff[i] = data[i] - 'A' + 10;
        }
        else if (data[i] >= '0' && data[i] <= '9') {
            workBuff[i] = data[i] - '0';
        }
        else {
            // Invalid character!
            LOGE(s_tag,
                 "Invalid character '%c' (%#02x) at position %d",
                 std::isprint(data[i]) == 0 ? ' ' : data[i],
                 data[i],
                 i);
            command = Command::Invalid;
            return;
        }
    }

    if (command == Command::SetBitRate) { this->data.bitrate = bitRateFromChar(data[1]); }
    else if (command == Command::SetMode) {
        this->data.mode = modeFromStr(data[1]);
    }
    else if (command == Command::SetAutoRetry) {
        this->data.autoRetransmit = autoRetransmitFromStr(data[1]);
    }
    else if (commandIsTransmit(command)) {
        this->data.packetData.isExtended =
          (command == Command::TransmitExtRemoteFrame) || (command == Command::TransmitExtDataFrame);
        this->data.packetData.isRemote =
          (command == Command::TransmitRemoteFrame) || (command == Command::TransmitExtRemoteFrame);

        // Save the CAN ID based on the ID type.
        static constexpr size_t s_stdIdLen = 3;
        static constexpr size_t s_extIdLen = 8;
        size_t                  idLen      = this->data.packetData.isExtended ? s_extIdLen : s_stdIdLen;

        if (len < idLen) {
            LOGE(s_tag, "Not enough data for header, need %d, got %d", idLen, len);
            command = Command::Invalid;
            return;
        }

        uint8_t* ptr = &workBuff[0];
        for (size_t i = 0; i < idLen; i++, ptr++) {
            this->data.packetData.id *= 16;
            this->data.packetData.id += *ptr;
        }

        len -= idLen;
        if (!this->data.packetData.isRemote) {
            if (len == 0) {
                LOGE(s_tag, "DLC byte missing!");
                command = Command::Invalid;
                return;
            }

            this->data.packetData.dataLen = *ptr;
            ptr++;
            len--;

            if (this->data.packetData.dataLen > 8) {
                LOGE(s_tag, "Indicated data length is too big! (DLC = %d, max is 8)", this->data.packetData.dataLen);
                command = Command::Invalid;
                return;
            }

            if (this->data.packetData.dataLen * 2 > len) {
                LOGE(s_tag,
                     "Unexpected number of data bytes! Expected %d, got %d",
                     this->data.packetData.dataLen * 2,
                     len);
                command = Command::Invalid;
                return;
            }

            // Copy the packet data to the buffer.
            for (size_t i = 0; i < this->data.packetData.dataLen; i++) {
                this->data.packetData.data[i] = (ptr[0] << 4) & ptr[1];
                ptr += 2;
            }
        }
    }
}

Packet::Packet(const FDCAN_RxHeaderTypeDef& header, const uint8_t* data, size_t len)
{
}

Packet::Packet(const FDCAN_TxHeaderTypeDef& header, const uint8_t* data, size_t len)
{
}

int8_t Packet::toSerial(uint8_t* outBuff, size_t outBuffLen) const
{
    return 0;
}

std::optional<FDCAN_RxHeaderTypeDef> Packet::toFDCANRxHeader() const
{
    return FDCAN_RxHeaderTypeDef();
}

std::optional<FDCAN_TxHeaderTypeDef> Packet::toFDCANTxHeader() const
{
    return FDCAN_TxHeaderTypeDef();
}
}    // namespace SlCan
