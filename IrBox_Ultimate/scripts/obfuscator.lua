local CryptoEngine = {}

CryptoEngine.key = "IrBoxObf2024"

function CryptoEngine.encrypt(data, key)
    local result = ""
    local k = key or CryptoEngine.key
    
    for i = 1, #data do
        local byte = string.byte(data, i)
        local keyByte = string.byte(k, (i - 1) % #k + 1)
        local xored = bit32.bxor(byte, keyByte, 0x42)
        result = result .. string.format("%02X", xored)
    end
    
    return result
end

function CryptoEngine.decrypt(hexData, key)
    local raw = ""
    local k = key or CryptoEngine.key
    
    for i = 1, #hexData, 2 do
        local hex = hexData:sub(i, i + 1)
        local byte = tonumber(hex, 16)
        raw = raw .. string.char(byte)
    end
    
    local result = ""
    for i = 1, #raw do
        local byte = string.byte(raw, i)
        local keyByte = string.byte(k, (i - 1) % #k + 1)
        local xored = bit32.bxor(byte, keyByte, 0x42)
        result = result .. string.char(xored)
    end
    
    return result
end

function obfuscateFile(inputPath, outputPath)
    local file = io.open(inputPath, "r")
    if not file then
        print("Error: Cannot open file " .. inputPath)
        return false
    end
    
    local content = file:read("*all")
    file:close()
    
    local obfuscated = CryptoEngine.encrypt(content, CryptoEngine.key)
    
    local outFile = io.open(outputPath, "w")
    if not outFile then
        print("Error: Cannot create file " .. outputPath)
        return false
    end
    
    outFile:write(obfuscated)
    outFile:close()
    
    print("Obfuscation complete!")
    print("Input: " .. inputPath .. " (" .. #content .. " bytes)")
    print("Output: " .. outputPath .. " (" .. #obfuscated .. " bytes)")
    
    return true
end

function deobfuscateFile(inputPath, outputPath)
    local file = io.open(inputPath, "r")
    if not file then
        print("Error: Cannot open file " .. inputPath)
        return false
    end
    
    local content = file:read("*all")
    file:close()
    
    local decrypted = CryptoEngine.decrypt(content, CryptoEngine.key)
    
    local outFile = io.open(outputPath, "w")
    if not outFile then
        print("Error: Cannot create file " .. outputPath)
        return false
    end
    
    outFile:write(decrypted)
    outFile:close()
    
    print("Deobfuscation complete!")
    print("Input: " .. inputPath .. " (" .. #content .. " bytes)")
    print("Output: " .. outputPath .. " (" .. #decrypted .. " bytes)")
    
    return true
end

if arg and #arg >= 3 then
    local mode = arg[1]
    local input = arg[2]
    local output = arg[3]
    
    if mode == "obf" or mode == "obfuscate" then
        obfuscateFile(input, output)
    elseif mode == "deobf" or mode == "deobfuscate" then
        deobfuscateFile(input, output)
    else
        print("Usage:")
        print("  lua obfuscator.lua obf <input.lua> <output.ibx>")
        print("  lua obfuscator.lua deobf <input.ibx> <output.lua>")
    end
else
    print("IrBox Obfuscator - Copyright (c) DeathAmir And IrAutoX")
    print("")
    print("Usage:")
    print("  lua obfuscator.lua obf <input.lua> <output.ibx>")
    print("  lua obfuscator.lua deobf <input.ibx> <output.lua>")
    print("")
    print("Example:")
    print("  lua obfuscator.lua obf parkour.lua parkour.ibx")
end

return CryptoEngine
