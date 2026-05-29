/**
 * @file bmp388.h
 * @brief Driver do sensor barométrico BMP388 via SPI.
 *
 * Fornece funções para inicialização, leitura de pressão, temperatura,
 * cálculo de altitude e calibração de referência ao nível do solo.
 *
 * Comunicação: SPI1 (compartilhado com ICM-20948)
 * Chip Select: PA4
 */

#ifndef BMP388_H
#define BMP388_H

#include <stdint.h>
#include "sensors.h"

/* ============================================================================
 * Endereços de Registradores do BMP388
 * ========================================================================= */

#define BMP388_REG_CHIP_ID      0x00    /**< Identificação do chip */
#define BMP388_REG_ERR_REG      0x02    /**< Registro de erros */
#define BMP388_REG_STATUS       0x03    /**< Status do sensor */
#define BMP388_REG_PRESS_DATA   0x04    /**< Dados de pressão (3 bytes: 0x04-0x06) */
#define BMP388_REG_TEMP_DATA    0x07    /**< Dados de temperatura (3 bytes: 0x07-0x09) */
#define BMP388_REG_EVENT        0x10    /**< Registro de eventos */
#define BMP388_REG_INT_STATUS   0x11    /**< Status de interrupção */
#define BMP388_REG_INT_CTRL     0x19    /**< Controle de interrupção */
#define BMP388_REG_IF_CONF      0x1A    /**< Configuração da interface */
#define BMP388_REG_PWR_CTRL     0x1B    /**< Controle de energia (habilita pressão/temperatura) */
#define BMP388_REG_OSR          0x1C    /**< Oversampling (resolução de pressão/temperatura) */
#define BMP388_REG_ODR          0x1D    /**< Taxa de saída de dados */
#define BMP388_REG_CONFIG       0x1F    /**< Configuração do filtro IIR */
#define BMP388_REG_CALIB_DATA   0x31    /**< Início dos dados de calibração (21 bytes) */
#define BMP388_REG_CMD          0x7E    /**< Registro de comandos */

/* ============================================================================
 * Constantes do BMP388
 * ========================================================================= */

/** Valor esperado do registrador CHIP_ID para verificação de identidade */
#define BMP388_CHIP_ID_ESPERADO 0x50

/** Comando de soft reset */
#define BMP388_CMD_SOFT_RESET   0xB6

/** Pressão padrão ao nível do mar em Pascals (referência ISA) */
#define BMP388_PRESSAO_NIVEL_MAR_PA     101325.0f

/* ============================================================================
 * Estrutura de Dados de Calibração
 * ========================================================================= */

/**
 * @brief Coeficientes de calibração do BMP388.
 *
 * Estes coeficientes são lidos dos registradores internos do sensor e
 * utilizados para compensar as leituras brutas de pressão e temperatura.
 */
typedef struct {
    /* Coeficientes de temperatura */
    float par_t1;   /**< Coeficiente de calibração T1 */
    float par_t2;   /**< Coeficiente de calibração T2 */
    float par_t3;   /**< Coeficiente de calibração T3 */

    /* Coeficientes de pressão */
    float par_p1;   /**< Coeficiente de calibração P1 */
    float par_p2;   /**< Coeficiente de calibração P2 */
    float par_p3;   /**< Coeficiente de calibração P3 */
    float par_p4;   /**< Coeficiente de calibração P4 */
    float par_p5;   /**< Coeficiente de calibração P5 */
    float par_p6;   /**< Coeficiente de calibração P6 */
    float par_p7;   /**< Coeficiente de calibração P7 */
    float par_p8;   /**< Coeficiente de calibração P8 */
    float par_p9;   /**< Coeficiente de calibração P9 */
    float par_p10;  /**< Coeficiente de calibração P10 */
    float par_p11;  /**< Coeficiente de calibração P11 */
} bmp388_calib_t;

/* ============================================================================
 * Funções do Driver BMP388
 * ========================================================================= */

/**
 * @brief Inicializa o sensor BMP388.
 *
 * Verifica o CHIP_ID, realiza soft reset, carrega coeficientes de calibração
 * e configura oversampling e filtro IIR.
 *
 * @return STATUS_OK em caso de sucesso, código de erro caso contrário.
 */
status_t bmp388_inicializar(void);

/**
 * @brief Lê a pressão atmosférica compensada.
 * @param[out] pressao_pa Ponteiro para receber a pressão em Pascals.
 * @return STATUS_OK em caso de sucesso.
 */
status_t bmp388_ler_pressao(float *pressao_pa);

/**
 * @brief Lê a temperatura compensada.
 * @param[out] temperatura_c Ponteiro para receber a temperatura em °C.
 * @return STATUS_OK em caso de sucesso.
 */
status_t bmp388_ler_temperatura(float *temperatura_c);

/**
 * @brief Calcula a altitude a partir da pressão atmosférica.
 *
 * Utiliza a fórmula barométrica com a pressão de referência ao nível do solo
 * (definida na calibração).
 *
 * @param[in]  pressao_pa Pressão atual em Pascals.
 * @param[out] altitude_m Ponteiro para receber a altitude em metros.
 * @return STATUS_OK em caso de sucesso.
 */
status_t bmp388_calcular_altitude(float pressao_pa, float *altitude_m);

/**
 * @brief Calibra a referência de altitude zero (nível do solo).
 *
 * Realiza múltiplas leituras de pressão ao nível do solo e calcula a média
 * para usar como referência no cálculo de altitude.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t bmp388_calibrar_zero(void);

/**
 * @brief Executa o autoteste do sensor BMP388.
 *
 * Verifica comunicação SPI, CHIP_ID e validade das leituras.
 *
 * @return STATUS_OK se o sensor passou no autoteste.
 */
status_t bmp388_autoteste(void);

#endif /* BMP388_H */
