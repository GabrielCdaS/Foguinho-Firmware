/**
 * @file sx1278.h
 * @brief Driver do rádio LoRa SX1278 via SPI.
 *
 * Fornece funções para inicialização, transmissão, recepção, configuração
 * de frequência, potência e parâmetros LoRa (SF, BW, CR).
 *
 * Comunicação: SPI2
 * Chip Select: PB12 | Reset: PB1 | DIO0: PB2
 */

#ifndef SX1278_H
#define SX1278_H

#include <stdint.h>
#include "sensors.h"

/* ============================================================================
 * Endereços de Registradores do SX1278
 * ========================================================================= */

#define SX1278_REG_FIFO             0x00    /**< FIFO de dados */
#define SX1278_REG_OP_MODE          0x01    /**< Modo de operação */
#define SX1278_REG_FRF_MSB          0x06    /**< Frequência portadora — byte alto */
#define SX1278_REG_FRF_MID          0x07    /**< Frequência portadora — byte médio */
#define SX1278_REG_FRF_LSB          0x08    /**< Frequência portadora — byte baixo */
#define SX1278_REG_PA_CONFIG        0x09    /**< Configuração do amplificador de potência */
#define SX1278_REG_PA_RAMP          0x0A    /**< Rampa do PA */
#define SX1278_REG_OCP              0x0B    /**< Proteção contra sobrecorrente */
#define SX1278_REG_LNA              0x0C    /**< Configuração do LNA */
#define SX1278_REG_FIFO_ADDR_PTR    0x0D    /**< Ponteiro de endereço da FIFO */
#define SX1278_REG_FIFO_TX_BASE     0x0E    /**< Endereço base TX da FIFO */
#define SX1278_REG_FIFO_RX_BASE     0x0F    /**< Endereço base RX da FIFO */
#define SX1278_REG_FIFO_RX_CURRENT  0x10    /**< Endereço do último pacote recebido */
#define SX1278_REG_IRQ_FLAGS_MASK   0x11    /**< Máscara de flags de interrupção */
#define SX1278_REG_IRQ_FLAGS        0x12    /**< Flags de interrupção */
#define SX1278_REG_RX_NB_BYTES      0x13    /**< Número de bytes recebidos */
#define SX1278_REG_RX_HEADER_CNT_MSB 0x14   /**< Contador de cabeçalhos recebidos (MSB) */
#define SX1278_REG_RX_HEADER_CNT_LSB 0x15   /**< Contador de cabeçalhos recebidos (LSB) */
#define SX1278_REG_RX_PKT_CNT_MSB  0x16    /**< Contador de pacotes recebidos (MSB) */
#define SX1278_REG_RX_PKT_CNT_LSB  0x17    /**< Contador de pacotes recebidos (LSB) */
#define SX1278_REG_PKT_RSSI         0x1A    /**< RSSI do último pacote recebido */
#define SX1278_REG_PKT_SNR          0x1B    /**< SNR do último pacote recebido */
#define SX1278_REG_MODEM_CONFIG_1   0x1D    /**< Configuração do modem 1 (BW, CR) */
#define SX1278_REG_MODEM_CONFIG_2   0x1E    /**< Configuração do modem 2 (SF, CRC) */
#define SX1278_REG_SYMB_TIMEOUT_LSB 0x1F    /**< Timeout de símbolo (LSB) */
#define SX1278_REG_PREAMBLE_MSB     0x20    /**< Comprimento do preâmbulo (MSB) */
#define SX1278_REG_PREAMBLE_LSB     0x21    /**< Comprimento do preâmbulo (LSB) */
#define SX1278_REG_PAYLOAD_LENGTH   0x22    /**< Comprimento do payload */
#define SX1278_REG_MAX_PAYLOAD_LEN  0x23    /**< Comprimento máximo do payload */
#define SX1278_REG_MODEM_CONFIG_3   0x26    /**< Configuração do modem 3 (AGC, otimização) */
#define SX1278_REG_FREQ_ERROR_MSB   0x28    /**< Erro de frequência (MSB) */
#define SX1278_REG_RSSI_WIDEBAND    0x2C    /**< RSSI em banda larga */
#define SX1278_REG_DETECT_OPTIMIZE  0x31    /**< Otimização de detecção */
#define SX1278_REG_INVERT_IQ        0x33    /**< Inversão de I/Q */
#define SX1278_REG_DETECT_THRESHOLD 0x37    /**< Limiar de detecção */
#define SX1278_REG_SYNC_WORD        0x39    /**< Palavra de sincronização */
#define SX1278_REG_DIO_MAPPING_1    0x40    /**< Mapeamento DIO0-DIO3 */
#define SX1278_REG_DIO_MAPPING_2    0x41    /**< Mapeamento DIO4-DIO5 */
#define SX1278_REG_VERSION          0x42    /**< Versão do chip */

/* ============================================================================
 * Configuração Padrão de Frequência
 * ========================================================================= */

#define SX1278_FREQUENCIA_915MHZ    915000000UL /**< Frequência ISM 915 MHz (Américas) */
#define SX1278_FREQUENCIA_433MHZ    433000000UL /**< Frequência ISM 433 MHz (Europa/Ásia) */
#define SX1278_FREQUENCIA_PADRAO    SX1278_FREQUENCIA_915MHZ /**< Frequência padrão utilizada */

/* ============================================================================
 * Configuração Padrão LoRa
 * ========================================================================= */

#define SX1278_POTENCIA_PADRAO_DBM  17      /**< Potência de transmissão padrão em dBm */
#define SX1278_TAMANHO_MAX_PACOTE   255     /**< Tamanho máximo de pacote em bytes */

