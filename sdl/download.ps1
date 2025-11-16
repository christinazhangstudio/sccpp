$tempPath = Join-Path (Get-Location) "tmp"
New-Item -ItemType Directory -Path $tempPath -Force | Out-Null

$installZipPath = Join-Path $tempPath "sdl.zip"

# Download FFmpeg (libs/includes/DLL/exe (shared means DLL's are also included))
$installUrl = "https://github.com/libsdl-org/SDL/releases/download/release-3.2.26/SDL3-devel-3.2.26-mingw.tar.gz"

(New-Object System.Net.WebClient).DownloadFile($installUrl, $installZipPath)
tar -xzf $installZipPath -C $tempPath

# clean up any existing dirs
Remove-Item -Path "include" -Recurse -Force -ErrorAction Ignore
Remove-Item -Path "lib" -Recurse -Force -ErrorAction Ignore
Remove-Item -Path "bin" -Recurse -Force -ErrorAction Ignore

$installRoot = "$tempPath/SDL3-3.2.26/x86_64-w64-mingw32"
Move-Item "$($installRoot)/bin" -Destination (Get-Location) -Force
Move-Item "$($installRoot)/include" -Destination (Get-Location) -Force
Move-Item "$($installRoot)/lib"     -Destination (Get-Location) -Force

# remove the intermediate SDL3 folder because it's annoying
# don't do this, because internal imports rely on it
# $targetDir = "$(Get-Location)/include/SDL3"
# $parentDir = Split-Path -Path $targetDir -Parent

# Get-ChildItem -Path $targetDir | ForEach-Object {
#     Move-Item -LiteralPath $_ -Destination $parentDir
# }

# Remove-Item $targetDir

Remove-Item -Path $tempPath -Recurse -Force -ErrorAction Ignore

Write-Host "SDL dev (include/lib) and shared binaries extracted successfully."