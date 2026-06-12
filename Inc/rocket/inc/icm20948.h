/**
 * @file icm20948.h
 * @brief Driver da IMU de 9 eixos ICM-20948 via SPI.
 *
 * Fornece funções para inicialização, leitura de acelerômetro, giroscópio
 * e magnetômetro, configuração de escalas e autoteste.
 *
 * Comunicação: SPI1 (compartilhado com BMP388)
 * Chip Select: PB0
 */

#ifndef ICM20948_H
#define ICM20948_H

#include <stdint.h>
#include "sensors.h"

/* ============================================================================
 * Endereços de Registradores — Banco de Usuário 0
 * ========================================================================= */

#define ICM20948_REG_WHO_AM_I       0x00    /**< Identificação do dispositivo */
#define ICM20948_REG_USER_CTRL      0x03    /**< Controle do usuário */
#define ICM20948_REG_LP_CONFIG      0x05    /**< Configuração de baixo consumo */
#define ICM20948_REG_PWR_MGMT_1     0x06    /**< Gerenciamento de energia 1 */
#define ICM20948_REG_PWR_MGMT_2     0x07    /**< Gerenciamento de energia 2 */
#define ICM20948_REG_INT_PIN_CFG    0x0F    /**< Configuração do pino de interrupção */
#define ICM20948_REG_INT_ENABLE     0x10    /**< Habilitação de interrupções */
#define ICM20948_REG_INT_STATUS     0x19    /**< Status de interrupção */
#define ICM20948_REG_ACCEL_XOUT_H   0x2D    /**< Acelerômetro eixo X — byte alto */
#define ICM20948_REG_ACCEL_XOUT_L   0x2E    /**< Acelerômetro eixo X — byte baixo */
#define ICM20948_REG_ACCEL_YOUT_H   0x2F    /**< Acelerômetro eixo Y — byte alto */
#define ICM20948_REG_ACCEL_YOUT_L   0x30    /**< Acelerômetro eixo Y — byte baixo */
#define ICM20948_REG_ACCEL_ZOUT_H   0x31    /**< Acelerômetro eixo Z — byte alto */
#define ICM20948_REG_ACCEL_ZOUT_L   0x32    /**< Acelerômetro eixo Z — byte baixo */
#define ICM20948_REG_GYRO_XOUT_H    0x33    /**< Giroscópio eixo X — byte alto */
#define ICM20948_REG_GYRO_XOUT_L    0x34    /**< Giroscópio eixo X — byte baixo */
#define ICM20948_REG_GYRO_YOUT_H    0x35    /**< Giroscópio eixo Y — byte alto */
#define ICM20948_REG_GYRO_YOUT_L    0x36    /**< Giroscópio eixo Y — byte baixo */
#define ICM20948_REG_GYRO_ZOUT_H    0x37    /**< Giroscópio eixo Z — byte alto */
#define ICM20948_REG_GYRO_ZOUT_L    0x38    /**< Giroscópio eixo Z — byte baixo */
#define ICM20948_REG_TEMP_OUT_H     0x39    /**< Temperatura — byte alto */
#define ICM20948_REG_TEMP_OUT_L     0x3A    /**< Temperatura — byte baixo */

/* ============================================================================
 * Endereços de Registradores — Banco de Usuário 2 (Configuração)
 * ========================================================================= */

#define ICM20948_REG_GYRO_SMPLRT_DIV    0x00    /**< Divisor de taxa do giroscópio */
#define ICM20948_REG_GYRO_CONFIG_1      0x01    /**< Configuração do giroscópio 1 */
#define ICM20948_REG_GYRO_CONFIG_2      0x02    /**< Configuração do giroscópio 2 */
#define ICM20948_REG_ACCEL_SMPLRT_DIV_1 0x10    /**< Divisor de taxa do acelerômetro (MSB) */
#define ICM20948_REG_ACCEL_SMPLRT_DIV_2 0x11    /**< Divisor de taxa do acelerômetro (LSB) */
#define ICM20948_REG_ACCEL_CONFIG       0x14    /**< Configuração do acelerômetro */
#define ICM20948_REG_ACCEL_CONFIG_2     0x15    /**< Configuração do acelerômetro 2 */

/* ============================================================================
 * Seleção de Banco de Registradores
 * ========================================================================= */

#define ICM20948_REG_BANK_SEL       0x7F    /**< Seleção do banco de registradores */

/** Valores para seleção de banco */
#define ICM20948_BANK_0             0x00    /**< Banco de usuário 0 */
#define ICM20948_BANK_1             0x10    /**< Banco de usuário 1 */
#define ICM20948_BANK_2             0x20    /**< Banco de usuário 2 */
#define ICM20948_BANK_3             0x30    /**< Banco de usuário 3 */

/* ============================================================================
 * Constantes do ICM-20948
 * ========================================================================= */

/** Valor esperado do registrador WHO_AM_I para verificação de identidade */
#define ICM20948_WHO_AM_I_ESPERADO  0xEA

