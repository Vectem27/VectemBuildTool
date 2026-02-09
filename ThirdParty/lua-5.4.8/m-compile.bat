@echo off
setlocal

:: Couleurs
set "COL_NONE="
set "COL_GREEN="

set "BUILD_DIR=build"
set "OUTPUT_DIR=bin"

:: Clean
if "%~1"=="-c" (
    echo Clean lua build...
    if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
    exit /b 0
)

:: Build
echo Compile lua...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%" || exit /b 1

cmake .. || exit /b 1
cmake --build . || exit /b 1
