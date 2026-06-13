/**
 * @file recovery.c
 * @brief Implementação do sistema de recuperação pirotécnica.
 */

#include "recovery.h"
#include "flight_state.h"
#include "hal_plataforma.h"
#include "pinout.h"
#include "stm32f411_hw.h"

static bool sistema_armado = false;
static bool principal_acionado = false;
static bool emergencia_acionado = false;
static volatile bool pulso_principal_ativo = false;
static volatile bool pulso_emergencia_ativo = false;
static volatile uint32_t inicio_pulso_principal_ms = 0;
static volatile uint32_t inicio_pulso_emergencia_ms = 0;

status_t recuperacao_inicializar(void) {
    hw_gpio_output(PINO_RECUP_PRINCIPAL_PORTA, PINO_RECUP_PRINCIPAL_PINO, false);
    hw_gpio_output(PINO_RECUP_EMERGENCIA_PORTA, PINO_RECUP_EMERGENCIA_PINO, false);
    hw_gpio_input_pullup(PINO_CHAVE_ARMAMENTO_PORTA, PINO_CHAVE_ARMAMENTO_PINO);

    sistema_armado = false;
    principal_acionado = false;
    emergencia_acionado = false;
    pulso_principal_ativo = false;
    pulso_emergencia_ativo = false;

    return STATUS_OK;
}

void recuperacao_processar(void) {
    uint32_t agora = plataforma_obter_tick_ms();
    if (pulso_principal_ativo &&
        (agora - inicio_pulso_principal_ms) >= TEMPO_PULSO_PIROTECNICO_MS) {
        hw_gpio_write(PINO_RECUP_PRINCIPAL_PORTA, PINO_RECUP_PRINCIPAL_PINO, false);
        pulso_principal_ativo = false;
    }
    if (pulso_emergencia_ativo &&
        (agora - inicio_pulso_emergencia_ms) >= TEMPO_PULSO_PIROTECNICO_MS) {
        hw_gpio_write(PINO_RECUP_EMERGENCIA_PORTA, PINO_RECUP_EMERGENCIA_PINO, false);
        pulso_emergencia_ativo = false;
    }
}

status_t recuperacao_armar(void) {
    if (hw_gpio_read(PINO_CHAVE_ARMAMENTO_PORTA, PINO_CHAVE_ARMAMENTO_PINO)) {
        return STATUS_ERRO_GENERICO;
    }
    sistema_armado = true;
    return STATUS_OK;
}

status_t recuperacao_desarmar(void) {
    hw_gpio_write(PINO_RECUP_PRINCIPAL_PORTA, PINO_RECUP_PRINCIPAL_PINO, false);
    hw_gpio_write(PINO_RECUP_EMERGENCIA_PORTA, PINO_RECUP_EMERGENCIA_PINO, false);
    pulso_principal_ativo = false;
    pulso_emergencia_ativo = false;
    sistema_armado = false;
    return STATUS_OK;
}

status_t recuperacao_acionar_principal(void) {
    if (!sistema_armado) return STATUS_ERRO_GENERICO;

    flight_state_t estado = fsm_obter_estado();
    if (estado != FSM_APOGEE && estado != FSM_DESCENT) {
        return STATUS_ERRO_GENERICO; /* Segurança: não acionar fora desses estados */
    }

    if (principal_acionado || pulso_principal_ativo) return STATUS_ERRO_GENERICO;
    hw_gpio_write(PINO_RECUP_PRINCIPAL_PORTA, PINO_RECUP_PRINCIPAL_PINO, true);
    inicio_pulso_principal_ms = plataforma_obter_tick_ms();
    pulso_principal_ativo = true;
    principal_acionado = true;
    return STATUS_OK;
}

status_t recuperacao_acionar_emergencia(void) {
    if (!sistema_armado) return STATUS_ERRO_GENERICO;

    flight_state_t estado = fsm_obter_estado();
    if (estado != FSM_APOGEE && estado != FSM_DESCENT) {
        return STATUS_ERRO_GENERICO; /* Segurança: não acionar fora desses estados */
    }

    if (emergencia_acionado || pulso_emergencia_ativo) return STATUS_ERRO_GENERICO;
    hw_gpio_write(PINO_RECUP_EMERGENCIA_PORTA, PINO_RECUP_EMERGENCIA_PINO, true);
    inicio_pulso_emergencia_ms = plataforma_obter_tick_ms();
    pulso_emergencia_ativo = true;
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
    if (!canal_principal || !canal_emergencia) return STATUS_ERRO_GENERICO;
    *canal_principal = false;
    *canal_emergencia = false;
    return STATUS_ERRO_GENERICO;
}