/** Comando de reset do dispositivo (bit DEVICE_RESET no PWR_MGMT_1) */
#define ICM20948_CMD_RESET          0x80

/** Seleciona o melhor clock disponível (auto) */
#define ICM20948_CLKSEL_AUTO        0x01

/* ============================================================================
 * Enumerações de Escala
 * ========================================================================= */

/**
 * @brief Escalas de fundo de escala do acelerômetro.
 */
typedef enum {
    ICM20948_ACCEL_2G  = 0,     /**< ±2 g */
    ICM20948_ACCEL_4G,          /**< ±4 g */
    ICM20948_ACCEL_8G,          /**< ±8 g */
    ICM20948_ACCEL_16G          /**< ±16 g */
} icm20948_escala_accel_t;

/**
 * @brief Escalas de fundo de escala do giroscópio.
 */
typedef enum {
    ICM20948_GYRO_250DPS  = 0,  /**< ±250 °/s */
    ICM20948_GYRO_500DPS,       /**< ±500 °/s */
    ICM20948_GYRO_1000DPS,      /**< ±1000 °/s */
    ICM20948_GYRO_2000DPS       /**< ±2000 °/s */
} icm20948_escala_gyro_t;

/* ============================================================================
 * Estrutura de Dados Brutos
 * ========================================================================= */

/**
 * @brief Dados brutos (não compensados) lidos diretamente dos registradores.
 *
 * Os valores são inteiros de 16 bits com sinal, representando as leituras
 * brutas antes da conversão para unidades de engenharia.
 */
typedef struct {
    int16_t accel_x;    /**< Acelerômetro eixo X (bruto) */
    int16_t accel_y;    /**< Acelerômetro eixo Y (bruto) */
    int16_t accel_z;    /**< Acelerômetro eixo Z (bruto) */
    int16_t gyro_x;     /**< Giroscópio eixo X (bruto) */
    int16_t gyro_y;     /**< Giroscópio eixo Y (bruto) */
    int16_t gyro_z;     /**< Giroscópio eixo Z (bruto) */
    int16_t mag_x;      /**< Magnetômetro eixo X (bruto) */
    int16_t mag_y;      /**< Magnetômetro eixo Y (bruto) */
    int16_t mag_z;      /**< Magnetômetro eixo Z (bruto) */
} icm20948_dados_brutos_t;

/* ============================================================================
 * Funções do Driver ICM-20948
 * ========================================================================= */

/**
 * @brief Inicializa o sensor ICM-20948.
 *
 * Verifica WHO_AM_I, realiza reset, configura clock, escalas padrão
 * e habilita acelerômetro, giroscópio e magnetômetro.
 *
 * @return STATUS_OK em caso de sucesso, código de erro caso contrário.
 */
status_t icm20948_inicializar(void);

/**
 * @brief Lê os dados do acelerômetro convertidos para unidades de g.
 * @param[out] ax Aceleração no eixo X em g.
 * @param[out] ay Aceleração no eixo Y em g.
 * @param[out] az Aceleração no eixo Z em g.
 * @return STATUS_OK em caso de sucesso.
 */
status_t icm20948_ler_acelerometro(float *ax, float *ay, float *az);

/**
 * @brief Lê os dados do giroscópio convertidos para graus por segundo.
 * @param[out] gx Velocidade angular no eixo X em °/s.
 * @param[out] gy Velocidade angular no eixo Y em °/s.
 * @param[out] gz Velocidade angular no eixo Z em °/s.
 * @return STATUS_OK em caso de sucesso.
 */
status_t icm20948_ler_giroscopio(float *gx, float *gy, float *gz);

/**
 * @brief Lê os dados do magnetômetro convertidos para microteslas.
 *
 * O magnetômetro interno (AK09916) é acessado via barramento auxiliar I2C
 * do ICM-20948.
 *
 * @param[out] mx Campo magnético no eixo X em µT.
 * @param[out] my Campo magnético no eixo Y em µT.
 * @param[out] mz Campo magnético no eixo Z em µT.
 * @return STATUS_OK em caso de sucesso.
 */
status_t icm20948_ler_magnetometro(float *mx, float *my, float *mz);

/**
 * @brief Executa o autoteste do sensor ICM-20948.
 *
 * Verifica comunicação SPI, WHO_AM_I e validade das leituras de todos os eixos.
 *
 * @return STATUS_OK se o sensor passou no autoteste.
 */
status_t icm20948_autoteste(void);

/**
 * @brief Configura a escala de fundo de escala do acelerômetro.
 * @param[in] escala Escala desejada (2G, 4G, 8G ou 16G).
 * @return STATUS_OK em caso de sucesso.
 */
status_t icm20948_configurar_escala_accel(icm20948_escala_accel_t escala);

/**
 * @brief Configura a escala de fundo de escala do giroscópio.
 * @param[in] escala Escala desejada (250, 500, 1000 ou 2000 °/s).
 * @return STATUS_OK em caso de sucesso.
 */
status_t icm20948_configurar_escala_gyro(icm20948_escala_gyro_t escala);

#endif /* ICM20948_H */
