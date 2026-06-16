/**
 * @file pinout.h
 * @brief Mapeamento de pinos do STM32F411CEU6 para o computador de bordo do foguete.
 *
 * Define todos os pinos utilizados no hardware, organizados por periférico.
 * Cada pino é definido por porta (GPIO_PORTA_x) e número do pino.
 */

#ifndef PINOUT_H
#define PINOUT_H

/* ============================================================================
 * Identificadores de Porta GPIO
 * ========================================================================= */

#define GPIO_PORTA_A    0   /**< Porta GPIO A */
#define GPIO_PORTA_B    1   /**< Porta GPIO B */
#define GPIO_PORTA_C    2   /**< Porta GPIO C */

/* ============================================================================
 * SPI1 — Barramento de Sensores (BMP388, ICM-20948)
 * ========================================================================= */

#define PINO_SPI1_SCK_PORTA     GPIO_PORTA_A    /**< SPI1 SCK - Porta */
#define PINO_SPI1_SCK_PINO      5               /**< SPI1 SCK - PA5 */

#define PINO_SPI1_MISO_PORTA    GPIO_PORTA_A    /**< SPI1 MISO - Porta */
#define PINO_SPI1_MISO_PINO     6               /**< SPI1 MISO - PA6 */

#define PINO_SPI1_MOSI_PORTA    GPIO_PORTA_A    /**< SPI1 MOSI - Porta */
#define PINO_SPI1_MOSI_PINO     7               /**< SPI1 MOSI - PA7 */

/* ============================================================================
 * BMP388 — Barômetro (SPI1)
 * ========================================================================= */

#define PINO_BMP388_CS_PORTA    GPIO_PORTA_A    /**< BMP388 Chip Select - Porta */
#define PINO_BMP388_CS_PINO     4               /**< BMP388 Chip Select - PA4 */

/* ============================================================================
 * ICM-20948 — IMU 9 eixos (SPI1)
 * ========================================================================= */

#define PINO_ICM20948_CS_PORTA  GPIO_PORTA_B    /**< ICM-20948 Chip Select - Porta */
#define PINO_ICM20948_CS_PINO   0               /**< ICM-20948 Chip Select - PB0 */

/* ============================================================================
 * SPI2 — Rádio LoRa (SX1278)
 * ========================================================================= */

#define PINO_SPI2_SCK_PORTA     GPIO_PORTA_B    /**< SPI2 SCK - Porta */
#define PINO_SPI2_SCK_PINO      13              /**< SPI2 SCK - PB13 */

#define PINO_SPI2_MISO_PORTA    GPIO_PORTA_B    /**< SPI2 MISO - Porta */
#define PINO_SPI2_MISO_PINO     14              /**< SPI2 MISO - PB14 */

#define PINO_SPI2_MOSI_PORTA    GPIO_PORTA_B    /**< SPI2 MOSI - Porta */
#define PINO_SPI2_MOSI_PINO     15              /**< SPI2 MOSI - PB15 */

/* ============================================================================
 * SX1278 — Rádio LoRa (SPI2)
 * ========================================================================= */

#define PINO_SX1278_CS_PORTA    GPIO_PORTA_B    /**< SX1278 Chip Select - Porta */
#define PINO_SX1278_CS_PINO     12              /**< SX1278 Chip Select - PB12 */

#define PINO_SX1278_RST_PORTA   GPIO_PORTA_B    /**< SX1278 Reset - Porta */
#define PINO_SX1278_RST_PINO    1               /**< SX1278 Reset - PB1 */

#define PINO_SX1278_DIO0_PORTA  GPIO_PORTA_B    /**< SX1278 DIO0 (interrupção) - Porta */
#define PINO_SX1278_DIO0_PINO   2               /**< SX1278 DIO0 - PB2 */

/* ============================================================================
 * SPI3 — Cartão SD (MicroSD)
 * ========================================================================= */

