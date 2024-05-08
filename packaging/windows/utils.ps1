function Remove-SafeItem {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )
    
    try {
        $resolvedPaths = Resolve-Path -Path $Path -ErrorAction SilentlyContinue
        
        if (-not $resolvedPaths) {
            Write-Host "No valid path found for the pattern: '$Path'."
            return
        }
        foreach ($resolvedPath in $resolvedPaths) {
            if (Test-Path $resolvedPath.Path) {
                if (Test-Path $resolvedPath.Path -PathType Container) {
                    Remove-Item -Path $resolvedPath.Path -Recurse -Force -Verbose
                    Write-Host "Directory '$($resolvedPath.Path)' has been removed."
                }
                elseif (Test-Path $resolvedPath.Path -PathType Leaf) {
                    Remove-Item -Path $resolvedPath.Path -Force -Verbose
                    Write-Host "File '$($resolvedPath.Path)' has been removed."
                }
            }
            else {
                Write-Host "The resolved path '$($resolvedPath.Path)' does not exist or has been removed."
            }
        }
    }
    catch {
        Write-Error "An error occurred: $($_.Exception.Message)"
    }
}
