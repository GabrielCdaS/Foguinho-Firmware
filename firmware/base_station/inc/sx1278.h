/**
 * @file sx1278.h
 * @brief Driver do módulo de rádio LoRa SX1278 para a estação base.
 *
 * Interface de controle do transceptor SX1278 via SPI, configurado
 * primariamente para modo de recepção contínua. A estação base
 * permanece escutando pacotes de telemetria do foguete e transmite
 * comandos quando solicitado pelo PC.
 *
 * Comunicação: SPI (MOSI, MISO, SCK, NSS)
 * Pinos adicionais: DIO0 (interrupção de pacote recebido), RESET
 */

#ifndef SX1278_H
#define SX1278_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================== */
/*                    Inclusões de Dependências                   */
/* ============================================================== */

#include <stdint.h>
#include <stdbool.h>

/* Tipo status_t definido em protocol.h */
#include "protocol.h"

/* ============================================================== */
/*                  Registradores do SX1278                       */
/* ============================================================== */

/** @name Registradores principais do SX1278
 *  Endereços dos registradores mais utilizados no modo LoRa.
 *  @{
 */
#define SX1278_REG_FIFO                 0x00  /**< FIFO de dados TX/RX */
#define SX1278_REG_OP_MODE              0x01  /**< Modo de operação */
#define SX1278_REG_FRF_MSB              0x06  /**< Frequência MSB */
#define SX1278_REG_FRF_MID              0x07  /**< Frequência MID */
#define SX1278_REG_FRF_LSB              0x08  /**< Frequência LSB */
#define SX1278_REG_PA_CONFIG            0x09  /**< Configuração do amplificador de potência */
#define SX1278_REG_PA_RAMP              0x0A  /**< Rampa do amplificador */
#define SX1278_REG_OCP                  0x0B  /**< Proteção contra sobrecorrente */
#define SX1278_REG_LNA                  0x0C  /**< Configuração do LNA */
#define SX1278_REG_FIFO_ADDR_PTR       0x0D  /**< Ponteiro do endereço FIFO */
#define SX1278_REG_FIFO_TX_BASE_ADDR   0x0E  /**< Endereço base TX no FIFO */
#define SX1278_REG_FIFO_RX_BASE_ADDR   0x0F  /**< Endereço base RX no FIFO */
#define SX1278_REG_FIFO_RX_CURRENT     0x10  /**< Endereço atual RX no FIFO */
#define SX1278_REG_IRQ_FLAGS_MASK      0x11  /**< Máscara de flags de interrupção */
#define SX1278_REG_IRQ_FLAGS           0x12  /**< Flags de interrupção */
#define SX1278_REG_RX_NB_BYTES         0x13  /**< Número de bytes recebidos */
#define SX1278_REG_PKT_SNR_VALUE       0x19  /**< SNR do último pacote recebido */
#define SX1278_REG_PKT_RSSI_VALUE      0x1A  /**< RSSI do último pacote recebido */
#define SX1278_REG_RSSI_VALUE          0x1B  /**< Valor atual do RSSI */
#define SX1278_REG_MODEM_CONFIG_1      0x1D  /**< Configuração do modem 1 (BW, CR) */
#define SX1278_REG_MODEM_CONFIG_2      0x1E  /**< Configuração do modem 2 (SF, CRC) */
#define SX1278_REG_SYMB_TIMEOUT_LSB    0x1F  /**< Timeout de símbolo (LSB) */
#define SX1278_REG_PREAMBLE_MSB        0x20  /**< Comprimento do preâmbulo (MSB) */
#define SX1278_REG_PREAMBLE_LSB        0x21  /**< Comprimento do preâmbulo (LSB) */
#define SX1278_REG_PAYLOAD_LENGTH      0x22  /**< Comprimento do payload */
#define SX1278_REG_MAX_PAYLOAD_LENGTH  0x23  /**< Comprimento máximo do payload */
#define SX1278_REG_MODEM_CONFIG_3      0x26  /**< Configuração do modem 3 (AGC, LDO) */
#define SX1278_REG_DETECTION_OPTIMIZE  0x31  /**< Otimização de detecção */
#define SX1278_REG_DETECTION_THRESHOLD 0x37  /**< Limiar de detecção */
#define SX1278_REG_DIO_MAPPING_1       0x40  /**< Mapeamento DIO 0-3 */
#define SX1278_REG_DIO_MAPPING_2       0x41  /**< Mapeamento DIO 4-5 */
#define SX1278_REG_VERSION             0x42  /**< Versão do chip */
#define SX1278_REG_PA_DAC              0x4D  /**< DAC do amplificador de potência */
/** @} */

/* ============================================================== */
/*                   Modos de Operação                            */
/* ============================================================== */

/** @name Modos de operação do SX1278
 *  Bits do registrador RegOpMode para seleção do modo.
 *  @{
 */
