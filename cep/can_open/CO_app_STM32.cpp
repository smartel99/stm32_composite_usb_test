/*
 * CANopen main program file.
 *
 * This file is a template for other microcontrollers.
 *
 * @file        main_generic.c
 * @author      Hamed Jafarzadeh 	2022
 * 				Janez Paternoster	2021
 * @copyright   2021 Janez Paternoster
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
 */
#include "CO_app_STM32.h"
#include "CANopen.h"
#include "main.h"

#include "CO_storageBlank.h"
#include "OD.h"

#include <cstdio>
#include <logging/logger.h>
// It will be set by canopen_app_init and will be used across app to get access to CANOpen objects
CanopenNodeStm32* canopenNodeStm32;

/* Printf function of CanOpen app */
#define log_printf(macropar_message, ...) LOGI("CANopen", macropar_message __VA_OPT__(, ) __VA_ARGS__)

/* default values for CO_CANopenInit() */
#define NMT_CONTROL                                                                                                    \
    CO_NMT_STARTUP_TO_OPERATIONAL                                                                                      \
    | CO_NMT_ERR_ON_ERR_REG | CO_ERR_REG_GENERIC_ERR | CO_ERR_REG_COMMUNICATION
#define FIRST_HB_TIME        500
#define SDO_SRV_TIMEOUT_TIME 1000
#define SDO_CLI_TIMEOUT_TIME 500
#define SDO_CLI_BLOCK        false
#define OD_STATUS_BITS       nullptr

namespace {
/* Global variables and objects */
CO_t* CO = nullptr; /* CANopen object */

// Global variables
uint32_t         timeOld, timeCurrent;
CO_ReturnError_t err;
uint32_t         storageInitError = 0;

const char* canOpenErrorToStr(CO_ReturnError_t err)
{
    switch (err) {
        case CO_ERROR_NO: return "No error";
        case CO_ERROR_ILLEGAL_ARGUMENT: return "Illegal Argument";
        case CO_ERROR_OUT_OF_MEMORY: return "Out of Memory";
        case CO_ERROR_TIMEOUT: return "Timeout";
        case CO_ERROR_ILLEGAL_BAUDRATE: return "Illegal Baudrate";
        case CO_ERROR_RX_OVERFLOW: return "RX Overflow";
        case CO_ERROR_RX_PDO_OVERFLOW: return "RX PDO Overflow";
        case CO_ERROR_RX_MSG_LENGTH: return "RX MSG Length";
        case CO_ERROR_RX_PDO_LENGTH: return "RX PDO Length";
        case CO_ERROR_TX_OVERFLOW: return "TX Overflow";
        case CO_ERROR_TX_PDO_WINDOW: return "TX PDO Window";
        case CO_ERROR_TX_UNCONFIGURED: return "TX Uncofigured";
        case CO_ERROR_OD_PARAMETERS: return "OD Parameters";
        case CO_ERROR_DATA_CORRUPT: return "Data Corrupt";
        case CO_ERROR_CRC: return "CRC Error";
        case CO_ERROR_TX_BUSY: return "TX Busy";
        case CO_ERROR_WRONG_NMT_STATE: return "Wrong NMT State";
        case CO_ERROR_SYSCALL: return "Syscall Error";
        case CO_ERROR_INVALID_STATE: return "Invalid State";
        case CO_ERROR_NODE_ID_UNCONFIGURED_LSS: return "Unconfigured LSS Node ID";
        default: return "Unknown";
    }
}
}    // namespace

