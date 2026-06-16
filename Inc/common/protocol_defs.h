/**
 * @file protocol_defs.h
 * @brief Definições do protocolo de comunicação entre foguete e estação base.
 *
 * Este arquivo contém as estruturas de pacotes de telemetria e comandos,
 * enumerações de estados de voo e IDs de comandos, além de constantes
 * compartilhadas do protocolo de rádio LoRa.
 *
 * Compartilhado entre o firmware do foguete e da estação base.
 */

#ifndef PROTOCOL_DEFS_H
#define PROTOCOL_DEFS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================
 * Constantes do protocolo
 * ============================================================ */

/** Byte de cabeçalho que identifica o início de cada pacote */
#define PROTOCOL_HEADER_BYTE        (0xAA)

/** Frequência de envio de telemetria (Hz) */
#define PROTOCOL_TELEMETRY_FREQ_HZ  (10)

/** Período entre pacotes de telemetria (ms) */
#define PROTOCOL_TELEMETRY_PERIOD_MS (100)

/** Tamanho total do pacote de telemetria em bytes */
#define PROTOCOL_PACKET_SIZE        (34)

/* ============================================================
 * Enumeração dos estados de voo (FSM)
 * ============================================================ */

/**
 * @brief Estados da máquina de estados finita (FSM) de voo.
 *
 * Cada estado representa uma fase distinta do voo do foguete,
 * desde a inicialização até o pouso confirmado.
 */
typedef enum {
    FSM_BOOT    = 0,  /**< Inicialização do sistema */
    FSM_IDLE    = 1,  /**< Ocioso — aguardando comando de armamento */
    FSM_ARMED   = 2,  /**< Armado — pronto para lançamento */
    FSM_ASCENT  = 3,  /**< Subida — motor ativo ou em ascensão balística */
    FSM_APOGEE  = 4,  /**< Apogeu — ponto mais alto da trajetória */
    FSM_DESCENT = 5,  /**< Descida — em queda com ou sem paraquedas */
    FSM_LANDED  = 6   /**< Pousado — pouso confirmado */
} flight_state_t;

/* ============================================================
 * Enumeração dos IDs de comando
 * ============================================================ */

/**
 * @brief Identificadores de comandos enviados pela estação base.
 */
typedef enum {
    CMD_ARM       = 0x01,  /**< Armar o foguete */
    CMD_DISARM    = 0x02,  /**< Desarmar o foguete */
    CMD_PING      = 0x03,  /**< Verificação de conectividade */
    CMD_START_LOG = 0x04,  /**< Iniciar gravação no cartão SD */
    CMD_STOP_LOG  = 0x05   /**< Parar gravação no cartão SD */
} command_id_t;

/** Resultados compartilhados nas respostas aos comandos. */
typedef enum {
    COMMAND_RESULT_OK = 0,
    COMMAND_RESULT_INVALID_STATE = 1,
    COMMAND_RESULT_SAFETY_LOCK = 2,
    COMMAND_RESULT_STORAGE_ERROR = 3,
    COMMAND_RESULT_HARDWARE_ERROR = 4
} command_result_t;

/* ============================================================
 * Estrutura do pacote de telemetria (empacotada)
 * ============================================================ */

#pragma pack(push, 1)

