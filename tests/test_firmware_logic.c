#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "crc16.h"
#include "command_executor.h"
#include "fusion.h"
#include "gps.h"
#include "protocol_defs.h"
#include "stm32f411_hw.h"
#include "sx1278.h"
#include "telemetry.h"

volatile uint32_t ms_ticks = 0;
volatile bool tick_pendente = false;

static flight_state_t estado_teste = FSM_IDLE;
static status_t status_recuperacao = STATUS_OK;
static status_t status_fsm = STATUS_OK;
static status_t status_log = STATUS_OK;
static bool log_pronto = false;
static bool desarmou_recuperacao = false;

flight_state_t fsm_obter_estado(void) {
    return estado_teste;
}

status_t fsm_processar_comando(command_id_t comando) {
    if (status_fsm != STATUS_OK) return status_fsm;
    if (comando == CMD_ARM) estado_teste = FSM_ARMED;
    if (comando == CMD_DISARM) estado_teste = FSM_IDLE;
    return STATUS_OK;
}

status_t recuperacao_armar(void) {
    return status_recuperacao;
}

status_t recuperacao_desarmar(void) {
    desarmou_recuperacao = true;
    return STATUS_OK;
}

status_t datalogger_abrir_arquivo(const char *nome) {
    assert(nome != NULL);
    if (status_log == STATUS_OK) log_pronto = true;
    return status_log;
}

status_t datalogger_fechar(void) {
    if (status_log == STATUS_OK) log_pronto = false;
    return status_log;
}

bool datalogger_esta_pronto(void) {
    return log_pronto;
}

status_t hw_usart2_init(uint32_t baudrate) {
    return baudrate > 0U ? STATUS_OK : STATUS_ERRO_GENERICO;
}

bool hw_usart2_rx_ready(void) {
    return false;
}

uint8_t hw_usart2_read(void) {
    return 0;
}

static uint8_t radio_buffer[SX1278_TAMANHO_MAX_PACOTE];
static uint8_t radio_tamanho = 0;

status_t sx1278_transmitir(const uint8_t *dados, uint8_t tamanho) {
    assert(dados != NULL);
    assert(tamanho <= sizeof(radio_buffer));
    for (uint8_t i = 0; i < tamanho; ++i) radio_buffer[i] = dados[i];
    radio_tamanho = tamanho;
    return STATUS_OK;
}

float fusao_obter_altitude(void) {
    return 123.5f;
}

float fusao_obter_velocidade_vertical(void) {
    return -4.25f;
}

static bool perto(float atual, float esperado, float tolerancia) {
    return fabsf(atual - esperado) <= tolerancia;
}

static bool inteiro_perto(int32_t atual, int32_t esperado, int32_t tolerancia) {
    int64_t diferenca = (int64_t)atual - esperado;
    if (diferenca < 0) diferenca = -diferenca;
    return diferenca <= tolerancia;
}

static void testar_gps(void) {
    float latitude;
    float longitude;
    float altitude;
    uint8_t satelites;
    bool fix;

    assert(gps_inicializar() == STATUS_OK);
    ms_ticks = 100U;
    assert(gps_processar_sentenca_nmea(
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"
    ) == STATUS_OK);
    assert(gps_obter_dados(&latitude, &longitude, &altitude, &satelites, &fix) == STATUS_OK);
    assert(fix);
    assert(satelites == 8U);
    assert(perto(latitude, 48.1173f, 0.0001f));
    assert(perto(longitude, 11.516666f, 0.0001f));
    assert(perto(altitude, 545.4f, 0.01f));

    ms_ticks = 500U;
    assert(gps_processar_sentenca_nmea(
        "$GPRMC,123519,A,4807.038,S,01131.000,W,022.4,084.4,230394,003.1,W*65"
    ) == STATUS_OK);
    assert(gps_obter_dados(&latitude, &longitude, &altitude, &satelites, &fix) == STATUS_OK);
    assert(fix);
    assert(latitude < 0.0f);
    assert(longitude < 0.0f);

    assert(gps_processar_sentenca_nmea(
        "$GPGGA,123519,0000.000,N,00000.000,E,1,01,0.9,1.0,M,0.0,M,,*00"
    ) == STATUS_ERRO_CRC);

    ms_ticks = 4000U;
    assert(gps_obter_dados(&latitude, &longitude, &altitude, &satelites, &fix) == STATUS_OK);
    assert(!fix);
    assert(latitude < 0.0f);
    assert(longitude < 0.0f);
}

