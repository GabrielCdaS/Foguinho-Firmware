/**
 * @file sx1278.c
 * @brief Adaptação da API do projeto ao driver oficial Semtech SX1276.
 */

#include "sx1278.h"

#include <string.h>

#include "hal_plataforma.h"
#include "semtech_port.h"
#include "spi.h"
#include "sx1276-board.h"
#include "sx1276/sx1276.h"

#define SX1278_TX_TIMEOUT_MS 3000U
#define SX1278_PREAMBLE_LEN  8U
#define SX1278_SYMBOL_TIMEOUT 1023U
#define SX1278_REG_VERSION   0x42U
#define SX1278_VERSION       0x12U

static bool inicializado;
static bool recepcao_ativa;
static bool transmissao_ativa;
static bool pacote_pronto;
static status_t resultado_assincrono = STATUS_OK;
static int16_t ultimo_rssi;
static uint8_t pacote_recebido[SX1278_TAMANHO_MAX_PACOTE];
static uint8_t tamanho_recebido;

static uint32_t frequencia = SX1278_FREQUENCIA_PADRAO;
static int8_t potencia = SX1278_POTENCIA_PADRAO_DBM;
static sx1278_sf_t spreading_factor = SX1278_SF_PADRAO;
static sx1278_bw_t largura_banda = SX1278_BW_PADRAO;
static sx1278_cr_t coding_rate = SX1278_CR_PADRAO;

static uint32_t semtech_bandwidth(void)
{
    return (uint32_t)largura_banda - (uint32_t)SX1278_BW_125K;
}

static void configurar_rx(bool continuo)
{
    SX1276SetRxConfig(MODEM_LORA, semtech_bandwidth(), (uint32_t)spreading_factor,
                      (uint8_t)coding_rate, 0U, SX1278_PREAMBLE_LEN,
                      SX1278_SYMBOL_TIMEOUT, false, 0U, true, false, 0U,
                      false, continuo);
}

static void configurar_tx(void)
{
    SX1276SetTxConfig(MODEM_LORA, potencia, 0U, semtech_bandwidth(),
                      (uint32_t)spreading_factor, (uint8_t)coding_rate,
                      SX1278_PREAMBLE_LEN, false, true, false, 0U, false,
                      SX1278_TX_TIMEOUT_MS);
}

static void ao_tx_concluido(void)
{
    SX1276SetStby();
    transmissao_ativa = false;
    resultado_assincrono = STATUS_OK;
}

static void ao_tx_timeout(void)
{
    SX1276SetStby();
    transmissao_ativa = false;
    resultado_assincrono = STATUS_ERRO_TIMEOUT;
}

static void ao_rx_concluido(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    (void)snr;
    if (size > SX1278_TAMANHO_MAX_PACOTE) size = SX1278_TAMANHO_MAX_PACOTE;
    memcpy(pacote_recebido, payload, size);
    tamanho_recebido = (uint8_t)size;
    ultimo_rssi = rssi;
    pacote_pronto = true;
}

static void ao_rx_timeout(void)
{
    recepcao_ativa = false;
    resultado_assincrono = STATUS_ERRO_TIMEOUT;
}

static void ao_rx_erro(void)
{
    recepcao_ativa = false;
    resultado_assincrono = STATUS_ERRO_CRC;
}

static RadioEvents_t eventos = {
    .TxDone = ao_tx_concluido,
    .TxTimeout = ao_tx_timeout,
    .RxDone = ao_rx_concluido,
    .RxTimeout = ao_rx_timeout,
    .RxError = ao_rx_erro,
    .FhssChangeChannel = NULL,
    .CadDone = NULL,
    .GnssDone = NULL,
    .WifiDone = NULL
};

status_t sx1278_inicializar(void)
{
    SpiInit(&SX1276.Spi, SPI_2, PB_15, PB_14, PB_13, PB_12);
    SX1276IoInit();
    SX1276Init(&eventos);
    if (SX1276Read(SX1278_REG_VERSION) != SX1278_VERSION) {
        SX1276SetSleep();
        return STATUS_ERRO_RADIO;
    }

    SX1276SetChannel(frequencia);
    SX1276SetPublicNetwork(false);
    SX1276SetMaxPayloadLength(MODEM_LORA, SX1278_TAMANHO_MAX_PACOTE);
    configurar_tx();
    inicializado = true;
    return sx1278_modo_standby();
}

