/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PYRO_MAIN_Pin GPIO_PIN_13
#define PYRO_MAIN_GPIO_Port GPIOC
#define PYRO_EMERGENCY_Pin GPIO_PIN_14
#define PYRO_EMERGENCY_GPIO_Port GPIOC
#define ARM_SWITCH_Pin GPIO_PIN_15
#define ARM_SWITCH_GPIO_Port GPIOC
#define BATTERY_ADC_Pin GPIO_PIN_0
#define BATTERY_ADC_GPIO_Port GPIOA
#define GPS_TX_Pin GPIO_PIN_2
#define GPS_TX_GPIO_Port GPIOA
#define GPS_RX_Pin GPIO_PIN_3
#define GPS_RX_GPIO_Port GPIOA
#define BMP388_CS_Pin GPIO_PIN_4
#define BMP388_CS_GPIO_Port GPIOA
#define SENSORS_SCK_Pin GPIO_PIN_5
#define SENSORS_SCK_GPIO_Port GPIOA
#define SENSORS_MISO_Pin GPIO_PIN_6
#define SENSORS_MISO_GPIO_Port GPIOA
#define SENSORS_MOSI_Pin GPIO_PIN_7
#define SENSORS_MOSI_GPIO_Port GPIOA
#define ICM20948_CS_Pin GPIO_PIN_0
#define ICM20948_CS_GPIO_Port GPIOB
#define SX1278_RESET_Pin GPIO_PIN_1
#define SX1278_RESET_GPIO_Port GPIOB
#define SX1278_DIO0_Pin GPIO_PIN_2
#define SX1278_DIO0_GPIO_Port GPIOB
#define SX1278_CS_Pin GPIO_PIN_12
#define SX1278_CS_GPIO_Port GPIOB
#define LORA_SCK_Pin GPIO_PIN_13
#define LORA_SCK_GPIO_Port GPIOB
#define LORA_MISO_Pin GPIO_PIN_14
#define LORA_MISO_GPIO_Port GPIOB
#define LORA_MOSI_Pin GPIO_PIN_15
#define LORA_MOSI_GPIO_Port GPIOB
#define DEBUG_TX_Pin GPIO_PIN_9
#define DEBUG_TX_GPIO_Port GPIOA
#define DEBUG_RX_Pin GPIO_PIN_10
#define DEBUG_RX_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SD_CS_Pin GPIO_PIN_15
#define SD_CS_GPIO_Port GPIOA
#define SD_SCK_Pin GPIO_PIN_3
#define SD_SCK_GPIO_Port GPIOB
#define SD_MISO_Pin GPIO_PIN_4
#define SD_MISO_GPIO_Port GPIOB
#define SD_MOSI_Pin GPIO_PIN_5
#define SD_MOSI_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_8
#define BUZZER_GPIO_Port GPIOB
#define STATUS_LED_Pin GPIO_PIN_9
#define STATUS_LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
