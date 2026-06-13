/**
 * @file sx1278.c
 * @brief Implementação do driver do rádio LoRa SX1278.
 */

#include "sx1278.h"
#include "hal_plataforma.h"
#include "pinout.h"
#include "stm32f411_hw.h"

static int16_t last_rssi = 0;
static bool inicializado = false;
static bool recepcao_ativa = false;

#define SX1278_MODO_LORA       0x80U
#define SX1278_MODO_SLEEP      0x00U
#define SX1278_MODO_STANDBY    0x01U
#define SX1278_MODO_TX         0x03U
#define SX1278_MODO_RX_CONT    0x05U
#define SX1278_MODO_RX_SINGLE  0x06U
#define SX1278_IRQ_RX_DONE     0x40U
#define SX1278_IRQ_CRC_ERROR   0x20U
#define SX1278_IRQ_TX_DONE     0x08U

static status_t sx_escrever(uint8_t reg, uint8_t valor) {
    uint8_t dados[2] = {(uint8_t)(reg | 0x80U), valor};
    hw_gpio_write(PINO_SX1278_CS_PORTA, PINO_SX1278_CS_PINO, false);
    status_t status = hw_spi_transfer(HW_SPI2, dados, NULL, sizeof(dados));
    hw_gpio_write(PINO_SX1278_CS_PORTA, PINO_SX1278_CS_PINO, true);
    return status;
}

static status_t sx_ler(uint8_t reg, uint8_t *valor) {
    uint8_t tx[2] = {(uint8_t)(reg & 0x7FU), 0xFFU};
    uint8_t rx[2];
    if (!valor) return STATUS_ERRO_GENERICO;
    hw_gpio_write(PINO_SX1278_CS_PORTA, PINO_SX1278_CS_PINO, false);
    status_t status = hw_spi_transfer(HW_SPI2, tx, rx, sizeof(tx));
    hw_gpio_write(PINO_SX1278_CS_PORTA, PINO_SX1278_CS_PINO, true);
    if (status == STATUS_OK) *valor = rx[1];
    return status;
}

static status_t sx_fifo_escrever(const uint8_t *dados, uint8_t tamanho) {
    uint8_t comando = SX1278_REG_FIFO | 0x80U;
    hw_gpio_write(PINO_SX1278_CS_PORTA, PINO_SX1278_CS_PINO, false);
    status_t status = hw_spi_transfer(HW_SPI2, &comando, NULL, 1);
    if (status == STATUS_OK) status = hw_spi_transfer(HW_SPI2, dados, NULL, tamanho);
    hw_gpio_write(PINO_SX1278_CS_PORTA, PINO_SX1278_CS_PINO, true);
    return status;
}

static status_t sx_fifo_ler(uint8_t *dados, uint8_t tamanho) {
    uint8_t comando = SX1278_REG_FIFO & 0x7FU;
    hw_gpio_write(PINO_SX1278_CS_PORTA, PINO_SX1278_CS_PINO, false);
    status_t status = hw_spi_transfer(HW_SPI2, &comando, NULL, 1);
    if (status == STATUS_OK) status = hw_spi_transfer(HW_SPI2, NULL, dados, tamanho);
    hw_gpio_write(PINO_SX1278_CS_PORTA, PINO_SX1278_CS_PINO, true);
    return status;
}

