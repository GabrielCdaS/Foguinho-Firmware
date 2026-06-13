/**
 * @file fusion.c
 * @brief Implementação do módulo de fusão sensorial.
 */

#include "fusion.h"
#include "bmp388.h"
#include "flight_defs.h"
#include <math.h>

static estado_fusao_t estado;
static float altitude_referencia = 0.0f;
static bool calibrado = false;

/* Variáveis para filtro simples / derivada */
static float ultima_altitude = 0.0f;
static uint32_t ultimo_tempo = 0;
static float velocidade_filtrada = 0.0f;
static float velocidade_anterior = 0.0f;
static uint8_t confirmacao_lancamento = 0;
static uint8_t confirmacao_apogeu = 0;
static uint32_t inicio_estabilidade_ms = 0;

status_t fusao_inicializar(void) {
    estado.altitude_m = 0.0f;
    estado.velocidade_vertical_ms = 0.0f;
    estado.aceleracao_vertical_ms2 = 0.0f;
    estado.timestamp_ms = 0;

    calibrado = false;
    ultima_altitude = 0.0f;
    ultimo_tempo = 0;
    velocidade_filtrada = 0.0f;
    velocidade_anterior = 0.0f;
    confirmacao_lancamento = 0;
    confirmacao_apogeu = 0;
    inicio_estabilidade_ms = 0;
    return STATUS_OK;
}

status_t fusao_atualizar(const dados_sensores_t *dados) {
    if (!dados) return STATUS_ERRO_GENERICO;
    if (!calibrado) return STATUS_ERRO_GENERICO;

    float dt = 0.0f;
    if (ultimo_tempo > 0 && dados->timestamp_ms > ultimo_tempo) {
        dt = (float)(dados->timestamp_ms - ultimo_tempo) / 1000.0f;
    }

    /* Derivada barométrica com filtro passa-baixas para rejeitar ruído. */
    float altitude_atual = dados->altitude_m - altitude_referencia;

    if (dt > 0.0f && dt < 0.5f) {
        float velocidade_barometrica = (altitude_atual - ultima_altitude) / dt;
        velocidade_filtrada = 0.8f * velocidade_filtrada + 0.2f * velocidade_barometrica;
        estado.velocidade_vertical_ms = velocidade_filtrada;
    } else {
        estado.velocidade_vertical_ms = 0.0f;
    }

    /* Considera aceleração z (ajustada para remover 1g) em m/s2 */
    estado.aceleracao_vertical_ms2 = (dados->aceleracao_z_g - 1.0f) * 9.80665f;
    estado.altitude_m = altitude_atual;
    estado.timestamp_ms = dados->timestamp_ms;

    float accel_g = fabsf(dados->aceleracao_z_g);
    if (accel_g >= LIMIAR_ACELERACAO_LANCAMENTO_G) {
        if (confirmacao_lancamento < 10U) confirmacao_lancamento++;
    } else {
        confirmacao_lancamento = 0;
    }

    if (velocidade_anterior > LIMIAR_VELOCIDADE_APOGEU_MS &&
        estado.velocidade_vertical_ms <= 0.0f && estado.altitude_m > 10.0f) {
        if (confirmacao_apogeu < 3U) confirmacao_apogeu++;
    } else if (estado.velocidade_vertical_ms > LIMIAR_VELOCIDADE_APOGEU_MS) {
        confirmacao_apogeu = 0;
    }

    if (fabsf(estado.velocidade_vertical_ms) < 0.5f &&
        fabsf(estado.aceleracao_vertical_ms2) < 2.0f) {
        if (inicio_estabilidade_ms == 0U) inicio_estabilidade_ms = dados->timestamp_ms;
    } else {
        inicio_estabilidade_ms = 0;
    }

    velocidade_anterior = estado.velocidade_vertical_ms;
    ultima_altitude = altitude_atual;
    ultimo_tempo = dados->timestamp_ms;

    return STATUS_OK;
}

float fusao_obter_altitude(void) {
    return estado.altitude_m;
}

float fusao_obter_velocidade_vertical(void) {
    return estado.velocidade_vertical_ms;
}

float fusao_obter_aceleracao_vertical(void) {
    return estado.aceleracao_vertical_ms2;
}

bool fusao_detectar_lancamento(void) {
    return confirmacao_lancamento >= 3U;
}

bool fusao_detectar_apogeu(void) {
    return confirmacao_apogeu >= 1U;
}

bool fusao_detectar_pouso(void) {
    return inicio_estabilidade_ms != 0U &&
           (estado.timestamp_ms - inicio_estabilidade_ms) >= TEMPO_CONFIRMACAO_POUSO_MS;
}

status_t fusao_calibrar(void) {
    status_t status = bmp388_calibrar_zero();
    if (status != STATUS_OK) return status;
    altitude_referencia = 0.0f;
    ultima_altitude = 0.0f;
    ultimo_tempo = 0;
    calibrado = true;
    return STATUS_OK;
}
