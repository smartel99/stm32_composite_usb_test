/**
 ******************************************************************************
 * @file    usbd_DCDC.c
 * @author  MCD Application Team
 * @brief   This file provides the high layer firmware functions to manage the
 *          following functionalities of the USB DCDC Class:
 *           - Initialization and Configuration of high and low layer
 *           - Enumeration as DCDC Device (and enumeration for each implemented memory interface)
 *           - OUT/IN data transfer
 *           - Command IN transfer (class requests management)
 *           - Error management
 *
 *  @verbatim
 *
 *          ===================================================================
 *                                DCDC Class Driver Description
 *          ===================================================================
 *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
 *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
 *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
 *           This driver implements the following aspects of the specification:
 *             - Device descriptor management
 *             - Configuration descriptor management
 *             - Enumeration as DCDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
 *             - Requests management (as described in section 6.2 in specification)
 *             - Abstract Control Model compliant
 *             - Union Functional collection (using 1 IN endpoint for control)
 *             - Data interface class
 *
 *           These aspects may be enriched or modified for a specific user application.
 *
 *            This driver doesn't implement the following aspects of the specification
 *            (but it is possible to manage these features with some modifications on this driver):
 *             - Any class-specific aspect relative to communication classes should be managed by user application.
 *             - All communication classes other than PSTN are not managed
 *
 *  @endverbatim
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                      www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "../Inc/usbd_dcdc.h"
#include "usbd_ctlreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
 * @{
 */


/** @defgroup USBD_DCDC
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_DCDC_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */


/** @defgroup USBD_DCDC_Private_Defines
 * @{
 */
/**
 * @}
 */


/** @defgroup USBD_DCDC_Private_Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup USBD_DCDC_Private_FunctionPrototypes
 * @{
 */


static uint8_t USBD_DCDC_Init(USBD_HandleTypeDef* pdev, uint8_t cfgidx);

static uint8_t USBD_DCDC_DeInit(USBD_HandleTypeDef* pdev, uint8_t cfgidx);

static uint8_t USBD_DCDC_Setup(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);

static uint8_t USBD_DCDC_DataIn(USBD_HandleTypeDef* pdev, uint8_t epnum);

static uint8_t USBD_DCDC_DataOut(USBD_HandleTypeDef* pdev, uint8_t epnum);

static uint8_t USBD_DCDC_EP0_RxReady(USBD_HandleTypeDef* pdev);

static uint8_t* USBD_DCDC_GetFSCfgDesc(uint16_t* length);

static uint8_t* USBD_DCDC_GetHSCfgDesc(uint16_t* length);

static uint8_t* USBD_DCDC_GetOtherSpeedCfgDesc(uint16_t* length);

static uint8_t* USBD_DCDC_GetOtherSpeedCfgDesc(uint16_t* length);

uint8_t* USBD_DCDC_GetDeviceQualifierDescriptor(uint16_t* length);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_DCDC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0xEF,
  0x02,
  0x01,
  0x40,
  0x01,
  0x00,
};

/**
 * @}
 */

/** @defgroup USBD_DCDC_Private_Variables
 * @{
 */


/* DCDC interface class callbacks structure */
USBD_ClassTypeDef USBD_DCDC = {
  USBD_DCDC_Init,
  USBD_DCDC_DeInit,
  USBD_DCDC_Setup,
  NULL, /* EP0_TxSent, */
  USBD_DCDC_EP0_RxReady,
  USBD_DCDC_DataIn,
  USBD_DCDC_DataOut,
  NULL,
  NULL,
  NULL,
  USBD_DCDC_GetHSCfgDesc,
  USBD_DCDC_GetFSCfgDesc,
  USBD_DCDC_GetOtherSpeedCfgDesc,
  USBD_DCDC_GetDeviceQualifierDescriptor,
};

