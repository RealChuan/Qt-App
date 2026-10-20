# ============================================================
# cmake packaging helper library (Windows)
#   Constraint: pure functions only - no directory cleanup, no directory
#   creation, no DevShell entry. All orchestration lives in package.ps1.
#
#   ASCII only on purpose: Windows PowerShell 5.1 decodes BOM-less scripts
#   with the ANSI code page, so non-ASCII text would break parsing.
# ============================================================

# packaging/ directory: version.txt, fonts.url and other shared files live here
$script:PackagingDir = Split-Path $PSScriptRoot -Parent

# Read the project version (single source of truth: packaging/version.txt)
function Get-ProjectVersion {
    $file = Join-Path $script:PackagingDir 'version.txt'
    if (-not (Test-Path $file)) { throw "Missing version file: $file" }
    $ver = (Get-Content $file -Raw).Trim()
    if (-not $ver) { throw "Version file is empty: $file" }
    return $ver
}

# -------------------- signing config (currently disabled) --------------------
$SignConfig = @{
    Sha1Thumbprint = '************************************'
    TimestampUrl   = 'http://timestamp.digicert.com'
    SignToolPath   = ''
}
$script:_CachedSignTool = $null

# ============================================================
# Generic
# ============================================================
function Remove-SafeItem {
    param([Parameter(Mandatory)]$Path)
    try {
        $resolved = Resolve-Path $Path -ErrorAction SilentlyContinue
        if (-not $resolved) { return }
        foreach ($r in $resolved) {
            if (Test-Path $r) {
                Remove-Item $r -Recurse -Force -Verbose
                Write-Host "Removed: $($r.Path)"
            }
        }
    }
    catch { Write-Warning "Remove-SafeItem failed: $_" }
}

# ============================================================
# Font download (replaces the old fonts.cmake; 7z comes from PATH)
# ============================================================
function Invoke-FontDownload {
    param(
        [Parameter(Mandatory)][string]$DestDir,
        [string]$Url
    )
    if (-not $Url) {
        $urlFile = Join-Path $script:PackagingDir 'fonts.url'
        if (-not (Test-Path $urlFile)) { throw "[fonts] Missing URL file: $urlFile" }
        $Url = (Get-Content $urlFile -Raw).Trim()
    }
    $archive = Join-Path $env:TEMP "fonts.7z"
    New-Item -ItemType Directory -Path $DestDir -Force | Out-Null

    Write-Host "[fonts] Downloading $Url"
    Invoke-WebRequest -Uri $Url -OutFile $archive -UseBasicParsing
    if (-not (Test-Path $archive)) { throw "[fonts] Download failed: $Url" }

    Write-Host "[fonts] Extracting -> $DestDir"
    & 7z x $archive "-o$DestDir" -y | Out-Null
    if ($LASTEXITCODE) { throw "[fonts] Extraction failed (exit $LASTEXITCODE)" }
    Remove-Item $archive -Force

    $n = (Get-ChildItem $DestDir -Recurse -File | Measure-Object).Count
    if ($n -eq 0) { throw "[fonts] Extraction produced no files: $DestDir" }
    Write-Host "[fonts] Done, $n files" -ForegroundColor Green
}

# ============================================================
# 7z portable archive
#   Keep the same layout as ubuntu / macOS: cd into the source directory
#   first, so the archive root contains the program files directly.
# ============================================================
function New-SevenZipArchive {
    param(
        [Parameter(Mandatory)][string]$SourceDir,
        [Parameter(Mandatory)][string]$Destination
    )
    if (-not (Test-Path $SourceDir)) { throw "[7z] Source directory does not exist: $SourceDir" }

    $sevenZip = (Get-Command 7z.exe -ErrorAction SilentlyContinue).Source
    if (-not $sevenZip) { throw "[7z] 7z.exe not found, install 7-Zip and add it to PATH" }

    $destDir = Split-Path $Destination -Parent
    if (-not (Test-Path $destDir)) { New-Item -ItemType Directory -Path $destDir -Force | Out-Null }
    Remove-SafeItem $Destination

    Write-Host "[7z] $SourceDir -> $Destination" -ForegroundColor DarkGray
    Push-Location $SourceDir
    try {
        & $sevenZip a -t7z -mx=9 -mmt "$Destination" '*'
        if ($LASTEXITCODE) { throw "[7z] Archiving failed (exit $LASTEXITCODE): $Destination" }
    }
    finally {
        Pop-Location
    }
}

