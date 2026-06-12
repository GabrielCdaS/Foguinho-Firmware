/**
 * @file fusion.c
 * @brief Implementação do módulo de fusão sensorial.
 */

#include "fusion.h"
#include "flight_defs.h"

static estado_fusao_t estado;
static float altitude_referencia = 0.0f;
static bool calibrado = false;

/* Variáveis para filtro simples / derivada */
static float ultima_altitude = 0.0f;
static uint32_t ultimo_tempo = 0;

status_t fusao_inicializar(void) {
    estado.altitude_m = 0.0f;
    estado.velocidade_vertical_ms = 0.0f;
    estado.aceleracao_vertical_ms2 = 0.0f;
    estado.timestamp_ms = 0;

    calibrado = false;
    return STATUS_OK;
}

status_t fusao_atualizar(const dados_sensores_t *dados) {
    if (!dados) return STATUS_ERRO_GENERICO;
    if (!calibrado) return STATUS_ERRO_GENERICO;

    float dt = 0.0f;
    if (ultimo_tempo > 0 && dados->timestamp_ms > ultimo_tempo) {
        dt = (float)(dados->timestamp_ms - ultimo_tempo) / 1000.0f;
    }

    /* Filtro complementar simplificado ou apenas atribuição para mock */
    float altitude_atual = dados->altitude_m - altitude_referencia;

    if (dt > 0.0f) {
        estado.velocidade_vertical_ms = (altitude_atual - ultima_altitude) / dt;
    } else {
        estado.velocidade_vertical_ms = 0.0f;
    }

    /* Considera aceleração z (ajustada para remover 1g) em m/s2 */
    estado.aceleracao_vertical_ms2 = (dados->aceleracao_z_g - 1.0f) * 9.80665f;
    estado.altitude_m = altitude_atual;
    estado.timestamp_ms = dados->timestamp_ms;

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
    float accel_g = estado.aceleracao_vertical_ms2 / 9.80665f + 1.0f;
    return (accel_g > LIMIAR_ACELERACAO_LANCAMENTO_G);
}

bool fusao_detectar_apogeu(void) {
    return (estado.velocidade_vertical_ms < LIMIAR_VELOCIDADE_APOGEU_MS && estado.altitude_m > 10.0f);
}

bool fusao_detectar_pouso(void) {
    float abs_vel = estado.velocidade_vertical_ms < 0 ? -estado.velocidade_vertical_ms : estado.velocidade_vertical_ms;
    return (abs_vel < 0.5f && estado.altitude_m < LIMIAR_VARIACAO_ALTITUDE_POUSO_M);
}

status_t fusao_calibrar(void) {
    /* Exige algumas leituras para calibração, aqui apenas simulamos com 0 */
    altitude_referencia = 0.0f;
    calibrado = true;
    return STATUS_OK;
}
