/**
 * @file stm32f411_hw.h
 * @brief Pequena camada HAL usada pelo firmware da estação base.
 */

#ifndef BASE_STATION_STM32F411_HW_H
#define BASE_STATION_STM32F411_HW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "protocol.h"

typedef enum {
    HW_SPI1 = 0,
    HW_SPI2,
    HW_SPI3
} hw_spi_t;

void hw_clock_init(void);

void hw_gpio_output(uint8_t port, uint8_t pin, bool initial_high);
void hw_gpio_input_pullup(uint8_t port, uint8_t pin);
void hw_gpio_analog(uint8_t port, uint8_t pin);
void hw_gpio_write(uint8_t port, uint8_t pin, bool high);
bool hw_gpio_read(uint8_t port, uint8_t pin);

status_t hw_spi_init(hw_spi_t spi, uint8_t baud_div);
status_t hw_spi_set_baud(hw_spi_t spi, uint8_t baud_div);
status_t hw_spi_transfer(hw_spi_t spi, const uint8_t *tx, uint8_t *rx, size_t length);

status_t hw_adc1_init_channel(uint8_t channel);
status_t hw_adc1_read(uint16_t *value);

status_t hw_usart1_init(uint32_t baudrate);
bool hw_usart1_rx_ready(void);
status_t hw_usart1_write(const uint8_t *dados, size_t tamanho, uint32_t timeout_ms);
status_t hw_usart1_read(uint8_t *byte, uint32_t timeout_ms);

status_t hw_usart2_init(uint32_t baudrate);
bool hw_usart2_rx_ready(void);
uint8_t hw_usart2_read(void);

#endif /* BASE_STATION_STM32F411_HW_H */