/* ============================================================================
 * Enumerações de Parâmetros LoRa
 * ========================================================================= */

/**
 * @brief Fator de espalhamento (Spreading Factor).
 *
 * Valores maiores aumentam o alcance mas reduzem a taxa de dados.
 */
typedef enum {
    SX1278_SF7  = 7,    /**< SF7 — maior taxa, menor alcance */
    SX1278_SF8  = 8,    /**< SF8 */
    SX1278_SF9  = 9,    /**< SF9 */
    SX1278_SF10 = 10,   /**< SF10 */
    SX1278_SF11 = 11,   /**< SF11 */
    SX1278_SF12 = 12    /**< SF12 — menor taxa, maior alcance */
} sx1278_sf_t;

/**
 * @brief Largura de banda do sinal LoRa.
 *
 * Valores maiores aumentam a taxa de dados mas reduzem a sensibilidade.
 */
typedef enum {
    SX1278_BW_7K8   = 0,   /**< 7,8 kHz */
    SX1278_BW_10K4  = 1,   /**< 10,4 kHz */
    SX1278_BW_15K6  = 2,   /**< 15,6 kHz */
    SX1278_BW_20K8  = 3,   /**< 20,8 kHz */
    SX1278_BW_31K25 = 4,   /**< 31,25 kHz */
    SX1278_BW_41K7  = 5,   /**< 41,7 kHz */
    SX1278_BW_62K5  = 6,   /**< 62,5 kHz */
    SX1278_BW_125K  = 7,   /**< 125 kHz */
    SX1278_BW_250K  = 8,   /**< 250 kHz */
    SX1278_BW_500K  = 9    /**< 500 kHz */
} sx1278_bw_t;

/**
 * @brief Taxa de codificação (Coding Rate) para correção de erros FEC.
 *
 * Valores maiores fornecem maior proteção contra erros com overhead maior.
 */
typedef enum {
    SX1278_CR_4_5 = 1,     /**< 4/5 — menor overhead */
    SX1278_CR_4_6 = 2,     /**< 4/6 */
    SX1278_CR_4_7 = 3,     /**< 4/7 */
    SX1278_CR_4_8 = 4      /**< 4/8 — maior proteção */
} sx1278_cr_t;

/* ============================================================================
 * Valores Padrão de Configuração LoRa
 * ========================================================================= */

#define SX1278_SF_PADRAO    SX1278_SF7      /**< Fator de espalhamento padrão */
#define SX1278_BW_PADRAO    SX1278_BW_125K  /**< Largura de banda padrão */
#define SX1278_CR_PADRAO    SX1278_CR_4_5   /**< Taxa de codificação padrão */

/* ============================================================================
 * Funções do Driver SX1278
 * ========================================================================= */

/**
 * @brief Inicializa o rádio SX1278 no modo LoRa.
 *
 * Realiza reset por hardware, verifica versão do chip, configura frequência,
 * potência e parâmetros LoRa padrão.
 *
 * @return STATUS_OK em caso de sucesso, STATUS_ERRO_RADIO caso contrário.
 */
status_t sx1278_inicializar(void);

/**
 * @brief Transmite dados via LoRa.
 * @param[in] dados  Ponteiro para o buffer de dados a transmitir.
 * @param[in] tamanho Número de bytes a transmitir (máx. SX1278_TAMANHO_MAX_PACOTE).
 * @return STATUS_OK em caso de sucesso.
 */
status_t sx1278_transmitir(const uint8_t *dados, uint8_t tamanho);

/**
 * @brief Recebe dados via LoRa com timeout.
 * @param[out] buffer     Buffer para armazenar os dados recebidos.
 * @param[out] tamanho    Ponteiro para receber o número de bytes recebidos.
 * @param[in]  timeout_ms Timeout máximo de espera em milissegundos.
 * @return STATUS_OK se dados foram recebidos, STATUS_ERRO_TIMEOUT se expirou.
 */
status_t sx1278_receber(uint8_t *buffer, uint8_t *tamanho, uint32_t timeout_ms);

/**
 * @brief Configura a frequência de operação do rádio.
 * @param[in] frequencia_hz Frequência em Hz (ex: 915000000 para 915 MHz).
 * @return STATUS_OK em caso de sucesso.
 */
status_t sx1278_configurar_frequencia(uint32_t frequencia_hz);

/**
 * @brief Configura a potência de transmissão.
 * @param[in] potencia_dbm Potência em dBm (tipicamente 2 a 20 dBm).
 * @return STATUS_OK em caso de sucesso.
 */
status_t sx1278_configurar_potencia(int8_t potencia_dbm);

/**
 * @brief Configura os parâmetros de modulação LoRa.
 * @param[in] sf Fator de espalhamento (SF7 a SF12).
 * @param[in] bw Largura de banda.
 * @param[in] cr Taxa de codificação.
 * @return STATUS_OK em caso de sucesso.
 */
status_t sx1278_configurar_lora(sx1278_sf_t sf, sx1278_bw_t bw, sx1278_cr_t cr);

/**
 * @brief Lê o RSSI do último pacote recebido.
 * @return Valor do RSSI em dBm.
 */
int16_t sx1278_ler_rssi(void);

/**
 * @brief Coloca o rádio em modo sleep (baixo consumo).
 * @return STATUS_OK em caso de sucesso.
 */
status_t sx1278_modo_sleep(void);

/**
 * @brief Coloca o rádio em modo standby.
 * @return STATUS_OK em caso de sucesso.
 */
status_t sx1278_modo_standby(void);

#endif /* SX1278_H */
