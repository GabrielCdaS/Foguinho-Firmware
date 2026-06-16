/**
 * @file telemetry_parser.h
 * @brief Parser de pacotes de telemetria byte a byte (máquina de estados).
 *
 * Implementa um parser que processa bytes individuais recebidos
 * via rádio LoRa e reconstrói pacotes de telemetria completos.
 * Utiliza uma máquina de estados para rastrear o progresso da
 * montagem do pacote.
 *
 * Fluxo da máquina de estados:
 *   AGUARDANDO_HEADER → RECEBENDO_DADOS → PACOTE_COMPLETO
 *                    ↘                  ↗
 *                      →  ERRO_CRC  →
 *
 * Após PACOTE_COMPLETO ou ERRO_CRC, o parser retorna automaticamente
 * para AGUARDANDO_HEADER ao alimentar o próximo byte.
 */

#ifndef TELEMETRY_PARSER_H
#define TELEMETRY_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================== */
/*                    Inclusões de Dependências                   */
/* ============================================================== */

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"
#include "protocol_defs.h"

/* ============================================================== */
/*                     Tipos e Enumerações                        */
/* ============================================================== */

/**
 * @brief Estados da máquina de estados do parser de telemetria.
 *
 * Representa o estado atual do processo de reconstrução de
 * um pacote de telemetria a partir de bytes individuais.
 */
typedef enum {
    /** Aguardando o byte de header (início do pacote) */
    PARSER_AGUARDANDO_HEADER = 0,

    /** Recebendo bytes de dados do pacote */
    PARSER_RECEBENDO_DADOS   = 1,

    /** Pacote completo recebido e validado com sucesso */
    PARSER_PACOTE_COMPLETO   = 2,

    /** Pacote recebido mas falhou na verificação CRC-16 */
    PARSER_ERRO_CRC          = 3
} parser_estado_t;

/* ============================================================== */
/*                   Funções de Inicialização                     */
/* ============================================================== */

/**
 * @brief Inicializa o parser de telemetria.
 *
 * Reseta a máquina de estados para o estado inicial, zera os
 * contadores de estatísticas e prepara o buffer interno para
 * receber novos bytes.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t parser_inicializar(void);

/* ============================================================== */
/*                  Funções de Processamento                      */
/* ============================================================== */

/**
 * @brief Alimenta um byte ao parser para processamento.
 *
 * Processa um byte individual no contexto da máquina de estados.
 * Quando todos os bytes de um pacote são recebidos, verifica o
 * CRC-16 e retorna o estado correspondente.
 *
 * @param[in] byte  Byte recebido a ser processado.
 *
 * @return Estado atual do parser após processar o byte:
 *   - PARSER_AGUARDANDO_HEADER: aguardando mais bytes do header.
 *   - PARSER_RECEBENDO_DADOS: montando o pacote, aguardando mais dados.
 *   - PARSER_PACOTE_COMPLETO: pacote completo e CRC válido.
 *   - PARSER_ERRO_CRC: pacote completo mas CRC inválido.
 */
parser_estado_t parser_alimentar_byte(uint8_t byte);

/**
 * @brief Obtém o último pacote de telemetria completo.
 *
 * Copia o pacote de telemetria validado para a estrutura fornecida.
 * Deve ser chamado após parser_alimentar_byte() retornar
 * PARSER_PACOTE_COMPLETO.
 *
 * @param[out] pacote  Ponteiro para a estrutura de destino.
 *
 * @return STATUS_OK se o pacote foi copiado com sucesso.
 * @return STATUS_ERRO_GENERICO se não há pacote válido disponível.
 */
status_t parser_obter_pacote(telemetry_packet_t *pacote);

/* ============================================================== */
/*                  Funções de Controle                           */
/* ============================================================== */

/**
 * @brief Reseta o parser para o estado inicial.
 *
 * Descarta qualquer pacote parcialmente recebido e retorna
 * a máquina de estados para PARSER_AGUARDANDO_HEADER.
 * Não reseta os contadores de estatísticas.
 */
void parser_resetar(void);

/**
 * @brief Obtém o estado atual da máquina de estados do parser.
 *
 * @return Estado atual do parser (parser_estado_t).
 */
parser_estado_t parser_obter_estado(void);

/* ============================================================== */
/*                   Funções de Estatísticas                      */
/* ============================================================== */

/**
 * @brief Retorna o número de pacotes válidos processados.
 *
 * Contabiliza pacotes que foram completamente recebidos e
 * passaram na verificação CRC-16.
 *
 * @return Contagem de pacotes válidos desde a última inicialização.
 */
uint32_t parser_pacotes_validos(void);

/**
 * @brief Retorna o número de erros de CRC detectados.
 *
 * Contabiliza pacotes que foram completamente recebidos mas
 * falharam na verificação CRC-16.
 *
 * @return Contagem de erros CRC desde a última inicialização.
 */
uint32_t parser_erros_crc(void);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_PARSER_H */
