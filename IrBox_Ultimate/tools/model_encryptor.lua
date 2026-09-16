local CryptoEngine = {}

CryptoEngine.KEY = "IrBoxModelKey2024"

function CryptoEngine.encrypt(data, key)
    local result = ""
    for i = 1, #data do
        local byte = string.byte(data, i)
        local keyByte = string.byte(key, (i - 1) % #key + 1)
        local xored = bit32.bxor(byte, keyByte, i % 256)
        result = result .. string.char(xored)
    end
    return result
end

function createCube(size)
    local s = size / 2.0
    local vertices = {
        {-s, -s, -s}, {s, -s, -s}, {s, s, -s}, {-s, s, -s},
        {-s, -s, s}, {s, -s, s}, {s, s, s}, {-s, s, s}
    }
    
    local indices = {
        1, 2, 3, 1, 3, 4,
        5, 7, 6, 5, 8, 7,
        1, 4, 8, 1, 8, 5,
        2, 6, 7, 2, 7, 3,
        4, 3, 7, 4, 7, 8,
        1, 5, 6, 1, 6, 2
    }
    
    local data = tostring(#vertices) .. " "
    for _, v in ipairs(vertices) do
        data = data .. v[1] .. " " .. v[2] .. " " .. v[3] .. " "
    end
    data = data .. tostring(#indices) .. " "
    for _, idx in ipairs(indices) do
        data = data .. idx .. " "
    end
    
    return data
end

function encryptModel(modelData)
    return CryptoEngine.encrypt(modelData, CryptoEngine.KEY)
end

print("Generating IrBox models...")

local cube = createCube(1.0)
local encrypted = encryptModel(cube)

local file = io.open("assets/models/cube.ibx", "wb")
if file then
    file:write(encrypted)
    file:close()
    print("Created: assets/models/cube.ibx (encrypted)")
    print("Original size: " .. #cube .. " bytes")
    print("Encrypted size: " .. #encrypted .. " bytes")
else
    print("Error: Cannot create model file")
end

local platform = createCube(3.0)
local encPlatform = encryptModel(platform)

local pfile = io.open("assets/models/platform.ibx", "wb")
if pfile then
    pfile:write(encPlatform)
    pfile:close()
    print("Created: assets/models/platform.ibx (encrypted)")
end

print("\nModels generated successfully!")
print("Format: .ibx (IrBox encrypted model format)")
print("Encryption: XOR with key + position-based obfuscation")
