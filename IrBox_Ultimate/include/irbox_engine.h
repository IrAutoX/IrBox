#ifndef IRBOX_ENGINE_H
#define IRBOX_ENGINE_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <lua.hpp>
#include <enet/enet.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <cstring>

#define IRBOX_VERSION "1.0.0"
#define IRBOX_COPYRIGHT "Copyright (c) 2024 DeathAmir And IrAutoX"
#define IRBOX_MODEL_KEY "IrBoxModelKey2024"
#define IRBOX_SCRIPT_KEY "IrBoxObf2024"
#define IRBOX_ADMIN_KEY "IrBoxAdmin2024"

namespace irbox {

struct Vector3 {
    float x, y, z;
    Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    void normalize() { float l = length(); if(l > 0) { x/=l; y/=l; z/=l; } }
};

struct Color {
    float r, g, b, a;
    Color(float _r=1, float _g=1, float _b=1, float _a=1) : r(_r), g(_g), b(_b), a(_a) {}
};

class CryptoEngine {
public:
    static std::string encrypt(const std::string& data, const std::string& key) {
        std::string result = data;
        for(size_t i = 0; i < data.size(); ++i) {
            result[i] = data[i] ^ key[i % key.size()] ^ (i & 0xFF);
        }
        return result;
    }
    
    static std::string decrypt(const std::string& data, const std::string& key) {
        return encrypt(data, key);
    }
    
    static std::string toHex(const std::string& data) {
        std::ostringstream oss;
        for(unsigned char c : data) {
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
        return oss.str();
    }
    
    static std::string fromHex(const std::string& hex) {
        std::string result;
        for(size_t i = 0; i < hex.length(); i += 2) {
            result += (char)std::stoi(hex.substr(i, 2), nullptr, 16);
        }
        return result;
    }
};

class Model {
private:
    std::string name;
    std::vector<Vector3> vertices;
    std::vector<unsigned int> indices;
    bool encrypted;
    
public:
    Model(const std::string& n) : name(n), encrypted(false) {}
    
    void addVertex(float x, float y, float z) {
        vertices.emplace_back(x, y, z);
    }
    
    void addIndex(unsigned int idx) {
        indices.push_back(idx);
    }
    
    void buildCube(float size = 1.0f) {
        float s = size / 2.0f;
        addVertex(-s, -s, -s); addVertex(s, -s, -s); addVertex(s, s, -s); addVertex(-s, s, -s);
        addVertex(-s, -s, s); addVertex(s, -s, s); addVertex(s, s, s); addVertex(-s, s, s);
        
        unsigned int base = 0;
        addIndex(base); addIndex(base+1); addIndex(base+2); addIndex(base); addIndex(base+2); addIndex(base+3);
        addIndex(base+4); addIndex(base+6); addIndex(base+5); addIndex(base+4); addIndex(base+7); addIndex(base+6);
        addIndex(base); addIndex(base+3); addIndex(base+7); addIndex(base); addIndex(base+7); addIndex(base+4);
        addIndex(base+1); addIndex(base+5); addIndex(base+6); addIndex(base+1); addIndex(base+6); addIndex(base+2);
        addIndex(base+3); addIndex(base+2); addIndex(base+6); addIndex(base+3); addIndex(base+6); addIndex(base+7);
        addIndex(base); addIndex(base+4); addIndex(base+5); addIndex(base); addIndex(base+5); addIndex(base+1);
    }
    
    std::string serialize() const {
        std::ostringstream oss;
        oss << vertices.size() << " ";
        for(const auto& v : vertices) oss << v.x << " " << v.y << " " << v.z << " ";
        oss << indices.size() << " ";
        for(auto idx : indices) oss << idx << " ";
        return oss.str();
    }
    
    std::string encryptModel() const {
        std::string data = serialize();
        return CryptoEngine::encrypt(data, IRBOX_MODEL_KEY);
    }
    
    bool saveToFile(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if(!file) return false;
        std::string encrypted = encryptModel();
        file.write(encrypted.c_str(), encrypted.size());
        return true;
    }
    
