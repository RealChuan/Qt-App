# ASCII only on purpose: Windows PowerShell 5.1 decodes BOM-less scripts with
# the ANSI code page, so non-ASCII text would break parsing.

$ErrorActionPreference = 'Stop'

$AppName = "Qt-App"

Set-Location $PSScriptRoot\..\..
$ProjectDir = (Get-Location).Path
Write-Host "Project: $ProjectDir"

Import-Module (Join-Path $ProjectDir "packaging\windows\utils.ps1")

# Version: single source of truth is packaging/version.txt
$Version = Get-ProjectVersion
$BuildDate = Get-Date -Format 'yyyyMMdd'
Write-Host "Version: $Version ($BuildDate)"

# Font extraction and the 7z portable archive both need 7z; check early so the
# failure does not show up after a full build.
if (-not (Get-Command 7z.exe -ErrorAction SilentlyContinue)) {
    throw "7z.exe not found (needed for font extraction and the 7z archive), install 7-Zip and add it to PATH"
}

# ============================================================
# Environment (CI injects these; locally the defaults are fine)
# ============================================================
$VcpkgRootDefault = if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } else { 'C:\vcpkg' }

# ============================================================
# Build configuration center
#   Each architecture is one complete description; field names match the
#   helper function parameters.
#   - QtDir: CI reads QT_ROOT_X64 / QT_ROOT_ARM64, locally falls back to QT_ROOT_DIR
#   - DevShell / Generator / BuildType: may differ per architecture
# ============================================================
$BuildList = @(
    @{
        Name        = 'x64'
        Enabled     = $true
        Arch        = 'x64'
        HostArch    = 'x64'
        Generator   = 'Ninja'
        BuildType   = 'RelWithDebInfo'
        OpenSslTrip = 'x64-windows'   # dynamic triplet: only this one has openssl DLLs
        QtDir       = if ($env:QT_ROOT_X64) { $env:QT_ROOT_X64 } else { $env:QT_ROOT_DIR }
        DevShell    = 'packaging\windows\Enter-VsDevShell.ps1'
        BuildDir    = 'build\x64'
        PacketDir   = 'packaging\packet\x64'
        InnoArgs    = "/DMyArch=x64 /DMyAppVersion=$Version /DMyAppFileVersion=$Version.0"
    },
    @{
        Name        = 'arm64'
        Enabled     = $false  # arm64 is not enabled in CI yet; set to $true locally
        Arch        = 'arm64'
        HostArch    = 'x64'          # cross-compile arm64 from an x64 host
        Generator   = 'Ninja'
        BuildType   = 'RelWithDebInfo'
        OpenSslTrip = 'arm64-windows'   # dynamic triplet: only this one has openssl DLLs
        QtDir       = if ($env:QT_ROOT_ARM64) { $env:QT_ROOT_ARM64 } else { $env:QT_ROOT_DIR }
        DevShell    = 'packaging\windows\Enter-VsDevShell.ps1'
        BuildDir    = 'build\arm64'
        PacketDir   = 'packaging\packet\arm64'
        InnoArgs    = "/DMyArch=arm64 /DMyAppVersion=$Version /DMyAppFileVersion=$Version.0"
    }
)

$ArchFilter = if ($env:PKG_ARCH) { $env:PKG_ARCH } else { 'all' }
$ReleasesDir = Join-Path $ProjectDir "packaging\releases"
New-Item -ItemType Directory -Path $ReleasesDir -Force | Out-Null

# ============================================================
# Prepare shared resources: fonts and the Inno Chinese translation (once)
# ============================================================
$FontStage = Join-Path $ProjectDir "packaging\packet\_fonts"
$FontCount = 0
if (Test-Path $FontStage) {
    $FontCount = (Get-ChildItem $FontStage -Recurse -File | Measure-Object).Count
}
if ($FontCount -gt 0) {
    Write-Host "`n==================== Reusing fonts ($FontCount files) ====================" -ForegroundColor Green
    Write-Host "  $FontStage (delete this directory to download again)" -ForegroundColor DarkGray
}
else {
    Remove-SafeItem $FontStage
    Write-Host "`n==================== Downloading fonts ====================" -ForegroundColor Green
    Invoke-FontDownload -DestDir $FontStage
}

Write-Host "`n==================== Preparing Inno Chinese translation ====================" -ForegroundColor Green
Get-InnoChineseTranslation -DestDir (Join-Path $ProjectDir 'packaging\windows') | Out-Null

