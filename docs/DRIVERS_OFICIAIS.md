# Drivers oficiais

O firmware usa dependências oficiais fixadas como submódulos Git:

- `firmware/Drivers/STM32F4xx_HAL_Driver`: HAL/LL oficial da ST.
- `firmware/Drivers/CMSIS/Device/ST/STM32F4xx`: definições CMSIS do STM32F4.
- `firmware/Drivers/CMSIS/Core`: CMSIS Core oficial da Arm.
- `firmware/Middlewares/LoRaMac-node`: driver SX1276/77/78/79 oficial da Semtech.

Após clonar o projeto, inicialize as dependências com:

```powershell
git submodule update --init --recursive
```

## Integração STM32F411

O alvo oficial é `STM32F411CEU6` (`STM32F411CEUx`, UFQFPN48). `CEU7` não é
um código comercial válido da ST.

`Foguinho.ioc` define no STM32CubeMX:

- HSI/PLL em 96 MHz, APB1 em 48 MHz, APB2 em 96 MHz e clock USB em 48 MHz;
- USB OTG FS em modo device com classe CDC para a estacao de solo na Black Pill;
- ADC1 canal 0 para a bateria;
- SPI1 para sensores, SPI2 para o SX1278 e SPI3 para o cartão SD;
- USART1 para depuração e USART2 para o GPS;
- GPIOs com estados iniciais seguros para chip selects e canais pirotécnicos.

Os arquivos em `firmware/Core/` foram derivados da geração oficial e fornecem os
handles HAL e a inicialização da plataforma. `Src/platform/stm32f411/stm32f411_hw.c`
preserva a API interna do projeto para o foguete e para a base, mas usa esses
handles em vez de configurar registradores ou periféricos localmente.

## Regeneração com STM32CubeMX

O projeto foi validado com STM32CubeMX 6.17.0 e STM32Cube FW_F4 V1.28.3.
O pacote completo oficial também foi instalado no repositório local do
CubeMX em `~/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3`.

Para gerar uma cópia limpa e isolada:

```powershell
.\scripts\generate-cubemx.ps1
```

A saída fica em `build/cubemx-generated`, sem sobrescrever a aplicação nem os
submódulos. O CMake continua sendo o projeto mantido pela extensão oficial
STM32 para VS Code.

## Integração SX1278

`Src/rocket/sx1278.c` é uma fachada sobre o `sx1276.c` oficial.
`Src/base_station/sx1278.c` faz a mesma ponte para a estacao de solo.
`Src/platform/stm32f411/semtech_port.c` implementa as interfaces de placa
exigidas pela Semtech usando a HAL do STM32.

O driver oficial aceita, na API LoRa utilizada, larguras de banda de 125, 250
e 500 kHz. Por isso, a fachada rejeita as opções menores em vez de entrar no
tratamento fatal existente no código de referência.

O módulo de rádio desta placa usa `PA_BOOST`, SPI2 e os pinos definidos em
`Inc/platform/stm32f411/pinout.h`.

## GPS u-blox NEO-M9N

Nao foi adicionada uma nova dependencia para o GPS. O NEO-M9N ja entrega
NMEA por UART de fabrica, e o firmware atual (`Src/rocket/gps.c`) faz o
parsing de `$GPGGA`, `$GNGGA`, `$GPRMC` e `$GNRMC`, com validacao de checksum
e timeout de fix.

Conforme a folha de dados oficial do NEO-M9N-00B, a interface UART padrao
opera em 38400 bps, 8N1, com mensagens NMEA habilitadas. Por isso
`GPS_BAUDRATE` foi ajustado para 38400 em `Inc/rocket/config.h`.

Um driver/protocolo UBX so passa a ser necessario se o firmware precisar
configurar o receptor em tempo de execucao, por exemplo taxa de navegacao,
constelacoes, filtros, modo dinamico ou saida binaria `NAV-PVT`. Para a
telemetria atual, NMEA e suficiente.

Fontes oficiais:

- https://content.u-blox.com/sites/default/files/NEO-M9N-00B_DataSheet_UBX-19014285.pdf
- https://content.u-blox.com/sites/default/files/NEO-M9N_Integrationmanual_UBX-19014286.pdf

## BMP388

Nao foi adicionada uma nova dependencia para o BMP388 neste momento. O projeto
ja possui driver SPI proprio em `Src/rocket/bmp388.c`, com leitura de `CHIP_ID`,
reset, coeficientes de calibracao, compensacao de pressao/temperatura,
calculo de altitude relativa e autoteste.

A Bosch fornece o `BMP3_SensorAPI` oficial, que e a referencia recomendada
caso a equipe queira substituir a compensacao local por uma implementacao
mantida pelo fabricante. A troca nao e obrigatoria para a funcionalidade
existente, mas e uma boa etapa de endurecimento antes de ensaios de voo.

Fonte oficial:

- https://github.com/boschsensortec/BMP3_SensorAPI

## ICM-20948

Nao foi adicionada uma nova dependencia para o ICM-20948 neste momento. O
projeto ja possui driver SPI proprio em `Src/rocket/icm20948.c`, cobrindo
`WHO_AM_I`, reset, selecao de banco, escalas de acelerometro/giroscopio e
leitura do magnetometro AK09916 pelo barramento auxiliar I2C interno.

A TDK/InvenSense disponibiliza o pacote oficial `DK-20948 SmartMotion eMD`.
Ele e indicado se o firmware precisar do DMP, calibracao/fusao oficial,
recursos avancados de movimento ou validacao mais proxima do fabricante. Para
as leituras brutas usadas atualmente, o modulo local cobre o necessario.

Fontes oficiais:

- https://invensense.tdk.com/products/motion-tracking/9-axis/icm-20948
- https://invensense.tdk.com/developers/software-downloads

## Altimetro Blue Raven

Nao encontrei driver local nem protocolo integrado para o Blue Raven. Pelo
material publico da Featherweight, ele se comporta como altimetro/computador de
voo independente, com configuracao e leitura de dados via aplicativo, e nao
como um sensor simples SPI/I2C/UART equivalente ao BMP388 ou ICM-20948.

Antes de qualquer codigo novo, e necessario confirmar qual interface fisica e
qual protocolo serao usados para telemetria com o STM32. Sem um protocolo
documentado de saida em tempo real, nao ha driver oficial para baixar ou
integrar no firmware do foguete.

Fonte oficial:

- https://www.featherweightaltimeters.com/blue-raven-altimeter.html

## Estacao de solo Black Pill

A estacao de solo agora possui o alvo CMake `foguinho_base`, compilado para a
placa STM32F411CEU6 Black Pill. Ela reutiliza a plataforma STM32F411 comum, o
driver oficial da Semtech para o SX1278 e a pilha oficial ST USB Device CDC.

O fluxo principal em `Src/base_station/main.c` inicializa HAL, clock, GPIO, SPI2,
USB CDC e radio LoRa. Em seguida, processa continuamente os pacotes de radio e
encaminha frames do protocolo pela USB nativa, mantendo a API interna
`usb_bridge_*` para o restante da base.

Builds principais:

```powershell
cmake --preset Debug
cmake --build build\Debug --target foguinho
cmake --build build\Debug --target foguinho_base
```

O STM32CubeMX 6.17.0 foi usado para manter o `.ioc`. Nesta maquina, a execucao
em lote pelo script `scripts/generate-cubemx.ps1` abriu o processo Java, mas nao
concluiu a geracao automaticamente; por isso a validacao final foi feita com o
CMake/Ninja/GCC oficiais instalados pela extensao STM32.
