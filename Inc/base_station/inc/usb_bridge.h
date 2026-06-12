/**
 * @file usb_bridge.h
 * @brief Interface USB CDC (porta COM virtual) para comunicação com o PC.
 *
 * Implementa a ponte de comunicação entre a estação base e o
 * software de controle no PC. Utiliza o periférico USB OTG
 * Full-Speed do STM32F411CEU7 no modo CDC (Communications Device
 * Class) para emular uma porta serial.
 *
 * Funcionalidades principais:
 *   - Encaminhar pacotes de telemetria recebidos do foguete para o PC
 *   - Receber comandos do PC para envio ao foguete
 *   - Enviar informações de status da estação base (RSSI, SNR, etc.)
 */

#ifndef USB_BRIDGE_H
#define USB_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================== */
/*                    Inclusões de Dependências                   */
/* ============================================================== */

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"
#include "../../common/inc/protocol_defs.h"

/* ============================================================== */
/*                   Funções de Inicialização                     */
/* ============================================================== */

/**
 * @brief Inicializa a interface USB CDC da estação base.
 *
 * Configura o periférico USB OTG Full-Speed, inicializa os
 * buffers de transmissão e recepção, e prepara o dispositivo
 * para enumeração pelo PC host.
 *
 * @return STATUS_OK em caso de sucesso.
 * @return STATUS_ERRO_USB se houve falha na inicialização.
 */
status_t usb_bridge_inicializar(void);

/* ============================================================== */
/*                   Funções de Transmissão                       */
/* ============================================================== */

/**
 * @brief Envia dados brutos via USB CDC para o PC.
 *
 * Transmite um bloco de bytes pelo endpoint CDC para o
 * software de controle no PC.
 *
 * @param[in] dados    Ponteiro para o buffer de dados a enviar.
 * @param[in] tamanho  Número de bytes a enviar.
 *
 * @return STATUS_OK se enviado com sucesso.
 * @return STATUS_ERRO_USB se houve falha na transmissão.
 * @return STATUS_ERRO_TIMEOUT se o PC não consumiu os dados a tempo.
 */
status_t usb_bridge_enviar(const uint8_t *dados, uint16_t tamanho);

/**
 * @brief Envia um pacote de telemetria formatado para o PC.
 *
 * Serializa a estrutura de telemetria e a transmite via USB CDC.
 * O pacote é enviado no formato binário definido pelo protocolo
 * comum (34 bytes), incluindo header e CRC.
 *
 * @param[in] pacote  Ponteiro para a estrutura de pacote de telemetria.
 *
 * @return STATUS_OK se enviado com sucesso.
 * @return STATUS_ERRO_USB se houve falha na transmissão.
 */
status_t usb_bridge_enviar_pacote_telemetria(const telemetry_packet_t *pacote);

/**
 * @brief Envia informações de status da estação base para o PC.
 *
 * Transmite um pacote contendo indicadores de qualidade do link
 * de rádio e contagem de pacotes recebidos, permitindo ao PC
 * monitorar a saúde da comunicação.
 *
 * @param[in] rssi               RSSI do último pacote recebido.
 * @param[in] snr                SNR do último pacote recebido.
 * @param[in] pacotes_recebidos  Contagem total de pacotes recebidos.
 *
 * @return STATUS_OK se enviado com sucesso.
 * @return STATUS_ERRO_USB se houve falha na transmissão.
 */
status_t usb_bridge_enviar_status(uint8_t rssi, uint8_t snr, uint32_t pacotes_recebidos);

/* ============================================================== */
/*                    Funções de Recepção                         */
/* ============================================================== */

/**
 * @brief Verifica se há dados disponíveis para leitura via USB.
 *
 * Consulta o buffer de recepção USB para determinar se o PC
 * enviou dados (comandos) para a estação base.
 *
 * @return true se há dados disponíveis, false caso contrário.
 */
bool usb_bridge_dados_disponiveis(void);

/**
 * @brief Recebe dados do PC via USB CDC.
 *
 * Lê os dados disponíveis no buffer de recepção USB e os
 * copia para o buffer fornecido. O tamanho efetivamente
 * lido é retornado no parâmetro tamanho.
 *
 * @param[out] buffer   Ponteiro para o buffer de destino.
 * @param[out] tamanho  Ponteiro para armazenar o número de bytes lidos.
 *
 * @return STATUS_OK se dados foram lidos com sucesso.
 * @return STATUS_ERRO_USB se houve falha na recepção.
 * @return STATUS_ERRO_TIMEOUT se não há dados disponíveis.
 */
status_t usb_bridge_receber(uint8_t *buffer, uint16_t *tamanho);

/* ============================================================== */
/*                   Funções de Estado                            */
/* ============================================================== */

/**
 * @brief Verifica se o USB está conectado e enumerado pelo PC.
 *
 * Retorna o estado da conexão USB CDC. A conexão é considerada
 * ativa quando o PC enumerou o dispositivo e abriu a porta COM.
 *
 * @return true se conectado e pronto para comunicação, false caso contrário.
 */
bool usb_bridge_esta_conectado(void);

#ifdef __cplusplus
}
#endif

#endif /* USB_BRIDGE_H */