/* USB DCDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_DCDC_CfgHSDesc[] __ALIGN_END = {
  /*Configuration Descriptor*/
  0x09,                        /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_DCDC_CONFIG_DESC_SIZ,    /* wTotalLength:no of returned bytes */
  0x00,
  0x04, /* bNumInterfaces: 4 interface */
  0x01, /* bConfigurationValue: Configuration value */
  0x00, /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0, /* bmAttributes: self powered */
  0x32, /* MaxPower 0 mA */

  /*---------------------------------------------------------------------------*/
  /* IAD */
  0x00, /* bFirstInterface */
  0x02, /* bInterfaceCount */
  0x02, /* bFunctionClass: CDC */
  0x02, /* bFunctionSubClass - Abstract Control Model */
  0x01, /* bFunctionProtocol - Common AT commands */
  0x02, /* iFunction */

  /*---------------------------------------------------------------------------*/

  /*Interface Descriptor */
  0x09,                    /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00, /* bInterfaceNumber: Number of Interface */
  0x00, /* bAlternateSetting: Alternate setting */
  0x01, /* bNumEndpoints: One endpoints used */
  0x02, /* bInterfaceClass: Communication Interface Class */
  0x02, /* bInterfaceSubClass: Abstract Control Model */
  0x01, /* bInterfaceProtocol: Common AT commands */
  0x00, /* iInterface: */

  /*Header Functional Descriptor*/
  0x05, /* bLength: Endpoint Descriptor size */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x00, /* bDescriptorSubtype: Header Func Desc */
  0x10, /* bDCDCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x01, /* bDescriptorSubtype: Call Management Func Desc */
  0x00, /* bmCapabilities: D0+D1 */
  0x01, /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x02, /* bDescriptorSubtype: Abstract Control Management desc */
  0x02, /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x06, /* bDescriptorSubtype: Union func desc */
  0x00, /* bMasterInterface: Communication class interface */
  0x01, /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,       /* bDescriptorType: Endpoint */
  DCDC_CMD_EP,                  /* bEndpointAddress */
  0x03,                         /* bmAttributes: Interrupt */
  LOBYTE(DCDC_CMD_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_CMD_PACKET_SIZE),
  DCDC_HS_BINTERVAL, /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,                    /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
  0x01,                    /* bInterfaceNumber: Number of Interface */
  0x00,                    /* bAlternateSetting: Alternate setting */
  0x02,                    /* bNumEndpoints: Two endpoints used */
  0x0A,                    /* bInterfaceClass: CDC */
  0x00,                    /* bInterfaceSubClass: */
  0x00,                    /* bInterfaceProtocol: */
  0x00,                    /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,                                 /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType: Endpoint */
  DCDC_OUT_EP,                          /* bEndpointAddress */
  0x02,                                 /* bmAttributes: Bulk */
  LOBYTE(DCDC_DATA_HS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_DATA_HS_MAX_PACKET_SIZE),
  0x00, /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,                                 /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType: Endpoint */
  DCDC_IN_EP,                           /* bEndpointAddress */
  0x02,                                 /* bmAttributes: Bulk */
  LOBYTE(DCDC_DATA_HS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_DATA_HS_MAX_PACKET_SIZE),
  0x00 /* bInterval: ignore for Bulk transfer */
  ,
  /*---------------------------------------------------------------------------*/
  /* IAD */
  0x02, /* bFirstInterface */
  0x02, /* bInterfaceCount */
  0x02, /* bFunctionClass: CDC */
  0x02, /* bFunctionSubClass - Abstract Control Model */
  0x01, /* bFunctionProtocol - Common AT commands */
  0x02, /* iFunction */

  /*---------------------------------------------------------------------------*/

  /*Interface Descriptor */
  0x09,                    /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x02, /* bInterfaceNumber: Number of Interface */
  0x00, /* bAlternateSetting: Alternate setting */
  0x01, /* bNumEndpoints: One endpoints used */
  0x02, /* bInterfaceClass: Communication Interface Class */
  0x02, /* bInterfaceSubClass: Abstract Control Model */
  0x01, /* bInterfaceProtocol: Common AT commands */
  0x00, /* iInterface: */

  /*Header Functional Descriptor*/
  0x05, /* bLength: Endpoint Descriptor size */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x00, /* bDescriptorSubtype: Header Func Desc */
  0x10, /* bDCDCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x01, /* bDescriptorSubtype: Call Management Func Desc */
  0x00, /* bmCapabilities: D0+D1 */
  0x01, /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x02, /* bDescriptorSubtype: Abstract Control Management desc */
  0x02, /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x06, /* bDescriptorSubtype: Union func desc */
  0x00, /* bMasterInterface: Communication class interface */
  0x01, /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,       /* bDescriptorType: Endpoint */
  DCDC_CMD_EP2,                 /* bEndpointAddress */
  0x03,                         /* bmAttributes: Interrupt */
  LOBYTE(DCDC_CMD_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_CMD_PACKET_SIZE),
  DCDC_HS_BINTERVAL, /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,                    /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
  0x03,                    /* bInterfaceNumber: Number of Interface */
  0x00,                    /* bAlternateSetting: Alternate setting */
  0x02,                    /* bNumEndpoints: Two endpoints used */
  0x0A,                    /* bInterfaceClass: CDC */
  0x00,                    /* bInterfaceSubClass: */
  0x00,                    /* bInterfaceProtocol: */
  0x00,                    /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,                                 /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType: Endpoint */
  DCDC_OUT_EP2,                         /* bEndpointAddress */
  0x02,                                 /* bmAttributes: Bulk */
  LOBYTE(DCDC_DATA_HS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_DATA_HS_MAX_PACKET_SIZE),
  0x00, /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,                                 /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType: Endpoint */
  DCDC_IN_EP2,                          /* bEndpointAddress */
  0x02,                                 /* bmAttributes: Bulk */
  LOBYTE(DCDC_DATA_HS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_DATA_HS_MAX_PACKET_SIZE),
  0x00 /* bInterval: ignore for Bulk transfer */
};


