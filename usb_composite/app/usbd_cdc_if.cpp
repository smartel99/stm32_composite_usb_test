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
/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE 2048
#define APP_TX_DATA_SIZE 2048
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
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t g_usbRxBuffer[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t g_usbTxBuffer[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
const char*                     g_usbTag = "USB";    // Not static so that another TU can set the USB's loggers sink(s).
static constexpr Logging::Level s_level  = Logging::Level::debug;
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
namespace {
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
    USBD_DCDC_SetTxBuffer(&hUsbDeviceFS, cdc, &g_usbTxBuffer[0], 0);
    USBD_DCDC_SetRxBuffer(&hUsbDeviceFS, cdc, &g_usbRxBuffer[0]);

    Logging::Logger::setLevel(g_usbTag, s_level);
    LOGI(g_usbTag, "Initialized");

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
    LOGI(g_usbTag, "De-initialized");
    Logging::Logger::clearLevel(g_usbTag);

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
            LOGD(g_usbTag,
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

    LOGT(g_usbTag, "Received %d bytes on control: (%#02x) %s", length, cmd, cdcCmdToStr(cmd));
    LOG_BUFFER_HEXDUMP_LEVEL(g_usbTag, Logging::Level::trace, pbuf, length);

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
    USBD_DCDC_HandleTypeDef* hcdc = (USBD_DCDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
    if (&hcdc->frasyCdc == cdc) { CDC_Transmit_FS(&hcdc->frasyCdc, pbuf, *len); }
    if (&hcdc->debugCdc == cdc) { CDC_Transmit_FS(&hcdc->debugCdc, pbuf, *len); }

    if (auto status = USBD_DCDC_SetRxBuffer(&hUsbDeviceFS, cdc, &pbuf[0]); status != USBD_OK) {
        LOGE(g_usbTag,
             "Unable to set rx buffer: (%#02x) %s",
             status,
             usbStatusToStr(static_cast<USBD_StatusTypeDef>(status)));
    }
    if (auto status = USBD_DCDC_ReceivePacket(&hUsbDeviceFS, cdc); status != USBD_OK) {
        LOGE(g_usbTag,
             "Unable to receive packet: (%#02x) %s",
             status,
             usbStatusToStr(static_cast<USBD_StatusTypeDef>(status)));
    }

    LOGD(g_usbTag, "Received %d bytes on %s's CDC", *len, cdc == &hcdc->frasyCdc ? "Frasy" : "Debug");
    LOG_BUFFER_HEXDUMP_LEVEL(g_usbTag, Logging::Level::trace, pbuf, *len);

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
 * @param  Buf: Buffer of data to be sent
 * @param  Len: Number of data to be sent (in bytes)
 * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
 */
uint8_t CDC_Transmit_FS(USBD_CDC_HandleTypeDef* cdc, uint8_t* Buf, uint16_t Len)
{
    uint8_t result = USBD_OK;
    /* USER CODE BEGIN 7 */
    if (cdc->TxState != 0) { return USBD_BUSY; }
    USBD_DCDC_SetTxBuffer(&hUsbDeviceFS, cdc, Buf, Len);
    result = USBD_DCDC_TransmitPacket(&hUsbDeviceFS, cdc);
    if (result != USBD_OK) {
        LOGE(g_usbTag,
             "Unable to transmit packet: (%#02x) %s",
             result,
             usbStatusToStr(static_cast<USBD_StatusTypeDef>(result)));
    }
    /* USER CODE END 7 */
    return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
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