# ============================================================
# Per architecture: explicit flow (clean -> DevShell -> configure -> build -> install -> ...)
# ============================================================
foreach ($cfg in $BuildList) {
    if (-not $cfg.Enabled) {
        Write-Host "Skipping [$($cfg.Name)] (Enabled=false)" -ForegroundColor Yellow
        continue
    }
    if ($ArchFilter -ne 'all' -and $ArchFilter -ne $cfg.Name) {
        Write-Host "Skipping [$($cfg.Name)] (PKG_ARCH=$ArchFilter)" -ForegroundColor Yellow
        continue
    }

    if (-not $cfg.QtDir -or -not (Test-Path $cfg.QtDir)) {
        throw "[$($cfg.Name)] Qt directory not found (QtDir='$($cfg.QtDir)'). Set QT_ROOT_$($cfg.Name.ToUpper()) or QT_ROOT_DIR."
    }

    $buildDir = Join-Path $ProjectDir $cfg.BuildDir
    $packetDir = Join-Path $ProjectDir $cfg.PacketDir
    $devShell = Join-Path $ProjectDir $cfg.DevShell
    # windeployqt only understands the dynamic triplet directory (the static-md
    # triplet linked by the build ships .lib files only)
    $opensslRoot = Join-Path $VcpkgRootDefault "installed\$($cfg.OpenSslTrip)"

    Write-Host "`n==================== Building $($cfg.Name) ====================" -ForegroundColor Green
    Write-Host "  QtDir      : $($cfg.QtDir)"
    Write-Host "  OpenSSL    : $opensslRoot"
    Write-Host "  Arch/Host  : $($cfg.Arch) / $($cfg.HostArch)"
    Write-Host "  Generator  : $($cfg.Generator)"
    Write-Host "  BuildType  : $($cfg.BuildType)"

    # 1. Clean the build directory (flow preparation)
    Remove-SafeItem $buildDir

    # 2. Enter the MSVC developer environment (flow; explicit, not via utils)
    Write-Host "[$($cfg.Name)] Entering DevShell (arch=$($cfg.Arch), host_arch=$($cfg.HostArch))" -ForegroundColor Cyan
    & $devShell -Arch $cfg.Arch -HostArch $cfg.HostArch
    if ($LASTEXITCODE) { throw "[$($cfg.Name)] Enter-VsDevShell failed" }

    # 3. configure (tool)
    Invoke-CMakeConfigure `
        -SourceDir  $ProjectDir `
        -BuildDir   $buildDir `
        -Generator  $cfg.Generator `
        -BuildType  $cfg.BuildType `
        -QtDir      $cfg.QtDir

    # 4. build (tool)
    Invoke-CMakeBuild -BuildDir $buildDir -BuildType $cfg.BuildType

    # 5. Recreate the packet directory (flow preparation)
    Remove-SafeItem $packetDir
    New-Item -ItemType Directory -Path $packetDir -Force | Out-Null

    # 6. install -> packet (single command, no wrapper needed)
    cmake --install $buildDir --prefix $packetDir
    if ($LASTEXITCODE) { throw "[$($cfg.Name)] cmake install failed" }

    # 7. Merge fonts (flow)
    $fontDst = Join-Path $packetDir "resources\fonts"
    New-Item -ItemType Directory -Path $fontDst -Force | Out-Null
    Copy-Item "$FontStage\*" -Destination $fontDst -Recurse -Force

    # 8. Lift the application plugins to the packet root (flow)
    $appPluginDir = Join-Path $packetDir "plugins\$($AppName.ToLower())"
    if (Test-Path $appPluginDir) {
        Copy-Item "$appPluginDir\*" -Destination $packetDir -Recurse -Force
    }

    # 9. windeployqt (tool)
    Invoke-CMakeDeployment `
        -QtDir        $cfg.QtDir `
        -PacketDir    $packetDir `
        -OpenSslRoot  $opensslRoot `
        -AppPluginDir "plugins\$($AppName.ToLower())"

    # 10. Drop debug / intermediate artifacts (flow)
    foreach ($pat in @("*.ilk", "*.pdb", "*plugin*.dll")) {
        Remove-SafeItem (Join-Path $packetDir $pat)
    }

    # 11. Code signing - disabled for now; uncomment the block below to enable
    # Invoke-SignBinaries -Files @(
    #     (Join-Path $packetDir "$($AppName).exe")
    # )

    # 12. Show the packet contents once: 7z and the installer below are both
    #     built from this directory and do not change it.
    Show-PacketTree -Path $packetDir

    # 13. 7z portable archive (kept first: it archives the directory right after
    #     deployment, before any other tool touches it)
    New-SevenZipArchive `
        -SourceDir   $packetDir `
        -Destination (Join-Path $ReleasesDir "${AppName}_${Version}_${BuildDate}_windows_$($cfg.Arch).7z")

    # 14. Inno Setup (tool)
    INNO_Setup_Package `
        -Path      (Join-Path $ProjectDir 'packaging\windows\app.iss') `
        -ExtraArgs $cfg.InnoArgs

    Set-Location $ProjectDir
}

Write-Host "`nAll done." -ForegroundColor Green
