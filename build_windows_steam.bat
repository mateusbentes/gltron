@echo off
REM Build script for GLTron with Steam multiplayer on Windows

echo ========================================
echo GLTron Windows Build with Steam Support
echo ========================================

REM Check if Visual Studio is available
where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: Visual Studio compiler not found!
    echo Please run this script from a Visual Studio Developer Command Prompt
    pause
    exit /b 1
)

REM Check if CMake is available
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: CMake not found!
    echo Please install CMake and add it to PATH
    pause
    exit /b 1
)

REM Check if Steamworks SDK exists
if not exist "steamworks\sdk\public\steam\steam_api.h" (
    echo Error: Steamworks SDK not found!
    echo Please download Steamworks SDK and extract to steamworks\ folder
    pause
    exit /b 1
)

REM Create build directory
if not exist "build_win" mkdir build_win
cd build_win

echo.
echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DUSE_STEAMWORKS=ON ^
    -DUSE_SOUND=ON ^
    -DCMAKE_BUILD_TYPE=Release

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building GLTron...
cmake --build . --config Release --parallel

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Copying Steam files...

REM Copy Steam API DLL
if exist "..\steamworks\sdk\redistributable_bin\win64\steam_api64.dll" (
    copy "..\steamworks\sdk\redistributable_bin\win64\steam_api64.dll" "Release\" >nul
) else (
    copy "..\steamworks\sdk\redistributable_bin\steam_api.dll" "Release\" >nul
)

REM Copy steam_appid.txt
copy "..\steam_appid.txt" "Release\" >nul

REM Copy game assets
echo Copying game assets...
copy "..\*.sgi" "Release\" >nul 2>&1
copy "..\*.wav" "Release\" >nul 2>&1
copy "..\*.it" "Release\" >nul 2>&1
copy "..\*.ftx" "Release\" >nul 2>&1
copy "..\*.obj" "Release\" >nul 2>&1
copy "..\*.mtl" "Release\" >nul 2>&1
copy "..\menu.txt" "Release\" >nul
copy "..\settings.txt" "Release\" >nul

echo.
echo ========================================
echo Build complete!
echo ========================================
echo.
echo Executable: build_win\Release\gltron.exe
echo.
echo To run GLTron with multiplayer:
echo 1. Make sure Steam is running
echo 2. Run: build_win\Release\gltron.exe
echo 3. Go to Multiplayer menu for online play
echo.
echo Note: Using App ID 480 (Spacewar) for testing
echo.
pause
