/**
 * @file datalogger.c
 * @brief Implementação da gravação no cartão SD.
 */

#include "datalogger.h"
#include <string.h>

static bool datalogger_pronto = false;
static uint32_t bytes_gravados = 0;

status_t datalogger_inicializar(void) {
    /* TODO: Inicializar SPI3 (SCK PB3, MISO PB4, MOSI PB5) e CS (PA15) */
    /* TODO: Inicializar driver FATFS e montar volume (f_mount) */

    /* Simulação */
    datalogger_pronto = true;
    return STATUS_OK;
}

status_t datalogger_abrir_arquivo(const char *nome) {
    (void)nome; /* Evitar warning de não uso na simulação */
    if (!datalogger_pronto) return STATUS_ERRO_SD;

    /* TODO: Abrir arquivo usando FATFS (f_open com FA_WRITE | FA_CREATE_ALWAYS) */
    bytes_gravados = 0;
    return STATUS_OK;
}

status_t datalogger_escrever(const void *dados, uint16_t tamanho) {
    if (!datalogger_pronto) return STATUS_ERRO_SD;

    /* TODO: Escrever no arquivo (f_write) */
    /* UINT bytes_escritos; f_write(&file, dados, tamanho, &bytes_escritos); */

    bytes_gravados += tamanho;
    return STATUS_OK;
}

status_t datalogger_escrever_sensores(const dados_sensores_t *dados) {
    if (!dados) return STATUS_ERRO_GENERICO;
    return datalogger_escrever(dados, sizeof(dados_sensores_t));
}

status_t datalogger_flush(void) {
    if (!datalogger_pronto) return STATUS_ERRO_SD;

    /* TODO: Sincronizar sistema de arquivos (f_sync) */
    return STATUS_OK;
}

status_t datalogger_fechar(void) {
    if (!datalogger_pronto) return STATUS_ERRO_SD;

    /* TODO: Fechar arquivo (f_close) */
    return STATUS_OK;
}

uint32_t datalogger_bytes_escritos(void) {
    return bytes_gravados;
}

bool datalogger_esta_pronto(void) {
    return datalogger_pronto;
}