/* USB DCDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_DCDC_CfgFSDesc[] __ALIGN_END = {
  /*Configuration Descriptor*/
  0x09,                        /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  0x8D,                        /* wTotalLength:no of returned bytes */
  0x00,
  0x04, /* bNumInterfaces: 2 interface */
  0x01, /* bConfigurationValue: Configuration value */
  0x00, /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0, /* bmAttributes: self powered */
  0x32, /* MaxPower 0 mA */

  /*---------------------------------------------------------------------------*/
  /* IAD */
  0x08,
  USB_DESC_TYPE_IAD,
  0x00, /* bFirstInterface */
  0x02, /* bInterfaceCount */
  0x02, /* bFunctionClass: CDC */
  0x02, /* bFunctionSubClass - Abstract Control Model */
  0x01, /* bFunctionProtocol - Common AT commands */
  0x02, /* iFunction */
  /*---------------------------------------------------------------------------*/
  /*Interface Descriptor */
  0x09,                    /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00, /* bInterfaceNumber: Number of Interface */
  0x00, /* bAlternateSetting: Alternate setting */
  0x01, /* bNumEndpoints: One endpoints used */
  0x02, /* bInterfaceClass: Communication Interface Class */
  0x02, /* bInterfaceSubClass: Abstract Control Model */
  0x01, /* bInterfaceProtocol: Common AT commands */
  0x00, /* iInterface: */

  /*Header Functional Descriptor*/
  0x05, /* bLength: Endpoint Descriptor size */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x00, /* bDescriptorSubtype: Header Func Desc */
  0x10, /* bDCDCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x01, /* bDescriptorSubtype: Call Management Func Desc */
  0x00, /* bmCapabilities: D0+D1 */
  0x01, /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x02, /* bDescriptorSubtype: Abstract Control Management desc */
  0x02, /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x06, /* bDescriptorSubtype: Union func desc */
  0x00, /* bMasterInterface: Communication class interface */
  0x01, /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,       /* bDescriptorType: Endpoint */
  DCDC_CMD_EP,                  /* bEndpointAddress */
  0x03,                         /* bmAttributes: Interrupt */
  LOBYTE(DCDC_CMD_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_CMD_PACKET_SIZE),
  DCDC_FS_BINTERVAL, /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,                    /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
  0x01,                    /* bInterfaceNumber: Number of Interface */
  0x00,                    /* bAlternateSetting: Alternate setting */
  0x02,                    /* bNumEndpoints: Two endpoints used */
  0x0A,                    /* bInterfaceClass: DCDC */
  0x00,                    /* bInterfaceSubClass: */
  0x00,                    /* bInterfaceProtocol: */
  0x00,                    /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,                                 /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType: Endpoint */
  DCDC_OUT_EP,                          /* bEndpointAddress */
  0x02,                                 /* bmAttributes: Bulk */
  LOBYTE(DCDC_DATA_FS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_DATA_FS_MAX_PACKET_SIZE),
  0x00, /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,                                 /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType: Endpoint */
  DCDC_IN_EP,                           /* bEndpointAddress */
  0x02,                                 /* bmAttributes: Bulk */
  LOBYTE(DCDC_DATA_FS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_DATA_FS_MAX_PACKET_SIZE),
  0x00 /* bInterval: ignore for Bulk transfer */
  ,
  /*---------------------------------------------------------------------------*/
  /* IAD */
  0x08,
  USB_DESC_TYPE_IAD,
  0x02, /* bFirstInterface */
  0x02, /* bInterfaceCount */
  0x02, /* bFunctionClass: CDC */
  0x02, /* bFunctionSubClass - Abstract Control Model */
  0x01, /* bFunctionProtocol - Common AT commands */
  0x02, /* iFunction */
  /*---------------------------------------------------------------------------*/
  /*Interface Descriptor */
  0x09,                    /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x02, /* bInterfaceNumber: Number of Interface */
  0x00, /* bAlternateSetting: Alternate setting */
  0x01, /* bNumEndpoints: One endpoints used */
  0x02, /* bInterfaceClass: Communication Interface Class */
  0x02, /* bInterfaceSubClass: Abstract Control Model */
  0x01, /* bInterfaceProtocol: Common AT commands */
  0x00, /* iInterface: */

  /*Header Functional Descriptor*/
  0x05, /* bLength: Endpoint Descriptor size */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x00, /* bDescriptorSubtype: Header Func Desc */
  0x10, /* bDCDCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x01, /* bDescriptorSubtype: Call Management Func Desc */
  0x00, /* bmCapabilities: D0+D1 */
  0x01, /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x02, /* bDescriptorSubtype: Abstract Control Management desc */
  0x02, /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x06, /* bDescriptorSubtype: Union func desc */
  0x00, /* bMasterInterface: Communication class interface */
  0x01, /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,       /* bDescriptorType: Endpoint */
  DCDC_CMD_EP2,                 /* bEndpointAddress */
  0x03,                         /* bmAttributes: Interrupt */
  LOBYTE(DCDC_CMD_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_CMD_PACKET_SIZE),
  DCDC_FS_BINTERVAL, /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,                    /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
  0x03,                    /* bInterfaceNumber: Number of Interface */
  0x00,                    /* bAlternateSetting: Alternate setting */
  0x02,                    /* bNumEndpoints: Two endpoints used */
  0x0A,                    /* bInterfaceClass: DCDC */
  0x00,                    /* bInterfaceSubClass: */
  0x00,                    /* bInterfaceProtocol: */
  0x00,                    /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,                                 /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType: Endpoint */
  DCDC_OUT_EP2,                         /* bEndpointAddress */
  0x02,                                 /* bmAttributes: Bulk */
  LOBYTE(DCDC_DATA_FS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_DATA_FS_MAX_PACKET_SIZE),
  0x00, /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,                                 /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType: Endpoint */
  DCDC_IN_EP2,                          /* bEndpointAddress */
  0x02,                                 /* bmAttributes: Bulk */
  LOBYTE(DCDC_DATA_FS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_DATA_FS_MAX_PACKET_SIZE),
  0x00 /* bInterval: ignore for Bulk transfer */



};

