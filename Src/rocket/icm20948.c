/**
 * @file icm20948.c
 * @brief Implementação do driver da IMU ICM-20948.
 */

#include "icm20948.h"
#include "hal_plataforma.h"
#include "pinout.h"
#include "stm32f411_hw.h"

/* ============================================================================
 * Variáveis Internas
 * ========================================================================= */
static icm20948_escala_accel_t escala_accel_atual = ICM20948_ACCEL_16G;
static icm20948_escala_gyro_t escala_gyro_atual = ICM20948_GYRO_2000DPS;
static bool inicializado = false;
static float ultimo_mx = 0.0f;
static float ultimo_my = 0.0f;
static float ultimo_mz = 0.0f;

#define ICM_REG_EXT_SLV_SENS_DATA_00 0x3BU
#define ICM_REG_I2C_MST_CTRL          0x01U
#define ICM_REG_I2C_SLV0_ADDR         0x03U
#define ICM_REG_I2C_SLV0_REG          0x04U
#define ICM_REG_I2C_SLV0_CTRL         0x05U
#define ICM_REG_I2C_SLV4_ADDR         0x13U
#define ICM_REG_I2C_SLV4_REG          0x14U
#define ICM_REG_I2C_SLV4_CTRL         0x15U
#define ICM_REG_I2C_SLV4_DO           0x16U
#define AK09916_ADDR                  0x0CU
#define AK09916_REG_ST1               0x10U
#define AK09916_REG_CNTL2             0x31U
#define AK09916_REG_CNTL3             0x32U

static status_t icm_escrever(uint8_t reg, uint8_t valor) {
    uint8_t dados[2] = {(uint8_t)(reg & 0x7FU), valor};
    hw_gpio_write(PINO_ICM20948_CS_PORTA, PINO_ICM20948_CS_PINO, false);
    status_t status = hw_spi_transfer(HW_SPI1, dados, NULL, sizeof(dados));
    hw_gpio_write(PINO_ICM20948_CS_PORTA, PINO_ICM20948_CS_PINO, true);
    return status;
}

static status_t icm_ler(uint8_t reg, uint8_t *dados, uint8_t tamanho) {
    uint8_t comando = (uint8_t)(reg | 0x80U);
    if (!dados || tamanho == 0U) return STATUS_ERRO_GENERICO;
    hw_gpio_write(PINO_ICM20948_CS_PORTA, PINO_ICM20948_CS_PINO, false);
    status_t status = hw_spi_transfer(HW_SPI1, &comando, NULL, 1);
    if (status == STATUS_OK) status = hw_spi_transfer(HW_SPI1, NULL, dados, tamanho);
    hw_gpio_write(PINO_ICM20948_CS_PORTA, PINO_ICM20948_CS_PINO, true);
    return status;
}

static status_t icm_banco(uint8_t banco) {
    return icm_escrever(ICM20948_REG_BANK_SEL, banco);
}