# ============================================================
# Inno Setup Chinese translation (project file first, download as fallback)
# ============================================================
function Get-InnoChineseTranslation {
    param(
        [Parameter(Mandatory)][string]$DestDir,
        [string]$Url = "https://raw.githubusercontent.com/kira-96/Inno-Setup-Chinese-Simplified-Translation/main/ChineseSimplified.isl"
    )
    $dest = Join-Path $DestDir "ChineseSimplified.isl"
    if (Test-Path $dest) {
        Write-Host "[i18n] Already present: $dest" -ForegroundColor DarkGray
        return $dest
    }
    New-Item -ItemType Directory -Path $DestDir -Force | Out-Null
    Write-Host "[i18n] Downloading Chinese translation: $Url"
    Invoke-WebRequest -Uri $Url -OutFile $dest -UseBasicParsing
    if (-not (Test-Path $dest)) { throw "[i18n] Download failed: $Url" }
    $lines = (Get-Content $dest | Measure-Object).Count
    Write-Host "[i18n] Done, $lines lines -> $dest" -ForegroundColor Green
    return $dest
}

# ============================================================
# CMake: configure only (cmake -S -B)
#   Does not clean the directory, create directories or enter DevShell.
# ============================================================
function Invoke-CMakeConfigure {
    param(
        [Parameter(Mandatory)][string]$SourceDir,
        [Parameter(Mandatory)][string]$BuildDir,
        [Parameter(Mandatory)][string]$Generator,
        [Parameter(Mandatory)][string]$BuildType,
        [Parameter(Mandatory)][string]$QtDir,
        [string]$TargetArch
    )
    $cmakeArgs = @(
        "-S", $SourceDir,
        "-B", $BuildDir,
        "-G", $Generator,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_PREFIX_PATH=$QtDir"
    )
    if ($TargetArch) { $cmakeArgs += "-DTARGET_ARCH=$TargetArch" }

    Write-Host "cmake $($cmakeArgs -join ' ')" -ForegroundColor Cyan
    cmake @cmakeArgs
    if ($LASTEXITCODE) { throw "cmake configure failed" }
}

# ============================================================
# CMake: build only (cmake --build)
# ============================================================
function Invoke-CMakeBuild {
    param(
        [Parameter(Mandatory)][string]$BuildDir,
        [Parameter(Mandatory)][string]$BuildType
    )
    Write-Host "cmake --build $BuildDir --config $BuildType" -ForegroundColor Cyan
    cmake --build $BuildDir --config $BuildType
    if ($LASTEXITCODE) { throw "cmake build failed" }
}

# ============================================================
# Qt deployment (windeployqt from the target Qt directory, architecture safe)
# ============================================================
function Invoke-CMakeDeployment {
    param(
        [Parameter(Mandatory)][string]$QtDir,
        [Parameter(Mandatory)][string]$PacketDir,
        [string]$OpenSslRoot,
        [string[]]$ExcludeNames = @('crashpad_handler.exe'),
        # Application plugin directory (relative to PacketDir). package.ps1
        # copies the plugins to the PacketDir root first; only the exe and these
        # plugin copies are handed to windeployqt.
        [string]$AppPluginDir
    )
    $windeployqt = Join-Path $QtDir 'bin\windeployqt.exe'
    if (-not (Test-Path $windeployqt)) { throw "windeployqt not found: $windeployqt" }

    $pluginDir = Join-Path $PacketDir 'plugins'
    New-Item -ItemType Directory -Path $pluginDir -Force | Out-Null

    # Only select our own binaries. Qt's own Qt6*.dll / d3dcompiler_47.dll /
    # opengl32sw.dll are not Qt executables; passing them makes windeployqt
    # fail with a non-zero exit code.
    $appPluginNames = @()
    if ($AppPluginDir) {
        $appPluginPath = Join-Path $PacketDir $AppPluginDir
        if (Test-Path $appPluginPath) {
            $appPluginNames = @(Get-ChildItem $appPluginPath -Filter *.dll -File |
                ForEach-Object { $_.Name })
        }
    }

    $targets = @()
    $targets += @(Get-ChildItem $PacketDir -Filter *.exe -File |
        Where-Object { $ExcludeNames -notcontains $_.Name })
    $targets += @(Get-ChildItem $PacketDir -Filter *.dll -File |
        Where-Object { $appPluginNames -contains $_.Name })

    if (-not $targets) { Write-Warning "[windeployqt] No executable found to deploy"; return }

    # Pass the whole batch at once: dependency resolution runs once instead of
    # once per binary, which is an order of magnitude faster.
    $wdArgs = @('--plugindir', $pluginDir, '--force', '--compiler-runtime')
    if ($OpenSslRoot -and (Test-Path $OpenSslRoot)) {
        $wdArgs += @('--openssl-root', $OpenSslRoot)
    }
    $wdArgs += ($targets | ForEach-Object { $_.FullName })

    Write-Host "[windeployqt] Processing $($targets.Count) binaries: $(($targets | ForEach-Object { $_.Name }) -join ', ')" -ForegroundColor DarkGray
    & $windeployqt @wdArgs
    if ($LASTEXITCODE) { throw "windeployqt failed (exit $LASTEXITCODE)" }
}

