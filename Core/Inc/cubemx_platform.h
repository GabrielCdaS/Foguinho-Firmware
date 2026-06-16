#ifndef CUBEMX_PLATFORM_H
#define CUBEMX_PLATFORM_H

#include "stm32f4xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void cubemx_platform_init(void);

#endif
