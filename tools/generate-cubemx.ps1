param(
    [string]$OutputDirectory = (Join-Path $PSScriptRoot "..\build\cubemx-generated"),
    [string]$CubeMxExecutable
)

$ErrorActionPreference = "Stop"

$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
$iocPath = Join-Path $projectRoot "Foguinho.ioc"
$outputPath = [System.IO.Path]::GetFullPath($OutputDirectory)
$generationRoot = [System.IO.Path]::GetFullPath((Join-Path $projectRoot "build"))
$generationPrefix = $generationRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar) +
    [System.IO.Path]::DirectorySeparatorChar

if (-not $outputPath.StartsWith($generationPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "A pasta de geracao deve permanecer dentro de: $generationRoot"
}

if (-not $CubeMxExecutable) {
    $candidates = @(
        (Join-Path $env:LOCALAPPDATA "Programs\STM32CubeMX\STM32CubeMX.exe"),
        (Join-Path ${env:ProgramFiles} "STMicroelectronics\STM32Cube\STM32CubeMX\STM32CubeMX.exe")
    )
    $CubeMxExecutable = $candidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
}

if (-not $CubeMxExecutable -or -not (Test-Path -LiteralPath $CubeMxExecutable)) {
    throw "STM32CubeMX.exe nao encontrado. Informe -CubeMxExecutable."
}

if (Test-Path -LiteralPath $outputPath) {
    Remove-Item -LiteralPath $outputPath -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $outputPath | Out-Null
$commandFile = Join-Path ([System.IO.Path]::GetTempPath()) ("foguinho-cubemx-{0}.txt" -f [guid]::NewGuid())
$generationIocPath = Join-Path $outputPath "Foguinho.ioc"

try {
    Copy-Item -LiteralPath $iocPath -Destination $generationIocPath -Force
    @(
        "config load `"$generationIocPath`""
        "project generate `"$outputPath`""
        "exit"
    ) | Set-Content -LiteralPath $commandFile -Encoding ascii

    $cubeMxDirectory = Split-Path -Parent $CubeMxExecutable
    $javaExecutable = Join-Path $cubeMxDirectory "jre\bin\java.exe"
    if (-not (Test-Path -LiteralPath $javaExecutable)) {
        throw "Runtime Java do STM32CubeMX nao encontrado em: $javaExecutable"
    }

    & $javaExecutable `
        "-Djavax.net.ssl.trustStoreType=WINDOWS-ROOT" `
        "-Dsun.java2d.d3d=false" `
        "--add-exports" "java.desktop/sun.awt=ALL-UNNAMED" `
        "--add-opens" "java.desktop/java.awt=ALL-UNNAMED" `
        "-Dfile.encoding=UTF8" `
        "-classpath" "$CubeMxExecutable;anything" `
        "com.st.microxplorer.maingui.STM32CubeMX" `
        "-q" $commandFile
    if ($LASTEXITCODE -ne 0) {
        throw "STM32CubeMX encerrou com o codigo $LASTEXITCODE."
    }
} finally {
    Remove-Item -LiteralPath $commandFile -Force -ErrorAction SilentlyContinue
}

Write-Host "Codigo CubeMX gerado em: $outputPath"