__ALIGN_BEGIN uint8_t USBD_DCDC_OtherSpeedCfgDesc[USB_DCDC_CONFIG_DESC_SIZ] __ALIGN_END = {
  0x09, /* bLength: Configuation Descriptor size */
  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION,
  USB_DCDC_CONFIG_DESC_SIZ,
  0x00,
  0x02, /* bNumInterfaces: 2 interfaces */
  0x01, /* bConfigurationValue: */
  0x04, /* iConfiguration: */
  0xC0, /* bmAttributes: */
  0x32, /* MaxPower 100 mA */

  /*Interface Descriptor */
  0x09,                    /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00, /* bInterfaceNumber: Number of Interface */
  0x00, /* bAlternateSetting: Alternate setting */
  0x01, /* bNumEndpoints: One endpoints used */
  0x02, /* bInterfaceClass: Communication Interface Class */
  0x02, /* bInterfaceSubClass: Abstract Control Model */
  0x01, /* bInterfaceProtocol: Common AT commands */
  0x00, /* iInterface: */

  /*Header Functional Descriptor*/
  0x05, /* bLength: Endpoint Descriptor size */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x00, /* bDescriptorSubtype: Header Func Desc */
  0x10, /* bDCDCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x01, /* bDescriptorSubtype: Call Management Func Desc */
  0x00, /* bmCapabilities: D0+D1 */
  0x01, /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x02, /* bDescriptorSubtype: Abstract Control Management desc */
  0x02, /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05, /* bFunctionLength */
  0x24, /* bDescriptorType: CS_INTERFACE */
  0x06, /* bDescriptorSubtype: Union func desc */
  0x02, /* bMasterInterface: Communication class interface */
  0x03, /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,       /* bDescriptorType: Endpoint */
  DCDC_CMD_EP,                  /* bEndpointAddress */
  0x03,                         /* bmAttributes: Interrupt */
  LOBYTE(DCDC_CMD_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(DCDC_CMD_PACKET_SIZE),
  DCDC_FS_BINTERVAL, /* bInterval: */

  /*---------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,                    /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
  0x01,                    /* bInterfaceNumber: Number of Interface */
  0x00,                    /* bAlternateSetting: Alternate setting */
  0x02,                    /* bNumEndpoints: Two endpoints used */
  0x0A,                    /* bInterfaceClass: DCDC */
  0x00,                    /* bInterfaceSubClass: */
  0x00,                    /* bInterfaceProtocol: */
  0x00,                    /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,                   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: Endpoint */
  DCDC_OUT_EP,            /* bEndpointAddress */
  0x02,                   /* bmAttributes: Bulk */
  0x40,                   /* wMaxPacketSize: */
  0x00,
  0x00, /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,                   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: Endpoint */
  DCDC_IN_EP,             /* bEndpointAddress */
  0x02,                   /* bmAttributes: Bulk */
  0x40,                   /* wMaxPacketSize: */
  0x00,
  0x00 /* bInterval */
};

/**
 * @}
 */

/** @defgroup USBD_DCDC_Private_Functions
 * @{
 */

/**
 * @brief  USBD_DCDC_Init
 *         Initialize the DCDC interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_DCDC_Init(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
{
    uint8_t                  ret = 0U;
    USBD_DCDC_HandleTypeDef* hDCDC;

    if (pdev->dev_speed == USBD_SPEED_HIGH) {
        /* Open EP IN */
        USBD_LL_OpenEP(pdev, DCDC_IN_EP, USBD_EP_TYPE_BULK, DCDC_DATA_HS_IN_PACKET_SIZE);

        pdev->ep_in[DCDC_IN_EP & 0xFU].is_used = 1U;

        /* Open EP OUT */
        USBD_LL_OpenEP(pdev, DCDC_OUT_EP, USBD_EP_TYPE_BULK, DCDC_DATA_HS_OUT_PACKET_SIZE);

        pdev->ep_out[DCDC_OUT_EP & 0xFU].is_used = 1U;


        // CDC 2

        /* Open EP IN */
        USBD_LL_OpenEP(pdev, DCDC_IN_EP2, USBD_EP_TYPE_BULK, DCDC_DATA_HS_IN_PACKET_SIZE);

        pdev->ep_in[DCDC_IN_EP2 & 0xFU].is_used = 1U;

        /* Open EP OUT */
        USBD_LL_OpenEP(pdev, DCDC_OUT_EP2, USBD_EP_TYPE_BULK, DCDC_DATA_HS_OUT_PACKET_SIZE);

        pdev->ep_out[DCDC_OUT_EP2 & 0xFU].is_used = 1U;
    }
    else {
        /* Open EP IN */
        USBD_LL_OpenEP(pdev, DCDC_IN_EP, USBD_EP_TYPE_BULK, DCDC_DATA_FS_IN_PACKET_SIZE);

        pdev->ep_in[DCDC_IN_EP & 0xFU].is_used = 1U;

        /* Open EP OUT */
        USBD_LL_OpenEP(pdev, DCDC_OUT_EP, USBD_EP_TYPE_BULK, DCDC_DATA_FS_OUT_PACKET_SIZE);

        pdev->ep_out[DCDC_OUT_EP & 0xFU].is_used = 1U;

        // CDC 2
        /* Open EP IN */
        USBD_LL_OpenEP(pdev, DCDC_IN_EP2, USBD_EP_TYPE_BULK, DCDC_DATA_FS_IN_PACKET_SIZE);

        pdev->ep_in[DCDC_IN_EP2 & 0xFU].is_used = 1U;

        /* Open EP OUT */
        USBD_LL_OpenEP(pdev, DCDC_OUT_EP2, USBD_EP_TYPE_BULK, DCDC_DATA_FS_OUT_PACKET_SIZE);

        pdev->ep_out[DCDC_OUT_EP2 & 0xFU].is_used = 1U;
    }
    /* Open Command IN EP */
    USBD_LL_OpenEP(pdev, DCDC_CMD_EP, USBD_EP_TYPE_INTR, DCDC_CMD_PACKET_SIZE);
    pdev->ep_in[DCDC_CMD_EP & 0xFU].is_used = 1U;

    // CDC 2
    /* Open Command IN EP */
    USBD_LL_OpenEP(pdev, DCDC_CMD_EP2, USBD_EP_TYPE_INTR, DCDC_CMD_PACKET_SIZE);
    pdev->ep_in[DCDC_CMD_EP2 & 0xFU].is_used = 1U;

    pdev->pClassData = USBD_malloc(sizeof(USBD_DCDC_HandleTypeDef));

    if (pdev->pClassData == NULL) { ret = 1U; }
    else {
        hDCDC = (USBD_DCDC_HandleTypeDef*)pdev->pClassData;

        /* Init  physical Interface components */
        // TODO: user handlers for two cdc
        ((USBD_DCDC_ItfTypeDef*)pdev->pUserData)->Init(&hDCDC->frasyCdc);
        ((USBD_DCDC_ItfTypeDef*)pdev->pUserData)->Init(&hDCDC->debugCdc);

        /* Init Xfer states */
        hDCDC->frasyCdc.TxState = 0U;
        hDCDC->frasyCdc.RxState = 0U;

        hDCDC->debugCdc.TxState = 0U;
        hDCDC->debugCdc.RxState = 0U;


        if (pdev->dev_speed == USBD_SPEED_HIGH) {
            /* Prepare Out endpoint to receive next packet */
            USBD_LL_PrepareReceive(pdev, DCDC_OUT_EP, hDCDC->frasyCdc.RxBuffer, DCDC_DATA_HS_OUT_PACKET_SIZE);
            /* Prepare Out endpoint to receive next packet */
            USBD_LL_PrepareReceive(pdev, DCDC_OUT_EP, hDCDC->frasyCdc.RxBuffer, DCDC_DATA_HS_OUT_PACKET_SIZE);

            /* Prepare Out endpoint to receive next packet */
            USBD_LL_PrepareReceive(pdev, DCDC_OUT_EP2, hDCDC->debugCdc.RxBuffer, DCDC_DATA_HS_OUT_PACKET_SIZE);
            /* Prepare Out endpoint to receive next packet */
            USBD_LL_PrepareReceive(pdev, DCDC_OUT_EP2, hDCDC->debugCdc.RxBuffer, DCDC_DATA_HS_OUT_PACKET_SIZE);
        }
        else {
            /* Prepare Out endpoint to receive next packet */
            USBD_LL_PrepareReceive(pdev, DCDC_OUT_EP, hDCDC->frasyCdc.RxBuffer, DCDC_DATA_FS_OUT_PACKET_SIZE);

            /* Prepare Out endpoint to receive next packet */
            USBD_LL_PrepareReceive(pdev, DCDC_OUT_EP2, hDCDC->debugCdc.RxBuffer, DCDC_DATA_FS_OUT_PACKET_SIZE);
        }
    }
    return ret;
}

