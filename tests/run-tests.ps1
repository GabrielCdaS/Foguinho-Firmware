$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$outputDir = Join-Path $root "build\host-tests"
$rocketOutput = Join-Path $outputDir "test_firmware_logic.exe"
$baseOutput = Join-Path $outputDir "test_base_station.exe"
$commonInc = Join-Path $root "Inc\common"
$rocketInc = Join-Path $root "Inc\rocket"
$baseStationInc = Join-Path $root "Inc\base_station"
$platformInc = Join-Path $root "Inc\platform\stm32f411"
$commonSrc = Join-Path $root "Src\common"
$rocketSrc = Join-Path $root "Src\rocket"
$baseStationSrc = Join-Path $root "Src\base_station"
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $vsWhere)) {
    throw "Visual Studio Installer (vswhere.exe) not found."
}

$installationPath = & $vsWhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath
$vsDevCmd = Join-Path $installationPath "Common7\Tools\VsDevCmd.bat"

if (-not $installationPath -or -not (Test-Path $vsDevCmd)) {
    throw "Visual Studio C++ build tools not found."
}

New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

$compileRocket = @(
    "cl /nologo /std:c11 /W4 /DHOST_TEST"
    "/I`"$commonInc`""
    "/I`"$platformInc`""
    "/I`"$rocketInc`""
    "`"$PSScriptRoot\test_firmware_logic.c`""
    "`"$commonSrc\crc16.c`""
    "`"$commonSrc\flight_defs.c`""
    "`"$rocketSrc\command_executor.c`""
    "`"$rocketSrc\gps.c`""
    "`"$rocketSrc\telemetry.c`""
    "/Fe:test_firmware_logic.exe"
) -join " "

$compileBase = @(
    "cl /nologo /std:c11 /W4 /DHOST_TEST"
    "/I`"$commonInc`""
    "/I`"$baseStationInc`""
    "`"$PSScriptRoot\test_base_station.c`""
    "`"$commonSrc\crc16.c`""
    "`"$baseStationSrc\telemetry_parser.c`""
    "`"$baseStationSrc\command.c`""
    "`"$baseStationSrc\protocol.c`""
    "/Fe:test_base_station.exe"
) -join " "

$command = "`"$vsDevCmd`" -arch=x64 -host_arch=x64 && cd /d `"$outputDir`" && $compileRocket && $compileBase"

cmd.exe /d /s /c $command
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& $rocketOutput
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& $baseOutput
exit $LASTEXITCODE
