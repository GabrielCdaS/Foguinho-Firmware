/**
 * @file recovery.h
 * @brief Sistema de recuperação pirotécnica de duplo canal.
 *
 * Gerencia o armamento e acionamento dos canais pirotécnicos principal
 * (paraquedas) e de emergência (backup). Inclui verificação de continuidade
 * dos ignitores e proteções de segurança baseadas no estado de voo.
 *
 * Canal principal: PC13
 * Canal de emergência: PC14
 * Chave de armamento: PC15
 *
 * SEGURANÇA: As funções de acionamento verificam o estado de voo antes
 * de permitir a detonação. O acionamento só é permitido nos estados
 * APOGEE ou DESCENT.
 */

#ifndef RECOVERY_H
#define RECOVERY_H

#include <stdint.h>
#include <stdbool.h>
#include "sensors.h"

/* ============================================================================
 * Configurações do Sistema de Recuperação
 * ========================================================================= */

/** Duração do pulso de acionamento pirotécnico em milissegundos */
#define TEMPO_PULSO_PIROTECNICO_MS  1000

/* ============================================================================
 * Funções do Sistema de Recuperação
 * ========================================================================= */

/**
 * @brief Inicializa o sistema de recuperação.
 *
 * Configura os GPIOs dos canais pirotécnicos como saída (nível baixo)
 * e a chave de armamento como entrada com pull-up.
 * O sistema inicia no estado desarmado.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t recuperacao_inicializar(void);

/**
 * @brief Arma o sistema de recuperação.
 *
 * Habilita os canais pirotécnicos para acionamento. Requer que a chave
 * de armamento física esteja na posição correta.
 *
 * @return STATUS_OK em caso de sucesso, STATUS_ERRO_GENERICO se a chave
 *         de armamento não estiver ativa.
 */
status_t recuperacao_armar(void);

/**
 * @brief Desarma o sistema de recuperação.
 *
 * Desabilita os canais pirotécnicos, impedindo qualquer acionamento.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t recuperacao_desarmar(void);

/**
 * @brief Aciona o canal pirotécnico principal (paraquedas).
 *
 * Gera um pulso de TEMPO_PULSO_PIROTECNICO_MS no canal principal (PC13).
 *
 * SEGURANÇA: Verifica se o sistema está armado e se o estado de voo
 * permite o acionamento antes de executar.
 *
 * @return STATUS_OK em caso de sucesso, STATUS_ERRO_GENERICO se não
 *         estiver armado ou estado de voo incorreto.
 */
status_t recuperacao_acionar_principal(void);

/**
 * @brief Aciona o canal pirotécnico de emergência (backup).
 *
 * Gera um pulso de TEMPO_PULSO_PIROTECNICO_MS no canal de emergência (PC14).
 * Utilizado quando o canal principal falha ou como redundância.
 *
 * SEGURANÇA: Verifica se o sistema está armado e se o estado de voo
 * permite o acionamento antes de executar.
 *
 * @return STATUS_OK em caso de sucesso, STATUS_ERRO_GENERICO se não
 *         estiver armado ou estado de voo incorreto.
 */
status_t recuperacao_acionar_emergencia(void);

/**
 * @brief Verifica se o sistema de recuperação está armado.
 * @return true se armado, false se desarmado.
 */
bool recuperacao_esta_armado(void);

/**
 * @brief Verifica se o canal principal já foi acionado.
 * @return true se o canal principal foi acionado.
 */
bool recuperacao_principal_acionado(void);

/**
 * @brief Verifica se o canal de emergência já foi acionado.
 * @return true se o canal de emergência foi acionado.
 */
bool recuperacao_emergencia_acionado(void);

/**
 * @brief Verifica a continuidade elétrica dos ignitores pirotécnicos.
 *
 * Realiza uma medição de baixa corrente para verificar se os ignitores
 * estão conectados corretamente sem acioná-los.
 *
 * @param[out] canal_principal  true se o ignitor principal tem continuidade.
 * @param[out] canal_emergencia true se o ignitor de emergência tem continuidade.
 * @return STATUS_OK em caso de sucesso na verificação.
 */
status_t recuperacao_verificar_continuidade(bool *canal_principal, bool *canal_emergencia);

#endif /* RECOVERY_H */
