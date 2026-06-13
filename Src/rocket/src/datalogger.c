/**
 * @file datalogger.c
 * @brief Implementação da gravação no cartão SD.
 */

#include "datalogger.h"
#include <string.h>

#if defined(__has_include)
#  if __has_include("ff.h")
#    include "ff.h"
#    define DATALOGGER_TEM_FATFS 1
#  endif
#endif

#ifndef DATALOGGER_TEM_FATFS
#define DATALOGGER_TEM_FATFS 0
#endif

static bool cartao_montado = false;
static bool arquivo_aberto = false;
static uint32_t bytes_gravados = 0;
static uint16_t buffer_usado = 0;

#if DATALOGGER_TEM_FATFS
static uint8_t buffer_escrita[DATALOGGER_TAMANHO_BUFFER];
static FATFS sistema_arquivos;
static FIL arquivo_log;

static status_t gravar_buffer(void) {
    UINT escritos = 0;
    if (buffer_usado == 0U) return STATUS_OK;
    if (!arquivo_aberto ||
        f_write(&arquivo_log, buffer_escrita, buffer_usado, &escritos) != FR_OK ||
        escritos != buffer_usado) {
        return STATUS_ERRO_SD;
    }
    bytes_gravados += escritos;
    buffer_usado = 0;
    return STATUS_OK;
}
#endif

status_t datalogger_inicializar(void) {
    bytes_gravados = 0;
    buffer_usado = 0;
    arquivo_aberto = false;
#if DATALOGGER_TEM_FATFS
    cartao_montado = f_mount(&sistema_arquivos, "", 1) == FR_OK;
    return cartao_montado ? STATUS_OK : STATUS_ERRO_SD;
#else
    cartao_montado = false;
    return STATUS_ERRO_SD;
#endif
}

status_t datalogger_abrir_arquivo(const char *nome) {
    if (!cartao_montado || !nome || nome[0] == '\0') return STATUS_ERRO_SD;
#if DATALOGGER_TEM_FATFS
    if (f_open(&arquivo_log, nome, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return STATUS_ERRO_SD;
    arquivo_aberto = true;
    bytes_gravados = 0;
    buffer_usado = 0;
    return STATUS_OK;
#else
    (void)nome;
    return STATUS_ERRO_SD;
#endif
}

status_t datalogger_escrever(const void *dados, uint16_t tamanho) {
    if (!arquivo_aberto || (!dados && tamanho > 0U)) return STATUS_ERRO_SD;
#if DATALOGGER_TEM_FATFS
    const uint8_t *origem = (const uint8_t *)dados;
    while (tamanho > 0U) {
        uint16_t espaco = (uint16_t)(DATALOGGER_TAMANHO_BUFFER - buffer_usado);
        uint16_t bloco = tamanho < espaco ? tamanho : espaco;
        memcpy(&buffer_escrita[buffer_usado], origem, bloco);
        buffer_usado = (uint16_t)(buffer_usado + bloco);
        origem += bloco;
        tamanho = (uint16_t)(tamanho - bloco);
        if (buffer_usado == DATALOGGER_TAMANHO_BUFFER && gravar_buffer() != STATUS_OK) {
            return STATUS_ERRO_SD;
        }
    }
    return STATUS_OK;
#else
    (void)dados;
    (void)tamanho;
    return STATUS_ERRO_SD;
#endif
}

status_t datalogger_escrever_sensores(const dados_sensores_t *dados) {
    if (!dados) return STATUS_ERRO_GENERICO;
    return datalogger_escrever(dados, sizeof(dados_sensores_t));
}

status_t datalogger_flush(void) {
    if (!arquivo_aberto) return STATUS_ERRO_SD;
#if DATALOGGER_TEM_FATFS
    if (gravar_buffer() != STATUS_OK) return STATUS_ERRO_SD;
    return f_sync(&arquivo_log) == FR_OK ? STATUS_OK : STATUS_ERRO_SD;
#else
    return STATUS_ERRO_SD;
#endif
}

status_t datalogger_fechar(void) {
    if (!arquivo_aberto) return STATUS_ERRO_SD;
#if DATALOGGER_TEM_FATFS
    if (datalogger_flush() != STATUS_OK || f_close(&arquivo_log) != FR_OK) return STATUS_ERRO_SD;
    arquivo_aberto = false;
    return STATUS_OK;
#else
    return STATUS_ERRO_SD;
#endif
}

uint32_t datalogger_bytes_escritos(void) {
    return bytes_gravados;
}

bool datalogger_esta_pronto(void) {
    return cartao_montado && arquivo_aberto;
}
