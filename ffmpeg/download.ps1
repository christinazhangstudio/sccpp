$tempPath = Join-Path (Get-Location) "tmp"
New-Item -ItemType Directory -Path $tempPath -Force | Out-Null

$sharedZipPath = Join-Path $tempPath "ffmpeg.shared.zip"

# Download FFmpeg (libs/includes/DLL/exe (shared means DLL's are also included))
$sharedUrl = "https://github.com/BtbN/FFmpeg-Builds/releases/latest/download/ffmpeg-master-latest-win64-gpl-shared.zip"

(New-Object System.Net.WebClient).DownloadFile($sharedUrl, $sharedZipPath)
Expand-Archive $sharedZipPath -DestinationPath $tempPath -Force

Remove-Item -Path "include" -Recurse -Force -ErrorAction Ignore
Remove-Item -Path "lib" -Recurse -Force -ErrorAction Ignore
Remove-Item -Path "bin" -Recurse -Force -ErrorAction Ignore

$sharedRoot = Get-ChildItem -Directory "$tempPath/ffmpeg-*-win64-gpl-shared"
Move-Item "$($sharedRoot.FullName)/bin" -Destination (Get-Location) -Force
Move-Item "$($sharedRoot.FullName)/include" -Destination (Get-Location) -Force
Move-Item "$($sharedRoot.FullName)/lib"     -Destination (Get-Location) -Force

Remove-Item -Path $tempPath -Recurse -Force -ErrorAction Ignore

Write-Host "FFmpeg dev (include/lib) and shared binaries extracted successfully."