#define PINO_SPI3_SCK_PORTA     GPIO_PORTA_B    /**< SPI3 SCK - Porta */
#define PINO_SPI3_SCK_PINO      3               /**< SPI3 SCK - PB3 */

#define PINO_SPI3_MISO_PORTA    GPIO_PORTA_B    /**< SPI3 MISO - Porta */
#define PINO_SPI3_MISO_PINO     4               /**< SPI3 MISO - PB4 */

#define PINO_SPI3_MOSI_PORTA    GPIO_PORTA_B    /**< SPI3 MOSI - Porta */
#define PINO_SPI3_MOSI_PINO     5               /**< SPI3 MOSI - PB5 */

#define PINO_SD_CS_PORTA        GPIO_PORTA_A    /**< SD Card Chip Select - Porta */
#define PINO_SD_CS_PINO         15              /**< SD Card Chip Select - PA15 */

/* ============================================================================
 * USART1 — Debug Serial
 * ========================================================================= */

#define PINO_USART1_TX_PORTA    GPIO_PORTA_A    /**< USART1 TX (debug) - Porta */
#define PINO_USART1_TX_PINO     9               /**< USART1 TX - PA9 */

#define PINO_USART1_RX_PORTA    GPIO_PORTA_A    /**< USART1 RX (debug) - Porta */
#define PINO_USART1_RX_PINO     10              /**< USART1 RX - PA10 */

/* ============================================================================
 * USART2 — Módulo GPS (NEO-M8N / NEO-M9N)
 * ========================================================================= */

#define PINO_GPS_USART_TX_PORTA GPIO_PORTA_A    /**< GPS USART TX - Porta */
#define PINO_GPS_USART_TX_PINO  2               /**< GPS USART TX - PA2 */

#define PINO_GPS_USART_RX_PORTA GPIO_PORTA_A    /**< GPS USART RX - Porta */
#define PINO_GPS_USART_RX_PINO  3               /**< GPS USART RX - PA3 */

/* ============================================================================
 * ADC — Monitoramento de Bateria
 * ========================================================================= */

#define PINO_BATERIA_ADC_PORTA  GPIO_PORTA_A    /**< Tensão da bateria (ADC) - Porta */
#define PINO_BATERIA_ADC_PINO   0               /**< Tensão da bateria - PA0 */
#define PINO_BATERIA_ADC_CANAL  0               /**< Canal ADC1_CH0 */

/* ============================================================================
 * Sistema de Recuperação — Acionamento Pirotécnico
 * ========================================================================= */

#define PINO_RECUP_PRINCIPAL_PORTA  GPIO_PORTA_C    /**< Canal pirotécnico principal - Porta */
#define PINO_RECUP_PRINCIPAL_PINO   13              /**< Canal pirotécnico principal - PC13 */

#define PINO_RECUP_EMERGENCIA_PORTA GPIO_PORTA_C    /**< Canal pirotécnico de emergência - Porta */
#define PINO_RECUP_EMERGENCIA_PINO  14              /**< Canal pirotécnico de emergência - PC14 */

/* ============================================================================
 * Chave de Armamento
 * ========================================================================= */

#define PINO_CHAVE_ARMAMENTO_PORTA  GPIO_PORTA_C    /**< Chave de armamento - Porta */
#define PINO_CHAVE_ARMAMENTO_PINO   15              /**< Chave de armamento - PC15 */

/* ============================================================================
 * Interface do Usuário — LED e Buzzer
 * ========================================================================= */

#define PINO_LED_STATUS_PORTA   GPIO_PORTA_B    /**< LED de status - Porta */
#define PINO_LED_STATUS_PINO    9               /**< LED de status - PB9 */

#define PINO_BUZZER_PORTA       GPIO_PORTA_B    /**< Buzzer - Porta */
#define PINO_BUZZER_PINO        8               /**< Buzzer - PB8 */

#endif /* PINOUT_H */
