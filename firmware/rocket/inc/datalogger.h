/**
 * @file datalogger.h
 * @brief Interface do datalogger para gravação de dados em cartão SD.
 *
 * Fornece funções para inicialização, abertura/fechamento de arquivo,
 * escrita de dados brutos e de sensores, e gerenciamento de buffer.
 *
 * Comunicação: SPI3
 * Chip Select: PA15
 * Formato de gravação: binário (voo.bin)
 */

#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <stdint.h>
#include <stdbool.h>
#include "sensors.h"

/* ============================================================================
 * Configurações do Datalogger
 * ========================================================================= */

/** Tamanho do buffer interno de escrita (alinhado ao setor do SD = 512 bytes) */
#define DATALOGGER_TAMANHO_BUFFER       512

/** Nome padrão do arquivo de log de voo */
#define DATALOGGER_NOME_ARQUIVO_PADRAO  "voo.bin"

/* ============================================================================
 * Funções do Datalogger
 * ========================================================================= */

/**
 * @brief Inicializa o módulo de datalogger e o cartão SD.
 *
 * Configura o SPI3, detecta o cartão SD e prepara o sistema de arquivos.
 *
 * @return STATUS_OK em caso de sucesso, STATUS_ERRO_SD caso contrário.
 */
status_t datalogger_inicializar(void);

/**
 * @brief Abre (ou cria) um arquivo no cartão SD para gravação.
 * @param[in] nome Nome do arquivo a ser aberto (ex: "voo.bin").
 * @return STATUS_OK em caso de sucesso, STATUS_ERRO_SD caso contrário.
 */
status_t datalogger_abrir_arquivo(const char *nome);

/**
 * @brief Escreve dados brutos no arquivo de log.
 * @param[in] dados   Ponteiro para os dados a serem gravados.
 * @param[in] tamanho Número de bytes a serem gravados.
 * @return STATUS_OK em caso de sucesso.
 */
status_t datalogger_escrever(const void *dados, uint16_t tamanho);

/**
 * @brief Escreve uma amostra completa de dados dos sensores no log.
 *
 * Serializa a estrutura dados_sensores_t e grava no arquivo. Esta função
 * é uma conveniência sobre datalogger_escrever().
 *
 * @param[in] dados Ponteiro para a estrutura de dados dos sensores.
 * @return STATUS_OK em caso de sucesso.
 */
status_t datalogger_escrever_sensores(const dados_sensores_t *dados);

/**
 * @brief Força a escrita do buffer interno para o cartão SD.
 *
 * Deve ser chamado periodicamente ou em momentos críticos para garantir
 * que os dados não sejam perdidos em caso de perda de energia.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t datalogger_flush(void);

/**
 * @brief Fecha o arquivo de log e finaliza a gravação.
 *
 * Realiza flush final e atualiza os metadados do arquivo no sistema de arquivos.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t datalogger_fechar(void);

/**
 * @brief Retorna o número total de bytes escritos no arquivo atual.
 * @return Quantidade de bytes gravados desde a abertura do arquivo.
 */
uint32_t datalogger_bytes_escritos(void);

/**
 * @brief Verifica se o datalogger está pronto para gravação.
 *
 * Retorna verdadeiro se o cartão SD foi detectado, inicializado e um
 * arquivo está aberto para escrita.
 *
 * @return true se pronto para gravar, false caso contrário.
 */
bool datalogger_esta_pronto(void);

#endif /* DATALOGGER_H */
