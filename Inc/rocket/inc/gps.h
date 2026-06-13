/**
 * @file gps.h
 * @brief Interface do driver do módulo GPS (NEO-M8N / NEO-M9N) via USART2.
 *
 * Gerencia a inicialização da USART2, recebimento dos dados de NMEA
 * e extração das coordenadas geográficas, altitude e qualidade do sinal.
 */

#ifndef GPS_H
#define GPS_H

#include "sensors.h"
#include "config.h"

/** Tempo máximo sem uma sentença GPS válida antes de invalidar o fix. */
#define GPS_FIX_TIMEOUT_MS 3000U

/* ============================================================================
 * Funções da Interface do GPS
 * ========================================================================= */

/**
 * @brief Inicializa o módulo GPS.
 *
 * Configura a porta USART2 e prepara a recepção por DMA ou interrupção.
 *
 * @return STATUS_OK em caso de sucesso, código de erro caso contrário.
 */
status_t gps_inicializar(void);

/**
 * @brief Processa os bytes recebidos da USART do GPS.
 *
 * Deve ser chamado periodicamente no loop principal para realizar o parse
 * das sentenças NMEA acumuladas no buffer de recepção.
 *
 * @return STATUS_OK em caso de sucesso.
 */
status_t gps_processar(void);

/**
 * @brief Valida e processa uma sentença NMEA completa.
 *
 * A sentença deve iniciar com '$' e incluir checksum após '*'. Esta função
 * também é usada internamente pelo processamento do buffer da USART.
 *
 * @param[in] sentenca Sentença NMEA terminada em nulo.
 * @return STATUS_OK quando a sentença suportada é válida.
 */
status_t gps_processar_sentenca_nmea(const char *sentenca);

/**
 * @brief Obtém as últimas leituras de GPS válidas.
 *
 * @param[out] latitude     Ponteiro para receber a latitude (graus decimais).
 * @param[out] longitude    Ponteiro para receber a longitude (graus decimais).
 * @param[out] altitude     Ponteiro para receber a altitude em metros.
 * @param[out] satelites    Ponteiro para receber a quantidade de satélites.
 * @param[out] fix_valido   Ponteiro para receber a validade da leitura.
 *
 * @return STATUS_OK se a leitura foi retornada com sucesso.
 */
status_t gps_obter_dados(float *latitude, float *longitude, float *altitude, uint8_t *satelites, bool *fix_valido);

#endif /* GPS_H */
