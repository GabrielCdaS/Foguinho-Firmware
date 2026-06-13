#ifndef STM32F411_HW_H
#define STM32F411_HW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sensors.h"

typedef enum {
    HW_SPI1 = 0,
    HW_SPI2,
    HW_SPI3
} hw_spi_t;

void hw_clock_init(void);
void hw_delay_cycles(volatile uint32_t cycles);

void hw_gpio_output(uint8_t port, uint8_t pin, bool initial_high);
void hw_gpio_input_pullup(uint8_t port, uint8_t pin);
void hw_gpio_alternate(uint8_t port, uint8_t pin, uint8_t alternate);
void hw_gpio_analog(uint8_t port, uint8_t pin);
void hw_gpio_write(uint8_t port, uint8_t pin, bool high);
bool hw_gpio_read(uint8_t port, uint8_t pin);

status_t hw_spi_init(hw_spi_t spi, uint8_t baud_div);
status_t hw_spi_set_baud(hw_spi_t spi, uint8_t baud_div);
status_t hw_spi_transfer(hw_spi_t spi, const uint8_t *tx, uint8_t *rx, size_t length);

status_t hw_adc1_init_channel(uint8_t channel);
status_t hw_adc1_read(uint16_t *value);

status_t hw_usart2_init(uint32_t baudrate);
bool hw_usart2_rx_ready(void);
uint8_t hw_usart2_read(void);

#endif
