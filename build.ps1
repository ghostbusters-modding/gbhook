# gbhook: a mod loader for Ghostbusters: The Video Game Remastered
# Copyright (C) 2026 Colin Sullivan and contributors
# SPDX-License-Identifier: GPL-2.0-only
<#
.SYNOPSIS
    Builds gbhook and optionally installs it into the game folder.

.DESCRIPTION
    Needs Visual Studio 2022 or 2026 with "Desktop development with C++",
    or the standalone Build Tools. No IDE required.

.EXAMPLE
    .\build.ps1
    Build Release x64.

.EXAMPLE
    .\build.ps1 -Install
    Build, then copy dinput8.dll next to ghost.exe (Steam library auto-detected).
#>
[CmdletBinding()]
param(
    [ValidateSet('Release','Debug')][string]$Configuration = 'Release',
    [string]$GameDir,
    [switch]$Install,
    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path

function Find-MSBuild {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $found = & $vswhere -latest -prerelease -products '*' `
                            -requires Microsoft.Component.MSBuild `
                            -find 'MSBuild\**\Bin\amd64\MSBuild.exe' 2>$null
        if ($found) { return @($found)[0] }
    }
    $fallback = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($fallback) { return $fallback.Source }
    throw 'MSBuild not found. Install the VS Build Tools with the C++ workload.'
}

function Find-GameDir {
    $leaf = 'steamapps\common\Ghostbusters The Video Game Remastered'
    foreach ($r in @((Join-Path ${env:ProgramFiles(x86)} 'Steam'),
                     (Join-Path $env:ProgramFiles 'Steam'))) {
        $vdf = Join-Path $r 'steamapps\libraryfolders.vdf'
        if (Test-Path $vdf) {
            foreach ($m in [regex]::Matches((Get-Content $vdf -Raw), '"path"\s+"([^"]+)"')) {
                $c = Join-Path ($m.Groups[1].Value -replace '\\\\','\') $leaf
                if (Test-Path (Join-Path $c 'ghost.exe')) { return $c }
            }
        }
        $c = Join-Path $r $leaf
        if (Test-Path (Join-Path $c 'ghost.exe')) { return $c }
    }
    return $null
}

$msbuild = Find-MSBuild
$proj    = Join-Path $root 'GbHook.vcxproj'

if ($Clean) { & $msbuild $proj /t:Clean -p:Configuration=$Configuration -p:Platform=x64 -v:minimal -nologo }
Write-Host "building gbhook..." -ForegroundColor Cyan
& $msbuild $proj -p:Configuration=$Configuration -p:Platform=x64 -v:minimal -nologo
if ($LASTEXITCODE -ne 0) { throw 'gbhook failed to build' }

if (-not $Install) { return }

if (-not $GameDir) { $GameDir = Find-GameDir }
if (-not $GameDir -or -not (Test-Path (Join-Path $GameDir 'ghost.exe'))) {
    throw 'Game directory not found. Pass -GameDir "<path to ghost.exe>".'
}

# The loaded DLL can be renamed but not overwritten while the game is running.
if (Get-Process ghost -ErrorAction SilentlyContinue) {
    throw 'ghost.exe is running. Close the game before installing.'
}

$src = Join-Path $root "build\x64\$Configuration\dinput8.dll"
$dst = Join-Path $GameDir 'dinput8.dll'
Copy-Item $src $dst -Force
Write-Host ("installed dinput8.dll -> {0}  ({1})" -f $dst,
            (Get-FileHash $dst -Algorithm MD5).Hash.Substring(0,8)) -ForegroundColor Green

Write-Host "`nLaunch the game and read $GameDir\gbhook.log" -ForegroundColor Yellow
