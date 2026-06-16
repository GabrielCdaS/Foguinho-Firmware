# Relatório Técnico 3 — Firmware

**Projeto:** Computador de Bordo — Foguinho  
**Hardware:** STM32F411CEU6 (Cortex-M4, 96 MHz, 512 KB Flash, 128 KB SRAM)  
**Linguagem:** C11  

---

## 1. Evolução desde o Relatório 2

No Relatório 2 o firmware consistia apenas nos cabeçalhos (`.h`) que definiam a interface de cada módulo. A estrutura de diretórios (`common/`, `rocket/`, `base_station/`) já existia, mas nenhum arquivo de implementação havia sido escrito.

Nesta terceira entrega todos os módulos foram implementados e testados. As mudanças mais relevantes em relação ao que foi planejado são:

- **Implementação completa dos `.c`** — drivers de sensores, fusão, FSM, datalogger, telemetria e recuperação.
- **Protocolo de telemetria expandido** — o pacote passou de 23 para **34 bytes** com a adição dos campos de GPS (latitude, longitude, altitude e informações de satélite), necessários para localizar o foguete após o pouso.
- **Estado `FSM_LANDED`** — a FSM ganhou o estado terminal de código 6, que encerra a gravação no SD de forma segura.

---

## 2. Arquitetura e Escalonamento

O firmware roda sem RTOS. O SysTick gera uma interrupção a cada 1 ms e a cada 5 ms sinaliza um tick para o loop principal, resultando em uma malha de controle a **200 Hz**.

As tarefas são divididas por frequência:

| Frequência | Período | Tarefas |
|---:|---:|---|
| 200 Hz | 5 ms | Leitura de sensores (BMP388 + ICM-20948), fusão, atualização da FSM, gravação no SD, processamento do GPS (USART2), recepção LoRa e execução de comandos |
| 10 Hz | 100 ms | Montagem e envio do pacote de telemetria via SX1278 |
| 2 Hz | 500 ms | `f_sync` no cartão SD |
| 1 Hz | 1 s | Leitura da tensão de bateria (ADC) |

O controle dos pulsos pirotécnicos é feito dentro da própria ISR do SysTick (`recuperacao_processar`), de modo que o GPIO é desligado após 1 s mesmo que o loop principal trave.

---

## 3. Sensores e Fusão

O firmware lê três fontes de dados:

1. **BMP388** (barômetro, SPI1, CS em PA4) — pressão e temperatura. A pressão é convertida em altitude relativa ao solo, calibrada na inicialização.
2. **ICM-20948** (IMU 9 eixos, SPI1, CS em PB0) — aceleração, giroscópio e magnetômetro.
3. **NEO-M8N / NEO-M9N** (GPS, USART2, PA2/PA3, 9600 bps) — parsing das sentenças NMEA `$GPGGA` e `$GPRMC`.

A fusão sensorial (`fusion.c`) combina barômetro e IMU:

- A altitude vem do barômetro.
- A aceleração vertical desconta 1 g da leitura do eixo Z da IMU.
- A velocidade vertical é a derivada da altitude filtrada por um passa-baixas de 1ª ordem:

$$v_k = 0{,}8 \cdot v_{k-1} + 0{,}2 \cdot \frac{h_k - h_{k-1}}{\Delta t}$$

O GPS possui um timeout de 3 s (`GPS_FIX_TIMEOUT_MS`). Se nenhuma sentença válida chegar nesse intervalo, o bit de fix é limpo no pacote de telemetria, mas as últimas coordenadas conhecidas continuam sendo transmitidas para facilitar a busca.

---

## 4. Máquina de Estados (FSM)

A FSM (`flight_state.c`) possui 7 estados:

| Estado | Código | Gatilho de entrada |
|---|:---:|---|
| `BOOT` | 0 | Power-on. Transita para IDLE automaticamente. |
| `IDLE` | 1 | Aguarda comando de armamento. |
| `ARMED` | 2 | Comando `ARM` via rádio + chave física em PC15 ativa. Abre arquivo no SD. |
| `ASCENT` | 3 | Aceleração $\ge 3$ g por pelo menos 3 amostras consecutivas. |
| `APOGEE` | 4 | Velocidade vertical cruza zero (< 1 m/s) com altitude > 10 m. Aciona paraquedas principal. |
| `DESCENT` | 5 | 2 s após apogeu. |
| `LANDED` | 6 | Velocidade < 0,5 m/s e aceleração ≈ 1 g por 5 s. Fecha arquivo no SD. |

Regras de segurança em `flight_defs.c`:
- `ARM` só é aceito em `IDLE`; `DISARM` só em `ARMED`.
- Se o foguete ficar armado por mais de 10 min sem lançar, volta para `IDLE` automaticamente.