/**
 * @brief  USBD_DCDC_Init
 *         DeInitialize the DCDC layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_DCDC_DeInit(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
{
    uint8_t ret = 0U;

    /* Close EP IN */
    USBD_LL_CloseEP(pdev, DCDC_IN_EP);
    pdev->ep_in[DCDC_IN_EP & 0xFU].is_used = 0U;

    /* Close EP OUT */
    USBD_LL_CloseEP(pdev, DCDC_OUT_EP);
    pdev->ep_out[DCDC_OUT_EP & 0xFU].is_used = 0U;

    /* Close Command IN EP */
    USBD_LL_CloseEP(pdev, DCDC_CMD_EP);
    pdev->ep_in[DCDC_CMD_EP & 0xFU].is_used = 0U;

    /* Close EP IN */
    USBD_LL_CloseEP(pdev, DCDC_IN_EP2);
    pdev->ep_in[DCDC_IN_EP2 & 0xFU].is_used = 0U;

    /* Close EP OUT */
    USBD_LL_CloseEP(pdev, DCDC_OUT_EP2);
    pdev->ep_out[DCDC_OUT_EP2 & 0xFU].is_used = 0U;

    /* Close Command IN EP */
    USBD_LL_CloseEP(pdev, DCDC_CMD_EP2);
    pdev->ep_in[DCDC_CMD_EP2 & 0xFU].is_used = 0U;

    /* DeInit  physical Interface components */
    if (pdev->pClassData != NULL) {

        // TODO....
        USBD_DCDC_HandleTypeDef* hDCDC = (USBD_DCDC_HandleTypeDef*)pdev->pClassData;
        ((USBD_DCDC_ItfTypeDef*)pdev->pUserData)->DeInit(&hDCDC->frasyCdc);
        ((USBD_DCDC_ItfTypeDef*)pdev->pUserData)->DeInit(&hDCDC->debugCdc);
        USBD_free(pdev->pClassData);
        pdev->pClassData = NULL;
    }

    return ret;
}

