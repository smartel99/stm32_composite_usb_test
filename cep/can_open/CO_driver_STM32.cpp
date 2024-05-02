/*
 * CAN module object for STM32 (FD)CAN peripheral IP.
 *
 * This file is a template for other microcontrollers.
 *
 * @file        CO_driver.c
 * @ingroup     CO_driver
 * @author      Hamed Jafarzadeh 	2022
 * 				Tilen Marjerle		2021
 * 				Janez Paternoster	2020
 * @copyright   2004 - 2020 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Implementation Author:               Tilen Majerle <tilen@majerle.eu>
 */
#include "301/CO_driver.h"
#include "CO_app_STM32.h"

#include "can_manager.h"

#include <cstring>

#pragma clang diagnostic push
#pragma ide diagnostic   ignored "OCInconsistentNamingInspection"
#if defined(__cplusplus)
extern "C" {
#endif

/* Local CAN module object */
static CO_CANmodule_t* CANModule_local = nullptr; /* Local instance of global CAN module */

/* CAN masks for identifiers */
#define CANID_MASK 0x07FF /*!< CAN standard ID mask */
#define FLAG_RTR   0x8000 /*!< RTR flag, part of identifier */


void* CO_alloc(size_t num, size_t size)
{
    void* ptr = pvPortMalloc(num * size);
    if (ptr != nullptr) {
        std::memset(ptr, 0, num*size);
    }
    return ptr;
}

void CO_free(void* ptr)
{
    vPortFree(ptr);
}

/******************************************************************************/
void CO_CANsetConfigurationMode(void* CANptr)
{
    /* Put CAN module in configuration mode */
    if (CANptr != nullptr) { HAL_FDCAN_Stop(static_cast<CanopenNodeStm32*>(CANptr)->canHandle); }
}

/******************************************************************************/
void CO_CANsetNormalMode(CO_CANmodule_t* CANmodule)
{
    /* Put CAN module in normal mode */
    if (CANmodule->CANptr != nullptr) {
        if (HAL_FDCAN_Start(static_cast<CanopenNodeStm32*>(CANmodule->CANptr)->canHandle) == HAL_OK) {
            CANmodule->CANnormal = true;
        }
    }
}

/******************************************************************************/
CO_ReturnError_t CO_CANmodule_init(CO_CANmodule_t* CANmodule,
                                   void*           CANptr,
                                   CO_CANrx_t      rxArray[],
                                   uint16_t        rxSize,
                                   CO_CANtx_t      txArray[],
                                   uint16_t        txSize,
                                   uint16_t        CANbitRate)
{

    /* verify arguments */
    if (CANmodule == nullptr || rxArray == nullptr || txArray == nullptr) { return CO_ERROR_ILLEGAL_ARGUMENT; }

    /* Hold CANModule variable */
    CANmodule->CANptr = CANptr;

    /* Keep a local copy of CANModule */
    CANModule_local = CANmodule;

    /* Configure object variables */
    CANmodule->rxArray           = rxArray;
    CANmodule->rxSize            = rxSize;
    CANmodule->txArray           = txArray;
    CANmodule->txSize            = txSize;
    CANmodule->CANerrorStatus    = 0;
    CANmodule->CANnormal         = false;
    CANmodule->useCANrxFilters   = false; /* Do not use HW filters */
    CANmodule->bufferInhibitFlag = false;
    CANmodule->firstCANtxMessage = true;
    CANmodule->CANtxCount        = 0U;
    CANmodule->errOld            = 0U;

    /* Reset all variables */
    for (uint16_t i = 0U; i < rxSize; i++) {
        rxArray[i].ident          = 0U;
        rxArray[i].mask           = 0xFFFFU;
        rxArray[i].object         = nullptr;
        rxArray[i].CANrx_callback = nullptr;
    }
    for (uint16_t i = 0U; i < txSize; i++) {
        txArray[i].bufferFull = false;
    }

    /***************************************/
    /* STM32 related configuration */
    /***************************************/
    static_cast<CanopenNodeStm32*>(CANptr)->hwInitFunction();

    /*
     * Configure global filter that is used as last check if message did not pass any of other filters:
     *
     * We do not rely on hardware filters in this example
     * and are performing software filters instead
     *
     * Accept non-matching standard ID messages
     * Reject non-matching extended ID messages
     */
    if (HAL_FDCAN_ConfigGlobalFilter(static_cast<CanopenNodeStm32*>(CANptr)->canHandle,
                                     FDCAN_ACCEPT_IN_RX_FIFO0,
                                     FDCAN_ACCEPT_IN_RX_FIFO1,
                                     FDCAN_FILTER_REMOTE,
                                     FDCAN_FILTER_REMOTE) != HAL_OK) {
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }
    /* Enable notifications */
    /* Activate the CAN notification interrupts */
    if (HAL_FDCAN_ActivateNotification(static_cast<CanopenNodeStm32*>(CANptr)->canHandle,
                                       0 | FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO1_NEW_MESSAGE |
                                         FDCAN_IT_TX_COMPLETE | FDCAN_IT_TX_FIFO_EMPTY | FDCAN_IT_BUS_OFF |
                                         FDCAN_IT_ARB_PROTOCOL_ERROR | FDCAN_IT_DATA_PROTOCOL_ERROR |
                                         FDCAN_IT_ERROR_PASSIVE | FDCAN_IT_ERROR_WARNING,
                                       FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2) != HAL_OK) {
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }

    return CO_ERROR_NO;
}

/******************************************************************************/
void CO_CANmodule_disable(CO_CANmodule_t* CANmodule)
{
    if (CANmodule != nullptr && CANmodule->CANptr != nullptr) {
        HAL_FDCAN_Stop(static_cast<CanopenNodeStm32*>(CANmodule->CANptr)->canHandle);
    }
}

/******************************************************************************/
CO_ReturnError_t CO_CANrxBufferInit(CO_CANmodule_t* CANmodule,
                                    uint16_t        index,
                                    uint16_t        ident,
                                    uint16_t        mask,
                                    bool_t          rtr,
                                    void*           object,
                                    void            (*CANrx_callback)(void* object, void* message))
{
    CO_ReturnError_t ret = CO_ERROR_NO;

    if (CANmodule != nullptr && object != nullptr && CANrx_callback != nullptr && index < CANmodule->rxSize) {
        CO_CANrx_t* buffer = &CANmodule->rxArray[index];

        /* Configure object variables */
        buffer->object         = object;
        buffer->CANrx_callback = CANrx_callback;

        /*
         * Configure global identifier, including RTR bit
         *
         * This is later used for RX operation match case
         */
        buffer->ident = (ident & CANID_MASK) | (rtr ? FLAG_RTR : 0x00);
        buffer->mask  = (mask & CANID_MASK) | FLAG_RTR;

        /* Set CAN hardware module filter and mask. */
        if (CANmodule->useCANrxFilters) { __NOP(); }
    }
    else {
        ret = CO_ERROR_ILLEGAL_ARGUMENT;
    }

    return ret;
}

CO_CANtx_t* CO_CANtxBufferInit(
  CO_CANmodule_t* CANmodule, uint16_t index, uint16_t ident, bool_t rtr, uint8_t noOfBytes, bool_t syncFlag)
{
    CO_CANtx_t* buffer = nullptr;

    if (CANmodule != nullptr && index < CANmodule->txSize) {
        buffer = &CANmodule->txArray[index];

        /* CAN identifier, DLC and rtr, bit aligned with CAN module transmit buffer */
        buffer->ident      = ((uint32_t)ident & CANID_MASK) | ((uint32_t)(rtr ? FLAG_RTR : 0x00));
        buffer->DLC        = noOfBytes;
        buffer->bufferFull = false;
        buffer->syncFlag   = syncFlag;
    }
    return buffer;
}

/**
 * \brief           Send CAN message to network
 * This function must be called with atomic access.
 *
 * \param[in]       CANmodule: CAN module instance
 * \param[in]       buffer: Pointer to buffer to transmit
 */
static uint8_t prvSendCanMessage(CO_CANtx_t* buffer)
{
    CanManager::get().transmit(SlCan::Packet(buffer->ident, false, &buffer->data[0], buffer->DLC));
    return 1;
}

/******************************************************************************/
CO_ReturnError_t CO_CANsend(CO_CANmodule_t* CANmodule, CO_CANtx_t* buffer)
{
    CO_ReturnError_t err = CO_ERROR_NO;

    /* Verify overflow */
    if (buffer->bufferFull) {
        if (!CANmodule->firstCANtxMessage) {
            /* don't set error, if bootup message is still on buffers */
            CANmodule->CANerrorStatus |= CO_CAN_ERRTX_OVERFLOW;
        }
        err = CO_ERROR_TX_OVERFLOW;
    }

    /*
     * Send message to CAN network
     *
     * Lock interrupts for atomic operation
     */
    if (prvSendCanMessage(buffer)) { CANmodule->bufferInhibitFlag = buffer->syncFlag; }
    else {
        buffer->bufferFull = true;
        CANmodule->CANtxCount++;
    }

    return err;
}

/******************************************************************************/
void CO_CANclearPendingSyncPDOs(CO_CANmodule_t* CANmodule)
{
    uint32_t tpdoDeleted = 0U;

    CO_LOCK_CAN_SEND(CANmodule);
    /* Abort message from CAN module, if there is synchronous TPDO.
     * Take special care with this functionality. */
    if (/*messageIsOnCanBuffer && */ CANmodule->bufferInhibitFlag) {
        /* clear TXREQ */
        CANmodule->bufferInhibitFlag = false;
        tpdoDeleted                  = 1U;
    }
    /* delete also pending synchronous TPDOs in TX buffers */
    if (CANmodule->CANtxCount > 0) {
        for (uint16_t i = CANmodule->txSize; i > 0U; --i) {
            if (CANmodule->txArray[i].bufferFull) {
                if (CANmodule->txArray[i].syncFlag) {
                    CANmodule->txArray[i].bufferFull = false;
                    CANmodule->CANtxCount--;
                    tpdoDeleted = 2U;
                }
            }
        }
    }
    CO_UNLOCK_CAN_SEND(CANmodule);
    if (tpdoDeleted != 0u) { CANmodule->CANerrorStatus |= CO_CAN_ERRTX_PDO_LATE; }
}

/******************************************************************************/
/* Get error counters from the module. If necessary, function may use
 * different way to determine errors. */
namespace {
[[maybe_unused]] uint16_t rxErrors = 0;
[[maybe_unused]] uint16_t txErrors = 0;
[[maybe_unused]] uint16_t overflow = 0;

}    // namespace

void CO_CANmodule_process(CO_CANmodule_t* CANmodule)
{
    uint32_t err = 0;

    // CANOpen just care about Bus_off, Warning, Passive and Overflow
    // I didn't find overflow error register in STM32, if you find it please let me know
    err = ((FDCAN_HandleTypeDef*)((CanopenNodeStm32*)CANmodule->CANptr)->canHandle)->Instance->PSR &
          (FDCAN_PSR_BO | FDCAN_PSR_EW | FDCAN_PSR_EP);

    if (CANmodule->errOld != err) {
        uint16_t status = CANmodule->CANerrorStatus;

        CANmodule->errOld = err;

        if (err & FDCAN_PSR_BO) {
            status |= CO_CAN_ERRTX_BUS_OFF;
            // In this driver we expect that the controller is automatically handling the protocol exceptions.
        }
        else {
            /* recalculate CANerrorStatus, first clear some flags */
            status &= 0xFFFF ^ (CO_CAN_ERRTX_BUS_OFF | CO_CAN_ERRRX_WARNING | CO_CAN_ERRRX_PASSIVE |
                                CO_CAN_ERRTX_WARNING | CO_CAN_ERRTX_PASSIVE);

            if (err & FDCAN_PSR_EW) { status |= CO_CAN_ERRRX_WARNING | CO_CAN_ERRTX_WARNING; }

            if (err & FDCAN_PSR_EP) { status |= CO_CAN_ERRRX_PASSIVE | CO_CAN_ERRTX_PASSIVE; }
        }

        CANmodule->CANerrorStatus = status;
    }
}

#if defined(__cplusplus)
}
#endif

