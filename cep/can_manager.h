/**
 * @file    can_manager.h
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


#ifndef CEP_CAN_MANAGER_H
#define CEP_CAN_MANAGER_H

#include "fdcan.h"
#include "slcan/slcan.h"
#include "usbd_cdc_if.h"

#include <logging/logger.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <cstddef>
#include <cstdint>


class CanManager {
    static CanManager* s_instance;
    struct [[gnu::packed]] RxPacket;

public:
    static bool        init(CDC_DeviceInfo* usb, FDCAN_HandleTypeDef* hcan);
    static CanManager& get() { return *s_instance; }

    // Sends on both CAN and USB.
    void transmit(const SlCan::Packet& packet);
    void transmitFromIrq(const SlCan::Packet& packet);

private:
    explicit CanManager(CDC_DeviceInfo* usb, FDCAN_HandleTypeDef* hcan);

    friend void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs);
    friend void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo1ITs);
    friend void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t BufferIndexes);
    friend void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs);
    friend void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef* hfdcan);
    void        receiveFromIrq(const RxPacket& packet);

    void transmitPacketOverUsb(const SlCan::Packet& packet);
    void transmitPacketOverCan(const SlCan::Packet& packet, bool isFromRxTask);

    [[noreturn]] static void txTask(void* args);
    [[noreturn]] static void rxTask(void* args);

    void handleCanError();

private:
    static constexpr const char*    s_tag   = "CAN";
    static constexpr Logging::Level s_level = Logging::Level::info;


    CDC_DeviceInfo*      m_usb = nullptr;
    FDCAN_HandleTypeDef* m_can = nullptr;

    static constexpr size_t s_txTaskStackSize = 384;
    static constexpr size_t s_txTaskPriority  = 7;
    TaskHandle_t            m_txTask          = nullptr;
    static constexpr size_t s_txQueueSize     = 15;
    QueueHandle_t           m_txQueue         = nullptr;

    static constexpr size_t s_rxTaskStackSize = 512;
    static constexpr size_t s_rxTaskPriority  = 7;
    TaskHandle_t            m_rxTask          = nullptr;
    static constexpr size_t s_rxQueueSize     = 150;    //!< 24 bytes per item, 18 bytes when gnu::packed.
    QueueHandle_t           m_rxQueue         = nullptr;

    static constexpr size_t s_maxDroppedCanPackets = 5;
    size_t                  m_droppedCanPackets    = 0;

    static constexpr uint8_t s_maxAttemptsPerCanPacket = 5;
    uint8_t                  m_attemptsForCanPacket    = 0;

    bool m_rxTaskWaitingForTxRoom = false;
    bool m_txTaskWaitingForTxRoom = false;
};
#endif    // CEP_CAN_MANAGER_H
