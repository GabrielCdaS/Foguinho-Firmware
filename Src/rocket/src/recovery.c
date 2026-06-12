/**
 * @file recovery.c
 * @brief Implementação do sistema de recuperação pirotécnica.
 */

#include "recovery.h"
#include "flight_state.h"
#include "hal_plataforma.h"

static bool sistema_armado = false;
static bool principal_acionado = false;
static bool emergencia_acionado = false;

status_t recuperacao_inicializar(void) {
    /* TODO: Configurar GPIOs PC13 (Principal), PC14 (Emergência) como saída em nível baixo */
    /* TODO: Configurar PC15 (Chave de armamento) como entrada com pull-up */

    sistema_armado = false;
    principal_acionado = false;
    emergencia_acionado = false;

    return STATUS_OK;
}

status_t recuperacao_armar(void) {
    /* TODO: Verificar estado físico da chave de armamento em PC15 */
    /* if (chave_nao_ligada) return STATUS_ERRO_GENERICO; */

    sistema_armado = true;
    return STATUS_OK;
}

status_t recuperacao_desarmar(void) {
    sistema_armado = false;
    return STATUS_OK;
}

status_t recuperacao_acionar_principal(void) {
    if (!sistema_armado) return STATUS_ERRO_GENERICO;

    flight_state_t estado = fsm_obter_estado();
    if (estado != FSM_APOGEE && estado != FSM_DESCENT) {
        return STATUS_ERRO_GENERICO; /* Segurança: não acionar fora desses estados */
    }

    /* TODO: Colocar PC13 em nível alto */
    plataforma_delay_ms(TEMPO_PULSO_PIROTECNICO_MS);
    /* TODO: Colocar PC13 em nível baixo */

    principal_acionado = true;
    return STATUS_OK;
}

status_t recuperacao_acionar_emergencia(void) {
    if (!sistema_armado) return STATUS_ERRO_GENERICO;

    flight_state_t estado = fsm_obter_estado();
    if (estado != FSM_APOGEE && estado != FSM_DESCENT) {
        return STATUS_ERRO_GENERICO; /* Segurança: não acionar fora desses estados */
    }

    /* TODO: Colocar PC14 em nível alto */
    plataforma_delay_ms(TEMPO_PULSO_PIROTECNICO_MS);
    /* TODO: Colocar PC14 em nível baixo */

    emergencia_acionado = true;
    return STATUS_OK;
}

bool recuperacao_esta_armado(void) {
    return sistema_armado;
}

bool recuperacao_principal_acionado(void) {
    return principal_acionado;
}

bool recuperacao_emergencia_acionado(void) {
    return emergencia_acionado;
}

status_t recuperacao_verificar_continuidade(bool *canal_principal, bool *canal_emergencia) {
    /* TODO: Configurar pinos ADC para medir pequena tensão nos ignitores se houver circuito de medição */
    /* Simulação: assumindo que estão conectados */
    if (canal_principal) *canal_principal = true;
    if (canal_emergencia) *canal_emergencia = true;

    return STATUS_OK;
}
