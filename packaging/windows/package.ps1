$ErrorActionPreference = 'Stop'

Set-Location $PSScriptRoot\..\..
$ProjectDir = (Get-Location).Path
Write-Host $ProjectDir

Import-Module (Join-Path $ProjectDir "packaging\windows\utils.ps1")

Invoke-Expression -Command "$ProjectDir\scripts\windows\setVsDev.ps1"

$releases_dir = Join-Path $ProjectDir "packaging\releases"
$packet_dir = Join-Path $ProjectDir "packaging\packet"
$plugin_dir = Join-Path $packet_dir "plugins"
if (Test-Path $plugin_dir) {
    Copy-Item $plugin_dir/* -Destination $packet_dir -Recurse -Force -Verbose
}

$exeFiles = Get-ChildItem -Path $packet_dir -Filter *.exe | ForEach-Object { $_.FullName }
$dllFiles = Get-ChildItem -Path $packet_dir -Filter *.dll | ForEach-Object { $_.FullName }
$allFiles = $exeFiles + $dllFiles
foreach ($file in $allFiles) {
    if ($file -ne "crashpad_handler.exe" -and ($file.EndsWith(".exe") -or $file.EndsWith(".dll"))) {
        Write-Host "Deploying $file..."
        windeployqt --force --compiler-runtime $file
    }
}

$Remove_List_Relative = @(
    "*Test*",
    "*plugin*.dll",
    "examples"
)
$Remove_List_Absolute = $Remove_List_Relative | ForEach-Object {
    Join-Path $packet_dir $_
}
foreach ($item in $Remove_List_Absolute) {
    Remove-SafeItem -Path $item
}

# packaging with 7z
$zip_path = Join-Path $releases_dir "Qt-App.7z"
7z a -t7z -r -mx=9 -mmt $zip_path $packet_dir\*

# packaging with iscc
ISCC (Join-Path $PSScriptRoot "app.iss")

Write-Host "Packaging complete."

exit 0
