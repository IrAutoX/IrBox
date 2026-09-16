@echo off
echo ========================================
echo IrBox Ultimate Build Script for Windows
echo Copyright (c) DeathAmir And IrAutoX
echo ========================================
echo.

set MINGW_PATH=C:\msys64\mingw64\bin
set PATH=%MINGW_PATH%;%PATH%

echo [1/5] Building LuaJIT...
cd deps\luajit
mingw32-make clean 2>nul
mingw32-make mingw
if errorlevel 1 (
    echo ERROR: Failed to build LuaJIT
    pause
    exit /b 1
)
copy src\lua51.dll ..\..\build\ 2>nul
copy src\luajit-2.0.dll ..\..\build\ 2>nul
cd ..\..

echo.
echo [2/5] Building ENet...
cd deps\enet
autoreconf -fi 2>nul
.\configure --host=x86_64-w64-mingw32 --enable-static=yes --enable-shared=no 2>nul
mingw32-make clean 2>nul
mingw32-make
if errorlevel 1 (
    echo WARNING: ENet build had issues, continuing...
)
cd ..\..

echo.
echo [3/5] Building IrBox Engine...
mkdir build 2>nul
mingw64-make WINDOWS=1
if errorlevel 1 (
    echo ERROR: Failed to build IrBox
    pause
    exit /b 1
)

echo.
echo [4/5] Copying required DLLs...
copy %MINGW_PATH%\libgcc_s_seh-1.dll build\ 2>nul
copy %MINGW_PATH%\libstdc++-6.dll build\ 2>nul
copy %MINGW_PATH%\libwinpthread-1.dll build\ 2>nul

echo.
echo [5/5] Creating encrypted model files...
echo Models will be encrypted at runtime

echo.
echo ========================================
echo Build completed successfully!
echo Output files in build\ directory:
echo   - irbox.exe (main game)
echo   - irbox_studio.exe (studio tool)
echo   - lua51.dll (LuaJIT runtime)
echo   - libgcc_s_seh-1.dll
echo   - libstdc++-6.dll
echo   - libwinpthread-1.dll
echo ========================================
echo.
pause
