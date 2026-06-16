/**
 * @file config.h
 * @brief Configurações da estação base do projeto Foguinho.
 *
 * Define parâmetros de configuração do firmware da estação base,
 * incluindo versão, frequência do sistema, tamanhos de buffers,
 * timeouts e flags de funcionalidades.
 *
 * MCU: STM32F411CEU6
 * Arquitetura: Super-Loop (bare-metal)
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================== */
/*                    Inclusões de Dependências                   */
/* ============================================================== */

#include "protocol_defs.h"

/* ============================================================== */
/*                     Versão do Firmware                          */
/* ============================================================== */

/** @brief Versão major do firmware da estação base */
#define FIRMWARE_ESTACAO_VERSAO_MAJOR    0

/** @brief Versão minor do firmware da estação base */
#define FIRMWARE_ESTACAO_VERSAO_MINOR    1

/** @brief Versão patch do firmware da estação base */
#define FIRMWARE_ESTACAO_VERSAO_PATCH    0

/* ============================================================== */
/*                   Configuração do Sistema                      */
/* ============================================================== */

/** @brief Frequência do clock do sistema em Hz (96 MHz; USB FS em 48 MHz) */
#define CLOCK_SISTEMA_HZ                96000000UL

/* ============================================================== */
/*                  Tamanhos de Buffers (bytes)                   */
/* ============================================================== */

/** @brief Tamanho do buffer de transmissão USB em bytes */
#define TAMANHO_BUFFER_USB_TX           512

/** @brief Tamanho do buffer de recepção USB em bytes */
#define TAMANHO_BUFFER_USB_RX           512

/** @brief Tamanho do buffer de recepção do rádio em bytes */
#define TAMANHO_BUFFER_RADIO_RX         256

/* ============================================================== */
/*                  Configuração de Timeouts                      */
/* ============================================================== */

/** @brief Timeout de recepção de pacotes em milissegundos */
#define TIMEOUT_RECEPCAO_MS             1000

/** @brief Intervalo de heartbeat em milissegundos */
#define INTERVALO_HEARTBEAT_MS          1000

/* ============================================================== */
/*                   Flags de Funcionalidades                     */
/* ============================================================== */

/** @brief Habilitar LED de status (1 = habilitado, 0 = desabilitado) */
#define HABILITAR_LED_STATUS            1

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
