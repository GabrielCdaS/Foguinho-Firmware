/**
 * @file pinout.h
 * @brief Mapeamento de pinos do STM32F411CEU6 para a estação base.
 */

#ifndef BASE_STATION_PINOUT_H
#define BASE_STATION_PINOUT_H

#define GPIO_PORTA_A    0
#define GPIO_PORTA_B    1
#define GPIO_PORTA_C    2

/* SPI2 - Radio LoRa SX1278 */
#define PINO_SPI2_SCK_PORTA     GPIO_PORTA_B
#define PINO_SPI2_SCK_PINO      13
#define PINO_SPI2_MISO_PORTA    GPIO_PORTA_B
#define PINO_SPI2_MISO_PINO     14
#define PINO_SPI2_MOSI_PORTA    GPIO_PORTA_B
#define PINO_SPI2_MOSI_PINO     15

#define PINO_SX1278_CS_PORTA    GPIO_PORTA_B
#define PINO_SX1278_CS_PINO     12
#define PINO_SX1278_RST_PORTA   GPIO_PORTA_B
#define PINO_SX1278_RST_PINO    1
#define PINO_SX1278_DIO0_PORTA  GPIO_PORTA_B
#define PINO_SX1278_DIO0_PINO   2

/* USART1 - enlace serial com o PC/GUI via conversor USB-serial */
#define PINO_PC_USART_TX_PORTA  GPIO_PORTA_A
#define PINO_PC_USART_TX_PINO   9
#define PINO_PC_USART_RX_PORTA  GPIO_PORTA_A
#define PINO_PC_USART_RX_PINO   10

/* Interface local simples */
#define PINO_LED_STATUS_PORTA   GPIO_PORTA_B
#define PINO_LED_STATUS_PINO    9

#endif /* BASE_STATION_PINOUT_H */