#define SX1278_MODE_SLEEP              0x00  /**< Modo sleep (baixo consumo) */
#define SX1278_MODE_STANDBY            0x01  /**< Modo standby */
#define SX1278_MODE_FSTX               0x02  /**< Sintetizador de frequência TX */
#define SX1278_MODE_TX                 0x03  /**< Modo transmissão */
#define SX1278_MODE_FSRX               0x04  /**< Sintetizador de frequência RX */
#define SX1278_MODE_RX_CONTINUOUS      0x05  /**< Modo recepção contínua */
#define SX1278_MODE_RX_SINGLE          0x06  /**< Modo recepção única */
#define SX1278_MODE_LONG_RANGE         0x80  /**< Bit para habilitar modo LoRa */
/** @} */

/* ============================================================== */
/*                    Flags de Interrupção                        */
/* ============================================================== */

/** @name Flags de interrupção LoRa (RegIrqFlags)
 *  @{
 */
#define SX1278_IRQ_TX_DONE             0x08  /**< Transmissão concluída */
#define SX1278_IRQ_RX_DONE             0x40  /**< Pacote recebido */
#define SX1278_IRQ_PAYLOAD_CRC_ERROR   0x20  /**< Erro de CRC no payload */
#define SX1278_IRQ_VALID_HEADER        0x10  /**< Header válido recebido */
#define SX1278_IRQ_RX_TIMEOUT          0x80  /**< Timeout de recepção */
/** @} */

/* ============================================================== */
/*                  Enumerações de Configuração                   */
/* ============================================================== */

/**
 * @brief Fator de espalhamento (Spreading Factor) do LoRa.
 *
 * Valores maiores aumentam o alcance e a resistência a ruído,
 * mas reduzem a taxa de dados.
 */
typedef enum {
    SX1278_SF_6  = 6,   /**< SF6 — taxa máxima, menor alcance */
    SX1278_SF_7  = 7,   /**< SF7 — padrão, bom equilíbrio */
    SX1278_SF_8  = 8,   /**< SF8 */
    SX1278_SF_9  = 9,   /**< SF9 */
    SX1278_SF_10 = 10,  /**< SF10 */
    SX1278_SF_11 = 11,  /**< SF11 */
    SX1278_SF_12 = 12   /**< SF12 — maior alcance, menor taxa */
} sx1278_sf_t;

/**
 * @brief Largura de banda (Bandwidth) do sinal LoRa.
 *
 * Valores maiores aumentam a taxa de dados mas reduzem
 * a sensibilidade do receptor.
 */
typedef enum {
    SX1278_BW_7_8_KHZ   = 0,  /**< 7.8 kHz */
    SX1278_BW_10_4_KHZ  = 1,  /**< 10.4 kHz */
    SX1278_BW_15_6_KHZ  = 2,  /**< 15.6 kHz */
    SX1278_BW_20_8_KHZ  = 3,  /**< 20.8 kHz */
    SX1278_BW_31_25_KHZ = 4,  /**< 31.25 kHz */
    SX1278_BW_41_7_KHZ  = 5,  /**< 41.7 kHz */
    SX1278_BW_62_5_KHZ  = 6,  /**< 62.5 kHz */
    SX1278_BW_125_KHZ   = 7,  /**< 125 kHz (padrão) */
    SX1278_BW_250_KHZ   = 8,  /**< 250 kHz */
    SX1278_BW_500_KHZ   = 9   /**< 500 kHz */
} sx1278_bw_t;

/**
 * @brief Taxa de codificação (Coding Rate) do LoRa.
 *
 * Adiciona redundância para correção de erros. Valores maiores
 * oferecem mais robustez ao custo de taxa de dados.
 */
typedef enum {
    SX1278_CR_4_5 = 1,  /**< Coding Rate 4/5 */
    SX1278_CR_4_6 = 2,  /**< Coding Rate 4/6 */
    SX1278_CR_4_7 = 3,  /**< Coding Rate 4/7 */
    SX1278_CR_4_8 = 4   /**< Coding Rate 4/8 (máxima redundância) */
} sx1278_cr_t;

/* ============================================================== */
/*                  Funções de Inicialização                      */
/* ============================================================== */

/**
 * @brief Inicializa o módulo SX1278 via SPI.
 *
 * Configura o barramento SPI, reseta o módulo, verifica a versão
 * do chip e aplica a configuração padrão LoRa. Após a inicialização,
 * o módulo entra em modo de recepção contínua.
 *
 * @return STATUS_OK em caso de sucesso.
 * @return STATUS_ERRO_SPI se não for possível comunicar com o chip.
 * @return STATUS_ERRO_RADIO se a versão do chip for inválida.
 */
status_t sx1278_inicializar(void);

/* ============================================================== */
/*                  Funções de Transmissão e Recepção              */
/* ============================================================== */

