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
#include <utility>

namespace SlCan {
namespace {
constexpr const char* s_tag = "SlCan";

Command commandFromHeader(bool isExtended, bool isRemote)
{
    if (!isExtended && !isRemote) { return Command::TransmitDataFrame; }
    if (isExtended && !isRemote) { return Command::TransmitExtDataFrame; }
    if (!isExtended && isRemote) { return Command::TransmitRemoteFrame; }
    if (isExtended && isRemote) { return Command::TransmitExtRemoteFrame; }
    std::unreachable();
}
}    // namespace

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
        size_t idLen = this->data.packetData.isExtended ? s_extIdLen : s_stdIdLen;

        if (len < idLen) {
            LOGE(s_tag, "Not enough data for header, need %d, got %d", idLen, len);
            command = Command::Invalid;
            return;
        }

        uint8_t* ptr             = &workBuff[0];
        this->data.packetData.id = 0;
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
                this->data.packetData.data[i] = (ptr[0] << 4) | ptr[1];
                ptr += 2;
            }
        }
    }
}

Packet::Packet(uint32_t id, bool extended)
: command(extended ? Command::TransmitExtRemoteFrame : Command::TransmitRemoteFrame),
  data {
    .packetData =
      {
        .id         = id,
        .isExtended = extended,
        .isRemote   = true,
        .dataLen    = 0,
        .data       = {},
      },
  }
{
}

Packet::Packet(uint32_t id, bool extended, const uint8_t* data, size_t len)
: command(extended ? Command::TransmitExtDataFrame : Command::TransmitDataFrame),
  data {
    .packetData =
      {
        .id         = id,
        .isExtended = extended,
        .isRemote   = false,
        .dataLen    = static_cast<uint8_t>(len),
        .data       = {},
      },
  }
{
    configASSERT(len <= 8 && "DLC can't be more than 8!");
    std::copy(data, data + len, &this->data.packetData.data[0]);
}

Packet::Packet(const FDCAN_RxHeaderTypeDef& header, const uint8_t* data, size_t len)
: command(commandFromHeader(header.IdType == FDCAN_EXTENDED_ID, header.RxFrameType == FDCAN_REMOTE_FRAME)),
  data {
    .packetData =
      {
        .id         = header.Identifier,
        .isExtended = header.IdType == FDCAN_EXTENDED_ID,
        .isRemote   = header.RxFrameType == FDCAN_REMOTE_FRAME,
        .dataLen    = static_cast<uint8_t>(header.DataLength),
        .data       = {},
      },
  }
{
    configASSERT(header.DataLength == len && "len provided doesn't match header's value");
    configASSERT(len <= 8 && "DLC can't be more than 8!");
    std::copy(data, data + len, &this->data.packetData.data[0]);
}

Packet::Packet(const FDCAN_TxHeaderTypeDef& header, const uint8_t* data, size_t len)
: command(commandFromHeader(header.IdType == FDCAN_EXTENDED_ID, header.TxFrameType == FDCAN_REMOTE_FRAME)),
  data {
    .packetData =
      {
        .id         = header.Identifier,
        .isExtended = header.IdType == FDCAN_EXTENDED_ID,
        .isRemote   = header.TxFrameType == FDCAN_REMOTE_FRAME,
        .dataLen    = static_cast<uint8_t>(header.DataLength),
        .data       = {},
      },
  }
{
    configASSERT(header.DataLength == len && "len provided doesn't match header's value");
    configASSERT(len <= 8 && "DLC can't be more than 8!");
    std::copy(data, data + len, &this->data.packetData.data[0]);
}

