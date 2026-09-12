local CryptoEngine = {}

function CryptoEngine.obfuscate(code)
    local key = "IrBoxObf2024"
    local result = ""
    for i = 1, #code do
        local byte = string.byte(code, i)
        local keyByte = string.byte(key, (i - 1) % #key + 1)
        local encrypted = bit.bxor(byte, keyByte, 0xAA)
        result = result .. string.format("%02X", encrypted)
    end
    return result
end

function CryptoEngine.deobfuscate(hexData)
    local key = "IrBoxObf2024"
    local result = ""
    local i = 1
    while i <= #hexData do
        local hex = hexData:sub(i, i + 1)
        local byte = tonumber(hex, 16)
        local keyByte = string.byte(key, (#result) % #key + 1)
        local decrypted = bit.bxor(byte, keyByte, 0xAA)
        result = result .. string.char(decrypted)
        i = i + 2
    end
    return result
end

return CryptoEngine
