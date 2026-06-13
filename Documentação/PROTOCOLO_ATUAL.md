# Protocolo atual do firmware

Este documento registra o contrato binario implementado entre o foguete e a
estacao base. Ele complementa o documento original com os campos de GPS
necessarios para localizacao e recuperacao do foguete.

## Telemetria

A telemetria e transmitida a 10 Hz. Cada pacote possui 34 bytes, usa ordem de
bytes little-endian e termina com CRC-16/CCITT calculado sobre os primeiros
32 bytes.

| Offset | Campo | Tipo | Descricao |
|---:|---|---|---|
| 0 | `header` | `uint8_t` | Valor fixo `0xAA` |
| 1 | `packet_id` | `uint8_t` | Contador sequencial com wrap em 255 |
| 2 | `timestamp_ms` | `uint32_t` | Tempo desde o boot |
| 6 | `altitude_m` | `float` | Altitude relativa estimada |
| 10 | `vert_velocity_ms` | `float` | Velocidade vertical |
| 14 | `acceleration_g` | `float` | Modulo da aceleracao nos tres eixos |
| 18 | `battery_mv` | `uint16_t` | Tensao da bateria |
| 20 | `flight_state` | `uint8_t` | Estado da maquina de voo |
| 21 | `gps_latitude` | `int32_t` | Graus decimais multiplicados por 1e7 |
| 25 | `gps_longitude` | `int32_t` | Graus decimais multiplicados por 1e7 |
| 29 | `gps_altitude_m` | `int16_t` | Altitude GPS em metros |
| 31 | `gps_info` | `uint8_t` | Bit 7: fix valido; bits 0-6: satelites |
| 32 | `crc16` | `uint16_t` | CRC dos bytes 0 a 31 |

Quando o fix expira, as ultimas coordenadas conhecidas permanecem no pacote,
mas o bit de validade e limpo. Isso preserva a ultima posicao util sem
apresenta-la como uma leitura atual.

## Comandos e respostas

Pacotes de comando e resposta possuem 5 bytes: header, ID do comando, payload
e CRC-16. Na ida, o payload e reservado. Na resposta, o payload contem um
`command_result_t` compartilhado:

| Valor | Resultado |
|---:|---|
| 0 | Sucesso |
| 1 | Comando invalido no estado atual |
| 2 | Bloqueio de seguranca, como chave fisica desarmada |
| 3 | Falha de armazenamento |
| 4 | Falha de hardware |

Os comandos suportados sao:

| ID | Comando | Comportamento |
|---:|---|---|
| `0x01` | `ARM` | Arma a recuperacao e entra em `ARMED` |
| `0x02` | `DISARM` | Retorna a `IDLE` e desarma as saidas |
| `0x03` | `PING` | Responde sem alterar o estado |
| `0x04` | `START_LOG` | Abre o arquivo de voo quando o SD esta disponivel |
| `0x05` | `STOP_LOG` | Sincroniza e fecha o arquivo de voo |

Todo comando valido recebe uma resposta, inclusive quando sua execucao falha.