static int16_t s16_be(const uint8_t *p) {
    return (int16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static int16_t s16_le_local(const uint8_t *p) {
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static status_t ak09916_escrever(uint8_t reg, uint8_t valor) {
    if (icm_banco(ICM20948_BANK_3) != STATUS_OK ||
        icm_escrever(ICM_REG_I2C_SLV4_ADDR, AK09916_ADDR) != STATUS_OK ||
        icm_escrever(ICM_REG_I2C_SLV4_REG, reg) != STATUS_OK ||
        icm_escrever(ICM_REG_I2C_SLV4_DO, valor) != STATUS_OK ||
        icm_escrever(ICM_REG_I2C_SLV4_CTRL, 0x80U) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    plataforma_delay_ms(10);
    return STATUS_OK;
}

static status_t ak09916_inicializar(void) {
    if (icm_banco(ICM20948_BANK_0) != STATUS_OK ||
        icm_escrever(ICM20948_REG_USER_CTRL, 0x20U) != STATUS_OK ||
        icm_banco(ICM20948_BANK_3) != STATUS_OK ||
        icm_escrever(ICM_REG_I2C_MST_CTRL, 0x07U) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    if (ak09916_escrever(AK09916_REG_CNTL3, 0x01U) != STATUS_OK ||
        ak09916_escrever(AK09916_REG_CNTL2, 0x08U) != STATUS_OK) {
        return STATUS_ERRO_SENSOR;
    }
    if (icm_banco(ICM20948_BANK_3) != STATUS_OK ||
        icm_escrever(ICM_REG_I2C_SLV0_ADDR, AK09916_ADDR | 0x80U) != STATUS_OK ||
        icm_escrever(ICM_REG_I2C_SLV0_REG, AK09916_REG_ST1) != STATUS_OK ||
        icm_escrever(ICM_REG_I2C_SLV0_CTRL, 0x80U | 9U) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    return icm_banco(ICM20948_BANK_0);
}

/* ============================================================================
 * Funções do Módulo
 * ========================================================================= */

status_t icm20948_inicializar(void) {
    uint8_t id;
    hw_gpio_output(PINO_ICM20948_CS_PORTA, PINO_ICM20948_CS_PINO, true);
    status_t status = hw_spi_init(HW_SPI1, 3);
    if (status != STATUS_OK) return status;
    if (icm_banco(ICM20948_BANK_0) != STATUS_OK ||
        icm_ler(ICM20948_REG_WHO_AM_I, &id, 1) != STATUS_OK ||
        id != ICM20948_WHO_AM_I_ESPERADO) {
        return STATUS_ERRO_SENSOR;
    }

    if (icm_escrever(ICM20948_REG_PWR_MGMT_1, ICM20948_CMD_RESET) != STATUS_OK) return STATUS_ERRO_SPI;
    plataforma_delay_ms(100);
    if (icm_escrever(ICM20948_REG_PWR_MGMT_1, ICM20948_CLKSEL_AUTO) != STATUS_OK ||
        icm_escrever(ICM20948_REG_PWR_MGMT_2, 0x00U) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    plataforma_delay_ms(10);
    if (icm20948_configurar_escala_accel(ICM20948_ACCEL_16G) != STATUS_OK ||
        icm20948_configurar_escala_gyro(ICM20948_GYRO_2000DPS) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    status = ak09916_inicializar();
    if (status != STATUS_OK) return status;
    inicializado = true;
    return STATUS_OK;
}

status_t icm20948_ler_acelerometro(float *ax, float *ay, float *az) {
    uint8_t dados[6];
    static const float sensibilidade[] = {16384.0f, 8192.0f, 4096.0f, 2048.0f};
    if (!inicializado || !ax || !ay || !az) return STATUS_ERRO_GENERICO;
    if (icm_banco(ICM20948_BANK_0) != STATUS_OK ||
        icm_ler(ICM20948_REG_ACCEL_XOUT_H, dados, sizeof(dados)) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    *ax = (float)s16_be(&dados[0]) / sensibilidade[escala_accel_atual];
    *ay = (float)s16_be(&dados[2]) / sensibilidade[escala_accel_atual];
    *az = (float)s16_be(&dados[4]) / sensibilidade[escala_accel_atual];
    return STATUS_OK;
}

status_t icm20948_ler_giroscopio(float *gx, float *gy, float *gz) {
    uint8_t dados[6];
    static const float sensibilidade[] = {131.0f, 65.5f, 32.8f, 16.4f};
    if (!inicializado || !gx || !gy || !gz) return STATUS_ERRO_GENERICO;
    if (icm_banco(ICM20948_BANK_0) != STATUS_OK ||
        icm_ler(ICM20948_REG_GYRO_XOUT_H, dados, sizeof(dados)) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    *gx = (float)s16_be(&dados[0]) / sensibilidade[escala_gyro_atual];
    *gy = (float)s16_be(&dados[2]) / sensibilidade[escala_gyro_atual];
    *gz = (float)s16_be(&dados[4]) / sensibilidade[escala_gyro_atual];
    return STATUS_OK;
}

status_t icm20948_ler_magnetometro(float *mx, float *my, float *mz) {
    uint8_t dados[9];
    if (!inicializado || !mx || !my || !mz) return STATUS_ERRO_GENERICO;
    if (icm_banco(ICM20948_BANK_0) != STATUS_OK ||
        icm_ler(ICM_REG_EXT_SLV_SENS_DATA_00, dados, sizeof(dados)) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    if ((dados[0] & 0x01U) == 0U) {
        *mx = ultimo_mx;
        *my = ultimo_my;
        *mz = ultimo_mz;
        return STATUS_OK;
    }
    if ((dados[8] & 0x08U) != 0U) return STATUS_ERRO_SENSOR;
    ultimo_mx = (float)s16_le_local(&dados[1]) * 0.15f;
    ultimo_my = (float)s16_le_local(&dados[3]) * 0.15f;
    ultimo_mz = (float)s16_le_local(&dados[5]) * 0.15f;
    *mx = ultimo_mx;
    *my = ultimo_my;
    *mz = ultimo_mz;
    return STATUS_OK;
}

status_t icm20948_autoteste(void) {
    uint8_t id;
    if (icm_banco(ICM20948_BANK_0) != STATUS_OK ||
        icm_ler(ICM20948_REG_WHO_AM_I, &id, 1) != STATUS_OK) return STATUS_ERRO_SPI;
    return id == ICM20948_WHO_AM_I_ESPERADO ? STATUS_OK : STATUS_ERRO_SENSOR;
}

status_t icm20948_configurar_escala_accel(icm20948_escala_accel_t escala) {
    if (escala > ICM20948_ACCEL_16G) return STATUS_ERRO_GENERICO;
    if (icm_banco(ICM20948_BANK_2) != STATUS_OK ||
        icm_escrever(ICM20948_REG_ACCEL_CONFIG, (uint8_t)((escala << 1) | 0x01U)) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    escala_accel_atual = escala;
    return icm_banco(ICM20948_BANK_0);
}

status_t icm20948_configurar_escala_gyro(icm20948_escala_gyro_t escala) {
    if (escala > ICM20948_GYRO_2000DPS) return STATUS_ERRO_GENERICO;
    if (icm_banco(ICM20948_BANK_2) != STATUS_OK ||
        icm_escrever(ICM20948_REG_GYRO_CONFIG_1, (uint8_t)((escala << 1) | 0x01U)) != STATUS_OK) {
        return STATUS_ERRO_SPI;
    }
    escala_gyro_atual = escala;
    return icm_banco(ICM20948_BANK_0);
}