# ============================================================
# Inno Setup (ISCC lookup: INNO_SETUP_PATH > PATH > Program Files)
# ============================================================
function Find-InnoSetup {
    $explicit = $env:INNO_SETUP_PATH
    if ($explicit -and (Test-Path $explicit)) { return $explicit }

    $cmd = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    foreach ($base in @("${env:ProgramFiles(x86)}", "${env:ProgramFiles}")) {
        foreach ($ver in @('6', '5')) {
            $p = Join-Path $base "Inno Setup $ver\ISCC.exe"
            if (Test-Path $p) { return $p }
        }
    }
    return $null
}

function INNO_Setup_Package {
    param(
        [Parameter(Mandatory)][string]$Path,
        [string]$ExtraArgs = ''
    )
    $iscc = Find-InnoSetup
    if (-not $iscc) { throw "ISCC.exe not found (INNO_SETUP_PATH / PATH / Program Files)" }

    Write-Host "`nISCC: $iscc $Path $ExtraArgs"
    $innoArgs = @($Path)
    if ($ExtraArgs) {
        $innoArgs += $ExtraArgs.Split(' ', [System.StringSplitOptions]::RemoveEmptyEntries)
    }
    & $iscc @innoArgs
    if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed (exit $LASTEXITCODE): $Path" }
}

# ============================================================
# Code signing (currently disabled; the call site in package.ps1 is commented out)
# ============================================================
function Get-SignToolPath {
    if ($script:_CachedSignTool) { return $script:_CachedSignTool }

    $cfg = $script:SignConfig
    if ($cfg.SignToolPath -and (Test-Path $cfg.SignToolPath)) {
        $script:_CachedSignTool = $cfg.SignToolPath
        return $script:_CachedSignTool
    }

    $cmd = Get-Command signtool.exe -ErrorAction SilentlyContinue
    if ($cmd) {
        $script:_CachedSignTool = $cmd.Source
        return $script:_CachedSignTool
    }

    $kits = "${env:ProgramFiles(x86)}\Windows Kits\10\bin"
    if (-not (Test-Path $kits)) { return $null }
    $versions = Get-ChildItem $kits -Directory |
    Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } |
    Sort-Object { [version]$_.Name } -Descending

    foreach ($pref in @('x64', 'x86')) {
        foreach ($v in $versions) {
            $p = Join-Path $v.FullName "$pref\signtool.exe"
            if (Test-Path $p) {
                $script:_CachedSignTool = $p
                Write-Host "[sign] Auto-detected signtool: $p" -ForegroundColor DarkGray
                return $p
            }
        }
    }
    return $null
}

function Invoke-SignBinaries {
    param([Parameter(Mandatory)][string[]]$Files)
    $cfg = $script:SignConfig
    if ([string]::IsNullOrWhiteSpace($cfg.Sha1Thumbprint)) {
        throw "[sign] SignConfig.Sha1Thumbprint is not configured."
    }

    $found = $false
    foreach ($store in @('Cert:\CurrentUser\My', 'Cert:\LocalMachine\My')) {
        if (Test-Path (Join-Path $store $cfg.Sha1Thumbprint)) { $found = $true; break }
    }
    if (-not $found) {
        throw "[sign] Certificate not found: $($cfg.Sha1Thumbprint) (CurrentUser\My or LocalMachine\My)"
    }

    $signtool = Get-SignToolPath
    if (-not $signtool) { throw "[sign] signtool.exe not found" }

    Write-Host "[sign] signtool: $signtool" -ForegroundColor DarkGray
    Write-Host "[sign] thumbprint: $($cfg.Sha1Thumbprint)" -ForegroundColor DarkGray

    foreach ($file in $Files) {
        if (-not (Test-Path $file)) { throw "[sign] File to sign does not exist: $file" }
        Write-Host "[sign] $file" -ForegroundColor Cyan
        & $signtool sign `
            /tr $cfg.TimestampUrl `
            /td sha256 `
            /fd sha256 `
            /sha1 $cfg.Sha1Thumbprint `
            $file
        if ($LASTEXITCODE -ne 0) { throw "[sign] Failed (exit $LASTEXITCODE): $file" }
    }
    Write-Host "[sign] Done, $($Files.Count) files" -ForegroundColor Green
}