/**
 * @brief  USBD_DCDC_Setup
 *         Handle the DCDC specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t USBD_DCDC_Setup(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
{
    USBD_DCDC_HandleTypeDef* hDCDC       = (USBD_DCDC_HandleTypeDef*)pdev->pClassData;
    uint8_t                  ifalt       = 0U;
    uint16_t                 status_info = 0U;
    uint8_t                  ret         = USBD_OK;


    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
        case USB_REQ_TYPE_CLASS:
            if (req->wLength) {
                if (req->bmRequest & 0x80U) {
                    if (req->wValue == 0 || req->wValue == 2) {
                        USBD_CDC_HandleTypeDef* cdc = req->wValue >> 1 ? &hDCDC->debugCdc : &hDCDC->frasyCdc;
                        ((USBD_DCDC_ItfTypeDef*)pdev->pUserData)
                          ->Control(cdc, req->bRequest, (uint8_t*)(void*)cdc->data, req->wLength);

                        USBD_CtlSendData(pdev, (uint8_t*)(void*)cdc->data, req->wLength);
                    }
                }
                else {
                    if (req->wValue == 0 || req->wValue == 2) {
                        USBD_CDC_HandleTypeDef* cdc = req->wValue >> 1 ? &hDCDC->debugCdc : &hDCDC->frasyCdc;
                        cdc->CmdOpCode              = req->bRequest;
                        cdc->CmdLength              = (uint8_t)req->wLength;

                        USBD_CtlPrepareRx(pdev, (uint8_t*)(void*)cdc->data, req->wLength);
                    }
                }
            }
            else {
                USBD_CDC_HandleTypeDef* cdc = req->wValue >> 1 ? &hDCDC->debugCdc : &hDCDC->frasyCdc;
                ((USBD_DCDC_ItfTypeDef*)pdev->pUserData)->Control(cdc, req->bRequest, (uint8_t*)(void*)req, 0U);
            }
            break;

        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest) {
                case USB_REQ_GET_STATUS:
                    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                        USBD_CtlSendData(pdev, (uint8_t*)(void*)&status_info, 2U);
                    }
                    else {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_GET_INTERFACE:
                    if (pdev->dev_state == USBD_STATE_CONFIGURED) { USBD_CtlSendData(pdev, &ifalt, 1U); }
                    else {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_SET_INTERFACE:
                    if (pdev->dev_state != USBD_STATE_CONFIGURED) {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                default:
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                    break;
            }
            break;

        default:
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
    }

    return ret;
}

/**
 * @brief  USBD_DCDC_DataIn
 *         Data sent on non-control IN endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_DCDC_DataIn(USBD_HandleTypeDef* pdev, uint8_t epnum)
{
    USBD_DCDC_HandleTypeDef* hDCDC = (USBD_DCDC_HandleTypeDef*)pdev->pClassData;
    PCD_HandleTypeDef*       hpcd  = pdev->pData;

    if (pdev->pClassData != NULL) {
        if ((pdev->ep_in[epnum].total_length > 0U) &&
            ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U)) {
            /* Update the packet total length */
            pdev->ep_in[epnum].total_length = 0U;

            /* Send ZLP */
            USBD_LL_Transmit(pdev, epnum, NULL, 0U);
        }
        else {
            // if(epnum == DCDC_IN_EP & EP_ADDR_MSK)
            if (epnum == (DCDC_IN_EP & 0xFU)) { hDCDC->frasyCdc.TxState = 0U; }
            // if(epnum == DCDC_IN_EP2 & EP_ADDR_MSK)
            if (epnum == (DCDC_IN_EP2 & 0xFU)) { hDCDC->debugCdc.TxState = 0U; }
        }
        return USBD_OK;
    }
    else {
        return USBD_FAIL;
    }
}

