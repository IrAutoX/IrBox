irbox = {}

function irbox.showMessage(text)
    print("[UI] " .. text)
end

function irbox.spawnBlock(x, y, z, blockType)
    print("[PHYSICS] Spawning block at " .. x .. ", " .. y .. ", " .. z)
    return {x = x, y = y, z = z, type = blockType or 1}
end

function irbox.loadScript(name)
    print("[SCRIPT] Loading: " .. name)
end

function irbox.unlockAchievement(playerId, achievementId)
    print("[ACHIEVEMENT] Player " .. playerId .. " unlocked " .. achievementId)
end

parkour = {}
parkour.blocks = {}
parkour.player = {x = 0, y = 5, z = 0, vx = 0, vy = 0, vz = 0}

function parkour.init()
    irbox.showMessage("Welcome to Parkour!")
    for i = 0, 9 do
        table.insert(parkour.blocks, {x = i * 3, y = 0, z = 0, type = 1})
    end
    print("Parkour course created with 10 blocks")
end

function parkour.update(dt)
    local gravity = -9.8
    parkour.player.vy = parkour.player.vy + gravity * dt
    parkour.player.y = parkour.player.y + parkour.player.vy * dt
    
    if parkour.player.y < -10 then
        parkour.player.x = 0
        parkour.player.y = 5
        parkour.player.z = 0
        parkour.player.vy = 0
        print("Player died! Respawned.")
    end
    
    for i, block in ipairs(parkour.blocks) do
        if math.abs(parkour.player.x - block.x) < 1.5 and 
           math.abs(parkour.player.z - block.z) < 1.5 and
           parkour.player.y >= block.y and parkour.player.y < block.y + 1.5 then
            parkour.player.vy = 0
            parkour.player.y = block.y + 1.5
            
            if i == #parkour.blocks then
                irbox.unlockAchievement("player1", "master")
                print("ACHIEVEMENT UNLOCKED: استاد پارکور!")
            end
        end
    end
end

function parkour.jump()
    if parkour.player.vy == 0 or parkour.player.vy > -1 then
        parkour.player.vy = 5
        irbox.unlockAchievement("player1", "first_jump")
        print("Jump!")
    end
end

parkour.init()

while true do
    parkour.update(0.016)
    sleep(16)
end
