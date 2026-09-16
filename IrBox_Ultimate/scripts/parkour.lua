print_log("IrBox Parkour Game Script")
print_log("Copyright (c) DeathAmir And IrAutoX")

local player = {
    x = 0,
    y = 5,
    z = 0,
    velocityY = 0,
    grounded = false,
    health = 100,
    score = 0
}

local GRAVITY = -9.81
local JUMP_FORCE = 5.0
local TERMINAL_VELOCITY = -50.0

local blocks = {
    {x = 0, y = 0, z = 0, w = 10, h = 1, d = 10, type = "grass", solid = true},
    {x = 12, y = 1, z = 0, w = 3, h = 1, d = 3, type = "stone", solid = true},
    {x = 17, y = 2.5, z = 0, w = 3, h = 1, d = 3, type = "stone", solid = true},
    {x = 22, y = 4, z = 0, w = 3, h = 1, d = 3, type = "stone", solid = true},
    {x = 27, y = 5.5, z = 0, w = 3, h = 1, d = 3, type = "stone", solid = true},
    {x = 32, y = 7, z = 0, w = 5, h = 1, d = 5, type = "gold", solid = true}
}

local achievements = {
    first_jump = false,
    first_death = false,
    speedrun = false,
    perfectionist = false
}

function applyGravity(dt)
    player.velocityY = player.velocityY + GRAVITY * dt
    if player.velocityY < TERMINAL_VELOCITY then
        player.velocityY = TERMINAL_VELOCITY
    end
end

function checkCollision(x, y, z)
    local playerWidth = 0.6
    local playerHeight = 1.8
    
    for _, block in ipairs(blocks) do
        if block.solid then
            if x < block.x + block.w and x + playerWidth > block.x and
               y < block.y + block.h and y + playerHeight > block.y and
               z < block.z + block.d and z + playerWidth > block.z then
                return true
            end
        end
    end
    return false
end

function jump()
    if player.grounded then
        player.velocityY = JUMP_FORCE
        player.grounded = false
        print_log("Player jumped!")
        
        if not achievements.first_jump then
            achievements.first_jump = true
            give_achievement("first_jump")
        end
    end
end

function update(dt)
    applyGravity(dt)
    
    local newY = player.y + player.velocityY * dt
    
    if checkCollision(player.x, newY, player.z) then
        if player.velocityY < 0 then
            player.grounded = true
            player.velocityY = 0
            newY = math.ceil(newY)
        else
            player.velocityY = 0
            newY = player.y
        end
    else
        player.grounded = false
    end
    
    player.y = newY
    
    if player.y < -10 then
        player_die()
    end
end

function player_die()
    print_log("Player died! Respawning...")
    player.x = 0
    player.y = 5
    player.z = 0
    player.velocityY = 0
    player.grounded = false
    
    if not achievements.first_death then
        achievements.first_death = true
        give_achievement("first_death")
    end
    
    db_set("deaths", tostring(tonumber(db_get("deaths") or "0") + 1))
end

function spawn_block(x, y, z, blockType)
    table.insert(blocks, {
        x = x, y = y, z = z,
        w = 1, h = 1, d = 1,
        type = blockType or "stone",
        solid = true
    })
    print_log("Block spawned at (" .. x .. ", " .. y .. ", " .. z .. ")")
end

function destroy_block(index)
    if index >= 1 and index <= #blocks then
        local block = blocks[index]
        print_log("Block destroyed: " .. block.type)
        table.remove(blocks, index)
        player.score = player.score + 10
        
        if player.score >= 1000 then
            give_achievement("miner")
        end
    end
end

function check_win()
    if player.x >= 32 and player.y >= 7 then
        print_log("Congratulations! You completed the parkour!")
        player.score = player.score + 1000
        give_achievement("speedrun")
        give_achievement("perfectionist")
    end
end

function move_player(dx, dy, dz)
    player.x = player.x + dx
    player.y = player.y + dy
    player.z = player.z + dz
    
    if dy ~= 0 then
        check_win()
    end
end

db_set("player_name", db_get("player_name") or "Player")
db_set("play_time", tostring(tonumber(db_get("play_time") or "0") + 1))

print_log("Game initialized!")
print_log("Player: " .. db_get("player_name"))
print_log("Total play time: " .. db_get("play_time") .. " sessions")

return {
    update = update,
    jump = jump,
    move_player = move_player,
    spawn_block = spawn_block,
    destroy_block = destroy_block,
    get_player = function() return player end,
    get_blocks = function() return blocks end
}