/**
 * @brief  USBD_DCDC_DataOut
 *         Data received on non-control Out endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_DCDC_DataOut(USBD_HandleTypeDef* pdev, uint8_t epnum)
{
    USBD_DCDC_HandleTypeDef* hDCDC = (USBD_DCDC_HandleTypeDef*)pdev->pClassData;

    /* Get the received data length */
    uint32_t rxSize = USBD_LL_GetRxDataSize(pdev, epnum);

    /* USB data will be immediately processed, this allow next USB traffic being
    NAKed till the end of the application Xfer */
    if (pdev->pClassData != NULL) {
        USBD_CDC_HandleTypeDef* cdc = &hDCDC->frasyCdc;
        if (epnum == DCDC_OUT_EP2) { cdc = &hDCDC->debugCdc; }
        cdc->RxLength = rxSize;
        ((USBD_DCDC_ItfTypeDef*)pdev->pUserData)->Receive(cdc, cdc->RxBuffer, &cdc->RxLength);

        return USBD_OK;
    }
    else {
        return USBD_FAIL;
    }
}

/**
 * @brief  USBD_DCDC_EP0_RxReady
 *         Handle EP0 Rx Ready event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_DCDC_EP0_RxReady(USBD_HandleTypeDef* pdev)
{
    USBD_DCDC_HandleTypeDef* hDCDC = (USBD_DCDC_HandleTypeDef*)pdev->pClassData;

    if ((pdev->pUserData != NULL) && (hDCDC->frasyCdc.CmdOpCode != 0xFFU)) {
        ((USBD_DCDC_ItfTypeDef*)pdev->pUserData)
          ->Control(&hDCDC->frasyCdc,
                    hDCDC->frasyCdc.CmdOpCode,
                    (uint8_t*)(void*)hDCDC->frasyCdc.data,
                    (uint16_t)hDCDC->frasyCdc.CmdLength);
        hDCDC->frasyCdc.CmdOpCode = 0xFFU;
    }
    return USBD_OK;
}

/**
 * @brief  USBD_DCDC_GetFSCfgDesc
 *         Return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_DCDC_GetFSCfgDesc(uint16_t* length)
{
    *length = sizeof(USBD_DCDC_CfgFSDesc);
    return USBD_DCDC_CfgFSDesc;
}

/**
 * @brief  USBD_DCDC_GetHSCfgDesc
 *         Return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_DCDC_GetHSCfgDesc(uint16_t* length)
{
    *length = sizeof(USBD_DCDC_CfgHSDesc);
    return USBD_DCDC_CfgHSDesc;
}

/**
 * @brief  USBD_DCDC_GetCfgDesc
 *         Return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_DCDC_GetOtherSpeedCfgDesc(uint16_t* length)
{
    *length = sizeof(USBD_DCDC_OtherSpeedCfgDesc);
    return USBD_DCDC_OtherSpeedCfgDesc;
}

/**
 * @brief  DeviceQualifierDescriptor
 *         return Device Qualifier descriptor
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_DCDC_GetDeviceQualifierDescriptor(uint16_t* length)
{
    *length = sizeof(USBD_DCDC_DeviceQualifierDesc);
    return USBD_DCDC_DeviceQualifierDesc;
}

/**
 * @brief  USBD_DCDC_RegisterInterface
 * @param  pdev: device instance
 * @param  fops: CD  Interface callback
 * @retval status
 */
