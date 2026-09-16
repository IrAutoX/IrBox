local ModelEncryptor = {}

ModelEncryptor.key = "IrBoxModelKey2024"

function ModelEncryptor.encryptVertex(x, y, z, key)
    local k = #key
    return {
        x = math.sin(x) * k,
        y = math.cos(y) * k,
        z = math.tan(z + 1) * k
    }
end

function ModelEncryptor.decryptVertex(x, y, z, key)
    local k = #key
    return {
        x = math.asin(x / k),
        y = math.acos(y / k),
        z = math.atan(z / k) - 1
    }
end

function ModelEncryptor.xorEncrypt(data, key)
    local result = ""
    for i = 1, #data do
        local byte = string.byte(data, i)
        local keyByte = string.byte(key, (i - 1) % #key + 1)
        local xored = bit32 and bit32.bxor(byte, keyByte, 0x42) or (byte ~ keyByte ~ 0x42)
        result = result .. string.char(xored)
    end
    return result
end

function ModelEncryptor.toHex(data)
    local hex = ""
    for i = 1, #data do
        local byte = string.byte(data, i)
        hex = hex .. string.format("%02X", byte)
    end
    return hex
end

function ModelEncryptor.fromHex(hexData)
    local raw = ""
    for i = 1, #hexData, 2 do
        local byte = tonumber(hexData:sub(i, i + 1), 16)
        raw = raw .. string.char(byte)
    end
    return raw
end

function ModelEncryptor.encryptModel(inputPath, outputPath, key)
    local k = key or ModelEncryptor.key
    
    local file = io.open(inputPath, "rb")
    if not file then
        print("Error: Cannot open model file " .. inputPath)
        return false
    end
    
    local content = file:read("*all")
    file:close()
    
    local encrypted = ModelEncryptor.xorEncrypt(content, k)
    local hexEncoded = ModelEncryptor.toHex(encrypted)
    
    local header = "IRBOX_MODEL_V1\n"
    header = header .. "KEY_LEN:" .. #k .. "\n"
    header = header .. "DATA_LEN:" .. #content .. "\n"
    header = header .. "---BEGIN---\n"
    
    local outFile = io.open(outputPath, "wb")
    if not outFile then
        print("Error: Cannot create output file " .. outputPath)
        return false
    end
    
    outFile:write(header)
    outFile:write(hexEncoded)
    outFile:close()
    
    print("Model encryption complete!")
    print("Input: " .. inputPath .. " (" .. #content .. " bytes)")
    print("Output: " .. outputPath .. " (" .. #hexEncoded + #header .. " bytes)")
    print("Encryption key: " .. k)
    
    return true
end

function ModelEncryptor.decryptModel(inputPath, outputPath, key)
    local k = key or ModelEncryptor.key
    
    local file = io.open(inputPath, "rb")
    if not file then
        print("Error: Cannot open encrypted model " .. inputPath)
        return false
    end
    
    local content = file:read("*all")
    file:close()
    
    local headerEnd = content:find("---BEGIN---\n")
    if not headerEnd then
        print("Error: Invalid model format")
        return false
    end
    
    local hexData = content:sub(headerEnd + 12)
    local encrypted = ModelEncryptor.fromHex(hexData)
    local decrypted = ModelEncryptor.xorEncrypt(encrypted, k)
    
    local outFile = io.open(outputPath, "wb")
    if not outFile then
        print("Error: Cannot create output file " .. outputPath)
        return false
    end
    
    outFile:write(decrypted)
    outFile:close()
    
    print("Model decryption complete!")
    print("Input: " .. inputPath)
    print("Output: " .. outputPath .. " (" .. #decrypted .. " bytes)")
    
    return true
end

function ModelEncryptor.createSimpleCube(outputPath, size)
    local s = size or 1.0
    
    local vertices = {}
    for i = 0, 7 do
        local x = (i % 2) * s
        local y = (math.floor(i / 2) % 2) * s
        local z = (math.floor(i / 4) % 2) * s
        table.insert(vertices, {x = x, y = y, z = z})
    end
    
    local indices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        0, 1, 5, 0, 5, 4,
        2, 3, 7, 2, 7, 6,
        0, 3, 7, 0, 7, 4,
        1, 2, 6, 1, 6, 5
    }
    
    local data = "CUBE_MODEL_V1\n"
    data = data .. "VERTICES:" .. #vertices .. "\n"
    for _, v in ipairs(vertices) do
        data = data .. string.format("%.6f,%.6f,%.6f\n", v.x, v.y, v.z)
    end
    data = data .. "INDICES:" .. #indices .. "\n"
    for _, idx in ipairs(indices) do
        data = data .. idx .. ","
    end
    data = data .. "\n"
    
    local file = io.open(outputPath, "w")
    if file then
        file:write(data)
        file:close()
        print("Cube model created: " .. outputPath)
        return true
    end
    
    return false
end

if arg and #arg >= 3 then
    local mode = arg[1]
    local input = arg[2]
    local output = arg[3]
    local key = arg[4] or ModelEncryptor.key
    
    if mode == "enc" or mode == "encrypt" then
        ModelEncryptor.encryptModel(input, output, key)
    elseif mode == "dec" or mode == "decrypt" then
        ModelEncryptor.decryptModel(input, output, key)
    elseif mode == "cube" then
        local size = tonumber(arg[4]) or 1.0
        ModelEncryptor.createSimpleCube(input, size)
    else
        print("Usage:")
        print("  lua model_encryptor.lua enc <input.obj> <output.ibx> [key]")
        print("  lua model_encryptor.lua dec <input.ibx> <output.obj> [key]")
        print("  lua model_encryptor.lua cube <output.obj> [size]")
    end
else
    print("IrBox Model Encryptor - Copyright (c) DeathAmir And IrAutoX")
    print("")
    print("Usage:")
    print("  lua model_encryptor.lua enc <input.obj> <output.ibx> [key]")
    print("  lua model_encryptor.lua dec <input.ibx> <output.obj> [key]")
    print("  lua model_encryptor.lua cube <output.obj> [size]")
    print("")
    print("Default key: IrBoxModelKey2024")
end

return ModelEncryptor
