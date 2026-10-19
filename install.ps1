# TimSim installer for Windows (PowerShell 5.1 or newer).
#
#   irm https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.ps1 | iex
#
# It downloads timsim.exe from the latest GitHub release into
# %LOCALAPPDATA%\Programs\TimSim and adds that folder to your user PATH.
# No administrator rights are needed.
#
# Optional environment variables (set them before running the command):
#   $env:TIMSIM_INSTALL_DIR  = "C:\tools\timsim"   install somewhere else
#   $env:TIMSIM_DOWNLOAD_URL = "<url of timsim.exe>"  download from somewhere else
#   $env:TIMSIM_UNINSTALL    = "1"                  remove TimSim instead of installing
#
# Note: this script never calls "exit", because under "irm | iex" that would
# close your terminal window. Problems are reported with "throw" instead.

& {
    $ErrorActionPreference = 'Stop'
    $ProgressPreference = 'SilentlyContinue'   # makes downloads much faster on Windows PowerShell 5.1

    $defaultUrl = 'https://github.com/krishnacode22/TimSim/releases/latest/download/timsim.exe'
    $url = if ($env:TIMSIM_DOWNLOAD_URL) { $env:TIMSIM_DOWNLOAD_URL } else { $defaultUrl }
    $dir = if ($env:TIMSIM_INSTALL_DIR) { $env:TIMSIM_INSTALL_DIR } else { Join-Path $env:LOCALAPPDATA 'Programs\TimSim' }
    $exe = Join-Path $dir 'timsim.exe'

    # ---- user PATH helpers (the user PATH is stored in the registry) ----
    $cleanDir = $dir.TrimEnd('\')
    $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
    if ($null -eq $userPath) { $userPath = '' }
    $entries = @($userPath -split ';' | Where-Object { $_ -ne '' })
    $onPath = $false
    foreach ($e in $entries) { if ($e.TrimEnd('\') -ieq $cleanDir) { $onPath = $true } }

    Write-Host '========================================'

    # ---- uninstall mode ----
    if ($env:TIMSIM_UNINSTALL) {
        Write-Host '   Removing TimSim...'
        Write-Host '========================================'
        if (Test-Path $dir) { Remove-Item -Recurse -Force $dir -ErrorAction SilentlyContinue }
        $kept = @($entries | Where-Object { $_.TrimEnd('\') -ine $cleanDir })
        [Environment]::SetEnvironmentVariable('Path', ($kept -join ';'), 'User')
        Write-Host " TimSim removed from $dir"
        Write-Host '========================================'
        return
    }

    Write-Host '   Installing TimSim...'
    Write-Host '========================================'

    if (-not [Environment]::Is64BitOperatingSystem) {
        throw 'TimSim needs 64-bit Windows.'
    }

    # ---- download to a temporary file first ----
    $tmp = Join-Path ([IO.Path]::GetTempPath()) ('timsim-' + [guid]::NewGuid().ToString('N') + '.exe')
    Write-Host 'Downloading timsim.exe ...'
    try {
        [Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12
    } catch { }
    try {
        Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $tmp
    } catch {
        throw "Could not download TimSim from $url ($($_.Exception.Message)). Is there a release with a file named timsim.exe?"
    }

    try {
        # ---- check that it really is a Windows program that runs ----
        $bytes = [IO.File]::ReadAllBytes($tmp)
        if ($bytes.Length -lt 100000 -or $bytes[0] -ne 0x4D -or $bytes[1] -ne 0x5A) {
            throw 'The downloaded file is not a valid timsim.exe.'
        }
        $version = (& $tmp --version) 2>&1 | Out-String
        if ($LASTEXITCODE -ne 0 -or $version -notmatch 'timsim') {
            throw 'The downloaded timsim.exe does not run on this computer.'
        }
        $version = $version.Trim()

        # ---- install (a running timsim.exe can be renamed, not overwritten, so rename the old one) ----
        New-Item -ItemType Directory -Force -Path $dir | Out-Null
        $old = "$exe.old"
        if (Test-Path $old) { Remove-Item -Force $old -ErrorAction SilentlyContinue }
        if (Test-Path $exe) { Move-Item -Force $exe $old }
        Move-Item -Force $tmp $exe
        if (Test-Path $old) { Remove-Item -Force $old -ErrorAction SilentlyContinue }
    } finally {
        if (Test-Path $tmp) { Remove-Item -Force $tmp -ErrorAction SilentlyContinue }
    }

    # ---- add to PATH (this window and all future ones) ----
    $pathNote = ''
    if (-not $onPath) {
        $newPath = if ($userPath -eq '') { $cleanDir } else { $userPath.TrimEnd(';') + ';' + $cleanDir }
        [Environment]::SetEnvironmentVariable('Path', $newPath, 'User')
        $pathNote = 'added to your PATH'
    }
    if (($env:Path -split ';' | ForEach-Object { $_.TrimEnd('\') }) -inotcontains $cleanDir) {
        $env:Path = $env:Path.TrimEnd(';') + ';' + $cleanDir
    }

    Write-Host '========================================'
    Write-Host " TimSim installed successfully! ($version)"
    $location = " Location: $exe $pathNote".TrimEnd()
    Write-Host $location
    Write-Host " Type 'timsim' to run it, or 'timsim --help' for options."
    Write-Host " Remove it later:  `$env:TIMSIM_UNINSTALL=1; irm https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.ps1 | iex"
    Write-Host '========================================'
}