    const std::string& getName() const { return name; }
    size_t getVertexCount() const { return vertices.size(); }
    size_t getIndexCount() const { return indices.size(); }
};

class FontManager {
private:
    struct Glyph {
        unsigned int codepoint;
        int x, y, w, h;
        float u1, v1, u2, v2;
        int advance;
    };
    std::unordered_map<unsigned int, Glyph> glyphs;
    bool rtlEnabled;
    
public:
    FontManager() : rtlEnabled(true) {}
    
    void enableRTL(bool enable) { rtlEnabled = enable; }
    bool isRTLEnabled() const { return rtlEnabled; }
    
    std::string processText(const std::string& input) {
        if(!rtlEnabled) return input;
        std::string result = input;
        std::reverse(result.begin(), result.end());
        for(size_t i = 0; i < result.size(); ++i) {
            if(result[i] >= 0x0600 && result[i] <= 0x06FF) {
                continue;
            }
        }
        return result;
    }
};

class Achievement {
private:
    std::string id;
    std::string title;
    std::string description;
    bool unlocked;
    
public:
    Achievement(const std::string& _id, const std::string& _title, const std::string& _desc)
        : id(_id), title(_title), description(_desc), unlocked(false) {}
    
    void unlock() { unlocked = true; }
    bool isUnlocked() const { return unlocked; }
    const std::string& getId() const { return id; }
    const std::string& getTitle() const { return title; }
    const std::string& getDescription() const { return description; }
};

class AchievementSystem {
private:
    std::unordered_map<std::string, Achievement> achievements;
    
public:
    void registerAchievement(const std::string& id, const std::string& title, const std::string& desc) {
        achievements[id] = Achievement(id, title, desc);
    }
    
    void unlockAchievement(const std::string& id) {
        if(achievements.find(id) != achievements.end()) {
            achievements[id].unlock();
        }
    }
    
    bool isUnlocked(const std::string& id) const {
        auto it = achievements.find(id);
        return it != achievements.end() && it->second.isUnlocked();
    }
    
    int getUnlockedCount() const {
        int count = 0;
        for(const auto& pair : achievements) {
            if(pair.second.isUnlocked()) count++;
        }
        return count;
    }
};

class PhysicsBody {
private:
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    float mass;
    bool isStatic;
    
public:
    PhysicsBody(float m = 1.0f) : mass(m), isStatic(false) {}
    
    void setPosition(const Vector3& pos) { position = pos; }
    void setVelocity(const Vector3& vel) { velocity = vel; }
    void applyForce(const Vector3& force) {
        if(!isStatic) {
            acceleration = acceleration + (force * (1.0f / mass));
        }
    }
    
    void update(float deltaTime, const Vector3& gravity) {
        if(isStatic) return;
        velocity = velocity + (acceleration + gravity) * deltaTime;
        position = position + velocity * deltaTime;
        acceleration = Vector3(0, 0, 0);
    }
    
    const Vector3& getPosition() const { return position; }
    const Vector3& getVelocity() const { return velocity; }
    void setStatic(bool s) { isStatic = s; }
};

class Block {
private:
    int type;
    Vector3 position;
    bool solid;
    std::string name;
    
public:
    Block(int t, float x, float y, float z, const std::string& n, bool s = true)
        : type(t), position(x, y, z), solid(s), name(n) {}
    
    int getType() const { return type; }
    const Vector3& getPosition() const { return position; }
    bool isSolid() const { return solid; }
    const std::string& getName() const { return name; }
};

class LuaStateManager {
private:
    lua_State* L;
    std::unordered_map<std::string, std::string> loadedScripts;
    
    static int printToConsole(lua_State* L) {
        int n = lua_gettop(L);
        for(int i = 1; i <= n; i++) {
            if(lua_type(L, i) == LUA_TSTRING) {
                printf("%s ", lua_tostring(L, i));
            }
        }
        printf("\n");
        return 0;
    }
    
public:
    LuaStateManager() {
        L = luaL_newstate();
        luaL_openlibs(L);
        lua_register(L, "print", printToConsole);
        registerIrBoxAPI();
    }
    
    ~LuaStateManager() {
        if(L) lua_close(L);
    }
    
