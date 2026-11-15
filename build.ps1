#Requires -Version 5.0

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

Write-Host ""
Write-Host " ============================================" -ForegroundColor Cyan
Write-Host "  MadMcMan's BCE - Full Build" -ForegroundColor Cyan
Write-Host " ============================================" -ForegroundColor Cyan
Write-Host "  Tip: see README.md for submodule setup and build options" -ForegroundColor DarkGray
Write-Host ""

# ── Clean Bin ─────────────────────────────────────────────
if (Test-Path "build-lvgl\bin") {
    Write-Host "[1/3] Cleaning bin directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build-lvgl\bin"
}

# ── Configure ─────────────────────────────────────────────
Write-Host "[2/3] Configuring CMake..." -ForegroundColor Yellow
cmake -B "build-lvgl" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release `
    -DCONFIG_LV_BUILD_EXAMPLES=OFF `
    -DCONFIG_LV_BUILD_DEMOS=OFF `
    -DCONFIG_LV_USE_THORVG_INTERNAL=OFF .
if ($LASTEXITCODE -ne 0) {
    Write-Host "`n [ERROR] CMake configure failed!" -ForegroundColor Red
    exit 1
}
Write-Host " [OK] Configure done.`n" -ForegroundColor Green

# ── Build ──────────────────────────────────────────────────
Write-Host "[2/3] Building all targets...`n" -ForegroundColor Yellow

$jobs = @(
    @{ Name = "Standalone EXE"; Target = "MadMcManBCE-jack" },
    @{ Name = "VST3"; Target = "MadMcManBCE-vst3" },
    @{ Name = "CLAP"; Target = "MadMcManBCE-clap" }
)

$results = @{}

foreach ($job in $jobs) {
    Write-Host " Building $($job.Name)..." -ForegroundColor White
    cmake --build "build-lvgl" --config Release --target $job.Target -j $env:NUMBER_OF_PROCESSORS
    $results[$job.Name] = $LASTEXITCODE -eq 0
    Write-Host ""
}

# ── Summary ────────────────────────────────────────────────
Write-Host " ============================================" -ForegroundColor Cyan
Write-Host "[3/3] Build Summary" -ForegroundColor Cyan
Write-Host " ============================================`n" -ForegroundColor Cyan

$checks = @(
    @{ Label = "MadMcManBCE.exe"; Path = "build-lvgl\bin\MadMcManBCE.exe" },
    @{ Label = "MadMcManBCE.vst3"; Path = "build-lvgl\bin\MadMcManBCE.vst3" },
    @{ Label = "MadMcManBCE.clap"; Path = "build-lvgl\bin\MadMcManBCE.clap" }
)

$allOk = $true
foreach ($c in $checks) {
    if (Test-Path $c.Path) {
        Write-Host "  [OK]  $($c.Label)" -ForegroundColor Green
    }
    else {
        Write-Host "  [--]  $($c.Label)  - NOT FOUND" -ForegroundColor Red
        $allOk = $false
    }
}

Write-Host ""
Write-Host "  Output: $PSScriptRoot\build-lvgl\bin\" -ForegroundColor DarkGray
Write-Host ""

if ($allOk) {
    Write-Host " All targets built successfully!" -ForegroundColor Green
}
else {
    Write-Host " One or more targets failed. Check output above." -ForegroundColor Red
}

Write-Host ""
