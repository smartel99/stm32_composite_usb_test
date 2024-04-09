/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usbd_cdc_if.c
 * @version        : v3.0_Cube
 * @brief          : Usb device for Virtual Com Port.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include "vendor/logging/logger.h"

#include <FreeRTOS.h>
#include <string_view>

#ifdef __cplusplus
extern "C" {
#endif
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @brief Usb device library.
 * @{
 */

/** @addtogroup USBD_CDC_IF
 * @{
 */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
 * @brief Private types.
 * @{
 */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
 * @brief Private defines.
 * @{
 */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
 * @brief Private macros.
 * @{
 */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
 * @brief Private variables.
 * @{
 */


/* USER CODE BEGIN PRIVATE_VARIABLES */
const char* g_usbFrasyTag = "USB_FRASY";    // Not static so that another TU can set the USB's loggers sink(s).
const char* g_usbDebugTag = "USB_DEBUG";    // Not static so that another TU can set the USB's loggers sink(s).
/* USER CODE END PRIVATE_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
 * @brief Public variables.
 * @{
 */

extern "C" USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
 * @brief Private functions declaration.
 * @{
 */

static int8_t cdcInitFs(USBD_CDC_HandleTypeDef* cdc);
static int8_t cdcDeInitFs(USBD_CDC_HandleTypeDef* cdc);
static int8_t cdcControlFs(USBD_CDC_HandleTypeDef* cdc, uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t cdcReceiveFs(USBD_CDC_HandleTypeDef* cdc, uint8_t* pbuf, uint32_t* len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
struct CDC_DeviceInfo {
    USBD_CDC_HandleTypeDef* handle = nullptr;
    const char*             logTag = nullptr;
    Logging::Level          logLevel;
    uint8_t*                txBuffer     = nullptr;
    size_t                  txBufferSize = 0;
    uint8_t*                writeCursor  = nullptr;

    uint8_t* rxBuffer     = nullptr;
    size_t   rxBufferSize = 0;

    size_t   lastFlushed        = 0;
    uint32_t lastTxCompleteTime = 0;

    CDC_onReceive_t onReceive   = [](CDC_DeviceInfo*, void*, const uint8_t*, size_t) {};
    void*           onReceiveUD = nullptr;
};
namespace {

constexpr Logging::Level s_Frasylevel       = Logging::Level::debug;
constexpr Logging::Level s_Debuglevel       = Logging::Level::debug;
constexpr size_t         s_autoFlushAfterMs = 100;

/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
constexpr size_t s_frasyRxDataSize = 512;
constexpr size_t s_frasyTxDataSize = 512;
constexpr size_t s_debugRxDataSize = 512;
constexpr size_t s_debugTxDataSize = 1536;

/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t g_usbFrasyRxBuffer[s_frasyRxDataSize];
uint8_t g_usbDebugRxBuffer[s_debugRxDataSize];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t g_usbFrasyTxBuffer[s_frasyTxDataSize];
uint8_t g_usbDebugTxBuffer[s_debugTxDataSize];

CDC_DeviceInfo& deviceFromCdc(USBD_CDC_HandleTypeDef* handle)
{
    if (handle == g_usbFrasy.handle) { return g_usbFrasy; }
    if (handle == g_usbDebug.handle) { return g_usbDebug; }
    configASSERT(false && "Invalid CDC handle!");
}

constexpr const char* cdcCmdToStr(uint8_t cmd)
{
    switch (cmd) {
        case CDC_SEND_ENCAPSULATED_COMMAND: return "Send Encapsulated Command";
        case CDC_GET_ENCAPSULATED_RESPONSE: return "Get Encapsulated Response";
        case CDC_SET_COMM_FEATURE: return "Set Comm Feature";
        case CDC_GET_COMM_FEATURE: return "Get Comm Feature";
        case CDC_CLEAR_COMM_FEATURE: return "Clear Comm Feature";
        case CDC_SET_LINE_CODING: return "Set Line Coding";
        case CDC_GET_LINE_CODING: return "Get Line Coding";
        case CDC_SET_CONTROL_LINE_STATE: return "Set Control Line State";
        case CDC_SEND_BREAK: return "Send Break";
        default: return "Unknown";
    }
}

constexpr const char* usbStatusToStr(USBD_StatusTypeDef status)
{
    switch (status) {
        case USBD_OK: return "OK";
        case USBD_BUSY: return "Busy";
        case USBD_EMEM: return "Memory Error";
        case USBD_FAIL: return "Fail";
        default: return "Unknown";
    }
}

constexpr const char* stopBitsToStr(uint8_t stopBits)
{
    switch (stopBits) {
        case 0: return "1";
        case 1: return "1.5";
        case 2: return "2";
        default: return "<unknown>";
    }
}

constexpr const char* parityToStr(uint8_t parity)
{
    switch (parity) {
        case 0: return "None";
        case 1: return "Odd";
        case 2: return "Even";
        case 3: return "Mark";
        case 4: return "Space";
        default: return "<unknown>";
    }
}
}    // namespace

CDC_DeviceInfo g_usbFrasy {
  .handle       = &((USBD_DCDC_HandleTypeDef*)hUsbDeviceFS.pClassData)->frasyCdc,
  .logTag       = g_usbFrasyTag,
  .logLevel     = Logging::Level::info,
  .txBuffer     = &g_usbFrasyTxBuffer[0],
  .txBufferSize = s_frasyTxDataSize,
  .writeCursor  = &g_usbFrasyTxBuffer[0],
  .rxBuffer     = &g_usbFrasyRxBuffer[0],
  .rxBufferSize = s_frasyRxDataSize,
};

CDC_DeviceInfo g_usbDebug {
  .handle       = &((USBD_DCDC_HandleTypeDef*)hUsbDeviceFS.pClassData)->debugCdc,
  .logTag       = g_usbDebugTag,
  .logLevel     = Logging::Level::info,
  .txBuffer     = &g_usbDebugTxBuffer[0],
  .txBufferSize = s_debugTxDataSize,
  .writeCursor  = &g_usbDebugTxBuffer[0],
  .rxBuffer     = &g_usbDebugRxBuffer[0],
  .rxBufferSize = s_debugRxDataSize,
};
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
 * @}
 */

USBD_DCDC_ItfTypeDef g_usbdInterfaceFopsFs = {cdcInitFs, cdcDeInitFs, cdcControlFs, cdcReceiveFs};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Initializes the CDC media low layer over the FS USB IP
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t cdcInitFs(USBD_CDC_HandleTypeDef* cdc)
{
    /* USER CODE BEGIN 3 */
    /* Set Application Buffers */
    USBD_DCDC_HandleTypeDef* hcdc = (USBD_DCDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
    if (&hcdc->frasyCdc == cdc) { g_usbFrasy.handle = cdc; }
    if (&hcdc->debugCdc == cdc) { g_usbDebug.handle = cdc; }

    auto& device = deviceFromCdc(cdc);
    USBD_DCDC_SetTxBuffer(&hUsbDeviceFS, cdc, &device.txBuffer[0], 0);
    USBD_DCDC_SetRxBuffer(&hUsbDeviceFS, cdc, &device.rxBuffer[0]);
    Logging::Logger::setLevel(device.logTag, device.logLevel);
    LOGI(device.logTag, "Initialized endpoints");

    return (USBD_OK);
    /* USER CODE END 3 */
}

/**
 * @brief  DeInitializes the CDC media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t cdcDeInitFs([[maybe_unused]] USBD_CDC_HandleTypeDef* cdc)
{
    /* USER CODE BEGIN 4 */
    auto& device  = deviceFromCdc(cdc);
    device.handle = nullptr;
    LOGI(device.logTag, "De-initialized");
    Logging::Logger::clearLevel(device.logTag);

    return (USBD_OK);
    /* USER CODE END 4 */
}

/**
 * @brief  Manage the CDC class requests
 * @param  cmd: Command code
 * @param  pbuf: Buffer containing command data (request parameters)
 * @param  length: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t cdcControlFs(USBD_CDC_HandleTypeDef* cdc, uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
    /* USER CODE BEGIN 5 */
    auto& device = deviceFromCdc(cdc);
    switch (cmd) {
        case CDC_SEND_ENCAPSULATED_COMMAND: break;
        case CDC_GET_ENCAPSULATED_RESPONSE: break;
        case CDC_SET_COMM_FEATURE: break;
        case CDC_GET_COMM_FEATURE: break;
        case CDC_CLEAR_COMM_FEATURE:
            break;
            /*******************************************************************************/
            /* Line Coding Structure                                                       */
            /*-----------------------------------------------------------------------------*/
            /* Offset | Field       | Size | Value  | Description                          */
            /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
            /* 4      | bCharFormat |   1  | Number | Stop bits                            */
            /*                                        0 - 1 Stop bit                       */
            /*                                        1 - 1.5 Stop bits                    */
            /*                                        2 - 2 Stop bits                      */
            /* 5      | bParityType |  1   | Number | Parity                               */
            /*                                        0 - None                             */
            /*                                        1 - Odd                              */
            /*                                        2 - Even                             */
            /*                                        3 - Mark                             */
            /*                                        4 - Space                            */
            /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
            /*******************************************************************************/
        case CDC_SET_LINE_CODING:
        case CDC_GET_LINE_CODING:
            configASSERT(length == 7);
            LOGD(device.logTag,
                 "%s line coding, baud: %d, stops: %s, parity: %s, bits: %d",
                 cmd == CDC_SET_LINE_CODING ? "Setting" : "Getting",
                 *reinterpret_cast<uint32_t*>(&pbuf[0]),
                 stopBitsToStr(pbuf[4]),
                 parityToStr(pbuf[5]),
                 pbuf[6]);
            break;
        case CDC_SET_CONTROL_LINE_STATE: break;
        case CDC_SEND_BREAK: break;
        default: break;
    }

    return (USBD_OK);
    /* USER CODE END 5 */
}

/**
 * @brief  Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will block any OUT packet reception on USB endpoint
 *         until exiting this function. If you exit this function before transfer
 *         is complete on CDC interface (ie. using DMA controller) it will result
 *         in receiving more data while previous ones are still not sent.
 *
 * @param  pbuf: Buffer of data to be received
 * @param  len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t cdcReceiveFs(USBD_CDC_HandleTypeDef* cdc, uint8_t* pbuf, uint32_t* len)
{
    /* USER CODE BEGIN 6 */
    //    USBD_DCDC_HandleTypeDef* hcdc = (USBD_DCDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
    //    if (&hcdc->frasyCdc == cdc) { CDC_Transmit_FS(&hcdc->frasyCdc, pbuf, *len); }
    //    if (&hcdc->debugCdc == cdc) { CDC_Transmit_FS(&hcdc->debugCdc, pbuf, *len); }

    auto& device = deviceFromCdc(cdc);
    device.onReceive(&device, device.onReceiveUD, pbuf, *len);

    if (auto status = USBD_DCDC_SetRxBuffer(&hUsbDeviceFS, device.handle, &device.rxBuffer[0]); status != USBD_OK) {
        LOGE(device.logTag,
             "Unable to set rx buffer: (%#02x) %s",
             status,
             usbStatusToStr(static_cast<USBD_StatusTypeDef>(status)));
    }
    if (auto status = USBD_DCDC_ReceivePacket(&hUsbDeviceFS, device.handle); status != USBD_OK) {
        LOGE(device.logTag,
             "Unable to receive packet: (%#02x) %s",
             status,
             usbStatusToStr(static_cast<USBD_StatusTypeDef>(status)));
    }

    LOGD(device.logTag, "Received %d bytes", *len);
    LOG_BUFFER_HEXDUMP_LEVEL(g_usbDebugTag, Logging::Level::trace, pbuf, *len);

    return (USBD_OK);
    /* USER CODE END 6 */
}

/**
 * @brief  CDC_Transmit_FS
 *         Data to send over USB IN endpoint are sent over CDC interface
 *         through this function.
 *         @note
 *
 *
 * @param  buf: Buffer of data to be sent
 * @param  len: Number of data to be sent (in bytes)
 * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
 */
uint8_t CDC_Transmit_FS(CDC_DeviceInfo* device, const uint8_t* buf, uint16_t len)
{
    uint8_t result = USBD_OK;
    /* USER CODE BEGIN 7 */
    if (device->handle->TxState != 0) { return USBD_BUSY; }
    USBD_DCDC_SetTxBuffer(&hUsbDeviceFS, device->handle, const_cast<uint8_t*>(buf), len);
    result = USBD_DCDC_TransmitPacket(&hUsbDeviceFS, device->handle);
    if (result != USBD_OK) {
        LOGE(device->logTag,
             "Unable to transmit packet: (%#02x) %s",
             result,
             usbStatusToStr(static_cast<USBD_StatusTypeDef>(result)));
    }
    device->lastFlushed = HAL_GetTick();
    /* USER CODE END 7 */
    return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
void CDC_InitLoggers(void)
{
    Logging::Logger::setLevel(g_usbFrasy.logTag, g_usbFrasy.logLevel);
    Logging::Logger::setLevel(g_usbDebug.logTag, g_usbDebug.logLevel);
}

void CDC_SetOnReceived(CDC_DeviceInfo* device, CDC_onReceive_t onReceive, void* userData)
{
    configASSERT(device != nullptr);
    configASSERT(onReceive != nullptr);

    device->onReceive   = onReceive;
    device->onReceiveUD = userData;
}

bool CDC_IsBusy(CDC_DeviceInfo* device)
{
    return (device->handle->TxState != 0);
}

size_t CDC_GetRxBufferSize(CDC_DeviceInfo* device)
{
    return device->rxBufferSize;
}

size_t CDC_GetTxBufferSize(CDC_DeviceInfo* device)
{
    return device->txBufferSize;
}

size_t CDC_GetTxBufferTakenSize(CDC_DeviceInfo* device)
{
    return std::distance(&device->txBuffer[0], device->writeCursor);
}

size_t CDC_GetTxBufferAvailableSize(CDC_DeviceInfo* device)
{
    return device->txBufferSize - CDC_GetTxBufferTakenSize(device);
}

size_t CDC_Queue(CDC_DeviceInfo* device, const uint8_t* buf, size_t len)
{
    if (CDC_IsBusy(device) || CDC_GetTxBufferAvailableSize(device) < len) {
        // not enough room, send the buffer.
        CDC_SendQueue(device);
        return 0;
    }

    device->writeCursor = std::copy(buf, buf + len, device->writeCursor);

    if (HAL_GetTick() >= device->lastFlushed + s_autoFlushAfterMs) {
        // We haven't flushed in a while...
        CDC_SendQueue(device);
    }

    return len;
}

uint8_t CDC_SendQueue(CDC_DeviceInfo* device)
{
    auto res = CDC_Transmit_FS(device, &device->txBuffer[0], CDC_GetTxBufferTakenSize(device));
    if (res == USBD_OK) { device->writeCursor = &device->txBuffer[0]; }
    return res;
}

void CDCInternal_SetLastTxCompleteTimestamp(uint32_t timestamp, uint8_t endpoint)
{
    if (endpoint == 1) { g_usbFrasy.lastTxCompleteTime = timestamp; }
    if (endpoint == 2) { g_usbDebug.lastTxCompleteTime = timestamp; }
}


#ifdef __cplusplus
}
#endif
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
