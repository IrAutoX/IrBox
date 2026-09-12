local ModelEncryptor = {}

ModelEncryptor.key = "IrBoxModelKey2024"

function ModelEncryptor.encrypt(data)
    local key = self.key
    local result = ""
    for i = 1, #data do
        local byte = string.byte(data, i)
        local keyByte = string.byte(key, (i - 1) % #key + 1)
        local encrypted = bit.bxor(byte, keyByte, 0xAA)
        result = result .. string.char(encrypted)
    end
    return result
end

function ModelEncryptor.decrypt(data)
    return self:encrypt(data)
end

function ModelEncryptor.saveModel(model, filename)
    local data = ModelEncryptor.serialize(model)
    local encrypted = ModelEncryptor:encrypt(data)
    local file = io.open(filename, "wb")
    if file then
        file:write(encrypted)
        file:close()
        return true
    end
    return false
end

function ModelEncryptor.loadModel(filename)
    local file = io.open(filename, "rb")
    if file then
        local encrypted = file:read("*all")
        file:close()
        local data = ModelEncryptor:decrypt(encrypted)
        return ModelEncryptor.deserialize(data)
    end
    return nil
end

function ModelEncryptor.serialize(model)
    local data = ""
    data = data .. ModelEncryptor.packInt(#model.vertices)
    data = data .. ModelEncryptor.packInt(#model.indices)
    for _, v in ipairs(model.vertices) do
        data = data .. ModelEncryptor.packFloat(v.position.x)
        data = data .. ModelEncryptor.packFloat(v.position.y)
        data = data .. ModelEncryptor.packFloat(v.position.z)
        data = data .. ModelEncryptor.packFloat(v.normal.x)
        data = data .. ModelEncryptor.packFloat(v.normal.y)
        data = data .. ModelEncryptor.packFloat(v.normal.z)
        data = data .. ModelEncryptor.packFloat(v.texCoord.x)
        data = data .. ModelEncryptor.packFloat(v.texCoord.y)
    end
    for _, idx in ipairs(model.indices) do
        data = data .. ModelEncryptor.packInt(idx)
    end
    return data
end

function ModelEncryptor.packInt(val)
    return string.char(bit.band(val, 0xFF), bit.band(bit.rshift(val, 8), 0xFF), 
                       bit.band(bit.rshift(val, 16), 0xFF), bit.band(bit.rshift(val, 24), 0xFF))
end

function ModelEncryptor.packFloat(val)
    -- Simplified float packing
    return string.char(0, 0, 0, 0)
end

function ModelEncryptor.deserialize(data)
    return {vertices = {}, indices = {}}
end

return ModelEncryptor
