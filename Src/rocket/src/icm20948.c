/**
 * @file icm20948.c
 * @brief Implementação do driver da IMU ICM-20948.
 */

#include "icm20948.h"

/* ============================================================================
 * Variáveis Internas
 * ========================================================================= */
static icm20948_escala_accel_t escala_accel_atual = ICM20948_ACCEL_16G;
static icm20948_escala_gyro_t escala_gyro_atual = ICM20948_GYRO_2000DPS;

/* ============================================================================
 * Funções do Módulo
 * ========================================================================= */

status_t icm20948_inicializar(void) {
    /* TODO: Configurar SPI1, PB0 (CS) */

    /* TODO: Realizar reset */
    /* TODO: Configurar clock auto e banco de usuário 0 */
    /* TODO: Configurar escalas */

    return STATUS_OK;
}

status_t icm20948_ler_acelerometro(float *ax, float *ay, float *az) {
    /* TODO: Ler regs ACCEL_XOUT_H até ACCEL_ZOUT_L via SPI */
    /* Simulação: */
    if (ax) *ax = 0.0f;
    if (ay) *ay = 0.0f;
    if (az) *az = 1.0f; /* 1g no eixo Z em repouso */
    return STATUS_OK;
}

status_t icm20948_ler_giroscopio(float *gx, float *gy, float *gz) {
    /* TODO: Ler regs GYRO_XOUT_H até GYRO_ZOUT_L via SPI */
    /* Simulação: */
    if (gx) *gx = 0.0f;
    if (gy) *gy = 0.0f;
    if (gz) *gz = 0.0f;
    return STATUS_OK;
}

status_t icm20948_ler_magnetometro(float *mx, float *my, float *mz) {
    /* TODO: Acessar AK09916 via I2C passthrough ou master do ICM */
    /* Simulação: */
    if (mx) *mx = 30.0f;
    if (my) *my = 0.0f;
    if (mz) *mz = 40.0f;
    return STATUS_OK;
}

status_t icm20948_autoteste(void) {
    /* TODO: Ler WHO_AM_I e validar com ICM20948_WHO_AM_I_ESPERADO */
    return STATUS_OK;
}

status_t icm20948_configurar_escala_accel(icm20948_escala_accel_t escala) {
    escala_accel_atual = escala;
    /* TODO: Escrever no registrador ACCEL_CONFIG no banco 2 */
    return STATUS_OK;
}

status_t icm20948_configurar_escala_gyro(icm20948_escala_gyro_t escala) {
    escala_gyro_atual = escala;
    /* TODO: Escrever no registrador GYRO_CONFIG_1 no banco 2 */
    return STATUS_OK;
}
