@echo off
REM Library Manager Build Script
REM Author: Dawid Papaj

echo ========================================
echo Library Manager Build Script
echo ========================================
echo.

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build Release
echo.
echo Building Release configuration...
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo Executable: build\Release\LibraryManager.exe
echo ========================================
echo.

REM Ask to create installer
set /p CREATE_INSTALLER="Create installer? (y/n): "
if /i "%CREATE_INSTALLER%"=="y" (
    echo Creating installer with CPack...
    cpack -G NSIS
    if errorlevel 1 (
        echo Installer creation failed!
        echo Make sure NSIS is installed and in PATH.
    ) else (
        echo Installer created successfully!
    )
)

cd ..
pause
