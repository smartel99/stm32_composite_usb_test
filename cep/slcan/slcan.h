/**
 * @file    slcan.h
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


#ifndef CEP_SLCAN_H
#define CEP_SLCAN_H

#include "enums/auto_retransmit.h"
#include "enums/bitrates.h"
#include "enums/commands.h"
#include "enums/modes.h"

#include "fdcan.h"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace SlCan {
struct Packet {
    // maximum rx buffer len: extended CAN frame with timestamp
    static constexpr std::size_t s_mtu = 30;    // (sizeof("T1111222281122334455667788EA5F\r")+1)

    Command command = Command::Invalid;
    union {
        BitRates       bitrate;           // Active when command == Command::SetBitrate
        Modes          mode;              // Active when command == Command::SetMode
        AutoRetransmit autoRetransmit;    // Active when command == Command::SetAutoRetry
        struct {
            uint32_t id;
            bool     isExtended;
            bool     isRemote;
            uint8_t  dataLen;
            uint8_t  data[8];
        } packetData;    // Active when command == Command::Transmit*
    } data {.bitrate = BitRates::bInvalid};

    Packet() = default;
    explicit Packet(BitRates bitRates) : command(Command::SetBitRate), data {.bitrate = bitRates} {}
    explicit Packet(Modes mode) : command(Command::SetMode), data {.mode = mode} {}
    explicit Packet(AutoRetransmit autoRetransmit)
    : command(Command::SetAutoRetry), data {.autoRetransmit = autoRetransmit}
    {
    }
    static Packet openChannel()
    {
        Packet pkt {};
        pkt.command = Command::OpenChannel;
        return pkt;
    }
    static Packet closeChannel()
    {
        Packet pkt {};
        pkt.command = Command::CloseChannel;
        return pkt;
    }
    static Packet reportError()
    {
        Packet pkt {};
        pkt.command = Command::ReportError;
        return pkt;
    }
    static Packet getVersion()
    {
        Packet pkt {};
        pkt.command = Command::GetVersion;
        return pkt;
    }

    Packet(const uint8_t* data, size_t len);                                         // Serial -> CAN
    Packet(const FDCAN_RxHeaderTypeDef& header, const uint8_t* data, size_t len);    // CAN -> Serial
    Packet(const FDCAN_TxHeaderTypeDef& header, const uint8_t* data, size_t len);    // CAN -> Serial

    /**
     * Translates the CAN packet in SLCAN format.
     * @param outBuff Buffer where to write the SLCAN packet.
     * @param outBuffLen Size of the output buffer.
     * @return Number of bytes written to outBuff. -1 on error.
     */
    [[nodiscard]] int8_t                               toSerial(uint8_t* outBuff, size_t outBuffLen) const;
    [[nodiscard]] std::optional<FDCAN_RxHeaderTypeDef> toFDCANRxHeader() const;
    [[nodiscard]] std::optional<FDCAN_TxHeaderTypeDef> toFDCANTxHeader() const;
};
}    // namespace SlCan

#endif    // CEP_SLCAN_H
