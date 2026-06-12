/**
 * @file sensors.c
 * @brief Implementação da interface unificada de sensores.
 */

#include "sensors.h"
#include "bmp388.h"
#include "icm20948.h"
#include "battery.h"
#include "gps.h"
#include "hal_plataforma.h"

status_t sensores_inicializar(void) {
    status_t res;

    res = bmp388_inicializar();
    if (res != STATUS_OK) return res;

    res = icm20948_inicializar();
    if (res != STATUS_OK) return res;

    return STATUS_OK;
}

status_t sensores_ler_todos(dados_sensores_t *dados) {
    if (!dados) return STATUS_ERRO_GENERICO;

    dados->timestamp_ms = plataforma_obter_tick_ms();

    bmp388_ler_pressao(&dados->pressao_pa);
    bmp388_ler_temperatura(&dados->temperatura_c);
    bmp388_calcular_altitude(dados->pressao_pa, &dados->altitude_m);

    icm20948_ler_acelerometro(&dados->aceleracao_x_g, &dados->aceleracao_y_g, &dados->aceleracao_z_g);
    icm20948_ler_giroscopio(&dados->giroscopio_x_dps, &dados->giroscopio_y_dps, &dados->giroscopio_z_dps);
    icm20948_ler_magnetometro(&dados->magnetometro_x_ut, &dados->magnetometro_y_ut, &dados->magnetometro_z_ut);

    dados->tensao_bateria_mv = bateria_ler_tensao_mv();

    gps_obter_dados(&dados->latitude, &dados->longitude, &dados->gps_altitude_m, &dados->gps_satellites, &dados->gps_fix_valid);

    return STATUS_OK;
}

status_t sensores_autoteste(void) {
    status_t res;

    res = bmp388_autoteste();
    if (res != STATUS_OK) return res;

    res = icm20948_autoteste();
    if (res != STATUS_OK) return res;

    return STATUS_OK;
}