/* This function will basically setup the CANopen node */
int canopen_app_init(CanopenNodeStm32* canopenStm32)
{
    // Keep a copy global reference of canOpenSTM32 Object
    canopenNodeStm32 = canopenStm32;

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
    CO_storage_t       storage;
    CO_storage_entry_t storageEntries[] = {
      {
        .addr       = &OD_PERSIST_COMM,
        .len        = sizeof(OD_PERSIST_COMM),
        .subIndexOD = 2,
        .attr       = CO_storage_cmd | CO_storage_restore,
        .addrNV     = nullptr,
      },
    };
    uint8_t storageEntriesCount = sizeof(storageEntries) / sizeof(storageEntries[0]);
#endif

    /* Allocate memory */
    CO_config_t* config_ptr = nullptr;
#ifdef CO_MULTIPLE_OD
    /* example usage of CO_MULTIPLE_OD (but still single OD here) */
    CO_config_t co_config = {0};
    OD_INIT_CONFIG(co_config); /* helper macro from OD.h */
    co_config.CNT_LEDS    = 1;
    co_config.CNT_LSS_SLV = 1;
    config_ptr            = &co_config;
#endif /* CO_MULTIPLE_OD */

    uint32_t heapMemoryUsed;
    CO = CO_new(config_ptr, &heapMemoryUsed);
    if (CO == nullptr) {
        log_printf("Error: Can't allocate memory");
        return 1;
    }
    else {
        log_printf("Allocated %lu bytes for CANopen objects", heapMemoryUsed);
    }

    canopenNodeStm32->canOpenStack = CO;

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
    err = CO_storageBlank_init(&storage,
                               CO->CANmodule,
                               OD_ENTRY_H1010_storeParameters,
                               OD_ENTRY_H1011_restoreDefaultParameters,
                               &storageEntries[0],
                               storageEntriesCount,
                               &storageInitError);

    if (err != CO_ERROR_NO && err != CO_ERROR_DATA_CORRUPT) {
        log_printf("Error: Storage (%lu) %s", storageInitError, canOpenErrorToStr(err));
        return 2;
    }
#endif

    canopen_app_resetCommunication();
    return 0;
}

int canopen_app_resetCommunication()
{
    /* CANopen communication reset - initialize CANopen objects *******************/
    log_printf("CANopenNode - Reset communication...");

    /* Wait rt_thread. */
    CO->CANmodule->CANnormal = false;

    /* Enter CAN configuration. */
    CO_CANsetConfigurationMode(canopenNodeStm32);
    CO_CANmodule_disable(CO->CANmodule);

    /* initialize CANopen */
    err = CO_CANinit(CO, canopenNodeStm32, 0);    // Bitrate for STM32 microcontroller is being set in MXCube Settings
    if (err != CO_ERROR_NO) {
        log_printf("Error: CAN initialization failed: (%d) %s", err, canOpenErrorToStr(err));
        return 1;
    }

    CO_LSS_address_t lssAddress = {
      .identity =
        {
          .vendorID       = 0x12345678,    // OD_PERSIST_COMM.x1018_identity.vendor_ID,
          .productCode    = 0x9ABCDEF0,    // OD_PERSIST_COMM.x1018_identity.productCode,
          .revisionNumber = 0x12345678,    // OD_PERSIST_COMM.x1018_identity.revisionNumber,
          .serialNumber   = 0x9ABCDEF0,    // OD_PERSIST_COMM.x1018_identity.serialNumber,
        },
    };
    err = CO_LSSinit(CO, &lssAddress, &canopenNodeStm32->desiredNodeId, &canopenNodeStm32->baudrate);
    if (err != CO_ERROR_NO) {
        log_printf("Error: LSS slave initialization failed: (%d) %s", err, canOpenErrorToStr(err));
        return 2;
    }

    canopenNodeStm32->activeNodeId = canopenNodeStm32->desiredNodeId;
    uint32_t errInfo               = 0;

    err = CO_CANopenInit(CO,                                         /* CANopen object */
                         nullptr,                                    /* alternate NMT */
                         nullptr,                                    /* alternate em */
                         OD,                                         /* Object dictionary */
                         OD_STATUS_BITS,                             /* Optional OD_statusBits */
                         static_cast<CO_NMT_control_t>(NMT_CONTROL), /* CO_NMT_control_t */
                         FIRST_HB_TIME,                              /* firstHBTime_ms */
                         SDO_SRV_TIMEOUT_TIME,                       /* SDOserverTimeoutTime_ms */
                         SDO_CLI_TIMEOUT_TIME,                       /* SDOclientTimeoutTime_ms */
                         SDO_CLI_BLOCK,                              /* SDOclientBlockTransfer */
                         canopenNodeStm32->activeNodeId,
                         &errInfo);
    if (err != CO_ERROR_NO && err != CO_ERROR_NODE_ID_UNCONFIGURED_LSS) {
        if (err == CO_ERROR_OD_PARAMETERS) { log_printf("Error: Object Dictionary entry 0x%lX", errInfo); }
        else {
            log_printf("Error: CANopen initialization failed: (%d) %s", err, canOpenErrorToStr(err));
        }
        return 3;
    }

    err = CO_CANopenInitPDO(CO, CO->em, OD, canopenNodeStm32->activeNodeId, &errInfo);
    if (err != CO_ERROR_NO) {
        if (err == CO_ERROR_OD_PARAMETERS) { log_printf("Error: Object Dictionary entry 0x%lX", errInfo); }
        if (err == CO_ERROR_NODE_ID_UNCONFIGURED_LSS) { log_printf("Node ID not configured, skipping PDO init"); }
        else {
            log_printf("Error: PDO initialization failed: (%d) %s", err, canOpenErrorToStr(err));
        }
        return 4;
    }

    /* Configure Timer interrupt function for execution every 1 millisecond */
    HAL_TIM_Base_Start_IT(canopenNodeStm32->timerHandle);    // 1ms interrupt

    /* Configure CAN transmit and receive interrupt */

    /* Configure CANopen callbacks, etc */
    if (!CO->nodeIdUnconfigured) {

#if (CO_CONFIG_STORAGE) & CO_CONFIG_STORAGE_ENABLE
        if (storageInitError != 0) {
            CO_errorReport(CO->em, CO_EM_NON_VOLATILE_MEMORY, CO_EMC_HARDWARE, storageInitError);
        }
#endif
    }
    else {
        log_printf("CANopenNode - Node-id not initialized");
    }

    /* start CAN */
    CO_CANsetNormalMode(CO->CANmodule);

    log_printf("CANopenNode - Running...");
    timeOld = timeCurrent = HAL_GetTick();
    return 0;
}