status_t sx1278_inicializar(void) {
    uint8_t versao;
    hw_gpio_output(PINO_SX1278_CS_PORTA, PINO_SX1278_CS_PINO, true);
    hw_gpio_output(PINO_SX1278_RST_PORTA, PINO_SX1278_RST_PINO, true);
    hw_gpio_input_pullup(PINO_SX1278_DIO0_PORTA, PINO_SX1278_DIO0_PINO);
    if (hw_spi_init(HW_SPI2, 2) != STATUS_OK) return STATUS_ERRO_SPI;

    hw_gpio_write(PINO_SX1278_RST_PORTA, PINO_SX1278_RST_PINO, false);
    plataforma_delay_ms(2);
    hw_gpio_write(PINO_SX1278_RST_PORTA, PINO_SX1278_RST_PINO, true);
    plataforma_delay_ms(10);

    if (sx_ler(SX1278_REG_VERSION, &versao) != STATUS_OK || versao != 0x12U) {
        return STATUS_ERRO_RADIO;
    }
    if (sx1278_modo_sleep() != STATUS_OK ||
        sx1278_configurar_frequencia(SX1278_FREQUENCIA_PADRAO) != STATUS_OK ||
        sx1278_configurar_lora(SX1278_SF_PADRAO, SX1278_BW_PADRAO, SX1278_CR_PADRAO) != STATUS_OK ||
        sx1278_configurar_potencia(SX1278_POTENCIA_PADRAO_DBM) != STATUS_OK ||
        sx_escrever(SX1278_REG_FIFO_TX_BASE, 0x00U) != STATUS_OK ||
        sx_escrever(SX1278_REG_FIFO_RX_BASE, 0x00U) != STATUS_OK ||
        sx_escrever(SX1278_REG_MAX_PAYLOAD_LEN, 0xFFU) != STATUS_OK ||
        sx_escrever(SX1278_REG_SYNC_WORD, 0x12U) != STATUS_OK) {
        return STATUS_ERRO_RADIO;
    }
    inicializado = true;
    return sx1278_modo_standby();
}

status_t sx1278_transmitir(const uint8_t *dados, uint8_t tamanho) {
    if (!inicializado || !dados || tamanho == 0U) {
        return STATUS_ERRO_GENERICO;
    }

    recepcao_ativa = false;
    if (sx1278_modo_standby() != STATUS_OK ||
        sx_escrever(SX1278_REG_IRQ_FLAGS, 0xFFU) != STATUS_OK ||
        sx_escrever(SX1278_REG_FIFO_ADDR_PTR, 0x00U) != STATUS_OK ||
        sx_escrever(SX1278_REG_PAYLOAD_LENGTH, tamanho) != STATUS_OK ||
        sx_fifo_escrever(dados, tamanho) != STATUS_OK ||
        sx_escrever(SX1278_REG_DIO_MAPPING_1, 0x40U) != STATUS_OK ||
        sx_escrever(SX1278_REG_OP_MODE, SX1278_MODO_LORA | SX1278_MODO_TX) != STATUS_OK) {
        return STATUS_ERRO_RADIO;
    }

    uint32_t inicio = plataforma_obter_tick_ms();
    uint8_t flags = 0;
    do {
        if (sx_ler(SX1278_REG_IRQ_FLAGS, &flags) != STATUS_OK) return STATUS_ERRO_RADIO;
        if ((flags & SX1278_IRQ_TX_DONE) != 0U) {
            sx_escrever(SX1278_REG_IRQ_FLAGS, SX1278_IRQ_TX_DONE);
            return sx1278_modo_standby();
        }
    } while ((plataforma_obter_tick_ms() - inicio) < 3000U);
    sx1278_modo_standby();
    return STATUS_ERRO_TIMEOUT;
}

status_t sx1278_receber(uint8_t *buffer, uint8_t *tamanho, uint32_t timeout_ms) {
    uint8_t flags;
    uint8_t quantidade;
    uint8_t endereco;
    if (!inicializado || !buffer || !tamanho) return STATUS_ERRO_GENERICO;
    *tamanho = 0;
    if (!recepcao_ativa) {
        uint8_t modo = timeout_ms == 0U ? SX1278_MODO_RX_CONT : SX1278_MODO_RX_SINGLE;
        if (sx_escrever(SX1278_REG_IRQ_FLAGS, 0xFFU) != STATUS_OK ||
            sx_escrever(SX1278_REG_DIO_MAPPING_1, 0x00U) != STATUS_OK ||
            sx_escrever(SX1278_REG_OP_MODE, SX1278_MODO_LORA | modo) != STATUS_OK) {
            return STATUS_ERRO_RADIO;
        }
        recepcao_ativa = true;
    }

    uint32_t inicio = plataforma_obter_tick_ms();
    do {
        if (sx_ler(SX1278_REG_IRQ_FLAGS, &flags) != STATUS_OK) return STATUS_ERRO_RADIO;
        if ((flags & SX1278_IRQ_RX_DONE) != 0U) {
            sx_escrever(SX1278_REG_IRQ_FLAGS, flags);
            if ((flags & SX1278_IRQ_CRC_ERROR) != 0U) return STATUS_ERRO_CRC;
            if (sx_ler(SX1278_REG_RX_NB_BYTES, &quantidade) != STATUS_OK ||
                sx_ler(SX1278_REG_FIFO_RX_CURRENT, &endereco) != STATUS_OK ||
                sx_escrever(SX1278_REG_FIFO_ADDR_PTR, endereco) != STATUS_OK ||
                sx_fifo_ler(buffer, quantidade) != STATUS_OK) return STATUS_ERRO_RADIO;
            *tamanho = quantidade;
            uint8_t rssi;
            if (sx_ler(SX1278_REG_PKT_RSSI, &rssi) == STATUS_OK) last_rssi = (int16_t)rssi - 157;
            if (timeout_ms != 0U) {
                recepcao_ativa = false;
                sx1278_modo_standby();
            }
            return STATUS_OK;
        }
        if (timeout_ms == 0U) break;
    } while ((plataforma_obter_tick_ms() - inicio) < timeout_ms);
    if (timeout_ms != 0U) {
        recepcao_ativa = false;
        sx1278_modo_standby();
    }
    return STATUS_ERRO_TIMEOUT;
}