    void registerIrBoxAPI() {
        lua_newtable(L);
        
        lua_pushcfunction(L, [](lua_State* L) -> int {
            const char* text = lua_tostring(L, 1);
            printf("[IRBOX UI] %s\n", text ? text : "");
            return 0;
        });
        lua_setfield(L, -2, "showMessage");
        
        lua_pushcfunction(L, [](lua_State* L) -> int {
            const char* playerId = lua_tostring(L, 1);
            const char* achievementId = lua_tostring(L, 2);
            printf("[ACHIEVEMENT] Player %s unlocked %s\n", playerId, achievementId);
            return 0;
        });
        lua_setfield(L, -2, "unlockAchievement");
        
        lua_pushcfunction(L, [](lua_State* L) -> int {
            double x = lua_tonumber(L, 1);
            double y = lua_tonumber(L, 2);
            double z = lua_tonumber(L, 3);
            printf("[PHYSICS] Spawned block at (%.2f, %.2f, %.2f)\n", x, y, z);
            return 0;
        });
        lua_setfield(L, -2, "spawnBlock");
        
        lua_pushcfunction(L, [](lua_State* L) -> int {
            const char* scriptName = lua_tostring(L, 1);
            printf("[SCRIPT] Loaded script: %s\n", scriptName);
            return 0;
        });
        lua_setfield(L, -2, "loadScript");
        
        lua_setglobal(L, "irbox");
    }
    
    bool loadScript(const std::string& name, const std::string& content) {
        std::string decrypted = CryptoEngine::decrypt(CryptoEngine::fromHex(content), IRBOX_SCRIPT_KEY);
        if(luaL_loadbuffer(L, decrypted.c_str(), decrypted.size(), name.c_str()) != 0) {
            printf("Lua error loading %s: %s\n", name.c_str(), lua_tostring(L, -1));
            lua_pop(L, 1);
            return false;
        }
        if(lua_pcall(L, 0, 0, 0) != 0) {
            printf("Lua error running %s: %s\n", name.c_str(), lua_tostring(L, -1));
            lua_pop(L, 1);
            return false;
        }
        loadedScripts[name] = content;
        return true;
    }
    
    std::string obfuscateScript(const std::string& source) {
        std::string encrypted = CryptoEngine::encrypt(source, IRBOX_SCRIPT_KEY);
        return CryptoEngine::toHex(encrypted);
    }
    
    lua_State* getState() { return L; }
};

class NetworkServer {
private:
    ENetAddress address;
    ENetHost* host;
    ENetPeer* peers[64];
    int peerCount;
    bool running;
    LuaStateManager* luaState;
    
public:
    NetworkServer() : host(nullptr), peerCount(0), running(false), luaState(nullptr) {
        for(int i = 0; i < 64; i++) peers[i] = nullptr;
    }
    
    bool initialize(int port, LuaStateManager* state) {
        if(enet_initialize() != 0) return false;
        address.host = ENET_HOST_ANY;
        address.port = port;
        host = enet_host_create(&address, 64, 2, 0, 0);
        if(!host) { enet_deinitialize(); return false; }
        luaState = state;
        running = true;
        printf("[SERVER] Started on port %d\n", port);
        return true;
    }
    
    void update() {
        if(!host || !running) return;
        ENetEvent event;
        while(enet_host_service(host, &event, 0) > 0) {
            switch(event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("[SERVER] Client connected from %X\n", event.peer->address.host);
                    if(peerCount < 64) peers[peerCount++] = event.peer;
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("[SERVER] Client disconnected\n");
                    for(int i = 0; i < peerCount; i++) {
                        if(peers[i] == event.peer) {
                            peers[i] = peers[--peerCount];
                            break;
                        }
                    }
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    handlePacket(event.peer, event.packet);
                    enet_packet_destroy(event.packet);
                    break;
            }
        }
    }
    
    void handlePacket(ENetPeer* peer, ENetPacket* packet) {
        if(packet->dataLength < 1) return;
        char type = ((char*)packet->data)[0];
        if(type == 1 && luaState) {
            std::string script((char*)packet->data + 1, packet->dataLength - 1);
            luaState->loadScript("remote_script", script);
        }
    }
    
    void shutdown() {
        running = false;
        if(host) {
            enet_host_flush(host);
            enet_host_destroy(host);
            host = nullptr;
        }
        enet_deinitialize();
        printf("[SERVER] Shutdown complete\n");
    }
    
