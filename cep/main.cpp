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

#include "adc.h"
#include "cmsis_os.h"
#include "dac.h"
#include "gpio.h"
#include "opamp.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"

extern "C" void SystemClock_Config(void);
extern "C" void MX_FREERTOS_Init(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main()
{
    /* USER CODE BEGIN 1 */

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
    MX_ADC1_Init();
    MX_ADC2_Init();
    MX_ADC3_Init();
    MX_ADC4_Init();
    MX_DAC1_Init();
    MX_OPAMP3_Init();
    MX_OPAMP4_Init();
    MX_OPAMP5_Init();
    MX_SPI1_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_TIM5_Init();
    MX_TIM8_Init();
    MX_TIM16_Init();
    MX_TIM17_Init();
    MX_TIM20_Init();
    MX_USART1_UART_Init();
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
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}
