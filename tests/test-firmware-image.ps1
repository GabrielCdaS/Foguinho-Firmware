param(
    [ValidateSet("Debug", "Release")]
    [string]$Preset = "Debug"
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$elf = Join-Path $root "build\$Preset\foguinho.elf"
$toolchain = Join-Path $env:LOCALAPPDATA "stm32cube\bundles\gnu-tools-for-stm32\14.3.1+st.2\bin"
$nm = Join-Path $toolchain "arm-none-eabi-nm.exe"
$objdump = Join-Path $toolchain "arm-none-eabi-objdump.exe"
$objcopy = Join-Path $toolchain "arm-none-eabi-objcopy.exe"

foreach ($path in @($elf, $nm, $objdump, $objcopy)) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Arquivo necessario nao encontrado: $path"
    }
}

$symbols = & $nm -n $elf
$requiredSymbols = @(
    "SysTick_Handler",
    "USART2_IRQHandler",
    "cubemx_platform_init",
    "HAL_UART_MspInit",
    "SX1276Init"
)

foreach ($symbol in $requiredSymbols) {
    if (-not ($symbols | Select-String -SimpleMatch " $symbol")) {
        throw "Simbolo obrigatorio ausente do firmware: $symbol"
    }
}

$vectorTable = & $objdump -s -j .isr_vector $elf
if (-not $vectorTable) {
    throw "Tabela de vetores .isr_vector ausente."
}

$symbolAddresses = @{}
foreach ($line in $symbols) {
    if ($line -match '^([0-9a-fA-F]+)\s+\w\s+(\S+)$') {
        $symbolAddresses[$Matches[2]] = [Convert]::ToUInt32($Matches[1], 16)
    }
}

$vectorBinary = Join-Path ([System.IO.Path]::GetTempPath()) ("foguinho-vectors-{0}.bin" -f [guid]::NewGuid())
try {
    & $objcopy -O binary --only-section=.isr_vector $elf $vectorBinary
    if ($LASTEXITCODE -ne 0) {
        throw "Falha ao extrair a tabela de vetores."
    }
    $vectors = [System.IO.File]::ReadAllBytes($vectorBinary)
    foreach ($entry in @(
        @{ Name = "SysTick_Handler"; Index = 15 },
        @{ Name = "USART2_IRQHandler"; Index = 54 }
    )) {
        $actual = [BitConverter]::ToUInt32($vectors, $entry.Index * 4)
        $expected = $symbolAddresses[$entry.Name] -bor 1
        if ($actual -ne $expected) {
            throw ("Vetor {0} aponta para 0x{1:X8}, esperado 0x{2:X8}." -f
                $entry.Name, $actual, $expected)
        }
    }
} finally {
    Remove-Item -LiteralPath $vectorBinary -Force -ErrorAction SilentlyContinue
}

$mspSource = Get-Content -Raw (Join-Path $root "firmware\Core\Src\stm32f4xx_hal_msp.c")
if ($mspSource -notmatch 'HAL_NVIC_EnableIRQ\(USART2_IRQn\)') {
    throw "A interrupcao USART2 nao esta habilitada no MSP."
}

$cubeSource = Get-Content -Raw (Join-Path $root "firmware\Core\Src\main.c")
foreach ($safetyPattern in @(
    'PYRO_MAIN_Pin\|PYRO_EMERGENCY_Pin,\s*GPIO_PIN_RESET',
    'BMP388_CS_Pin\|SD_CS_Pin,\s*GPIO_PIN_SET',
    'ICM20948_CS_Pin\|SX1278_RESET_Pin\|SX1278_CS_Pin,\s*GPIO_PIN_SET'
)) {
    if ($cubeSource -notmatch $safetyPattern) {
        throw "Estado inicial de seguranca ausente: $safetyPattern"
    }
}

Write-Host "firmware image checks passed ($Preset)"