    bool isAdmin(const std::string& key) {
        return key == IRBOX_ADMIN_KEY;
    }
};

class Database {
private:
    std::string dbFile;
    std::unordered_map<std::string, std::string> data;
    
public:
    Database(const std::string& filename) : dbFile(filename) {
        load();
    }
    
    void load() {
        std::ifstream file(dbFile);
        if(file) {
            std::string line;
            while(std::getline(file, line)) {
                size_t pos = line.find('=');
                if(pos != std::string::npos) {
                    data[line.substr(0, pos)] = line.substr(pos + 1);
                }
            }
        }
    }
    
    void save() {
        std::ofstream file(dbFile);
        for(const auto& pair : data) {
            file << pair.first << "=" << pair.second << "\n";
        }
    }
    
    void set(const std::string& key, const std::string& value) {
        data[key] = value;
    }
    
    std::string get(const std::string& key, const std::string& def = "") {
        auto it = data.find(key);
        return it != data.end() ? it->second : def;
    }
    
    bool exists(const std::string& key) {
        return data.find(key) != data.end();
    }
};

class UIManager {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    FontManager fontMgr;
    AchievementSystem achievements;
    
public:
    UIManager() : window(nullptr), renderer(nullptr), running(false) {}
    
    bool initialize(const char* title, int width, int height) {
        if(SDL_Init(SDL_INIT_VIDEO) < 0) return false;
        window = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL);
        if(!window) return false;
        renderer = SDL_CreateRenderer(window, nullptr);
        if(!renderer) return false;
        running = true;
        
        achievements.registerAchievement("first_jump", "اولین پرش", "اولین پرش در پارکور را انجام دهید");
        achievements.registerAchievement("master", "استاد پارکور", "به پایان مرحله پارکور برسید");
        achievements.registerAchievement("collector", "کلکسیونر", "۱۰ بلاک جمع آوری کنید");
        
        return true;
    }
    
    void renderMainMenu() {
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 0, 102, 255, 255);
        SDL_Rect logoRect = {width/2 - 100, 100, 200, 60};
        SDL_RenderFillRect(renderer, &logoRect);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect checkRect = {width/2 - 20, 115, 40, 30};
        SDL_RenderFillRect(renderer, &checkRect);
        
        drawText("IrBox", width/2 - 40, 180, Color(1,1,1,1));
        drawText("بازی پارکور سه بعدی", width/2 - 80, 220, Color(0.8f,0.8f,0.8f,1));
        
        drawButton("شروع بازی", width/2 - 75, 300, 150, 40);
        drawButton("تنظیمات", width/2 - 75, 350, 150, 40);
        drawButton("پروفایل", width/2 - 75, 400, 150, 40);
        drawButton("خروج", width/2 - 75, 450, 150, 40);
        
        drawText(IRBOX_COPYRIGHT, width/2 - 100, height - 30, Color(0.5f,0.5f,0.5f,1));
    }
    
    void drawButton(const char* text, int x, int y, int w, int h) {
        SDL_SetRenderDrawColor(renderer, 0, 102, 255, 200);
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect border = {x-2, y-2, w+4, h+4};
        SDL_RenderDrawRect(renderer, &border);
        drawText(text, x + w/2 - strlen(text)*4, y + h/2 - 6, Color(1,1,1,1));
    }
    
    void drawText(const char* text, int x, int y, Color color) {
        SDL_SetRenderDrawColor(renderer, (int)(color.r*255), (int)(color.g*255), (int)(color.b*255), (int)(color.a*255));
    }
    
    int width, height;
    bool pollEvents(SDL_Event* event) {
        return SDL_PollEvent(event);
    }
    
    void present() {
        SDL_RenderPresent(renderer);
    }
    
    void clear() {
        SDL_RenderClear(renderer);
    }
    
    bool isRunning() const { return running; }
    void setRunning(bool r) { running = r; }
    
    AchievementSystem& getAchievements() { return achievements; }
    FontManager& getFontManager() { return fontMgr; }
    
