# IrBox Ultimate

**Copyright (c) DeathAmir And IrAutoX**

Advanced 3D game engine with LuaJIT scripting, encrypted models, and client-server architecture.

## Features

- **LuaJIT Integration**: Full Lua 5.1 compatibility with JIT compilation
- **Encrypted Models**: XOR + Hex encoding for .ibx model format
- **Script Obfuscation**: Built-in obfuscator for Lua scripts
- **Client-Server Architecture**: ENet-based networking
- **Database System**: Persistent storage with encryption
- **Achievement System**: Track player progress
- **Physics Engine**: Gravity, collision detection, resolution
- **UI System**: Buttons, panels, labels with RTL support
- **Persian Font Support**: Full RTL text rendering
- **Studio Tool**: Interactive development environment

## Directory Structure

```
IrBox_Ultimate/
├── src/                  # Source files
│   └── main.cpp          # Main entry point
├── include/              # Header files
│   └── irbox_engine.h    # Complete engine header
├── studio/               # Studio tool
│   └── studio.cpp        # Studio source
├── deps/                 # Dependencies
│   ├── luajit/           # LuaJIT source
│   └── enet/             # ENet networking
├── assets/
│   ├── models/           # 3D models (.obj, .ibx)
│   ├── fonts/            # Font files
│   └── textures/         # Texture files
├── scripts/              # Lua scripts
│   ├── parkour.lua       # Parkour game script
│   ├── obfuscator.lua    # Script obfuscator
│   └── model_encryptor.lua # Model encryption tool
├── build/                # Build output
├── Makefile              # Build system
├── build_dll.bat         # Windows build script
└── README.md             # This file
```

## Building

### Linux

```bash
# Install dependencies
sudo apt-get install build-essential autoconf automake libtool

# Build everything
make

# Run tests
make test

# Clean
make clean
```

### Windows (MinGW-w64)

```batch
# Using the batch script
build_dll.bat

# Or manually
mingw64-make WINDOWS=1
```

### Required DLLs (Windows)

The following DLLs are required and will be copied by `build_dll.bat`:
- `lua51.dll` (LuaJIT)
- `libgcc_s_seh-1.dll`
- `libstdc++-6.dll`
- `libwinpthread-1.dll`

## Usage

### Running the Game

```bash
# Linux
./build/irbox

# Windows
build\irbox.exe

# Server mode
./build/irbox --server --port 7777

# Connect to server
./build/irbox --connect localhost 7777
```

### Using Studio

```bash
# Run a script
./build/irbox_studio --run scripts/parkour.lua

# Start server with script upload
./build/irbox_studio --run scripts/parkour.lua --name parkour --server

# Encrypt a model
./build/irbox_studio --encrypt-model input.obj output.ibx MySecretKey

# Obfuscate a script
./build/irbox_studio --obfuscate script.lua script.ibx
```

### Lua Tools

```bash
# Obfuscate script
lua scripts/obfuscator.lua obf parkour.lua parkour.ibx

# Deobfuscate script
lua scripts/obfuscator.lua deobf parkour.ibx parkour.lua

# Encrypt model
lua scripts/model_encryptor.lua enc model.obj model.ibx IrBoxModelKey2024

# Decrypt model
lua scripts/model_encryptor.lua dec model.ibx model.obj IrBoxModelKey2024

# Create simple cube
lua scripts/model_encryptor.lua cube cube.obj 1.0
```

## API Reference

### Lua Functions

```lua
-- Logging
print_log(message)

-- Database
db_set(key, value)
db_get(key)

-- Blocks
spawn_block(x, y, z, type)
destroy_block(index)

-- Achievements
give_achievement(id)

-- Player
player_die()

-- Models
encrypt_model(path, key)

-- Networking
send_network_packet(type)
```

### Encryption Keys

| Type | Default Key |
|------|-------------|
| Models | `IrBoxModelKey2024` |
| Scripts | `IrBoxObf2024` |
| Database | `IrBoxDBKey2024` |
| Network | `IrBoxNetKey2024` |
| Admin | `IrBoxAdmin2024` |

## Security Features

1. **XOR Encryption**: All data encrypted with rotating XOR key
2. **Hex Encoding**: Binary data encoded as hexadecimal
3. **Model Encryption**: Vertex data transformed mathematically
4. **Script Obfuscation**: Lua scripts compiled to hex format
5. **Admin Authentication**: Server admin key verification

## Achievements

Built-in achievements:
- `welcome` - Welcome to IrBox!
- `first_jump` - Jump for the first time
- `first_death` - Fall into the void
- `speedrun` - Complete parkour in under 30 seconds
- `perfectionist` - Complete parkour without falling
- `builder` - Place 100 blocks
- `miner` - Break 100 blocks

## License

**Copyright (c) DeathAmir And IrAutoX**. All rights reserved.

This software is proprietary and confidential.