void canopen_app_process()
{
    /* loop for normal program execution ******************************************/
    /* get time difference since last function call */
    timeCurrent = HAL_GetTick();

    if ((timeCurrent - timeOld) > 0) {    // Make sure more than 1ms elapsed
        /* CANopen process */
        CO_NMT_reset_cmd_t resetStatus;
        uint32_t           timeDifferenceUs = (timeCurrent - timeOld) * 1000;
        timeOld                             = timeCurrent;
        resetStatus                         = CO_process(CO, false, timeDifferenceUs, nullptr);
        canopenNodeStm32->outStatusLedRed   = CO_LED_RED(CO->LEDs, CO_LED_CANopen);
        canopenNodeStm32->outStatusLedGreen = CO_LED_GREEN(CO->LEDs, CO_LED_CANopen);

        if (resetStatus == CO_RESET_COMM) {
            /* delete objects from memory */
            HAL_TIM_Base_Stop_IT(canopenNodeStm32->timerHandle);
            CO_CANsetConfigurationMode(canopenNodeStm32);
            CO_delete(CO);
            CO = nullptr;
            log_printf("CANopenNode Reset Communication request");
            canopen_app_init(canopenNodeStm32);    // Reset Communication routine
        }
        else if (resetStatus == CO_RESET_APP) {
            log_printf("CANopenNode Device Reset");
            HAL_NVIC_SystemReset();    // Reset the STM32 Microcontroller
        }
    }
}

/* Thread function executes in constant intervals, this function can be called from FreeRTOS tasks or Timers ********/
void canopen_app_interrupt(void)
{
    CO_LOCK_OD(CO->CANmodule);
    if (!CO->nodeIdUnconfigured && CO->CANmodule->CANnormal) {
        bool_t syncWas = false;
        /* get time difference since last function call */
        uint32_t timeDifferenceUs = 1000;    // 1ms second

#if (CO_CONFIG_SYNC) & CO_CONFIG_SYNC_ENABLE
        syncWas = CO_process_SYNC(CO, timeDifferenceUs, nullptr);
#endif
#if (CO_CONFIG_PDO) & CO_CONFIG_RPDO_ENABLE
        CO_process_RPDO(CO, syncWas, timeDifferenceUs, nullptr);
#endif
#if (CO_CONFIG_PDO) & CO_CONFIG_TPDO_ENABLE
        CO_process_TPDO(CO, syncWas, timeDifferenceUs, nullptr);
#endif

        /* Further I/O or nonblocking application code may go here. */
    }
    CO_UNLOCK_OD(CO->CANmodule);
}
