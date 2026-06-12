/**
 * @file sx1278.c
 * @brief Implementação do driver do rádio LoRa SX1278.
 */

#include "sx1278.h"
#include <string.h>

static int16_t last_rssi = 0;

status_t sx1278_inicializar(void) {
    /* TODO: Configurar SPI2, CS (PB12), RST (PB1), DIO0 (PB2) */
    /* TODO: Pulso de reset no pino RST */
    /* TODO: Verificar versão lendo SX1278_REG_VERSION (esperado 0x12) */

    /* Configuração inicial Padrão */
    sx1278_modo_sleep();
    sx1278_configurar_frequencia(SX1278_FREQUENCIA_PADRAO);
    sx1278_configurar_lora(SX1278_SF_PADRAO, SX1278_BW_PADRAO, SX1278_CR_PADRAO);
    sx1278_configurar_potencia(SX1278_POTENCIA_PADRAO_DBM);

    /* TODO: Configurar buffer FIFO base para TX e RX */

    sx1278_modo_standby();
    return STATUS_OK;
}

status_t sx1278_transmitir(const uint8_t *dados, uint8_t tamanho) {
    if (!dados || tamanho == 0 || tamanho > SX1278_TAMANHO_MAX_PACOTE) {
        return STATUS_ERRO_GENERICO;
    }

    sx1278_modo_standby();

    /* TODO: Escrever tamanho no PAYLOAD_LENGTH */
    /* TODO: Escrever dados na FIFO */
    /* TODO: Colocar modo de transmissão (TX) */
    /* TODO: Aguardar interrupção DIO0 (TxDone) */

    return STATUS_OK;
}

status_t sx1278_receber(uint8_t *buffer, uint8_t *tamanho, uint32_t timeout_ms) {
    (void)buffer;
    (void)tamanho;
    (void)timeout_ms;
    /* TODO: Colocar em modo RX contínuo ou único */
    /* TODO: Aguardar interrupção DIO0 (RxDone) ou timeout */
    /* TODO: Se receber, ler RX_NB_BYTES, limpar flags e ler a FIFO */

    return STATUS_ERRO_TIMEOUT; /* Simulação vazia */
}

status_t sx1278_configurar_frequencia(uint32_t frequencia_hz) {
    (void)frequencia_hz;
    /* TODO: Calcular (frequencia_hz / 32MHz) * 2^19 e gravar nos registradores FRF */
    return STATUS_OK;
}

status_t sx1278_configurar_potencia(int8_t potencia_dbm) {
    (void)potencia_dbm;
    /* TODO: Configurar PA_CONFIG e PA_DAC baseado na potência */
    return STATUS_OK;
}

status_t sx1278_configurar_lora(sx1278_sf_t sf, sx1278_bw_t bw, sx1278_cr_t cr) {
    (void)sf; (void)bw; (void)cr;
    /* TODO: Gravar configurações nos registradores MODEM_CONFIG_1 e MODEM_CONFIG_2 */
    return STATUS_OK;
}

int16_t sx1278_ler_rssi(void) {
    return last_rssi;
}

status_t sx1278_modo_sleep(void) {
    /* TODO: Escrever SLEEP e LORA_MODE no OP_MODE */
    return STATUS_OK;
}

status_t sx1278_modo_standby(void) {
    /* TODO: Escrever STDBY e LORA_MODE no OP_MODE */
    return STATUS_OK;
}