/**
 * @brief Transmite dados via LoRa.
 *
 * Envia um pacote de dados pelo módulo SX1278. Após a transmissão,
 * o módulo retorna automaticamente ao modo de recepção contínua.
 *
 * @param[in] dados     Ponteiro para o buffer de dados a transmitir.
 * @param[in] tamanho   Número de bytes a transmitir (máx. 255).
 *
 * @return STATUS_OK se transmitido com sucesso.
 * @return STATUS_ERRO_RADIO se houve falha na transmissão.
 * @return STATUS_ERRO_TIMEOUT se a transmissão excedeu o tempo limite.
 */
status_t sx1278_transmitir(const uint8_t *dados, uint8_t tamanho);

/**
 * @brief Recebe dados via LoRa com timeout.
 *
 * Aguarda a recepção de um pacote até o timeout especificado.
 * Se um pacote for recebido, os dados são copiados para o buffer
 * e o tamanho é atualizado.
 *
 * @param[out] buffer       Ponteiro para o buffer de destino.
 * @param[out] tamanho      Ponteiro para armazenar o número de bytes recebidos.
 * @param[in]  timeout_ms   Tempo máximo de espera em milissegundos.
 *
 * @return STATUS_OK se recebido com sucesso.
 * @return STATUS_ERRO_TIMEOUT se nenhum pacote foi recebido no tempo limite.
 * @return STATUS_ERRO_CRC se o pacote recebido falhou na verificação CRC.
 */
status_t sx1278_receber(uint8_t *buffer, uint8_t *tamanho, uint32_t timeout_ms);

/* ============================================================== */
/*                   Funções de Configuração                      */
/* ============================================================== */

/**
 * @brief Configura a frequência de operação do rádio.
 *
 * @param[in] frequencia_hz Frequência desejada em Hz (ex: 433000000 para 433 MHz).
 *
 * @return STATUS_OK em caso de sucesso.
 * @return STATUS_ERRO_SPI se houve falha na comunicação.
 */
status_t sx1278_configurar_frequencia(uint32_t frequencia_hz);

/**
 * @brief Configura a potência de transmissão.
 *
 * @param[in] potencia_dbm Potência desejada em dBm (2 a 20 dBm).
 *
 * @return STATUS_OK em caso de sucesso.
 * @return STATUS_ERRO_SPI se houve falha na comunicação.
 */
status_t sx1278_configurar_potencia(int8_t potencia_dbm);

/**
 * @brief Configura os parâmetros LoRa do módulo.
 *
 * Aplica a combinação de Spreading Factor, Bandwidth e Coding Rate
 * desejada. Os parâmetros devem ser consistentes entre transmissor
 * e receptor para comunicação bem-sucedida.
 *
 * @param[in] sf  Fator de espalhamento (SF6 a SF12).
 * @param[in] bw  Largura de banda do sinal.
 * @param[in] cr  Taxa de codificação para correção de erros.
 *
 * @return STATUS_OK em caso de sucesso.
 * @return STATUS_ERRO_SPI se houve falha na comunicação.
 */
status_t sx1278_configurar_lora(sx1278_sf_t sf, sx1278_bw_t bw, sx1278_cr_t cr);

/* ============================================================== */
/*                   Funções de Monitoramento                     */
/* ============================================================== */

/**
 * @brief Lê o RSSI do último pacote recebido.
 *
 * O RSSI (Received Signal Strength Indicator) indica a potência
 * do sinal do último pacote recebido.
 *
 * @return Valor do RSSI em dBm (valor negativo, ex: -120 dBm).
 */
int16_t sx1278_ler_rssi(void);

/**
 * @brief Lê o SNR do último pacote recebido.
 *
 * O SNR (Signal-to-Noise Ratio) indica a relação sinal-ruído
 * do último pacote recebido.
 *
 * @return Valor do SNR em dB (pode ser negativo).
 */
int8_t sx1278_ler_snr(void);

/* ============================================================== */
/*                   Funções de Modo de Operação                  */
/* ============================================================== */

/**
 * @brief Coloca o módulo em modo de recepção contínua.
 *
 * Neste modo, o módulo permanece escutando indefinidamente
 * por pacotes LoRa. Este é o modo padrão de operação da
 * estação base.
 *
 * @return STATUS_OK em caso de sucesso.
 * @return STATUS_ERRO_SPI se houve falha na comunicação.
 */
status_t sx1278_modo_recepcao_continua(void);

/**
 * @brief Verifica se há um pacote disponível para leitura.
 *
 * Consulta o flag de interrupção RX_DONE para determinar
 * se um pacote completo foi recebido e está pronto para
 * ser lido do FIFO.
 *
 * @return true se há um pacote disponível, false caso contrário.
 */
bool sx1278_pacote_disponivel(void);

#ifdef __cplusplus
}
#endif

#endif /* SX1278_H */