status_t sx1278_transmitir(const uint8_t *dados, uint8_t tamanho)
{
    if (!inicializado || dados == NULL || tamanho == 0U) return STATUS_ERRO_GENERICO;
    if (transmissao_ativa) return STATUS_ERRO_RADIO;

    SX1276SetStby();
    configurar_tx();
    resultado_assincrono = STATUS_OK;
    recepcao_ativa = false;
    transmissao_ativa = true;
    SX1276Send((uint8_t *)dados, tamanho);
    return STATUS_OK;
}

status_t sx1278_processar(void)
{
    status_t resultado;
    if (!inicializado) return STATUS_OK;
    semtech_port_process();
    resultado = resultado_assincrono;
    resultado_assincrono = STATUS_OK;
    return resultado;
}

status_t sx1278_receber(uint8_t *buffer, uint8_t *tamanho, uint32_t timeout_ms)
{
    uint32_t inicio;
    if (!inicializado || buffer == NULL || tamanho == NULL) return STATUS_ERRO_GENERICO;
    if (transmissao_ativa) return STATUS_ERRO_TIMEOUT;
    *tamanho = 0U;

    if (!recepcao_ativa) {
        pacote_pronto = false;
        resultado_assincrono = STATUS_OK;
        configurar_rx(timeout_ms == 0U);
        SX1276SetRx(timeout_ms);
        recepcao_ativa = true;
    }

    inicio = plataforma_obter_tick_ms();
    do {
        status_t resultado = sx1278_processar();
        if (pacote_pronto) {
            memcpy(buffer, pacote_recebido, tamanho_recebido);
            *tamanho = tamanho_recebido;
            pacote_pronto = false;
            if (timeout_ms != 0U) recepcao_ativa = false;
            return STATUS_OK;
        }
        if (resultado != STATUS_OK) return resultado;
        if (timeout_ms == 0U) break;
    } while ((plataforma_obter_tick_ms() - inicio) < timeout_ms);

    if (timeout_ms != 0U) {
        SX1276SetStby();
        recepcao_ativa = false;
    }
    return STATUS_ERRO_TIMEOUT;
}

status_t sx1278_configurar_frequencia(uint32_t frequencia_hz)
{
    if (!SX1276CheckRfFrequency(frequencia_hz)) return STATUS_ERRO_GENERICO;
    frequencia = frequencia_hz;
    if (inicializado) SX1276SetChannel(frequencia);
    return STATUS_OK;
}

status_t sx1278_configurar_potencia(int8_t potencia_dbm)
{
    if (potencia_dbm < 2 || potencia_dbm > 20) return STATUS_ERRO_GENERICO;
    potencia = potencia_dbm;
    if (inicializado) configurar_tx();
    return STATUS_OK;
}

status_t sx1278_configurar_lora(sx1278_sf_t sf, sx1278_bw_t bw, sx1278_cr_t cr)
{
    if (sf < SX1278_SF7 || sf > SX1278_SF12 ||
        bw < SX1278_BW_125K || bw > SX1278_BW_500K ||
        cr < SX1278_CR_4_5 || cr > SX1278_CR_4_8) {
        return STATUS_ERRO_GENERICO;
    }
    spreading_factor = sf;
    largura_banda = bw;
    coding_rate = cr;
    if (inicializado) {
        configurar_rx(true);
        configurar_tx();
    }
    return STATUS_OK;
}

int16_t sx1278_ler_rssi(void)
{
    return ultimo_rssi;
}

status_t sx1278_modo_sleep(void)
{
    if (!inicializado) return STATUS_ERRO_RADIO;
    SX1276SetSleep();
    recepcao_ativa = false;
    transmissao_ativa = false;
    return STATUS_OK;
}

status_t sx1278_modo_standby(void)
{
    if (!inicializado) return STATUS_ERRO_RADIO;
    SX1276SetStby();
    recepcao_ativa = false;
    transmissao_ativa = false;
    return STATUS_OK;
}
