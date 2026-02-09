@echo off
SETLOCAL

REM Couleurs
SET COL_NONE=
SET COL_GREEN=

SET BUILD_DIR=build
SET EXECUTABLE=VectemBuildTool
SET EXECUTABLE_DIR=bin

REM Vérifier le premier argument pour le nettoyage
IF "%~1"=="-c" (
    echo [1] Nettoyage du projet...
    rmdir /s /q "%BUILD_DIR%"
    exit /b 0
)

REM Compilation
echo [1] Compilation du projet...
IF NOT EXIST "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%" || exit /b 1

cmake -DCMAKE_BUILD_TYPE=Debug .. || exit /b 1
cmake --build . || exit /b 1

REM Exécution (optionnelle)
REM IF "%~1"=="-x" (
REM     echo [2] Exécution du programme...
REM     cd /d "..\%EXECUTABLE_DIR%" || exit /b 1
REM     "%EXECUTABLE%"
REM )

ENDLOCAL
