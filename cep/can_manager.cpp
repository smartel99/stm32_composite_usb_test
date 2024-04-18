/**
 * @file    can_manager.cpp
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
#include "can_manager.h"

#include "fdcan.h"

#include <utility>

CanManager* CanManager::s_instance = nullptr;

namespace {
// Buffer so that CanManager is located in the bss segment.
enum class Origin : uint8_t { Can = 0, Usb, Unknown };
alignas(CanManager) unsigned char g_canManagerBuff[sizeof(CanManager)];

constexpr const char* originToStr(Origin origin)
{
    switch (origin) {
        case Origin::Can: return "CAN";
        case Origin::Usb: return "USB";
        case Origin::Unknown:
        default: return "Unknown";
    }
}
}    // namespace

struct [[gnu::packed]] CanManager::RxPacket {
    SlCan::Packet packet = {};
    Origin        origin = {};
};

extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs)
{
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        // :D
        FDCAN_RxHeaderTypeDef rx;
        // In theory, we only need 8 bytes. But in practice, the protocol supports up to 64 bytes...
        static uint8_t data[64];

        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx, &data[0]) != HAL_OK) {
            // Unable to get frame.
            return;
        }

        CanManager::get().receiveFromIrq({.packet = {rx, &data[0], rx.DataLength}, .origin = Origin::Can});
    }
}

extern "C" void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo1ITs)
{
    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != 0) {
        // :D
        FDCAN_RxHeaderTypeDef rx;
        // In theory, we only need 8 bytes. But in practice, the protocol supports up to 64 bytes...
        static uint8_t data[64];

        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx, &data[0]) != HAL_OK) {
            // Unable to get frame.
            return;
        }

        CanManager::get().receiveFromIrq({.packet = {rx, &data[0], rx.DataLength}, .origin = Origin::Can});
    }
}

CanManager::CanManager(CDC_DeviceInfo* usb) : m_usb(usb)
{
    static_assert(std::is_trivially_copyable_v<CanManager::RxPacket>);
    Logging::Logger::setLevel(s_tag, s_level);

    CDC_SetOnReceived(
      usb,
      [](CDC_DeviceInfo* dev, void* ud, const uint8_t* data, size_t len) {
          if (len > 0 && data[len - 1] == '\r') {
              get().receiveFromIrq({.packet = {data, len}, .origin = Origin::Usb});
          }
      },
      nullptr);

    m_txQueue = xQueueCreate(s_txQueueSize, sizeof(SlCan::Packet));
    configASSERT(m_txQueue != nullptr);

    m_rxQueue = xQueueCreate(s_rxQueueSize, sizeof(RxPacket));
    configASSERT(m_rxQueue != nullptr);

    auto res = xTaskCreate(&txTask, "can_tx", s_txTaskStackSize, this, s_txTaskPriority, &m_txTask);
    configASSERT(res == pdPASS);

    res = xTaskCreate(&rxTask, "can_rx", s_rxTaskStackSize, this, s_rxTaskPriority, &m_rxTask);
    configASSERT(res == pdPASS);
}

bool CanManager::init(CDC_DeviceInfo* usb)
{
    if (s_instance != nullptr) {
        LOGE(s_tag, "Already initialized!");
        return false;
    }

    s_instance = new (&g_canManagerBuff[0]) CanManager(usb);
    return true;
}

void CanManager::transmit(const SlCan::Packet& packet)
{
    if (packet.command != SlCan::Command::Invalid) {
        // Don't queue invalid packets!
        xQueueSend(m_txQueue, &packet, portMAX_DELAY);
    }
}

void CanManager::transmitFromIrq(const SlCan::Packet& packet)
{
    if (packet.command != SlCan::Command::Invalid) {
        // Don't queue invalid packets!
        auto res = xQueueSendFromISR(m_txQueue, &packet, nullptr);
        // If we assert false here, we're not sending messages fast enough. Increase the size of the queue, or augment
        // the priority of the task.
        configASSERT(res == pdPASS && "Unable to queue packet");
    }
}

void CanManager::receiveFromIrq(const CanManager::RxPacket& packet)
{
    if (packet.packet.command != SlCan::Command::Invalid) {
        // Don't queue invalid packets!
        auto res = xQueueSendFromISR(m_rxQueue, &packet, nullptr);
        // If we assert false here, we're not reading messages fast enough. Increase the size of the queue, or augment
        // the priority of the task.
        configASSERT(res == pdPASS && "Unable to queue packet");
    }
}

void CanManager::transmitPacketOverUsb(const SlCan::Packet& packet)
{
    static int missed = 0;
    if (!CDC_IsConnected(m_usb)) { return; }
    uint8_t buff[SlCan::Packet::s_mtu];
    int8_t  len = packet.toSerial(&buff[0], sizeof(buff));
    if (len > 0) {
        // TODO We should take advantage of the USB Tx FIFO so that we can just yeet our data in.
        if (CDC_Queue(m_usb, &buff[0], len) == 0) {
            // Wait a lil bit, then retry.
            // Since the host *should* poll us every millisecond, only waiting for that amount of time
            // should be good enough:tm:
            vTaskDelay(pdMS_TO_TICKS(2));
            if (CDC_Queue(m_usb, &buff[0], len) == 0) {
                buff[len] = '\0';    // Remove the \r, it's fine to do so here, since we're dropping the packet lol
                LOGW(s_tag, "USB forward fail #%d: %s", ++missed, &buff[0]);
            }
        }
    }
}

[[noreturn]] void CanManager::txTask(void* args)
{
    configASSERT(args != nullptr);
    auto& that = *static_cast<CanManager*>(args);

    LOGD(s_tag, "TX task started");

    volatile bool t = true;
    while (t) {
        SlCan::Packet packet;
        if (xQueueReceive(that.m_txQueue, &packet, portMAX_DELAY) == pdTRUE) {
            if (SlCan::commandIsTransmit(packet.command)) {
                // Send on CAN and USB.
                auto header = packet.toFDCANTxHeader();
                if (!header.has_value()) { LOGE(s_tag, "Unable to convert packet to TX header!"); }
                else {
                    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &*header, &packet.data.packetData.data[0]);
                }

                that.transmitPacketOverUsb(packet);
            }
        }
    }

    vTaskDelete(nullptr);
    std::unreachable();
}

[[noreturn]] void CanManager::rxTask(void* args)
{
    configASSERT(args != nullptr);
    auto& that = *static_cast<CanManager*>(args);

    LOGD(s_tag, "RX task started");

    volatile bool t = true;
    while (t) {
        RxPacket packet;
        if (xQueueReceive(that.m_rxQueue, &packet, portMAX_DELAY) == pdTRUE) {
            if (SlCan::commandIsTransmit(packet.packet.command)) {
#define X(field) packet.packet.data.packetData.field
                //                LOGD(s_tag,
                //                     "Received packet on %s. ID: %#x, isExt: %d, isRem: %d, DLC: %d",
                //                     originToStr(packet.origin),
                //                     X(id),
                //                     X(isExtended),
                //                     X(isRemote),
                //                     X(dataLen));
                //                LOG_BUFFER_HEXDUMP_LEVEL(s_tag, Logging::Level::trace, &X(data[0]), X(dataLen));

                if (packet.origin == Origin::Usb) {
                    // Retransmit on CAN.
                    auto header = packet.packet.toFDCANTxHeader();
                    if (!header.has_value()) { LOGE(s_tag, "Unable to convert packet to TX header!"); }
                    else {
                        HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &*header, &X(data[0]));
                    }
                }
                else if (packet.origin == Origin::Can) {
                    that.transmitPacketOverUsb(packet.packet);
                }
#undef X
            }

            // Handle the packet.
        }
    }

    vTaskDelete(nullptr);
    std::unreachable();
}
