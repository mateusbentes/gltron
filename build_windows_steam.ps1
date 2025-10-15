# PowerShell build script for GLTron with Steam multiplayer on Windows

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "GLTron Windows Build with Steam Support" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Function to check if a command exists
function Test-Command {
    param($Command)
    try {
        Get-Command $Command -ErrorAction Stop | Out-Null
        return $true
    } catch {
        return $false
    }
}

# Check prerequisites
Write-Host "`nChecking prerequisites..." -ForegroundColor Yellow

if (-not (Test-Command "cmake")) {
    Write-Host "Error: CMake not found!" -ForegroundColor Red
    Write-Host "Please install CMake from https://cmake.org/download/" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

if (-not (Test-Path "steamworks\sdk\public\steam\steam_api.h")) {
    Write-Host "Error: Steamworks SDK not found!" -ForegroundColor Red
    Write-Host "Please download Steamworks SDK and extract to steamworks\ folder" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

# Detect Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = ""

if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
}

if ($vsPath -eq "") {
    Write-Host "Warning: Visual Studio not detected, trying MinGW..." -ForegroundColor Yellow
    $generator = "MinGW Makefiles"
    $buildCmd = "mingw32-make"
} else {
    Write-Host "Found Visual Studio at: $vsPath" -ForegroundColor Green
    $generator = "Visual Studio 17 2022"
    $buildCmd = "cmake --build . --config Release --parallel"
}

# Create build directory
$buildDir = "build_win"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}
Set-Location $buildDir

# Configure with CMake
Write-Host "`nConfiguring with CMake..." -ForegroundColor Yellow
if ($generator -eq "Visual Studio 17 2022") {
    cmake .. -G "$generator" -A x64 `
        -DUSE_STEAMWORKS=ON `
        -DUSE_SOUND=ON `
        -DCMAKE_BUILD_TYPE=Release
} else {
    cmake .. -G "$generator" `
        -DUSE_STEAMWORKS=ON `
        -DUSE_SOUND=ON `
        -DCMAKE_BUILD_TYPE=Release
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Build
Write-Host "`nBuilding GLTron..." -ForegroundColor Yellow
if ($generator -eq "Visual Studio 17 2022") {
    cmake --build . --config Release --parallel
} else {
    & $buildCmd -j4
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Determine output directory
$outputDir = if ($generator -eq "Visual Studio 17 2022") { "Release" } else { "." }

Write-Host "`nCopying Steam files..." -ForegroundColor Yellow

# Copy Steam API DLL
$steamDll64 = "..\steamworks\sdk\redistributable_bin\win64\steam_api64.dll"
$steamDll32 = "..\steamworks\sdk\redistributable_bin\steam_api.dll"

if (Test-Path $steamDll64) {
    Copy-Item $steamDll64 -Destination "$outputDir\" -Force
} elseif (Test-Path $steamDll32) {
    Copy-Item $steamDll32 -Destination "$outputDir\" -Force
}

# Copy steam_appid.txt
Copy-Item "..\steam_appid.txt" -Destination "$outputDir\" -Force

# Copy game assets
Write-Host "Copying game assets..." -ForegroundColor Yellow
$assets = @("*.sgi", "*.wav", "*.it", "*.ftx", "*.obj", "*.mtl", "menu.txt", "settings.txt")
foreach ($pattern in $assets) {
    Get-ChildItem "..\$pattern" -ErrorAction SilentlyContinue | 
        Copy-Item -Destination "$outputDir\" -Force
}

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Build complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Executable: $buildDir\$outputDir\gltron.exe" -ForegroundColor Cyan
Write-Host ""
Write-Host "To run GLTron with multiplayer:" -ForegroundColor Yellow
Write-Host "1. Make sure Steam is running"
Write-Host "2. Run: $buildDir\$outputDir\gltron.exe"
Write-Host "3. Go to Multiplayer menu for online play"
Write-Host ""
Write-Host "Features:" -ForegroundColor Cyan
Write-Host "- Quick Match: Automatically join or create a game"
Write-Host "- Spectator Mode: Watch other players after crashing"
Write-Host "- Steam Integration: Play with friends through Steam"
Write-Host ""
Write-Host "Note: Using App ID 480 (Spacewar) for testing" -ForegroundColor Yellow
Write-Host ""
Read-Host "Press Enter to exit"
