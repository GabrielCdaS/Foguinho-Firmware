/**
 * @file protocol.h
 * @brief Gerenciamento de protocolo da estação base.
 *
 * Define a interface de processamento de protocolo para a estação base,
 * incluindo recepção de pacotes LoRa do foguete, encaminhamento para
 * o PC via USB, e processamento de comandos recebidos do PC.
 *
 * Fluxo principal:
 *   Foguete → LoRa (SX1278) → Estação Base → USB CDC → PC
 *   PC → USB CDC → Estação Base → LoRa (SX1278) → Foguete
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================== */
/*                    Inclusões de Dependências                   */
/* ============================================================== */

#include <stdint.h>
#include <stdbool.h>
#include "../../common/inc/protocol_defs.h"

/* ============================================================== */
/*                     Tipos e Enumerações                        */
/* ============================================================== */

/**
 * @brief Códigos de status retornados pelas funções do sistema.
 *
 * Utilizado em todo o firmware da estação base para indicar
 * o resultado de operações.
 */
typedef enum {
    STATUS_OK            = 0,   /**< Operação concluída com sucesso */
    STATUS_ERRO_SPI      = 1,   /**< Erro na comunicação SPI */
    STATUS_ERRO_SENSOR   = 2,   /**< Erro ao acessar sensor */
    STATUS_ERRO_TIMEOUT  = 3,   /**< Timeout na operação */
    STATUS_ERRO_CRC      = 4,   /**< Falha na verificação CRC */
    STATUS_ERRO_SD       = 5,   /**< Erro no cartão SD */
    STATUS_ERRO_RADIO    = 6,   /**< Erro no módulo de rádio */
    STATUS_ERRO_USB      = 7,   /**< Erro na comunicação USB */
    STATUS_ERRO_GENERICO = 8    /**< Erro genérico não categorizado */
} status_t;

/* ============================================================== */
/*                   Funções de Inicialização                     */
/* ============================================================== */

/**
 * @brief Inicializa o módulo de protocolo da estação base.
 *
 * Configura os buffers internos, zera contadores de estatísticas
 * e prepara o sistema para processar pacotes.
 *
 * @return STATUS_OK em caso de sucesso, código de erro caso contrário.
 */
status_t protocolo_base_inicializar(void);

/* ============================================================== */
/*                  Funções de Processamento                      */
/* ============================================================== */

/**
 * @brief Processa pacotes recebidos via rádio LoRa.
 *
 * Verifica se há pacotes disponíveis no módulo SX1278, valida
 * o CRC, extrai os dados de telemetria e encaminha para o PC
 * via USB.
 *
 * @return STATUS_OK se processado com sucesso ou sem pacotes pendentes.
 * @return STATUS_ERRO_CRC se o pacote recebido falhou na verificação.
 * @return STATUS_ERRO_RADIO se houve erro na comunicação com o rádio.
 */
status_t protocolo_base_processar_radio(void);

/**
 * @brief Processa comandos recebidos via USB do PC.
 *
 * Verifica se há dados disponíveis na interface USB CDC,
 * interpreta os comandos recebidos e os encaminha para o
 * foguete via rádio LoRa.
 *
 * @return STATUS_OK se processado com sucesso ou sem dados pendentes.
 * @return STATUS_ERRO_USB se houve erro na comunicação USB.
 * @return STATUS_ERRO_RADIO se houve erro ao transmitir via rádio.
 */
status_t protocolo_base_processar_usb(void);

/* ============================================================== */
/*                    Funções de Estatísticas                     */
/* ============================================================== */

/**
 * @brief Retorna o número total de pacotes recebidos com sucesso.
 *
 * @return Contagem de pacotes válidos recebidos desde a inicialização.
 */
uint32_t protocolo_base_pacotes_recebidos(void);

/**
 * @brief Retorna o número de pacotes inválidos (falha CRC, etc.).
 *
 * @return Contagem de pacotes inválidos recebidos desde a inicialização.
 */
uint32_t protocolo_base_pacotes_invalidos(void);

/**
 * @brief Retorna o número estimado de pacotes perdidos.
 *
 * Calcula com base em lacunas no ID sequencial dos pacotes
 * recebidos. Uma lacuna de N no ID indica N-1 pacotes perdidos.
 *
 * @return Contagem estimada de pacotes perdidos desde a inicialização.
 */
uint32_t protocolo_base_pacotes_perdidos(void);

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_H */