/**
 * @brief Pacote de telemetria enviado do foguete para a estação base.
 *
 * Layout binário de 34 bytes, transmitido a cada 100 ms (10 Hz).
 *
 * | Campo            | Tipo     | Tamanho | Descrição                        |
 * |------------------|----------|---------|----------------------------------|
 * | header           | uint8_t  | 1 B     | Byte de cabeçalho (0xAA)        |
 * | packet_id        | uint8_t  | 1 B     | Número sequencial do pacote      |
 * | timestamp_ms     | uint32_t | 4 B     | Tempo desde o boot (ms)          |
 * | altitude_m       | float    | 4 B     | Altitude relativa (m)            |
 * | vert_velocity_ms | float    | 4 B     | Velocidade vertical (m/s)        |
 * | acceleration_g   | float    | 4 B     | Aceleração resultante (g)        |
 * | battery_mv       | uint16_t | 2 B     | Tensão da bateria (mV)           |
 * | flight_state     | uint8_t  | 1 B     | Estado atual da FSM              |
 * | gps_latitude     | int32_t  | 4 B     | Latitude escalada por 1e7        |
 * | gps_longitude    | int32_t  | 4 B     | Longitude escalada por 1e7       |
 * | gps_altitude_m   | int16_t  | 2 B     | Altitude do GPS em metros        |
 * | gps_info         | uint8_t  | 1 B     | Fix e satélites                  |
 * | crc16            | uint16_t | 2 B     | CRC-16/CCITT do pacote           |
 */
typedef struct {
    uint8_t  header;           /**< Byte de cabeçalho (PROTOCOL_HEADER_BYTE) */
    uint8_t  packet_id;        /**< Identificador sequencial do pacote */
    uint32_t timestamp_ms;     /**< Timestamp em milissegundos desde o boot */
    float    altitude_m;       /**< Altitude relativa em metros */
    float    vert_velocity_ms; /**< Velocidade vertical em m/s */
    float    acceleration_g;   /**< Aceleração resultante em g */
    uint16_t battery_mv;       /**< Tensão da bateria em milivolts */
    uint8_t  flight_state;     /**< Estado atual (flight_state_t) */
    int32_t  gps_latitude;     /**< Latitude escalada por 1e7 (ex: -23.4567890 -> -234567890) */
    int32_t  gps_longitude;    /**< Longitude escalada por 1e7 */
    int16_t  gps_altitude_m;   /**< Altitude do GPS em metros */
    uint8_t  gps_info;         /**< Bit 7: Fix válido (1) / inválido (0). Bits 0-6: Número de satélites */
    uint16_t crc16;            /**< CRC-16/CCITT para verificação de integridade */
} telemetry_packet_t;

#pragma pack(pop)

/* ============================================================
 * Estrutura do pacote de comando (empacotada)
 * ============================================================ */

#pragma pack(push, 1)

/**
 * @brief Pacote de comando enviado da estação base para o foguete.
 *
 * Estrutura compacta contendo o cabeçalho, o ID do comando,
 * um byte opcional de payload e o CRC-16 para validação.
 */
typedef struct {
    uint8_t  header;      /**< Byte de cabeçalho (PROTOCOL_HEADER_BYTE) */
    uint8_t  command_id;  /**< Identificador do comando (command_id_t) */
    uint8_t  payload;     /**< Byte de payload opcional */
    uint16_t crc16;       /**< CRC-16/CCITT para verificação de integridade */
} command_packet_t;

#pragma pack(pop)

/* ============================================================
 * Verificação estática do tamanho do pacote de telemetria
 * ============================================================ */

/**
 * Garante em tempo de compilação que o pacote de telemetria
 * possui exatamente o tamanho especificado pelo protocolo.
 */
_Static_assert(sizeof(telemetry_packet_t) == PROTOCOL_PACKET_SIZE,
               "Erro: tamanho de telemetry_packet_t deve ser 34 bytes");
_Static_assert(offsetof(telemetry_packet_t, gps_latitude) == 21,
               "Erro: layout de GPS da telemetria foi alterado");
_Static_assert(offsetof(telemetry_packet_t, crc16) == 32,
               "Erro: CRC deve ocupar os dois bytes finais da telemetria");
_Static_assert(sizeof(command_packet_t) == 5,
               "Erro: command_packet_t deve possuir 5 bytes");
_Static_assert(offsetof(command_packet_t, crc16) == 3,
               "Erro: CRC deve ocupar os dois bytes finais do comando");

#endif /* PROTOCOL_DEFS_H */
