local CryptoEngine = {}

CryptoEngine.KEY = "IrBoxObf2024"

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

function CryptoEngine.toHex(str)
    local hex = ""
    for i = 1, #str do
        hex = hex .. string.format("%02x", string.byte(str, i))
    end
    return hex
end

function obfuscateScript(source)
    local encrypted = CryptoEngine.encrypt(source, CryptoEngine.KEY)
    return CryptoEngine.toHex(encrypted)
end

if arg and arg[1] then
    local file = io.open(arg[1], "r")
    if file then
        local source = file:read("*all")
        file:close()
        local obfuscated = obfuscateScript(source)
        
        local outFile = io.open(arg[1] .. ".obf", "w")
        outFile:write(obfuscated)
        outFile:close()
        
        print("Obfuscated: " .. arg[1] .. " -> " .. arg[1] .. ".obf")
        print("Original size: " .. #source .. " bytes")
        print("Obfuscated size: " .. #obfuscated .. " bytes")
    else
        print("Error: Cannot open file " .. arg[1])
    end
else
    print("Usage: lua obfuscator.lua <script.lua>")
    print("Output: script.lua.obf")
end
