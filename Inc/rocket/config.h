/**
 * @file config.h
 * @brief Configurações gerais do sistema de firmware do foguete.
 *
 * Define parâmetros de versão, frequências de amostragem, telemetria,
 * datalogger, buffers e habilitações de módulos.
 *
 * MCU: STM32F411CEU6 (Cortex-M4, 96 MHz, 512 KB Flash, 128 KB SRAM)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* Inclui definições de protocolo compartilhadas entre foguete e estação base */
#include "protocol_defs.h"

/* ============================================================================
 * Versão do Firmware
 * ========================================================================= */

#define FIRMWARE_VERSAO_MAJOR   0   /**< Versão principal */
#define FIRMWARE_VERSAO_MINOR   1   /**< Versão secundária */
#define FIRMWARE_VERSAO_PATCH   0   /**< Correção */

/* ============================================================================
 * Clock do Sistema
 * ========================================================================= */

/** Frequência do clock principal do sistema em Hz (96 MHz; USB FS em 48 MHz) */
#define CLOCK_SISTEMA_HZ        96000000UL

/* ============================================================================
 * Amostragem de Sensores
 * ========================================================================= */

/** Frequência de amostragem dos sensores em Hz */
#define FREQ_AMOSTRAGEM_SENSORES_HZ     200

/** Período de amostragem dos sensores em milissegundos */
#define PERIODO_AMOSTRAGEM_SENSORES_MS  5

/* ============================================================================
 * Telemetria
 * ========================================================================= */

/** Frequência de envio de telemetria em Hz */
#define FREQ_TELEMETRIA_HZ      10

/** Período de envio de telemetria em milissegundos */
#define PERIODO_TELEMETRIA_MS   100

/* ============================================================================
 * Datalogger (Cartão SD)
 * ========================================================================= */

/** Frequência de gravação do datalogger em Hz */
#define FREQ_DATALOGGER_HZ      200

/* ============================================================================
 * Buffers de Comunicação
 * ========================================================================= */

/** Tamanho do buffer de transmissão em bytes */
#define TAMANHO_BUFFER_TX       256

/** Tamanho do buffer de recepção em bytes */
#define TAMANHO_BUFFER_RX       256

/* ============================================================================
 * Timeouts
 * ========================================================================= */

/** Timeout para comunicação com sensores em milissegundos */
#define TIMEOUT_SENSOR_MS       50

/* ============================================================================
 * Habilitações de Módulos
 * ========================================================================= */

/** Habilita o módulo de datalogger (gravação no SD) */
#define HABILITAR_DATALOGGER    1

/** Habilita o módulo de telemetria (transmissão LoRa) */
#define HABILITAR_TELEMETRIA    1

/** Habilita o módulo de recuperação (acionamento pirotécnico) */
#define HABILITAR_RECUPERACAO   1

/** Habilita o módulo de GPS (NEO-M8N/NEO-M9N) */
#define HABILITAR_GPS           1

/** Baudrate do módulo GPS (9600 para NEO-M8N, 38400 para NEO-M9N) */
#define GPS_BAUDRATE            9600

#endif /* CONFIG_H */