int8_t Packet::toSerial(uint8_t* outBuff, size_t outBuffLen) const
{
    if (outBuffLen < sizeOfSerialPacket()) {
        // Buffer not big enough!
        return -1;
    }

    uint8_t* ptr = outBuff;

    // Add character for frame type.
    *ptr = static_cast<uint8_t>(command);
    ++ptr;

    auto addToBuff = [](uint8_t* buff, uint8_t val) -> uint8_t* {
        // Convert the value to ASCII.
        if (val <= 9) { val += '0'; }
        else if (val >= 0xA && val <= 0xF) {
            val = (val - 10) + 'A';
        }
        *buff = val;
        buff++;
        return buff;
    };

    auto addIdToBuff = [&addToBuff](uint8_t* buff, uint32_t id, size_t idLen) -> uint8_t* {
        for (size_t i = idLen; i > 0; i--) {
            addToBuff(&buff[i - 1], id & 0xF);
            id >>= 4;
        }
        return buff + idLen;
    };

    auto addDataToBuff = [&addToBuff](uint8_t* buff, const uint8_t* data, uint8_t len) -> uint8_t* {
        buff = addToBuff(buff, len);
        for (uint8_t i = 0; i < len; i++) {
            buff = addToBuff(buff, data[i] >> 4);
            buff = addToBuff(buff, data[i] & 0xF);
        }
        return buff;
    };

    switch (command) {
        case Command::SetBitRate: ptr = addToBuff(ptr, static_cast<uint8_t>(data.bitrate)); break;
        case Command::SetMode: ptr = addToBuff(ptr, static_cast<uint8_t>(data.mode)); break;
        case Command::SetAutoRetry: ptr = addToBuff(ptr, static_cast<uint8_t>(data.autoRetransmit)); break;
        case Command::TransmitDataFrame:
            ptr = addIdToBuff(ptr, data.packetData.id, s_stdIdLen);
            ptr = addDataToBuff(ptr, &data.packetData.data[0], data.packetData.dataLen);
            break;
        case Command::TransmitExtDataFrame:
            ptr = addIdToBuff(ptr, data.packetData.id, s_extIdLen);
            ptr = addDataToBuff(ptr, &data.packetData.data[0], data.packetData.dataLen);
            break;
        case Command::TransmitRemoteFrame: ptr = addIdToBuff(ptr, data.packetData.id, s_stdIdLen); break;
        case Command::TransmitExtRemoteFrame: ptr = addIdToBuff(ptr, data.packetData.id, s_extIdLen); break;
        case Command::GetVersion:
        case Command::ReportError:
        case Command::OpenChannel:
        case Command::CloseChannel:
        case Command::Invalid:
        default: break;
    }

    // Line terminator.
    *ptr = '\r';
    ++ptr;

    return static_cast<int8_t>(ptr - outBuff);
}

std::optional<FDCAN_RxHeaderTypeDef> Packet::toFDCANRxHeader() const
{
    if (!commandIsTransmit(command)) { return std::nullopt; }
    return FDCAN_RxHeaderTypeDef {
      .Identifier            = data.packetData.id,
      .IdType                = data.packetData.isExtended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID,
      .RxFrameType           = data.packetData.isRemote ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME,
      .DataLength            = data.packetData.dataLen,
      .ErrorStateIndicator   = FDCAN_ESI_ACTIVE,
      .BitRateSwitch         = FDCAN_BRS_OFF,
      .FDFormat              = FDCAN_CLASSIC_CAN,
      .RxTimestamp           = 0,
      .FilterIndex           = 0,
      .IsFilterMatchingFrame = 0,
    };
}

std::optional<FDCAN_TxHeaderTypeDef> Packet::toFDCANTxHeader() const
{
    if (!commandIsTransmit(command)) { return std::nullopt; }
    return FDCAN_TxHeaderTypeDef {
      .Identifier          = data.packetData.id,
      .IdType              = data.packetData.isExtended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID,
      .TxFrameType         = data.packetData.isRemote ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME,
      .DataLength          = data.packetData.dataLen,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch       = FDCAN_BRS_OFF,
      .FDFormat            = FDCAN_CLASSIC_CAN,
      .TxEventFifoControl  = FDCAN_NO_TX_EVENTS,
      .MessageMarker       = 0,
    };
}

size_t Packet::sizeOfSerialPacket() const
{
    switch (command) {
        case Command::OpenChannel:
        case Command::CloseChannel:
        case Command::GetVersion:
        case Command::ReportError: return 2;    // Command byte + \r
        case Command::SetBitRate:
        case Command::SetMode:
        case Command::SetAutoRetry: return 3;    // Command byte + value byte + \r
        case Command::TransmitDataFrame: return 3 + s_stdIdLen + data.packetData.dataLen;       // "t123dxxxx\r"
        case Command::TransmitExtDataFrame: return 3 + s_extIdLen + data.packetData.dataLen;    // "T12345678dxxxx\r"
        case Command::TransmitRemoteFrame: return 1 + s_stdIdLen + 1;                           // "r123\r"
        case Command::TransmitExtRemoteFrame: return 1 + s_extIdLen + 1;                        // "R12345678\r"
        case Command::Invalid:
        default: return 0;
    }
}
}    // namespace SlCan
