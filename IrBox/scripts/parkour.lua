print("IrBox Parkour Script")
print("Copyright DeathAmir And IrAutoX")

local Players = {}
local Map = {}

function onInit()
    print("Parkour map initialized")
    createPlatform(0, -1, 0, 10, 1, 10)
    createObstacle(5, 0, 0, 2, 2, 2)
    createObstacle(-5, 1, 5, 2, 2, 2)
    createObstacle(0, 2, 10, 3, 1, 3)
end

function createPlatform(x, y, z, w, h, d)
    table.insert(Map, {
        type = "platform",
        position = {x = x, y = y, z = z},
        size = {w = w, h = h, d = d}
    })
end

function createObstacle(x, y, z, w, h, d)
    table.insert(Map, {
        type = "obstacle",
        position = {x = x, y = y, z = z},
        size = {w = w, h = h, d = d}
    })
end

function onPlayerJoin(playerId, username)
    print("Player joined: " .. username)
    Players[playerId] = {
        id = playerId,
        username = username,
        position = {x = 0, y = 0, z = 0},
        velocity = {x = 0, y = 0, z = 0},
        grounded = true
    }
end

function onPlayerLeave(playerId)
    print("Player left: " .. tostring(playerId))
    Players[playerId] = nil
end

function onUpdate(dt)
    for playerId, player in pairs(Players) do
        local gravity = -20.0
        player.velocity.y = player.velocity.y + gravity * dt
        player.position.y = player.position.y + player.velocity.y * dt
        
        if player.position.y <= 0 then
            player.position.y = 0
            player.velocity.y = 0
            player.grounded = true
        end
        
        if player.position.y < -10 then
            player.position.x = 0
            player.position.y = 0
            player.position.z = 0
            player.velocity.x = 0
            player.velocity.y = 0
            player.velocity.z = 0
        end
    end
end

onInit()