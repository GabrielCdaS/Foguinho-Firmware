$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$outputDir = Join-Path $root "build\host-tests"
$output = Join-Path $outputDir "test_firmware_logic.exe"
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

$compile = @(
    "cl /nologo /std:c11 /W4 /DHOST_TEST"
    "/I`"$root\Inc\common\inc`""
    "/I`"$root\Inc\rocket\inc`""
    "`"$PSScriptRoot\test_firmware_logic.c`""
    "`"$root\Src\common\src\crc16.c`""
    "`"$root\Src\common\src\flight_defs.c`""
    "`"$root\Src\rocket\src\command_executor.c`""
    "`"$root\Src\rocket\src\gps.c`""
    "`"$root\Src\rocket\src\telemetry.c`""
    "/Fe:test_firmware_logic.exe"
) -join " "

$command = "`"$vsDevCmd`" -arch=x64 -host_arch=x64 && cd /d `"$outputDir`" && $compile"

cmd.exe /d /s /c $command
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& $output
exit $LASTEXITCODE
