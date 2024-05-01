/**
 * @file    main.cpp
 * @author  Samuel Martel
 * @date    2024-03-25
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
#include "g473/Core/Inc/main.h"

#include "can_manager.h"
#include "can_open/CO_app_STM32.h"
#include "cli/cli.h"
#include "cmsis_os.h"
#include "fdcan.h"
#include "gpio.h"
#include "rng.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include <cm_backtrace.h>
#include <logging/logger.h>
#include <logging/proxy_sink.h>
#include <logging/uart_sink.h>

#include <exception>
#include <new>
#include <string>


extern "C" void SystemClock_Config(void);
extern "C" void MX_FREERTOS_Init(void);

extern "C" USBD_HandleTypeDef hUsbDeviceFS;

extern "C" void StartDefaultTask(void* args)
{
    Logging::Logger::setGetTime(&HAL_GetTick);
    auto* uartSink = Logging::Logger::addSink<Logging::MtUartSink>(&huart1);
    Logging::Logger::addSink<Logging::ProxySink>(g_usbDebugTag, uartSink);

    ROOT_LOGI("\n\n\n\rLogger Initialized.");

    CLI cli {&g_usbDebug};
    CanManager::init(&g_usbFrasy, &hfdcan1);

    CanopenNodeStm32 canOpenNodeSTM32 {};
    canOpenNodeSTM32.canHandle      = &hfdcan1;
    canOpenNodeSTM32.hwInitFunction = MX_FDCAN1_Init;
    canOpenNodeSTM32.timerHandle    = &htim7;
    // Device node ID should be read from a source (resistor divider, I/O, etc.)
    canOpenNodeSTM32.desiredNodeId  = 0x02;
    canOpenNodeSTM32.baudrate       = 1000;
    canopen_app_init(&canOpenNodeSTM32);

    volatile bool t       = true;
    size_t        counter = 0;
    while (t) {
        if (++counter >= 500) {
            counter = 0;
            HAL_GPIO_TogglePin(GPIO_OUT_LED_YELLOW_GPIO_Port, GPIO_OUT_LED_YELLOW_Pin);
        }

        // Reflect CANopenStatus on LEDs
        HAL_GPIO_WritePin(GPIO_OUT_LED_GREEN_GPIO_Port,
                          GPIO_OUT_LED_GREEN_Pin,
                          static_cast<GPIO_PinState>(canOpenNodeSTM32.outStatusLedGreen == 0u));
        HAL_GPIO_WritePin(GPIO_OUT_LED_RED_GPIO_Port,
                          GPIO_OUT_LED_RED_Pin,
                          static_cast<GPIO_PinState>(canOpenNodeSTM32.outStatusLedRed == 0u));
        canopen_app_process();
        // Sleep for 1ms, you can decrease it if required, in the canopen_app_process we will double check to make sure
        // 1ms passed
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    std::unreachable();
}

extern "C" [[gnu::used]] int _write([[maybe_unused]] int file, char* ptr, int len)
{
    HAL_UART_Transmit(&huart1, reinterpret_cast<const uint8_t*>(ptr), len, HAL_MAX_DELAY);
    return len;
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main()
{
    /* USER CODE BEGIN 1 */

    std::set_new_handler([] {
        ROOT_LOGE("Memory allocation failed, terminating");
        std::set_new_handler(nullptr);
    });

    std::set_terminate(Error_Handler);
    cm_backtrace_init("usb_test", "v0.1.0", "v0.1.0");
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_RNG_Init();
    MX_USART1_UART_Init();
    MX_FDCAN1_Init();
    MX_TIM7_Init();
    MX_TIM16_Init();
    MX_USB_Device_Init();
    /* USER CODE BEGIN 2 */
    /* USER CODE END 2 */

    /* Init scheduler */
    osKernelInitialize();

    /* Call init function for freertos objects (in freertos.c) */
    MX_FREERTOS_Init();

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    volatile bool t = true;
    while (t) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
