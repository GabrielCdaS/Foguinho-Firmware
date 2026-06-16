/**
 * @file fusion.h
 * @brief Fusão sensorial — barometria + IMU com validação cruzada.
 *
 * Combina dados do barômetro (BMP388) e da IMU (ICM-20948) para estimar
 * altitude, velocidade vertical e aceleração vertical com maior precisão.
 * Detecta eventos de voo: lançamento, apogeu e pouso.
 *
 * A fusão utiliza validação cruzada entre os sensores para aumentar
 * a confiabilidade das estimativas durante todas as fases do voo.
 */

#ifndef FUSION_H
#define FUSION_H

#include <stdint.h>
#include <stdbool.h>
#include "sensors.h"

/* ============================================================================
 * Estrutura de Estado da Fusão Sensorial
 * ========================================================================= */

/**
 * @brief Estado estimado pelo algoritmo de fusão sensorial.
 *
 * Contém as variáveis de estado resultantes da fusão entre barômetro e IMU.
 */
typedef struct {
    float altitude_m;               /**< Altitude estimada em metros (relativa ao solo) */
    float velocidade_vertical_ms;   /**< Velocidade vertical estimada em m/s (positivo = subindo) */
    float aceleracao_vertical_ms2;  /**< Aceleração vertical estimada em m/s² */
    uint32_t timestamp_ms;          /**< Timestamp da última atualização em milissegundos */
} estado_fusao_t;

/* ============================================================================
 * Funções do Módulo de Fusão Sensorial
 * ========================================================================= */

/**
 * @brief Inicializa o módulo de fusão sensorial.
 *
 * Zera as variáveis de estado e prepara os filtros internos.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t fusao_inicializar(void);

/**
 * @brief Atualiza o estado da fusão com uma nova amostra de sensores.
 *
 * Processa os dados do barômetro e da IMU, realiza validação cruzada
 * e atualiza as estimativas de altitude, velocidade e aceleração.
 *
 * @param[in] dados Ponteiro para os dados mais recentes dos sensores.
 * @return STATUS_OK em caso de sucesso.
 */
status_t fusao_atualizar(const dados_sensores_t *dados);

/**
 * @brief Obtém a altitude estimada pela fusão.
 * @return Altitude em metros (relativa ao solo calibrado).
 */
float fusao_obter_altitude(void);

/**
 * @brief Obtém a velocidade vertical estimada pela fusão.
 * @return Velocidade vertical em m/s (positivo = subindo).
 */
float fusao_obter_velocidade_vertical(void);

/**
 * @brief Obtém a aceleração vertical estimada pela fusão.
 * @return Aceleração vertical em m/s².
 */
float fusao_obter_aceleracao_vertical(void);

/**
 * @brief Detecta se o foguete foi lançado.
 *
 * Verifica se a aceleração e a taxa de variação de altitude indicam
 * que o foguete iniciou o voo propulsado.
 *
 * @return true se o lançamento foi detectado.
 */
bool fusao_detectar_lancamento(void);

/**
 * @brief Detecta se o foguete atingiu o apogeu.
 *
 * Verifica se a velocidade vertical cruzou zero (de positiva para negativa)
 * e a altitude está diminuindo consistentemente.
 *
 * @return true se o apogeu foi detectado.
 */
bool fusao_detectar_apogeu(void);

/**
 * @brief Detecta se o foguete pousou.
 *
 * Verifica se a velocidade vertical, aceleração e variação de altitude
 * estão próximas de zero por um período prolongado.
 *
 * @return true se o pouso foi detectado.
 */
bool fusao_detectar_pouso(void);

/**
 * @brief Calibra o módulo de fusão (referência de solo).
 *
 * Coleta múltiplas amostras para estabelecer as condições de referência
 * no solo (altitude zero, gravidade estática).
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t fusao_calibrar(void);

#endif /* FUSION_H */
