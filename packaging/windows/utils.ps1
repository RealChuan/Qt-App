function Remove-SafeItem {
    param([Parameter(Mandatory)]$Path)
    try {
        $resolved = Resolve-Path $Path -ErrorAction SilentlyContinue
        if (-not $resolved) { Write-Host "Pattern not found: $Path"; return }
        foreach ($r in $resolved) {
            if (Test-Path $r) {
                Remove-Item $r -Recurse -Force -Verbose
                Write-Host "Removed: $($r.Path)"
            }
        }
    }
    catch { Write-Error $_ }
}
