# Foguinho Firmware

Firmware e ferramentas de apoio do **Foguinho**, projeto desenvolvido para o
processo trainee da **Fênix Aerospace, equipe de foguetes da UFMG**.

A ideia do projeto é construir um computador de bordo simples, mas completo o
suficiente para um foguete experimental: ler sensores, estimar o estado do voo,
registrar dados em cartão SD, enviar telemetria por LoRa e permitir comandos a
partir de uma estação de solo.

O repositório também inclui uma estação de base em Python para acompanhar o voo
em tempo real e testar o protocolo sem depender sempre do hardware em bancada.

## O Que Tem Aqui

- `Src/rocket/` e `Inc/rocket/`: lógica embarcada do foguete.
- `Src/base_station/` e `Inc/base_station/`: firmware da estação de solo em uma
  Black Pill STM32F411.
- `Src/common/` e `Inc/common/`: protocolo, CRC e definições compartilhadas.
- `Src/platform/stm32f411/` e `Inc/platform/stm32f411/`: camada de hardware para
  o STM32F411.
- `firmware/Core/`: arquivos gerados a partir do STM32CubeMX.
- `firmware/Drivers/` e `firmware/Middlewares/`: dependências oficiais em
  submódulos Git.
- `tools/base_station_gui/`: interface gráfica da estação de solo.
- `tests/`: testes de lógica que rodam no PC, sem placa conectada.
- `docs/`: relatórios, protocolo e notas técnicas do projeto.

## Hardware Alvo

O alvo principal é o **STM32F411CEU6**, usado em placas Black Pill. O projeto usa:

- BMP388 por SPI para pressão, temperatura e altitude barométrica;
- ICM-20948 por SPI para IMU;
- GPS u-blox NEO-M9N por USART2, em NMEA a 38400 bps;
- rádio SX1278/LoRa por SPI2;
- cartão MicroSD por SPI3;
- USB CDC na estação de base;
- canais pirotécnicos de recuperação com bloqueio físico e lógico.

O clock, GPIOs e periféricos ficam definidos em `Foguinho.ioc`.

## Firmware do Foguete

O firmware roda sem RTOS. Um SysTick de 1 ms alimenta a malha principal, que
executa tarefas em frequências diferentes:

- 200 Hz: sensores, fusão, FSM, datalogger, GPS, recepção LoRa e comandos;
- 10 Hz: telemetria;
- 2 Hz: sincronização do SD;
- 1 Hz: leitura da bateria.

A máquina de estados cobre `BOOT`, `IDLE`, `ARMED`, `ASCENT`, `APOGEE`,
`DESCENT` e `LANDED`. O acionamento pirotécnico é feito no apogeu, com pulso
controlado por interrupção para não depender do loop principal continuar rodando.

Mais detalhes estão em `docs/Relatorio_3_Segmento_Firmware.md`.

## Protocolo

A telemetria tem 34 bytes, little-endian, enviada a 10 Hz. O pacote inclui:

- timestamp;
- altitude barométrica;
- velocidade vertical;
- aceleração;
- tensão da bateria;
- estado de voo;
- latitude, longitude, altitude GPS, fix e número de satélites;
- CRC-16/CCITT.

Os comandos usam pacotes de 5 bytes com CRC. Atualmente existem:

- `ARM`
- `DISARM`
- `PING`
- `START_LOG`
- `STOP_LOG`

O contrato binário está documentado em `docs/PROTOCOLO_ATUAL.md`.

## Estação de Solo

O alvo `foguinho_base` compila o firmware da base para a Black Pill. Ela recebe
pacotes LoRa, valida o protocolo e encaminha os dados pela USB CDC para o PC.

A interface desktop fica em `tools/base_station_gui/`. Ela foi feita em Python
com PySide6 e pyqtgraph, e mostra:

- estado da FSM;
- gráficos de altitude, velocidade e aceleração;
- dados de GPS;
- comandos para armamento, log e ping;
- gravação local de telemetria em CSV.

Ela também possui modo simulador, útil para demonstração e testes sem rádio, GPS
ou placa conectados.

Para rodar:

```powershell
cd tools\base_station_gui
python -m venv .venv
.venv\Scripts\Activate.ps1
pip install -r requirements.txt
python main.py
```

## Preparando o Repositório

Depois de clonar, inicialize os submódulos:

```powershell
git submodule update --init --recursive
```

Eles trazem HAL/LL da ST, CMSIS da Arm/ST e o LoRaMac-node usado como base para
o driver SX1278.

## Build

O projeto usa CMake, Ninja e a toolchain GNU para STM32 instalada pela extensão
STM32Cube para VS Code.

Configurar:

```powershell
cmake --preset Debug
```

Compilar o firmware do foguete:

```powershell
cmake --build --preset Debug --target foguinho
```

Compilar a estação de base:

```powershell
cmake --build --preset Debug --target foguinho_base
```

Também existe o preset `Release`.

## Testes

Os testes de lógica rodam em x64 com MSVC, sem precisar da placa:

```powershell
powershell -ExecutionPolicy Bypass -File tests\run-tests.ps1
```

Eles cobrem partes do GPS, CRC, telemetria, comandos e protocolo da base.

Depois de gerar o `.elf`, há uma checagem da imagem Debug:

```powershell
powershell -ExecutionPolicy Bypass -File tests\test-firmware-image.ps1 -Preset Debug
```

Esse script verifica símbolos, tabela de vetores e alguns estados iniciais de
segurança no código gerado.

## Documentação

Os principais documentos do projeto ficam em `docs/`:

- `Relatorio_3_Segmento_Firmware.md`: visão técnica do firmware.
- `PROTOCOLO_ATUAL.md`: formato dos pacotes de telemetria e comando.
- `DRIVERS_OFICIAIS.md`: notas sobre dependências oficiais e integração com
  STM32CubeMX, Semtech, GPS, BMP388 e ICM-20948.

Os PDFs e o `.docx` guardam as entregas anteriores e referências usadas durante
o trainee.

## Observações

Este repositório mistura código embarcado, documentação de desenvolvimento e
ferramentas de bancada porque ele nasceu como entrega de trainee, não como uma
biblioteca isolada. A estrutura atual tenta deixar isso mais organizado sem
esconder o caminho que o projeto percorreu.
