# Drivers oficiais

O firmware usa dependências oficiais fixadas como submódulos Git:

- `Drivers/STM32F4xx_HAL_Driver`: HAL/LL oficial da ST.
- `Drivers/CMSIS/Device/ST/STM32F4xx`: definições CMSIS do STM32F4.
- `Drivers/CMSIS/Core`: CMSIS Core oficial da Arm.
- `Middlewares/LoRaMac-node`: driver SX1276/77/78/79 oficial da Semtech.

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

Os arquivos em `Core/` foram derivados da geração oficial e fornecem os
handles HAL e a inicialização da plataforma. `Src/platform/stm32f411/stm32f411_hw.c`
preserva a API interna do projeto para o foguete e para a base, mas usa esses
handles em vez de configurar registradores ou periféricos localmente.

## Regeneração com STM32CubeMX

O projeto foi validado com STM32CubeMX 6.17.0 e STM32Cube FW_F4 V1.28.3.
O pacote completo oficial também foi instalado no repositório local do
CubeMX em `~/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.3`.

Para gerar uma cópia limpa e isolada:

```powershell
.\tools\generate-cubemx.ps1
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
em lote pelo script `tools/generate-cubemx.ps1` abriu o processo Java, mas nao
concluiu a geracao automaticamente; por isso a validacao final foi feita com o
CMake/Ninja/GCC oficiais instalados pela extensao STM32.
