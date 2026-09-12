@echo off
echo Building IrBox with MinGW64...
mingw64-make WINDOWS=1 clean
mingw64-make WINDOWS=1 all
echo Build complete!
echo Copying DLLs...
copy C:\msys64\mingw64\bin\lua5.1.dll build\ 2>nul
copy C:\msys64\mingw64\bin\libgcc_s_seh-1.dll build\ 2>nul
copy C:\msys64\mingw64\bin\libstdc++-6.dll build\ 2>nul
copy C:\msys64\mingw64\bin\libwinpthread-1.dll build\ 2>nul
echo DLLs copied!
echo IrBox is ready in build folder
pause
