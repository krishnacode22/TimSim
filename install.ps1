# TimSim Windows Installer Script
$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "   Installing TimSim for Windows..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Define installation directory in user's profile (no admin rights needed)
$InstallDir = "$HOME\.timsim"
if (!(Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
}

$ExePath = "$InstallDir\timsim.exe"

Write-Host "Downloading latest TimSim executable..."
$ReleaseUrl = "https://github.com/krishnacode22/TimSim/releases/latest/download/timsim.exe"

try {
    Invoke-WebRequest -Uri $ReleaseUrl -OutFile $ExePath
} catch {
    Write-Host "Error: Failed to download timsim.exe. Make sure you have published a GitHub Release with the binary attached!" -ForegroundColor Red
    exit 1
}

# Add installation directory to User PATH environment variable if not already present
$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($UserPath -notlike "*$InstallDir*") {
    [Environment]::SetEnvironmentVariable("Path", "$UserPath;$InstallDir", "User")
    Write-Host "Added $InstallDir to your Windows PATH." -ForegroundColor Green
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " TimSim installed successfully!" -ForegroundColor Green
Write-Host " Close this terminal, open a NEW Command Prompt or PowerShell, and type 'timsim'." -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