uint8_t USBD_DCDC_RegisterInterface(USBD_HandleTypeDef* pdev, USBD_DCDC_ItfTypeDef* fops)
{
    uint8_t ret = USBD_FAIL;

    if (fops != NULL) {
        pdev->pUserData = fops;
        ret             = USBD_OK;
    }

    return ret;
}

/**
 * @brief  USBD_DCDC_SetTxBuffer
 * @param  pdev: device instance
 * @param  pbuff: Tx Buffer
 * @retval status
 */
uint8_t USBD_DCDC_SetTxBuffer(USBD_HandleTypeDef* pdev, USBD_CDC_HandleTypeDef* cdc, uint8_t* pbuff, uint16_t length)
{
    // USBD_DCDC_HandleTypeDef   *hDCDC = (USBD_DCDC_HandleTypeDef *) pdev->pClassData;

    cdc->TxBuffer = pbuff;
    cdc->TxLength = length;

    return USBD_OK;
}


/**
 * @brief  USBD_DCDC_SetRxBuffer
 * @param  pdev: device instance
 * @param  pbuff: Rx Buffer
 * @retval status
 */
uint8_t USBD_DCDC_SetRxBuffer(USBD_HandleTypeDef* pdev, USBD_CDC_HandleTypeDef* cdc, uint8_t* pbuff)
{
    // TODO this code sucks...fix this
    // USBD_DCDC_HandleTypeDef   *hDCDC = (USBD_DCDC_HandleTypeDef *) pdev->pClassData;

    cdc->RxBuffer = pbuff;

    return USBD_OK;
}

/**
 * @brief  USBD_DCDC_TransmitPacket
 *         Transmit packet on IN endpoint
 * @param  pdev: device instance
 * @retval status
 */
uint8_t USBD_DCDC_TransmitPacket(USBD_HandleTypeDef* pdev, USBD_CDC_HandleTypeDef* cdc)
{
    USBD_DCDC_HandleTypeDef* hDCDC = (USBD_DCDC_HandleTypeDef*)pdev->pClassData;

    if (pdev->pClassData != NULL) {
        if (cdc->TxState == 0U) {
            /* Tx Transfer in progress */
            cdc->TxState = 1U;

            /* Update the packet total length */
            uint32_t epnum                         = cdc == &hDCDC->frasyCdc ? DCDC_IN_EP : DCDC_IN_EP2;
            pdev->ep_in[epnum & 0xFU].total_length = cdc->TxLength;

            /* Transmit next packet */
            USBD_LL_Transmit(pdev, epnum, cdc->TxBuffer, (uint16_t)cdc->TxLength);

            return USBD_OK;
        }
        else {
            return USBD_BUSY;
        }
    }
    else {
        return USBD_FAIL;
    }
}


/**
 * @brief  USBD_DCDC_ReceivePacket
 *         prepare OUT Endpoint for reception
 * @param  pdev: device instance
 * @retval status
 */
uint8_t USBD_DCDC_ReceivePacket(USBD_HandleTypeDef* pdev, USBD_CDC_HandleTypeDef* cdc)
{
    USBD_DCDC_HandleTypeDef* hDCDC = (USBD_DCDC_HandleTypeDef*)pdev->pClassData;

    /* Suspend or Resume USB Out process */
    if (pdev->pClassData != NULL) {
        if (pdev->dev_speed == USBD_SPEED_HIGH) {
            /* Prepare Out endpoint to receive next packet */
            return USBD_LL_PrepareReceive(
              pdev, cdc == &hDCDC->frasyCdc ? DCDC_OUT_EP : DCDC_OUT_EP2, cdc->RxBuffer, DCDC_DATA_HS_OUT_PACKET_SIZE);
        }
        else {
            /* Prepare Out endpoint to receive next packet */
            return USBD_LL_PrepareReceive(
              pdev, cdc == &hDCDC->frasyCdc ? DCDC_OUT_EP : DCDC_OUT_EP2, cdc->RxBuffer, DCDC_DATA_FS_OUT_PACKET_SIZE);
        }
    }
    else {
        return USBD_FAIL;
    }
}
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
