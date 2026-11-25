# Build do PSPalatro via WSL (sem sudo necessário).
# Uso: powershell -ExecutionPolicy Bypass -File build-pspalatro.ps1
$ErrorActionPreference = 'Stop'

$win = $PSScriptRoot
$drive = $win[0].ToString().ToLower()
$rest = $win.Substring(2) -replace '\\', '/'
$wslPath = "/mnt/$drive$rest"

$wslHome = (wsl -d Ubuntu -- sh -lc 'printf %s "$HOME"').Trim()

$envArgs = "HOME=$wslHome PSPDEV=$wslHome/pspdev LD_LIBRARY_PATH=$wslHome/sysroot/usr/lib/x86_64-linux-gnu PATH=$wslHome/sysroot/usr/bin:$wslHome/pspdev/bin:/usr/local/bin:/usr/bin:/bin"

wsl -d Ubuntu -- env -i $envArgs.Split(' ') bash -c ('set -e; cd "' + $wslPath + '"; make clean >/dev/null; make')
