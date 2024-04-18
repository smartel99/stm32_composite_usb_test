/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usbd_cdc_if.h
 * @version        : v3.0_Cube
 * @brief          : Header for usbd_cdc_if.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef USB_COMPOSITE_APP_USBD_CDC_IF_H
#    define USB_COMPOSITE_APP_USBD_CDC_IF_H

#    ifdef __cplusplus
extern "C" {
#    endif

/* Includes ------------------------------------------------------------------*/
#    include "usbd_dcdc.h"

/* USER CODE BEGIN INCLUDE */
#    include <stdbool.h>
#    include <stddef.h>
#    include <stdint.h>
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @brief For Usb device.
 * @{
 */

/** @defgroup USBD_CDC_IF USBD_CDC_IF
 * @brief Usb VCP device module
 * @{
 */

/** @defgroup USBD_CDC_IF_Exported_Defines USBD_CDC_IF_Exported_Defines
 * @brief Defines.
 * @{
 */
/* USER CODE BEGIN EXPORTED_DEFINES */

/* USER CODE END EXPORTED_DEFINES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Exported_Types USBD_CDC_IF_Exported_Types
 * @brief Types.
 * @{
 */

/* USER CODE BEGIN EXPORTED_TYPES */
typedef struct CDC_DeviceInfo CDC_DeviceInfo;

/**
 * Function called upon receiving data on a USB endpoint.
 *
 * CDC_DeviceInfo*: Pointer to the device that received the data.
 * void*: User Data.
 * const uint8_t*: Pointer to the data.
 * size_t: Number of bytes received.
 */
typedef void (*CDC_onReceive_t)(CDC_DeviceInfo*, void*, const uint8_t*, size_t);

/* USER CODE END EXPORTED_TYPES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Exported_Macros USBD_CDC_IF_Exported_Macros
 * @brief Aliases.
 * @{
 */

/* USER CODE BEGIN EXPORTED_MACRO */

/* USER CODE END EXPORTED_MACRO */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
 * @brief Public variables.
 * @{
 */

/** CDC Interface callback. */
extern USBD_DCDC_ItfTypeDef g_usbdInterfaceFopsFs;

/* USER CODE BEGIN EXPORTED_VARIABLES */
extern const char*    g_usbFrasyTag;
extern const char*    g_usbDebugTag;
extern CDC_DeviceInfo g_usbFrasy;
extern CDC_DeviceInfo g_usbDebug;
/* USER CODE END EXPORTED_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_CDC_IF_Exported_FunctionsPrototype USBD_CDC_IF_Exported_FunctionsPrototype
 * @brief Public functions declaration.
 * @{
 */

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
uint8_t CDC_Transmit_FS(CDC_DeviceInfo* device, const uint8_t* buf, uint16_t len);

/* USER CODE BEGIN EXPORTED_FUNCTIONS */
void CDC_InitLoggers(void);
void CDC_SetOnReceived(CDC_DeviceInfo* device, CDC_onReceive_t onReceive, void* userData);

bool CDC_IsConnected(CDC_DeviceInfo* device);
bool CDC_IsBusy(CDC_DeviceInfo* device);

size_t CDC_GetRxBufferSize(CDC_DeviceInfo* device);

size_t CDC_GetTxBufferSize(CDC_DeviceInfo* device);
size_t CDC_GetTxBufferTakenSize(CDC_DeviceInfo* device);
size_t CDC_GetTxBufferAvailableSize(CDC_DeviceInfo* device);
/**
 * Adds data to the TX queue.
 *
 * Automatically schedules the buffer to be sent when full, or when a message bigger than the space available tries to
 * be added.
 * @param device
 * @param buf
 * @param len
 * @return Number of bytes written to the queue, 0 on error.
 */
size_t CDC_Queue(CDC_DeviceInfo* device, const uint8_t* buf, size_t len);
/**
 * Transmit all queued data.
 * @param device
 * @return
 */
uint8_t CDC_SendQueue(CDC_DeviceInfo* device);


void CDCInternal_SetLastTxCompleteTimestamp(uint32_t timestamp, uint8_t endpoint);
void CDCInternal_SetConnectedState(CDC_DeviceInfo* device, bool state);

inline const char* usbdResToStr(USBD_StatusTypeDef res)
{
    switch (res) {
        case USBD_OK: return "OK";
        case USBD_BUSY: return "BUSY";
        case USBD_EMEM: return "EMEM";
        case USBD_FAIL: return "FAIL";
        default: return "Unknown";
    }
}
/* USER CODE END EXPORTED_FUNCTIONS */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#    ifdef __cplusplus
}
#    endif

#endif /* USB_COMPOSITE_APP_USBD_CDC_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