---

## 5. Protocolo de Telemetria

Pacotes de 34 bytes, little-endian, enviados a 10 Hz via SX1278 (LoRa, SPI2, CS em PB12):

| Offset | Campo | Tipo | Descrição |
|---:|---|---|---|
| 0 | `header` | `uint8_t` | `0xAA` |
| 1 | `packet_id` | `uint8_t` | Contador sequencial (wrap em 255) |
| 2 | `timestamp_ms` | `uint32_t` | Tempo desde o boot |
| 6 | `altitude_m` | `float` | Altitude barométrica relativa |
| 10 | `vert_velocity_ms` | `float` | Velocidade vertical |
| 14 | `acceleration_g` | `float` | Módulo da aceleração |
| 18 | `battery_mv` | `uint16_t` | Tensão da bateria |
| 20 | `flight_state` | `uint8_t` | Estado da FSM |
| 21 | `gps_latitude` | `int32_t` | Latitude × 10⁷ |
| 25 | `gps_longitude` | `int32_t` | Longitude × 10⁷ |
| 29 | `gps_altitude_m` | `int16_t` | Altitude GPS |
| 31 | `gps_info` | `uint8_t` | Bit 7: fix válido; bits 0–6: nº satélites |
| 32 | `crc16` | `uint16_t` | CRC-16/CCITT (bytes 0–31) |

O layout é forçado sem padding via `#pragma pack(push, 1)`, com `_Static_assert` confirmando os 34 bytes em tempo de compilação.

Comandos seguem o formato de 5 bytes: header + ID + payload + CRC-16. Toda instrução gera uma resposta com código de resultado (`OK`, `INVALID_STATE`, `SAFETY_LOCK`, `STORAGE_ERROR` ou `HARDWARE_ERROR`).

---

## 6. Recuperação Pirotécnica

O sistema possui dois canais de ejeção e duas camadas de bloqueio de segurança:

**Canais:**
- Principal: PC13 — acionado no apogeu.
- Emergência: PC14 — acionado caso o principal falhe.

**Bloqueios:**
- *Físico:* a chave de armamento no pino PC15 (pull-up) precisa estar conectada ao terra. Se estiver aberta, o firmware recusa o `ARM`.
- *Lógico:* os GPIOs pirotécnicos só são ativados nos estados `APOGEE` ou `DESCENT`.

O pulso dura 1 s e é controlado pela ISR do SysTick, sem bloquear o loop. Antes do voo, `recuperacao_verificar_continuidade` permite checar se os ignitores estão conectados.

---

## 7. Datalogger

O datalogger grava no cartão MicroSD via SPI3 (CS em PA15) usando a biblioteca FatFs. A cada tick (200 Hz), a struct `dados_sensores_t` (~72 bytes) é copiada para um buffer interno. Quando o buffer enche, os dados são escritos no arquivo.

A cada 500 ms é feito um `f_sync` para forçar a escrita física. Isso garante que, em caso de perda de energia, no máximo 0,5 s de dados sejam perdidos.

O arquivo é aberto automaticamente ao armar e fechado ao pousar.

---

## 8. Interface Gráfica de Solo

A GUI recebe a telemetria decodificada pela estação base (via USB) e exibe os dados ao operador em tempo real. Suas funções principais são:

- **Gráficos de voo** — altitude, velocidade e aceleração plotados a 10 Hz.
- **Orientação 3D** — visualização da atitude do foguete (pitch, roll, yaw) a partir dos dados da IMU e magnetômetro.
- **Mapa GPS** — posição do foguete em mapa, incluindo última posição conhecida para busca pós-pouso.
- **Painel de comandos** — botões para `ARM`, `DISARM`, `PING`, `START_LOG` e `STOP_LOG`, com confirmação em dois passos para comandos críticos.

```latex
\begin{figure}[htbp]
    \centering
    \includegraphics[width=0.95\textwidth]{ground_station_gui.png}
    \caption{Interface gráfica de solo — dashboard de telemetria e comandos.}
    \label{fig:ground_station_gui}
\end{figure}
```

---

## 9. Testes

Os testes unitários rodam nativamente em x64 (compilados com MSVC) sem dependência do hardware. O arquivo `test_firmware_logic.c` cobre:

- **GPS** — parsing de `$GPGGA` e `$GPRMC`, conversão de coordenadas e comportamento do timeout de fix.
- **Telemetria** — tamanho do pacote (34 bytes), alinhamento dos campos e cálculo do CRC-16.
- **Comandos e FSM** — rejeição de `ARM` fora de `IDLE`, rejeição sem chave física, rollback em caso de falha de hardware.
