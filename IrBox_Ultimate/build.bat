@echo off
echo ====================================
echo IrBox Build Script for Windows
echo Copyright (c) 2024 DeathAmir And IrAutoX
echo ====================================
echo.

set MINGW_PATH=C:\msys64\mingw64
set PATH=%MINGW_PATH%\bin;%PATH%

if not exist "build" mkdir build

echo [1/4] Building IrBox Client...
mingw32-make clean 2>nul
mingw32-make all

if errorlevel 1 (
    echo [ERROR] Build failed! Check dependencies.
    echo Required: SDL3, LuaJIT, ENet
    goto :copy_dlls
)

echo [2/4] Build successful!
echo.

:copy_dlls
echo [3/4] Copying DLLs...
if exist "%MINGW_PATH%\bin\SDL3.dll" copy "%MINGW_PATH%\bin\SDL3.dll" build\ >nul
if exist "%MINGW_PATH%\bin\lua51.dll" copy "%MINGW_PATH%\bin\lua51.dll" build\ >nul
if exist "%MINGW_PATH%\bin\libenet-1.3.18.dll" copy "%MINGW_PATH%\bin\libenet-1.3.18.dll" build\ >nul
if exist "%MINGW_PATH%\bin\libgcc_s_seh-1.dll" copy "%MINGW_PATH%\bin\libgcc_s_seh-1.dll" build\ >nul
if exist "%MINGW_PATH%\bin\libstdc++-6.dll" copy "%MINGW_PATH%\bin\libstdc++-6.dll" build\ >nul
if exist "%MINGW_PATH%\bin\libwinpthread-1.dll" copy "%MINGW_PATH%\bin\libwinpthread-1.dll" build\ >nul

echo [4/4] Creating distribution package...
if exist "IrBox_Distribution" rmdir /s /q IrBox_Distribution
mkdir IrBox_Distribution
mkdir IrBox_Distribution\bin
mkdir IrBox_Distribution\assets
mkdir IrBox_Distribution\scripts
mkdir IrBox_Distribution\server

copy build\IrBox.exe IrBox_Distribution\bin\
copy build\IrBoxServer.exe IrBox_Distribution\bin\
copy build\*.dll IrBox_Distribution\bin\ 2>nul

copy assets\*.* IrBox_Distribution\assets\ 2>nul
copy scripts\*.lua IrBox_Distribution\scripts\ 2>nul
copy server\*.* IrBox_Distribution\server\ 2>nul

copy README.md IrBox_Distribution\
copy INSTALL.txt IrBox_Distribution\

echo.
echo ====================================
echo Build Complete!
echo Distribution: IrBox_Distribution
echo Run: IrBox_Distribution\bin\IrBox.exe
echo ====================================
pause