    void shutdown() {
        if(renderer) SDL_DestroyRenderer(renderer);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

class GameEngine {
private:
    UIManager ui;
    LuaStateManager luaState;
    PhysicsBody player;
    std::vector<Block> blocks;
    AchievementSystem achievements;
    bool inGame;
    int collectedBlocks;
    
public:
    GameEngine() : inGame(false), collectedBlocks(0) {
        player.setPosition(Vector3(0, 5, 0));
        player.setVelocity(Vector3(0, 0, 0));
        
        for(int i = 0; i < 10; i++) {
            blocks.emplace_back(1, i * 3.0f, 0, 0, "platform", true);
        }
    }
    
    bool initialize() {
        if(!ui.initialize("IrBox - DeathAmir And IrAutoX", 1280, 720)) {
            return false;
        }
        printf("[ENGINE] Initialized successfully\n");
        return true;
    }
    
    void run() {
        SDL_Event event;
        while(ui.isRunning()) {
            while(ui.pollEvents(&event)) {
                if(event.type == SDL_EVENT_QUIT) {
                    ui.setRunning(false);
                }
                if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && !inGame) {
                    int mx = event.button.x, my = event.button.y;
                    if(mx > 565 && mx < 715 && my > 300 && my < 340) {
                        inGame = true;
                        startGame();
                    }
                    if(mx > 565 && mx < 715 && my > 450 && my < 490) {
                        ui.setRunning(false);
                    }
                }
            }
            
            ui.clear();
            if(inGame) {
                updateGame(0.016f);
                renderGame();
            } else {
                ui.renderMainMenu();
            }
            ui.present();
            SDL_Delay(16);
        }
    }
    
    void startGame() {
        printf("[GAME] Starting parkour game...\n");
        player.setPosition(Vector3(0, 5, 0));
        player.setVelocity(Vector3(0, 0, 0));
        collectedBlocks = 0;
    }
    
    void updateGame(float dt) {
        Vector3 gravity(0, -9.8f, 0);
        player.update(dt, gravity);
        
        if(player.getPosition().y < -10) {
            player.setPosition(Vector3(0, 5, 0));
            player.setVelocity(Vector3(0, 0, 0));
            printf("[GAME] Player died! Respawned.\n");
        }
        
        for(size_t i = 0; i < blocks.size(); i++) {
            Vector3 pos = player.getPosition();
            Vector3 bpos = blocks[i].getPosition();
            if(fabs(pos.x - bpos.x) < 1.5f && fabs(pos.z - bpos.z) < 1.5f &&
               pos.y >= bpos.y && pos.y < bpos.y + 1.5f) {
                player.setVelocity(Vector3(player.getVelocity().x, 0, player.getVelocity().z));
                player.setPosition(Vector3(pos.x, bpos.y + 1.5f, pos.z));
                
                if(i == blocks.size() - 1) {
                    achievements.unlockAchievement("master");
                    printf("[ACHIEVEMENT] Unlocked: استاد پارکور!\n");
                }
            }
        }
        
        if(collectedBlocks >= 10) {
            achievements.unlockAchievement("collector");
        }
    }
    
    void renderGame() {
        ui.clear();
        SDL_SetRenderDrawColor(ui.renderer, 135, 206, 235, 255);
        SDL_Rect screen = {0, 0, 1280, 720};
        SDL_RenderFillRect(ui.renderer, &screen);
        
        for(size_t i = 0; i < blocks.size(); i++) {
            int bx = 400 + i * 100;
            int by = 400;
            SDL_SetRenderDrawColor(ui.renderer, 139, 69, 19, 255);
            SDL_Rect brect = {bx, by, 80, 60};
            SDL_RenderFillRect(ui.renderer, &brect);
        }
        
        int px = 400;
        int py = 300 - (int)(player.getPosition().y * 20);
        SDL_SetRenderDrawColor(ui.renderer, 255, 0, 0, 255);
        SDL_Rect prect = {px, py, 30, 30};
        SDL_RenderFillRect(ui.renderer, &prect);
        
        char scoreText[64];
        sprintf(scoreText, "بلاک‌های جمع‌آوری شده: %d", collectedBlocks);
        ui.drawText(scoreText, 20, 20, Color(1,1,1,1));
        
        if(achievements.isUnlocked("master")) {
            ui.drawText("تبریک! شما برنده شدید!", 400, 100, Color(0,1,0,1));
        }
    }
    
    void shutdown() {
        ui.shutdown();
        printf("[ENGINE] Shutdown complete\n");
    }
};

} 

#endif
