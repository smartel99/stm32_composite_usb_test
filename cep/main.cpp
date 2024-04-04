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

#include "cmsis_os.h"
#include "gpio.h"
#include "usart.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include <exception>
#include <logging/logger.h>
#include <logging/proxy_sink.h>
#include <logging/uart_sink.h>
#include <cm_backtrace.h>

#include <new>


extern "C" void SystemClock_Config(void);
extern "C" void MX_FREERTOS_Init(void);

extern "C" USBD_HandleTypeDef hUsbDeviceFS;

extern "C" void StartDefaultTask(void* args)
{
    for (;;) {
        USBD_DCDC_HandleTypeDef* hcdc = (USBD_DCDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
//        ROOT_LOGI("Ping");
        if (auto res = CDC_Transmit_FS(&hcdc->frasyCdc, (uint8_t*)"hewwo :3\n", 9); res != USBD_OK) {
//            ROOT_LOGW("Unable to send on CDC1: %02x", res);
        }
        if (auto res = CDC_Transmit_FS(&hcdc->debugCdc, (uint8_t*)"not hewwo >:3\n", 14); res != USBD_OK) {
//            ROOT_LOGW("Unable to send on CDC2: %02x", res);
        }
        osDelay(1000);
    }
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
    MX_USART1_UART_Init();
    MX_USB_Device_Init();
    /* USER CODE BEGIN 2 */

    Logging::Logger::setGetTime(&HAL_GetTick);
    auto* uartSink = Logging::Logger::addSink<Logging::UartSink>(&huart1);
    Logging::Logger::addSink<Logging::ProxySink>(s_usbTag, uartSink);

    ROOT_LOGI("Logger Initialized.");

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
