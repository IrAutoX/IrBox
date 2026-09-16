# IrBox Ultimate - Game Engine

## Copyright
Copyright (c) 2024 DeathAmir And IrAutoX

## Description
IrBox is a powerful 3D game engine with Lua scripting, similar to Roblox. Features include:
- 3D rendering with SDL3
- LuaJIT scripting engine
- Encrypted models (.ibx format)
- Obfuscated scripts
- Client-Server architecture with ENet
- Persian/RTL font support
- Achievement system
- Physics engine
- Database storage
- Development Studio

## Requirements (Windows)
- MinGW64 (MSYS2)
- SDL3 development libraries
- LuaJIT
- ENet library

## Installation

### 1. Install MSYS2
Download from: https://www.msys2.org/

### 2. Install Dependencies
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-SDL3
pacman -S mingw-w64-x86_64-lua
```

### 3. Build ENet
```bash
git clone https://github.com/lsalzman/enet.git
cd enet
autoreconf -fi
./configure --host=x86_64-w64-mingw32
make
```

### 4. Build IrBox
```bash
build.bat
```
Or manually:
```bash
mingw32-make all
```

## Project Structure
```
IrBox/
├── src/           # Main client source
├── server/        # Server source
├── studio/        # Development Studio
├── include/       # Engine header
├── scripts/       # Lua scripts
├── tools/         # Model encryptor, obfuscator
├── assets/        # Models, textures, fonts
└── build/         # Compiled output
```

## Usage

### Run Client
```
build/IrBox.exe
```

### Run Server
```
build/IrBoxServer.exe
```

### Run Studio
```
mingw32-make studio
build/IrBoxStudio.exe
```

## Server Commands
- `quit` / `exit` - Stop server
- `admin <key>` - Admin authentication
- `exec <script>` - Execute Lua script
- `status` - Server status
- `help` - Show commands

## Script Encryption
Scripts are encrypted using XOR + Hex encoding.
Use the obfuscator tool:
```bash
lua scripts/obfuscator.lua myscript.lua
```

## Model Format
Models use .ibx extension (IrBox encrypted format).
Generate encrypted models:
```bash
lua tools/model_encryptor.lua
```

## API Reference

### irbox.showMessage(text)
Display UI message

### irbox.spawnBlock(x, y, z, type)
Spawn physics block

### irbox.unlockAchievement(playerId, achievementId)
Unlock achievement

### irbox.loadScript(name)
Load script file

## Admin Key
Default: `IrBoxAdmin2024`

## License
Proprietary - DeathAmir And IrAutoX

## Version
1.0.0
