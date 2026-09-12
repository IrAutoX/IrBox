# IrBox Game Engine

## Copyright
Copyright DeathAmir And IrAutoX

## Description
IrBox is a 3D game engine with Lua scripting support, inspired by Roblox. Features include:
- 3D rendering with OpenGL
- LuaJIT scripting with encryption/obfuscation
- Client-server architecture
- RTL/Farsi font support
- Encrypted model files
- Parkour game mode
- UI system with menus
- Physics engine
- Network multiplayer

## Build Instructions

### Windows (MinGW64)
```bash
mingw64-make WINDOWS=1
```

### Linux
```bash
make
```

## Usage

### Run Client
```bash
mingw64-make run-client
```

### Run Server
```bash
mingw64-make run-server
```

### Package
```bash
mingw64-make package
```

## Project Structure
```
IrBox/
├── src/           # Source code
├── include/       # Header files
├── assets/        # Game assets
│   ├── models/    # 3D models (encrypted .ibx format)
│   ├── textures/  # Textures
│   └── fonts/     # Fonts (including Farsi/RTL)
├── scripts/       # Lua scripts
├── server/        # Server code
├── build/         # Compiled binaries
└── Makefile       # Build configuration
```

## Features

### Security
- Model encryption with XOR cipher
- Lua script obfuscation
- Admin authentication system
- Secure network communication

### Scripting
- Full LuaJIT support
- Server-side script execution
- Client-side script download and compilation
- Obfuscated script transmission

### Graphics
- 3D model loading and rendering
- Encrypted model format (.ibx)
- Texture mapping
- Real-time rendering

### UI System
- Main menu
- Profile system
- Settings panel
- RTL text support
- Farsi font support

### Physics
- Gravity simulation
- Collision detection
- Player movement
- Jump mechanics

## License
Proprietary - Copyright DeathAmir And IrAutoX

## Version
1.0.0