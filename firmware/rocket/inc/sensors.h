/**
 * @file sensors.h
 * @brief Interface genérica de sensores e definições de tipos comuns.
 *
 * Define o tipo de status utilizado em todo o firmware, a estrutura de dados
 * dos sensores e as funções de interface para inicialização, leitura e autoteste.
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Códigos de Status do Sistema
 * ========================================================================= */

/**
 * @brief Enumeração de códigos de status retornados pelas funções do firmware.
 *
 * STATUS_OK indica sucesso. Todos os outros valores indicam tipos específicos
 * de erro para facilitar a depuração.
 */
typedef enum {
    STATUS_OK              = 0,  /**< Operação concluída com sucesso */
    STATUS_ERRO_SPI,             /**< Erro na comunicação SPI */
    STATUS_ERRO_SENSOR,          /**< Erro genérico de sensor (chip ID incorreto, etc.) */
    STATUS_ERRO_TIMEOUT,         /**< Timeout na operação */
    STATUS_ERRO_CRC,             /**< Erro de verificação CRC */
    STATUS_ERRO_SD,              /**< Erro no cartão SD */
    STATUS_ERRO_RADIO,           /**< Erro no rádio LoRa */
    STATUS_ERRO_GENERICO         /**< Erro genérico não categorizado */
} status_t;

/* ============================================================================
 * Estrutura de Dados dos Sensores
 * ========================================================================= */

/**
 * @brief Estrutura consolidada com todos os dados de sensores.
 *
 * Contém leituras do barômetro, IMU (acelerômetro, giroscópio, magnetômetro),
 * tensão da bateria e timestamp para sincronização.
 */
typedef struct {
    /* Barômetro (BMP388) */
    float altitude_m;           /**< Altitude calculada em metros (relativa ao solo) */
    float pressao_pa;           /**< Pressão atmosférica em Pascals */
    float temperatura_c;        /**< Temperatura em graus Celsius */

    /* Acelerômetro (ICM-20948) */
    float aceleracao_x_g;       /**< Aceleração no eixo X em g */
    float aceleracao_y_g;       /**< Aceleração no eixo Y em g */
    float aceleracao_z_g;       /**< Aceleração no eixo Z em g */

    /* Giroscópio (ICM-20948) */
    float giroscopio_x_dps;     /**< Velocidade angular no eixo X em °/s */
    float giroscopio_y_dps;     /**< Velocidade angular no eixo Y em °/s */
    float giroscopio_z_dps;     /**< Velocidade angular no eixo Z em °/s */

    /* Magnetômetro (ICM-20948 / AK09916) */
    float magnetometro_x_ut;    /**< Campo magnético no eixo X em µT */
    float magnetometro_y_ut;    /**< Campo magnético no eixo Y em µT */
    float magnetometro_z_ut;    /**< Campo magnético no eixo Z em µT */

    /* Bateria */
    float tensao_bateria_mv;    /**< Tensão da bateria em milivolts */

    /* Timestamp */
    uint32_t timestamp_ms;      /**< Timestamp da leitura em milissegundos */
} dados_sensores_t;

/* ============================================================================
 * Funções da Interface de Sensores
 * ========================================================================= */

/**
 * @brief Inicializa todos os sensores do sistema.
 * @return STATUS_OK em caso de sucesso, código de erro caso contrário.
 */
status_t sensores_inicializar(void);

/**
 * @brief Realiza a leitura de todos os sensores e preenche a estrutura de dados.
 * @param[out] dados Ponteiro para a estrutura que receberá os dados lidos.
 * @return STATUS_OK em caso de sucesso, código de erro caso contrário.
 */
status_t sensores_ler_todos(dados_sensores_t *dados);

/**
 * @brief Executa o autoteste de todos os sensores.
 * @return STATUS_OK se todos os sensores passaram no autoteste.
 */
status_t sensores_autoteste(void);

#endif /* SENSORS_H */