/**
 * \brief           Read message from RX FIFO
 * \param           hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
 *                      the configuration information for the specified FDCAN.
 * \param[in]       fifo: Fifo number to use for read
 * \param[in]       fifo_isrs: List of interrupts for respected FIFO
 */
void prv_read_can_received_msg(const SlCan::Packet& packet)
{
    CO_CANrxMsg_t rcvMsg;
    CO_CANrx_t*   buffer       = nullptr; /* receive message buffer from CO_CANmodule_t object. */
    uint16_t      index        = 0;       /* index of received message */
    uint32_t      rcvMsgIdent  = 0;       /* identifier of the received message */
    uint8_t       messageFound = 0;

    /* Setup identifier (with RTR) and length */
    rcvMsg.ident = packet.data.packetData.id | (packet.data.packetData.isRemote ? FLAG_RTR : 0x00);
    rcvMsg.dlc   = packet.data.packetData.dataLen;
    memcpy(&rcvMsg.data[0], &packet.data.packetData.data[0], rcvMsg.dlc);
    rcvMsgIdent = rcvMsg.ident;

    /*
     * Hardware filters are not used for the moment
     * \todo: Implement hardware filters...
     */
    if (CANModule_local->useCANrxFilters) { __BKPT(0); }
    else {
        /*
         * We are not using hardware filters, hence it is necessary
         * to manually match received message ID with all buffers
         */
        buffer = CANModule_local->rxArray;
        for (index = CANModule_local->rxSize; index > 0U; --index, ++buffer) {
            if (((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U) {
                messageFound = 1;
                break;
            }
        }
    }

    /* Call specific function, which will process the message */
    if ((messageFound != 0u) && buffer->CANrx_callback != nullptr) { buffer->CANrx_callback(buffer->object, &rcvMsg); }
}



#pragma clang diagnostic pop
