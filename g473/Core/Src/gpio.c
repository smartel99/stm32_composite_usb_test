/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PE2   ------> S_TIM3_CH1
     PE3   ------> S_TIM3_CH2
     PE4   ------> S_TIM3_CH3
     PE6   ------> TIM20_CH3N
     PF7   ------> S_TIM5_CH2
     PF8   ------> S_TIM5_CH3
     PF9   ------> TIM20_BKIN
     PF10   ------> TIM20_BKIN2
     PF1-OSC_OUT   ------> ADC2_IN10
     PC0   ------> ADC1_IN6
     PC1   ------> ADC1_IN7
     PC2   ------> SharedAnalog_PC2
     PC3   ------> SharedAnalog_PC3
     PA0   ------> S_TIM8_ETR
     PA1   ------> SharedAnalog_PA1
     PA2   ------> ADC1_IN3
     PA3   ------> ADC1_IN4
     PA4   ------> COMP_DAC11_group
     PA5   ------> COMP_DAC12_group
     PA6   ------> ADC2_IN3
     PA7   ------> ADC2_IN4
     PC4   ------> ADC2_IN5
     PC5   ------> ADC2_IN11
     PB0   ------> OPAMP3_VINP
     PB1   ------> OPAMP3_VOUT
     PB2   ------> OPAMP3_VINM
     PF12   ------> S_TIM20_CH1
     PF13   ------> S_TIM20_CH2
     PF14   ------> S_TIM20_CH3
     PE7   ------> S_TIM1_ETR
     PE9   ------> S_TIM1_CH1
     PE11   ------> S_TIM1_CH2
     PE12   ------> SharedAnalog_PE12
     PE13   ------> S_TIM1_CH3
     PE14   ------> ADC4_IN1
     PE15   ------> ADC4_IN2
     PB10   ------> OPAMP4_VINM
     PB11   ------> OPAMP4_VINP
     PB12   ------> OPAMP4_VOUT
     PB13   ------> ADC3_IN5
     PB14   ------> OPAMP5_VINP
     PB15   ------> OPAMP5_VINM
     PD8   ------> SharedAnalog_PD8
     PD9   ------> SharedAnalog_PD9
     PD10   ------> SharedAnalog_PD10
     PD11   ------> SharedAnalog_PD11
     PD12   ------> SharedAnalog_PD12
     PD13   ------> SharedAnalog_PD13
     PD14   ------> SharedAnalog_PD14
     PC6   ------> S_TIM8_CH1
     PC7   ------> S_TIM8_CH2
     PG0   ------> TIM20_CH1N
     PG1   ------> TIM20_CH2N
     PG2   ------> SPI1_SCK
     PG3   ------> SPI1_MISO
     PG4   ------> SPI1_MOSI
     PC8   ------> S_TIM8_CH3
     PA8   ------> OPAMP5_VOUT
     PF6   ------> S_TIM5_CH1
     PA15   ------> TIM1_BKIN
     PC10   ------> TIM8_CH1N
     PC11   ------> TIM8_CH2N
     PC12   ------> TIM8_CH3N
     PG5   ------> SPI1_NSS
     PD1   ------> TIM8_BKIN2
     PD3   ------> S_TIM2_CH1
     PD4   ------> S_TIM2_CH2
     PD7   ------> S_TIM2_CH3
     PB4   ------> TIM17_BKIN
     PB5   ------> TIM16_BKIN
     PB6   ------> S_TIM4_CH1
     PB7   ------> TIM8_BKIN
     PB8-BOOT0   ------> S_TIM4_CH3
     PB9   ------> TIM1_CH3N
     PE0   ------> S_TIM16_CH1
     PE1   ------> S_TIM17_CH1
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(M2_ENABLE1_GPIO_GPIO_Port, M2_ENABLE1_GPIO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_OUT_LED_GREEN_Pin|M1_ENABLE1_GPIO_Pin|M1_ENABLE2_GPIO_Pin|M2_ENABLE2_GPIO_Pin
                          |GPIO_OUT_INRUSH_Pin|GPIO_OUT_ID_ENABLE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(M3_ENABLE1_GPIO_GPIO_Port, M3_ENABLE1_GPIO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_OUT_M2_ENABLE_Pin|GPIO_OUT_M1_ENABLE_Pin|GPIO_OUT_M1_BRAKE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_OUT_M22_BRAKE_Pin|M3_ENABLE2_GPIO_Pin|GPIO_OUT_LED_YELLOW_Pin|GPIO_OUT_LED_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PEPin PEPin PEPin */
  GPIO_InitStruct.Pin = M1_HALL_H1_TIM3_CH1_Pin|M1_HALL_H2_TIM3_CH2_Pin|M1_HALL_H3_TIM3_CH3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = M2_ENABLE1_GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(M2_ENABLE1_GPIO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = M3_PWM_WL_TIM20_CH3N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_TIM20;
  HAL_GPIO_Init(M3_PWM_WL_TIM20_CH3N_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PFPin PFPin PFPin PFPin
                           PFPin PFPin */
  GPIO_InitStruct.Pin = GPIO_OUT_LED_GREEN_Pin|M1_ENABLE1_GPIO_Pin|M1_ENABLE2_GPIO_Pin|M2_ENABLE2_GPIO_Pin
                          |GPIO_OUT_INRUSH_Pin|GPIO_OUT_ID_ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PFPin PFPin PFPin */
  GPIO_InitStruct.Pin = M1_ENCB_TIM5_CH2_Pin|M1_ENCZ_TIM5_CH3_Pin|M1_ENCA_TIM5_CH1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_TIM5;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PFPin PFPin */
  GPIO_InitStruct.Pin = M3_TIM20_BKIN_Pin|M3_TIM20_BKIN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM20;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = ADC2_IN10_PFC_AC_V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC2_IN10_PFC_AC_V_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = BUTTON_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PCPin PCPin PCPin PCPin
                           PCPin PCPin */
  GPIO_InitStruct.Pin = M2_VBUS_ADC1_IN6_Pin|M2_RES_COS_ADC1_IN7_Pin|M2_CURR_V_ADC12_IN8_Pin|M2_CURR_W_ADC12_IN9_Pin
                          |M2_TEMP_ID_ADC2_IN5_Pin|PFC_AC_ZC_ADC2_IN11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = M1_TIM8_ETR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_TIM8;
  HAL_GPIO_Init(M1_TIM8_ETR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin PAPin
                           PAPin PAPin PAPin PA8
                           PA9 */
  GPIO_InitStruct.Pin = M1_RES_SIN_ADC12_IN2_Pin|M1_RES_COS_ADC1_IN3_Pin|M2_RES_SIN_ADC1_IN4_Pin|M1_RES_EX_DAC1_OUT1_Pin
                          |M2_RES_EX_DAC1_OUT2_Pin|M3_TEMP_ID_ADC2IN3_Pin|M2_CURR_U_ADC2_IN4_Pin|GPIO_PIN_8
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB11 PB12 PBPin PB14
                           PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|M1_VBUS_ADC3_IN5_Pin|GPIO_PIN_14
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PFPin PFPin PFPin */
  GPIO_InitStruct.Pin = M3_PWM_UH_TIM20_CH1_Pin|M3_PWM_VH_TIM20_CH2_Pin|M3_PWM_WH_TIM20_CH3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM20;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PE7 PEPin PEPin PEPin */
  GPIO_InitStruct.Pin = GPIO_PIN_7|M2_PWM_UH_TIM1_CH1_Pin|M2_PWM_VH_TIM1_CH2_Pin|M2_PWM_WL_TIM1_CH3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PE8 PE10 PEPin PEPin
                           PEPin */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10|M1_VOLT_V_ADC345_IN16_Pin|ADC4_IN1_MORPHO_Pin
                          |M1_TEMP_ID_ADC4_IN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PDPin PDPin PDPin PDPin
                           PDPin PDPin PDPin PD15 */
  GPIO_InitStruct.Pin = ADC45_IN12_PFC_Current1_Pin|ADC45_IN13_PFC_Current2_Pin|M1_VOLT_U_ADC345_IN7_Pin|M1_CURR_U_ADC345_IN8_Pin
                          |M1_CURR_V_ADC345_IN9_Pin|M1_VOLT_W_ADC345_IN10_Pin|M1_CURR_W_ADC345_IN11_Pin|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PCPin PCPin PCPin PCPin
                           PCPin PCPin */
  GPIO_InitStruct.Pin = M1_PWM_UH_TIM8_CH1_Pin|M1_PWM_VH_TIM8_CH2_Pin|M1_PWM_WH_TIM8_CH3_Pin|M1_PWM_UL_TIM8_CH1N_Pin
                          |M1_PWM_VL_TIM8_CH2N_Pin|M1_PWM_WL_TIM8_CH3N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_TIM8;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PGPin PGPin */
  GPIO_InitStruct.Pin = M3_PWM_UL_TIM20_CH1N_Pin|M3_PWM_VL_TIM20_CH2N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM20;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PG2 PG3 PG4 PG5 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = M3_ENABLE1_GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(M3_ENABLE1_GPIO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_TIM1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PGPin PGPin PGPin */
  GPIO_InitStruct.Pin = GPIO_OUT_M2_ENABLE_Pin|GPIO_OUT_M1_ENABLE_Pin|GPIO_OUT_M1_BRAKE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PDPin PDPin PDPin PDPin */
  GPIO_InitStruct.Pin = GPIO_OUT_M22_BRAKE_Pin|M3_ENABLE2_GPIO_Pin|GPIO_OUT_LED_YELLOW_Pin|GPIO_OUT_LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = M1_TIM8_BKIN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_TIM8;
  HAL_GPIO_Init(M1_TIM8_BKIN2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PDPin PDPin PDPin */
  GPIO_InitStruct.Pin = M2_ENCA_TIM2_CH1_Pin|M2_ENCB_TIM2_CH2_Pin|M2_ENCZ_TIM2_CH3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = PFC_TIM17_BKIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_TIM17;
  HAL_GPIO_Init(PFC_TIM17_BKIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = PFC_TIM16_BKIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM16;
  HAL_GPIO_Init(PFC_TIM16_BKIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin */
  GPIO_InitStruct.Pin = M2_HALL_H1_TIM4_CH1_Pin|M2_HALL_H3_TIM4_CH3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = M1_TIM8_BKIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_TIM8;
  HAL_GPIO_Init(M1_TIM8_BKIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = M2_PWM_WL_TIM1_CH3N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF12_TIM1_COMP1;
  HAL_GPIO_Init(M2_PWM_WL_TIM1_CH3N_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = PFC_PWM1_TIM16_CH1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_TIM16;
  HAL_GPIO_Init(PFC_PWM1_TIM16_CH1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = PFC_PWM2_TIM17_CH1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_TIM17;
  HAL_GPIO_Init(PFC_PWM2_TIM17_CH1_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