static void testar_telemetria(void) {
    dados_sensores_t dados = {0};
    telemetry_packet_t pacote;

    dados.timestamp_ms = 1234U;
    dados.aceleracao_x_g = 1.0f;
    dados.aceleracao_y_g = 2.0f;
    dados.aceleracao_z_g = 2.0f;
    dados.tensao_bateria_mv = 7420.0f;
    dados.latitude = -23.55052f;
    dados.longitude = -46.633308f;
    dados.gps_altitude_m = 760.0f;
    dados.gps_satellites = 12U;
    dados.gps_fix_valid = true;

    assert(telemetria_inicializar() == STATUS_OK);
    assert(telemetria_construir_pacote(&pacote, &dados, FSM_ASCENT) == STATUS_OK);
    assert(pacote.header == PROTOCOL_HEADER_BYTE);
    assert(pacote.timestamp_ms == 1234U);
    assert(perto(pacote.acceleration_g, 3.0f, 0.001f));
    assert(pacote.battery_mv == 7420U);
    assert(pacote.flight_state == FSM_ASCENT);
    assert(inteiro_perto(pacote.gps_latitude, -235505200, 20));
    assert(inteiro_perto(pacote.gps_longitude, -466333080, 20));
    assert(pacote.gps_altitude_m == 760);
    assert(pacote.gps_info == (uint8_t)(0x80U | 12U));
    assert(crc16_verificar(
        (const uint8_t *)&pacote,
        (uint16_t)(sizeof(pacote) - sizeof(pacote.crc16)),
        pacote.crc16
    ));

    assert(telemetria_enviar() == STATUS_OK);
    assert(radio_tamanho == sizeof(telemetry_packet_t));
    assert(telemetria_pacotes_enviados() == 1U);
}

static void testar_comandos(void) {
    estado_teste = FSM_IDLE;
    status_recuperacao = STATUS_OK;
    status_fsm = STATUS_OK;
    assert(comando_executar(CMD_ARM) == COMMAND_RESULT_OK);
    assert(estado_teste == FSM_ARMED);

    assert(comando_executar(CMD_ARM) == COMMAND_RESULT_INVALID_STATE);
    assert(comando_executar(CMD_PING) == COMMAND_RESULT_OK);
    assert(comando_executar(CMD_DISARM) == COMMAND_RESULT_OK);
    assert(estado_teste == FSM_IDLE);

    status_recuperacao = STATUS_ERRO_GENERICO;
    assert(comando_executar(CMD_ARM) == COMMAND_RESULT_SAFETY_LOCK);
    assert(estado_teste == FSM_IDLE);

    status_recuperacao = STATUS_OK;
    status_fsm = STATUS_ERRO_GENERICO;
    desarmou_recuperacao = false;
    assert(comando_executar(CMD_ARM) == COMMAND_RESULT_INVALID_STATE);
    assert(desarmou_recuperacao);

    status_fsm = STATUS_OK;
    status_log = STATUS_OK;
    log_pronto = false;
    assert(comando_executar(CMD_START_LOG) == COMMAND_RESULT_OK);
    assert(log_pronto);
    assert(comando_executar(CMD_STOP_LOG) == COMMAND_RESULT_OK);
    assert(!log_pronto);

    status_log = STATUS_ERRO_SD;
    assert(comando_executar(CMD_START_LOG) == COMMAND_RESULT_STORAGE_ERROR);
}

int main(void) {
    testar_gps();
    testar_telemetria();
    testar_comandos();
    puts("firmware logic tests passed");
    return 0;
}
