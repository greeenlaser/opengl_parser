@echo off
setlocal

:: Set working directory to this script's location
cd /d "%~dp0"

:: ================================
:: Clean old build folders
:: ================================
for %%D in (debug release) do (
    if exist "out/build/%%D" (
        echo [INFO] Removing old build folder: out/build/%%D
        rmdir /S /Q "out/build/%%D"
    )
)

:: =====================================
:: [MSVC Setup] Ensure cl.exe is available for Ninja
:: =====================================
set "VS_MAIN=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "VS_SIDE=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if exist "%VS_MAIN%" (
    call "%VS_MAIN%"
) else if exist "%VS_SIDE%" (
    call "%VS_SIDE%"
) else (
    echo [ERROR] Could not find vcvars64.bat from either Visual Studio Community or Build Tools.
    echo         Please install Visual Studio or Build Tools and try again.
    pause
    exit /b 1
)

:: ================================
:: Run builds
:: ================================
call :BuildPreset debug
::call :BuildPreset release

echo.
echo =====================================
echo [SUCCESS] All builds completed!
echo =====================================
pause
exit /b 0

:: ================================
:: Function: BuildPreset <preset>
:: ================================
:BuildPreset
set "PRESET=%~1"
if "%PRESET%"=="" (
    echo [ERROR] No preset name provided!
    exit /b 1
)

echo.
echo [INFO] Configuring preset: %PRESET%
cmake --preset %PRESET%
if errorlevel 1 (
    echo [ERROR] Configuration failed for %PRESET%
    exit /b 1
)

echo [INFO] Building preset: %PRESET%
cmake --build --preset %PRESET%
if errorlevel 1 (
    echo [ERROR] Build failed for %PRESET%
    exit /b 1
)

echo [SUCCESS] Finished building: %PRESET%
exit /b 0
