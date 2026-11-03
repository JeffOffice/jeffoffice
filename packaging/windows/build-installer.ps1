# Builds packaging\windows\JeffOffice-Setup.exe from dist\JeffOffice with Inno Setup (the ISCC on the Windows runner).
#   packaging\windows\build-installer.ps1 -Version 0.20.0 [-Write] [-Crunch]
# -Write and -Crunch are the WITH_WRITE and WITH_CRUNCH build defines (the workflow passes them when SHIP_WRITE and
# SHIP_CRUNCH are "true"). Run from the repository's top folder.
param([Parameter(Mandatory)][string]$Version, [switch]$Write, [switch]$Crunch)
$ErrorActionPreference = 'Stop'
# (On the runner, ISCC.exe on the PATH is only a Chocolatey shim, which has no version of its own.)
$iscc = Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'
if (-not (Test-Path $iscc)) { $iscc = (Get-Command ISCC.exe -ErrorAction SilentlyContinue).Source }
if (-not (Test-Path $iscc)) { throw "Inno Setup's ISCC.exe was not found" }
# The version goes in the log (ISCC prints "Compiler engine version: Inno Setup 6.x.y" when run with no arguments). The
# artwork assumes the image sizes of Inno Setup 6.6 and later.
$banner = (& $iscc 2>&1 | Out-String)
$m = [regex]::Match($banner, 'Compiler engine version: Inno Setup ([0-9]+\.[0-9]+(\.[0-9]+)?)')
Write-Host "ISCC: $iscc"
Write-Host ($banner.Split("`n") | Select-Object -First 3)
if (-not $m.Success) { throw "could not read the Inno Setup version from ISCC" }
$ver = $m.Groups[1].Value
Write-Host "Inno Setup $ver"
if ([version]$ver -lt [version]'6.6.0') { throw "Inno Setup $ver is too old: 6.6.0 or later is needed" }
# The license page shows the GPL as paragraphs, not the file's hard-wrapped lines.
python tools/reflow_license.py --rtf LICENSE packaging/windows/license.rtf
if ($LASTEXITCODE -ne 0) { throw 'reflowing the license failed' }
$defs = @("/DVERSION=$Version")
if ($Write) { $defs += '/DWITH_WRITE' }
if ($Crunch) { $defs += '/DWITH_CRUNCH' }
& $iscc @defs packaging/windows/installer.iss
if ($LASTEXITCODE -ne 0) { throw "ISCC failed ($LASTEXITCODE)" }
if (-not (Test-Path packaging/windows/JeffOffice-Setup.exe)) { throw 'JeffOffice-Setup.exe was not made' }