status_t sx1278_configurar_frequencia(uint32_t frequencia_hz) {
    if (frequencia_hz < 137000000UL || frequencia_hz > 525000000UL) return STATUS_ERRO_GENERICO;
    uint64_t frf = ((uint64_t)frequencia_hz << 19) / 32000000UL;
    if (sx_escrever(SX1278_REG_FRF_MSB, (uint8_t)(frf >> 16)) != STATUS_OK ||
        sx_escrever(SX1278_REG_FRF_MID, (uint8_t)(frf >> 8)) != STATUS_OK ||
        sx_escrever(SX1278_REG_FRF_LSB, (uint8_t)frf) != STATUS_OK) return STATUS_ERRO_RADIO;
    return STATUS_OK;
}

status_t sx1278_configurar_potencia(int8_t potencia_dbm) {
    if (potencia_dbm < 2 || potencia_dbm > 20) return STATUS_ERRO_GENERICO;
    uint8_t saida = (uint8_t)(potencia_dbm > 17 ? potencia_dbm - 5 : potencia_dbm - 2);
    if (sx_escrever(SX1278_REG_PA_CONFIG, (uint8_t)(0x80U | saida)) != STATUS_OK) {
        return STATUS_ERRO_RADIO;
    }
    return STATUS_OK;
}

status_t sx1278_configurar_lora(sx1278_sf_t sf, sx1278_bw_t bw, sx1278_cr_t cr) {
    if (sf < SX1278_SF7 || sf > SX1278_SF12 || bw > SX1278_BW_500K ||
        cr < SX1278_CR_4_5 || cr > SX1278_CR_4_8) return STATUS_ERRO_GENERICO;
    uint8_t config1 = (uint8_t)(((uint8_t)bw << 4) | ((uint8_t)cr << 1));
    uint8_t config2 = (uint8_t)(((uint8_t)sf << 4) | 0x04U);
    uint8_t config3 = 0x04U;
    if ((sf >= SX1278_SF11) && bw == SX1278_BW_125K) config3 |= 0x08U;
    if (sx_escrever(SX1278_REG_MODEM_CONFIG_1, config1) != STATUS_OK ||
        sx_escrever(SX1278_REG_MODEM_CONFIG_2, config2) != STATUS_OK ||
        sx_escrever(SX1278_REG_MODEM_CONFIG_3, config3) != STATUS_OK) return STATUS_ERRO_RADIO;
    return STATUS_OK;
}

int16_t sx1278_ler_rssi(void) {
    return last_rssi;
}

status_t sx1278_modo_sleep(void) {
    recepcao_ativa = false;
    return sx_escrever(SX1278_REG_OP_MODE, SX1278_MODO_LORA | SX1278_MODO_SLEEP);
}

status_t sx1278_modo_standby(void) {
    recepcao_ativa = false;
    return sx_escrever(SX1278_REG_OP_MODE, SX1278_MODO_LORA | SX1278_MODO_STANDBY);
}
