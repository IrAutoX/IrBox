#ifndef IRBOX_ENGINE_H
#define IRBOX_ENGINE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <thread>
#include <atomic>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "enet/enet.h"

namespace IrBox {

struct Vector3 {
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Vector3 operator+(const Vector3& o) const { return Vector3(x+o.x, y+o.y, z+o.z); }
    Vector3 operator-(const Vector3& o) const { return Vector3(x-o.x, y-o.y, z-o.z); }
    Vector3 operator*(float s) const { return Vector3(x*s, y*s, z*s); }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vector3 normalize() const { float l = length(); return l > 0 ? *this * (1.0f/l) : Vector3(); }
};

struct Color {
    float r, g, b, a;
    Color(float _r=1, float _g=1, float _b=1, float _a=1) : r(_r), g(_g), b(_b), a(_a) {}
};

struct Model {
    std::string id;
    std::vector<Vector3> vertices;
    std::vector<unsigned int> indices;
    std::string texturePath;
    bool isEncrypted;
    
    void encrypt(const std::string& key) {
        isEncrypted = true;
        for(auto& v : vertices) {
            v.x = std::sin(v.x) * key.length();
            v.y = std::cos(v.y) * key.length();
            v.z = std::tan(v.z + 1) * key.length();
        }
    }
    
    void decrypt(const std::string& key) {
        if(!isEncrypted) return;
        for(auto& v : vertices) {
            v.x = std::asin(v.x / key.length());
            v.y = std::acos(v.y / key.length());
            v.z = std::atan(v.z / key.length()) - 1;
        }
        isEncrypted = false;
    }
};

struct Block {
    Vector3 position;
    Vector3 size;
    std::string type;
    bool solid;
    bool destructible;
    int health;
    std::string dropItem;
    
    Block() : size(1,1,1), solid(true), destructible(true), health(100) {}
};

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    bool unlocked;
    std::function<void()> onUnlock;
};

struct Player {
    std::string username;
    std::string userId;
    Vector3 position;
    Vector3 velocity;
    bool isGrounded;
    int health;
    int score;
    std::vector<std::string> inventory;
    std::unordered_map<std::string, bool> achievements;
    
    Player() : health(100), score(0), isGrounded(false) {}
};

class CryptoEngine {
public:
    static std::string encrypt(const std::string& data, const std::string& key) {
        std::string result = data;
        for(size_t i = 0; i < data.size(); ++i) {
            result[i] = data[i] ^ key[i % key.size()] ^ 0x42;
        }
        std::string hex;
        char buf[3];
        for(unsigned char c : result) {
            sprintf(buf, "%02X", c);
            hex += buf;
        }
        return hex;
    }
    
    static std::string decrypt(const std::string& hexData, const std::string& key) {
        std::string raw;
        for(size_t i = 0; i < hexData.size(); i += 2) {
            unsigned int byte;
            sscanf(hexData.substr(i, 2).c_str(), "%x", &byte);
            raw += (char)byte;
        }
        std::string result = raw;
        for(size_t i = 0; i < raw.size(); ++i) {
            result[i] = raw[i] ^ key[i % key.size()] ^ 0x42;
        }
        return result;
    }
};

class Database {
private:
    std::unordered_map<std::string, std::string> data;
    std::mutex dbMutex;
    std::string dbFile;
    
public:
    Database(const std::string& filename = "irbox.db") : dbFile(filename) {
        load();
    }
    
    void set(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(dbMutex);
        data[key] = CryptoEngine::encrypt(value, "IrBoxDBKey2024");
        save();
    }
    
    std::string get(const std::string& key) {
        std::lock_guard<std::mutex> lock(dbMutex);
        auto it = data.find(key);
        if(it != data.end()) {
            return CryptoEngine::decrypt(it->second, "IrBoxDBKey2024");
        }
        return "";
    }
    
    bool exists(const std::string& key) {
        std::lock_guard<std::mutex> lock(dbMutex);
        return data.find(key) != data.end();
    }
    
    void remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(dbMutex);
        data.erase(key);
        save();
    }
    
private:
    void load() {
        std::ifstream file(dbFile);
        if(file.is_open()) {
            std::string line;
            while(std::getline(file, line)) {
                size_t pos = line.find('=');
                if(pos != std::string::npos) {
                    data[line.substr(0, pos)] = line.substr(pos+1);
                }
            }
        }
    }
    
    void save() {
        std::ofstream file(dbFile);
        for(const auto& kv : data) {
            file << kv.first << "=" << kv.second << "\n";
        }
    }
};

class LuaStateManager {
private:
    lua_State* L;
    Database* db;
    std::unordered_map<std::string, std::function<int(lua_State*)>> registeredFuncs;
    
public:
    LuaStateManager() {
        L = luaL_newstate();
        luaL_openlibs(L);
        db = nullptr;
        registerCoreFunctions();
    }
    
    ~LuaStateManager() {
        if(L) lua_close(L);
    }
    
    void setDatabase(Database* database) {
        db = database;
    }
    
    void registerCoreFunctions() {
        lua_register(L, "print_log", luaPrintLog);
        lua_register(L, "db_set", luaDbSet);
        lua_register(L, "db_get", luaDbGet);
        lua_register(L, "spawn_block", luaSpawnBlock);
        lua_register(L, "destroy_block", luaDestroyBlock);
        lua_register(L, "give_achievement", luaGiveAchievement);
        lua_register(L, "player_die", luaPlayerDie);
        lua_register(L, "encrypt_model", luaEncryptModel);
        lua_register(L, "send_network_packet", luaSendNetworkPacket);
    }
    
    bool loadScript(const std::string& scriptPath, const std::string& encryptKey = "") {
        std::ifstream file(scriptPath, std::ios::binary);
        if(!file.is_open()) return false;
        
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        
        if(!encryptKey.empty()) {
            content = CryptoEngine::decrypt(content, encryptKey);
        }
        
        if(luaL_loadstring(L, content.c_str()) || lua_pcall(L, 0, 0, 0)) {
            const char* err = lua_tostring(L, -1);
            printf("[Lua Error] %s\n", err);
            lua_pop(L, 1);
            return false;
        }
        return true;
    }
    
    bool loadObfuscatedScript(const std::string& hexScript, const std::string& key) {
        std::string decrypted = CryptoEngine::decrypt(hexScript, key);
        if(luaL_loadstring(L, decrypted.c_str()) || lua_pcall(L, 0, 0, 0)) {
            const char* err = lua_tostring(L, -1);
            printf("[Lua Obf Error] %s\n", err);
            lua_pop(L, 1);
            return false;
        }
        return true;
    }
    
    void pushVector3(const Vector3& v) {
        lua_newtable(L);
        lua_pushnumber(L, v.x); lua_setfield(L, -2, "x");
        lua_pushnumber(L, v.y); lua_setfield(L, -2, "y");
        lua_pushnumber(L, v.z); lua_setfield(L, -2, "z");
    }
    
    Vector3 getVector3(int idx) {
        Vector3 v;
        lua_getfield(L, idx, "x"); v.x = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_getfield(L, idx, "y"); v.y = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_getfield(L, idx, "z"); v.z = lua_tonumber(L, -1); lua_pop(L, 1);
        return v;
    }
    
    lua_State* getState() { return L; }
    
private:
    static int luaPrintLog(lua_State* L) {
        const char* msg = lua_tostring(L, 1);
        printf("[Lua] %s\n", msg);
        return 0;
    }
    
    static int luaDbSet(lua_State* L) {
        LuaStateManager* self = (LuaStateManager*)lua_touserdata(L, lua_upvalueindex(1));
        if(!self || !self->db) return 0;
        const char* key = lua_tostring(L, 1);
        const char* val = lua_tostring(L, 2);
        self->db->set(key, val);
        return 0;
    }
    
    static int luaDbGet(lua_State* L) {
        LuaStateManager* self = (LuaStateManager*)lua_touserdata(L, lua_upvalueindex(1));
        if(!self || !self->db) return 0;
        const char* key = lua_tostring(L, 1);
        std::string val = self->db->get(key);
        lua_pushstring(L, val.c_str());
        return 1;
    }
    
    static int luaSpawnBlock(lua_State* L) {
        printf("[Lua] Spawn block called\n");
        return 0;
    }
    
    static int luaDestroyBlock(lua_State* L) {
        printf("[Lua] Destroy block called\n");
        return 0;
    }
    
    static int luaGiveAchievement(lua_State* L) {
        const char* achId = lua_tostring(L, 1);
        printf("[Lua] Achievement unlocked: %s\n", achId);
        return 0;
    }
    
    static int luaPlayerDie(lua_State* L) {
        printf("[Lua] Player died\n");
        return 0;
    }
    
    static int luaEncryptModel(lua_State* L) {
        const char* modelPath = lua_tostring(L, 1);
        const char* key = lua_tostring(L, 2);
        printf("[Lua] Encrypting model: %s\n", modelPath);
        return 0;
    }
    
    static int luaSendNetworkPacket(lua_State* L) {
        const char* packetType = lua_tostring(L, 1);
        printf("[Lua] Sending network packet: %s\n", packetType);
        return 0;
    }
};

class NetworkManager {
private:
    ENetHost* host;
    ENetPeer* serverPeer;
    std::atomic<bool> connected;
    std::thread networkThread;
    std::mutex packetMutex;
    std::vector<std::pair<std::string, std::string>> outgoingPackets;
    
public:
    NetworkManager() : host(nullptr), serverPeer(nullptr), connected(false) {}
    
    ~NetworkManager() {
        disconnect();
    }
    
    bool connectToServer(const std::string& address, int port) {
        if(enet_initialize() != 0) return false;
        
        host = enet_host_create(NULL, 1, 2, 0, 0);
        if(!host) return false;
        
        ENetAddress serverAddress;
        enet_address_set_host(&serverAddress, address.c_str());
        serverAddress.port = port;
        
        serverPeer = enet_host_connect(host, &serverAddress, 2, 0);
        if(!serverPeer) return false;
        
        ENetEvent event;
        if(enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            connected = true;
            startNetworkThread();
            return true;
        }
        return false;
    }
    
    void disconnect() {
        connected = false;
        if(networkThread.joinable()) networkThread.join();
        if(serverPeer) enet_peer_disconnect(serverPeer, 0);
        if(host) enet_host_destroy(host);
        host = nullptr;
        serverPeer = nullptr;
        enet_deinitialize();
    }
    
    void sendPacket(const std::string& type, const std::string& data) {
        std::lock_guard<std::mutex> lock(packetMutex);
        std::string encrypted = CryptoEngine::encrypt(data, "IrBoxNetKey2024");
        outgoingPackets.push_back({type, encrypted});
    }
    
    void sendScriptToServer(const std::string& scriptName, const std::string& scriptContent) {
        std::string obfuscated = CryptoEngine::encrypt(scriptContent, "IrBoxObf2024");
        sendPacket("SCRIPT_UPLOAD", scriptName + "|" + obfuscated);
    }
    
    bool isConnected() const { return connected; }
    
private:
    void startNetworkThread() {
        networkThread = std::thread([this]() {
            while(connected) {
                ENetEvent event;
                if(enet_host_service(host, &event, 10) > 0) {
                    switch(event.type) {
                        case ENET_EVENT_TYPE_RECEIVE:
                            handlePacket(event.packet);
                            enet_packet_destroy(event.packet);
                            break;
                        case ENET_EVENT_TYPE_DISCONNECT:
                            connected = false;
                            break;
                        default: break;
                    }
                }
                
                std::lock_guard<std::mutex> lock(packetMutex);
                for(auto& pkt : outgoingPackets) {
                    ENetPacket* packet = enet_packet_create(
                        (pkt.first + "|" + pkt.second).c_str(),
                        pkt.first.size() + pkt.second.size() + 1,
                        ENET_PACKET_FLAG_RELIABLE
                    );
                    enet_peer_send(serverPeer, 0, packet);
                }
                outgoingPackets.clear();
                
                enet_host_flush(host);
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        });
    }
    
    void handlePacket(ENetPacket* packet) {
        std::string data((char*)packet->data, packet->dataLength);
        size_t sep = data.find('|');
        if(sep != std::string::npos) {
            std::string type = data.substr(0, sep);
            std::string content = data.substr(sep+1);
            
            if(type == "SCRIPT_DOWNLOAD") {
                std::string decrypted = CryptoEngine::decrypt(content, "IrBoxObf2024");
                printf("[Network] Script downloaded and decrypted\n");
            } else if(type == "MODEL_SYNC") {
                printf("[Network] Model sync received\n");
            }
        }
    }
};

class UIManager {
public:
    struct Button {
        std::string text;
        float x, y, w, h;
        std::function<void()> onClick;
        bool hovered;
    };
    
    struct Panel {
        std::string title;
        float x, y, w, h;
        bool visible;
        bool draggable;
    };
    
    struct Label {
        std::string text;
        float x, y;
        Color color;
        bool rtl;
    };

private:
    std::vector<Button> buttons;
    std::vector<Panel> panels;
    std::vector<Label> labels;
    class FontManager* fontMgr;
    
public:
    UIManager() : fontMgr(nullptr) {}
    
    void setFontManager(class FontManager* fm) { fontMgr = fm; }
    
    void addButton(const std::string& text, float x, float y, float w, float h, std::function<void()> onClick) {
        buttons.push_back({text, x, y, w, h, onClick, false});
    }
    
    void addPanel(const std::string& title, float x, float y, float w, float h, bool draggable = true) {
        panels.push_back({title, x, y, w, h, true, draggable});
    }
    
    void addLabel(const std::string& text, float x, float y, Color color = Color(), bool rtl = false) {
        labels.push_back({text, x, y, color, rtl});
    }
    
    void render() {
        for(auto& panel : panels) {
            if(panel.visible) renderPanel(panel);
        }
        for(auto& label : labels) {
            renderLabel(label);
        }
        for(auto& btn : buttons) {
            renderButton(btn);
        }
    }
    
    void handleMouseClick(float mx, float my) {
        for(auto& btn : buttons) {
            if(mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                if(btn.onClick) btn.onClick();
            }
        }
    }
    
private:
    void renderPanel(const Panel& p) {
        printf("[UI] Render panel: %s at (%.1f, %.1f)\n", p.title.c_str(), p.x, p.y);
    }
    
    void renderLabel(const Label& l) {
        std::string displayText = l.text;
        if(l.rtl) {
            displayText = processRTL(l.text);
        }
        printf("[UI] Render label: %s at (%.1f, %.1f)\n", displayText.c_str(), l.x, l.y);
    }
    
    std::string processRTL(const std::string& text) {
        return text;
    }
    
    void renderButton(const Button& b) {
        printf("[UI] Render button: %s at (%.1f, %.1f)\n", b.text.c_str(), b.x, b.y);
    }
};

class FontManager {
private:
    std::unordered_map<std::string, void*> fonts;
    std::unordered_map<wchar_t, std::vector<unsigned short>> persianGlyphs;
    
public:
    FontManager() {
        initPersianGlyphs();
    }
    
    bool loadFont(const std::string& name, const std::string& path) {
        printf("[Font] Loading: %s from %s\n", name.c_str(), path.c_str());
        fonts[name] = (void*)1;
        return true;
    }
    
    std::string processRTL(const std::string& text) {
        std::wstring wide(text.begin(), text.end());
        std::wstring processed;
        
        for(size_t i = 0; i < wide.size(); ++i) {
            wchar_t c = wide[i];
            if(c >= 0x0600 && c <= 0x06FF) {
                bool connected = (i < wide.size()-1 && wide[i+1] >= 0x0600 && wide[i+1] <= 0x06FF);
                processed += c;
                if(connected) processed += L'\u200D';
            } else {
                processed += c;
            }
        }
        
        return std::string(processed.begin(), processed.end());
    }
    
    void initPersianGlyphs() {
        persianGlyphs[L'آ'] = {0x0622};
        persianGlyphs[L'ب'] = {0x0628, 0x0627, 0x062A};
        persianGlyphs[L'پ'] = {0x067E};
        persianGlyphs[L'ت'] = {0x062A};
        persianGlyphs[L'ث'] = {0x062B};
        persianGlyphs[L'ج'] = {0x062C};
        persianGlyphs[L'چ'] = {0x0686};
        persianGlyphs[L'ح'] = {0x062D};
        persianGlyphs[L'خ'] = {0x062E};
        persianGlyphs[L'د'] = {0x062F};
        persianGlyphs[L'ذ'] = {0x0630};
        persianGlyphs[L'ر'] = {0x0631};
        persianGlyphs[L'ز'] = {0x0632};
        persianGlyphs[L'ژ'] = {0x0698};
        persianGlyphs[L'س'] = {0x0633};
        persianGlyphs[L'ش'] = {0x0634};
        persianGlyphs[L'ص'] = {0x0635};
        persianGlyphs[L'ض'] = {0x0636};
        persianGlyphs[L'ط'] = {0x0637};
        persianGlyphs[L'ظ'] = {0x0638};
        persianGlyphs[L'ع'] = {0x0639};
        persianGlyphs[L'غ'] = {0x063A};
        persianGlyphs[L'ف'] = {0x0641};
        persianGlyphs[L'ق'] = {0x0642};
        persianGlyphs[L'ک'] = {0x06A9};
        persianGlyphs[L'گ'] = {0x06AF};
        persianGlyphs[L'ل'] = {0x0644};
        persianGlyphs[L'م'] = {0x0645};
        persianGlyphs[L'ن'] = {0x0646};
        persianGlyphs[L'و'] = {0x0648};
        persianGlyphs[L'ه'] = {0x0647};
        persianGlyphs[L'ی'] = {0x06CC};
    }
};

class PhysicsEngine {
public:
    static constexpr float GRAVITY = -9.81f;
    static constexpr float TERMINAL_VELOCITY = -50.0f;
    
    static Vector3 applyGravity(const Vector3& velocity, float deltaTime) {
        Vector3 newVel = velocity + Vector3(0, GRAVITY, 0) * deltaTime;
        if(newVel.y < TERMINAL_VELOCITY) newVel.y = TERMINAL_VELOCITY;
        return newVel;
    }
    
    static bool checkCollision(const Vector3& pos, const Vector3& size, const std::vector<Block>& blocks) {
        for(const auto& block : blocks) {
            if(!block.solid) continue;
            
            if(pos.x < block.position.x + block.size.x &&
               pos.x + size.x > block.position.x &&
               pos.y < block.position.y + block.size.y &&
               pos.y + size.y > block.position.y &&
               pos.z < block.position.z + block.size.z &&
               pos.z + size.z > block.position.z) {
                return true;
            }
        }
        return false;
    }
    
    static Vector3 resolveCollision(const Vector3& pos, const Vector3& vel, const std::vector<Block>& blocks, float deltaTime) {
        Vector3 newPos = pos + vel * deltaTime;
        Vector3 playerSize(0.6f, 1.8f, 0.6f);
        
        if(checkCollision(newPos, playerSize, blocks)) {
            Vector3 groundedPos = pos;
            groundedPos.y = pos.y;
            if(!checkCollision(groundedPos, playerSize, blocks)) {
                return Vector3(vel.x, 0, vel.z);
            }
            return Vector3(0, vel.y, 0);
        }
        return vel;
    }
};

class GameServer {
private:
    ENetHost* host;
    std::vector<ENetPeer*> clients;
    std::mutex clientMutex;
    std::thread serverThread;
    std::atomic<bool> running;
    Database* db;
    std::string adminKey;
    
public:
    GameServer() : host(nullptr), running(false), db(nullptr), adminKey("IrBoxAdmin2024") {}
    
    ~GameServer() {
        stop();
    }
    
    bool start(int port, Database* database = nullptr) {
        db = database;
        if(enet_initialize() != 0) return false;
        
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;
        
        host = enet_host_create(&address, 64, 2, 0, 0);
        if(!host) return false;
        
        running = true;
        serverThread = std::thread([this]() { serverLoop(); });
        printf("[Server] Started on port %d\n", port);
        return true;
    }
    
    void stop() {
        running = false;
        if(serverThread.joinable()) serverThread.join();
        if(host) enet_host_destroy(host);
        host = nullptr;
        enet_deinitialize();
    }
    
    void setAdminKey(const std::string& key) {
        adminKey = key;
    }
    
    bool isAdmin(const std::string& providedKey) {
        return providedKey == adminKey;
    }
    
    void broadcastScript(const std::string& scriptName, const std::string& scriptContent) {
        std::string obfuscated = CryptoEngine::encrypt(scriptContent, "IrBoxObf2024");
        sendToAll("SCRIPT_BROADCAST", scriptName + "|" + obfuscated);
    }
    
    void sendModelToClient(ENetPeer* peer, const std::string& modelData) {
        std::string encrypted = CryptoEngine::encrypt(modelData, "IrBoxModelKey2024");
        sendToPeer(peer, "MODEL_DATA", encrypted);
    }
    
private:
    void serverLoop() {
        while(running) {
            ENetEvent event;
            if(enet_host_service(host, &event, 10) > 0) {
                switch(event.type) {
                    case ENET_EVENT_TYPE_CONNECT: {
                        std::lock_guard<std::mutex> lock(clientMutex);
                        clients.push_back(event.peer);
                        printf("[Server] Client connected: %p\n", (void*)event.peer);
                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT: {
                        std::lock_guard<std::mutex> lock(clientMutex);
                        clients.erase(std::remove(clients.begin(), clients.end(), event.peer), clients.end());
                        printf("[Server] Client disconnected: %p\n", (void*)event.peer);
                        break;
                    }
                    case ENET_EVENT_TYPE_RECEIVE: {
                        handlePacket(event.peer, event.packet);
                        enet_packet_destroy(event.packet);
                        break;
                    }
                    default: break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    void handlePacket(ENetPeer* peer, ENetPacket* packet) {
        std::string data((char*)packet->data, packet->dataLength);
        size_t sep = data.find('|');
        if(sep != std::string::npos) {
            std::string type = data.substr(0, sep);
            std::string content = data.substr(sep+1);
            
            if(type == "ADMIN_AUTH") {
                if(isAdmin(content)) {
                    sendToPeer(peer, "ADMIN_GRANTED", "OK");
                    printf("[Server] Admin authenticated: %p\n", (void*)peer);
                } else {
                    sendToPeer(peer, "ADMIN_DENIED", "Invalid key");
                }
            } else if(type == "SCRIPT_UPLOAD") {
                size_t innerSep = content.find('|');
                if(innerSep != std::string::npos) {
                    std::string scriptName = content.substr(0, innerSep);
                    std::string scriptData = content.substr(innerSep+1);
                    printf("[Server] Script uploaded: %s\n", scriptName.c_str());
                    broadcastScript(scriptName, CryptoEngine::decrypt(scriptData, "IrBoxObf2024"));
                }
            } else if(type == "PLAYER_ACTION") {
                printf("[Server] Player action: %s\n", content.c_str());
            }
        }
    }
    
    void sendToAll(const std::string& type, const std::string& data) {
        std::lock_guard<std::mutex> lock(clientMutex);
        for(auto* peer : clients) {
            sendToPeer(peer, type, data);
        }
    }
    
    void sendToPeer(ENetPeer* peer, const std::string& type, const std::string& data) {
        std::string packet = type + "|" + data;
        ENetPacket* enetPacket = enet_packet_create(
            packet.c_str(), packet.size() + 1, ENET_PACKET_FLAG_RELIABLE
        );
        enet_peer_send(peer, 0, enetPacket);
    }
};

class AchievementManager {
private:
    std::unordered_map<std::string, Achievement> achievements;
    Database* db;
    
public:
    AchievementManager() : db(nullptr) {}
    
    void setDatabase(Database* database) { db = database; }
    
    void registerAchievement(const std::string& id, const std::string& name, const std::string& desc) {
        achievements[id] = {id, name, desc, false, nullptr};
        if(db && db->exists("ach_" + id)) {
            achievements[id].unlocked = true;
        }
    }
    
    void unlock(const std::string& id) {
        auto it = achievements.find(id);
        if(it != achievements.end() && !it->second.unlocked) {
            it->second.unlocked = true;
            if(db) db->set("ach_" + id, "1");
            if(it->second.onUnlock) it->second.onUnlock();
            printf("[Achievement] Unlocked: %s - %s\n", it->second.name.c_str(), it->second.description.c_str());
        }
    }
    
    bool isUnlocked(const std::string& id) {
        auto it = achievements.find(id);
        return it != achievements.end() && it->second.unlocked;
    }
};

class IrBoxEngine {
private:
    lua_State* L;
    LuaStateManager luaState;
    NetworkManager network;
    UIManager ui;
    FontManager fontMgr;
    Database db;
    GameServer server;
    AchievementManager achievementMgr;
    
    std::vector<Model> models;
    std::vector<Block> blocks;
    Player player;
    
    bool running;
    std::string gameMode;
    
public:
    IrBoxEngine() : running(false), gameMode("parkour") {
        luaState.setDatabase(&db);
        achievementMgr.setDatabase(&db);
        // ui.setFontManager(&fontMgr);
        
        registerDefaultAchievements();
    }
    
    ~IrBoxEngine() {
        shutdown();
    }
    
    bool initialize() {
        printf("[IrBox Engine] Initializing...\n");
        printf("[IrBox Engine] Copyright (c) DeathAmir And IrAutoX\n");
        
        fontMgr.loadFont("persian", "assets/fonts/sahel.ttf");
        
        ui.addPanel("Main Menu", 100, 100, 400, 300);
        ui.addButton("Play Parkour", 150, 200, 300, 50, [this]() { startParkour(); });
        ui.addButton("Settings", 150, 260, 300, 50, []() { printf("[UI] Settings clicked\n"); });
        ui.addButton("Exit", 150, 320, 300, 50, [this]() { running = false; });
        
        ui.addLabel("ایر باکس - بازی پارکور", 200, 150, Color(1,1,1,1), true);
        
        createParkourBlocks();
        
        return true;
    }
    
    void run() {
        running = true;
        printf("[IrBox Engine] Running main loop...\n");
        
        double lastTime = 0;
        double currentTime = 0;
        float accumulator = 0;
        const float dt = 1.0f / 60.0f;
        
        while(running) {
            currentTime = getCurrentTime();
            double frameTime = currentTime - lastTime;
            lastTime = currentTime;
            accumulator += frameTime;
            
            while(accumulator >= dt) {
                update(dt);
                accumulator -= dt;
            }
            
            render();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
    
    void shutdown() {
        running = false;
        network.disconnect();
        server.stop();
        printf("[IrBox Engine] Shutdown complete\n");
    }
    
    void startParkour() {
        gameMode = "parkour";
        player.position = Vector3(0, 5, 0);
        player.velocity = Vector3(0, 0, 0);
        printf("[Game] Parkour mode started!\n");
    }
    
    void connectToServer(const std::string& address, int port) {
        if(network.connectToServer(address, port)) {
            printf("[Network] Connected to server: %s:%d\n", address.c_str(), port);
        } else {
            printf("[Network] Failed to connect to server\n");
        }
    }
    
    bool startLocalServer(int port) {
        return server.start(port, &db);
    }
    
    void uploadScript(const std::string& scriptPath, const std::string& scriptName) {
        std::ifstream file(scriptPath);
        if(file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            
            std::string obfuscated = CryptoEngine::encrypt(content, "IrBoxObf2024");
            
            if(network.isConnected()) {
                network.sendScriptToServer(scriptName, content);
                printf("[Studio] Script uploaded: %s\n", scriptName.c_str());
            } else if(server.isAdmin("IrBoxAdmin2024")) {
                server.broadcastScript(scriptName, content);
                printf("[Studio] Script broadcasted: %s\n", scriptName.c_str());
            }
        }
    }
    
    void loadModel(const std::string& path, const std::string& encryptKey = "") {
        Model model;
        model.id = path;
        
        std::ifstream file(path, std::ios::binary);
        if(file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            
            if(!encryptKey.empty()) {
                content = CryptoEngine::decrypt(content, encryptKey);
                model.decrypt(encryptKey);
            }
            
            model.isEncrypted = !encryptKey.empty();
            models.push_back(model);
            printf("[Model] Loaded: %s (encrypted: %s)\n", path.c_str(), model.isEncrypted ? "yes" : "no");
        }
    }
    
    void encryptAndSaveModel(const std::string& inputPath, const std::string& outputPath, const std::string& key) {
        Model model;
        model.id = inputPath;
        model.vertices.push_back(Vector3(1, 2, 3));
        model.vertices.push_back(Vector3(4, 5, 6));
        model.indices = {0, 1, 2};
        
        model.encrypt(key);
        
        std::string encryptedData = CryptoEngine::encrypt(inputPath, key);
        std::ofstream out(outputPath, std::ios::binary);
        out << encryptedData;
        out.close();
        
        printf("[Model] Encrypted and saved: %s\n", outputPath.c_str());
    }
    
    lua_State* getLuaState() { return luaState.getState(); }
    
    Database& getDatabase() { return db; }
    
    GameServer& getServer() { return server; }
    
    AchievementManager& getAchievementManager() { return achievementMgr; }
    
private:
    void update(float dt) {
        if(gameMode == "parkour") {
            updateParkour(dt);
        }
        
        ui.render();
    }
    
    void updateParkour(float dt) {
        player.velocity = PhysicsEngine::applyGravity(player.velocity, dt);
        player.velocity = PhysicsEngine::resolveCollision(player.position, player.velocity, blocks, dt);
        player.position = player.position + player.velocity * dt;
        
        player.isGrounded = (player.velocity.y == 0);
        
        if(player.position.y < -10) {
            player.position = Vector3(0, 5, 0);
            player.velocity = Vector3(0, 0, 0);
            achievementMgr.unlock("first_death");
            luaState.loadObfuscatedScript(CryptoEngine::encrypt("print_log('Player died!')", "IrBoxObf2024"), "IrBoxObf2024");
        }
    }
    
    void render() {
        printf("\r[Render] Frame | Player: (%.2f, %.2f, %.2f) | Blocks: %d | Models: %d",
               player.position.x, player.position.y, player.position.z,
               (int)blocks.size(), (int)models.size());
        fflush(stdout);
    }
    
    void createParkourBlocks() {
        blocks.clear();
        
        blocks.push_back(createBlock(0, 0, 0, 10, 1, 10, "grass"));
        blocks.push_back(createBlock(12, 1, 0, 3, 1, 3, "stone"));
        blocks.push_back(createBlock(17, 2.5, 0, 3, 1, 3, "stone"));
        blocks.push_back(createBlock(22, 4, 0, 3, 1, 3, "stone"));
        blocks.push_back(createBlock(27, 5.5, 0, 3, 1, 3, "stone"));
        blocks.push_back(createBlock(32, 7, 0, 5, 1, 5, "gold"));
        
        achievementMgr.registerAchievement("first_jump", "First Jump", "Jump for the first time");
        achievementMgr.registerAchievement("first_death", "Oops!", "Fall into the void");
        achievementMgr.registerAchievement("speedrun", "Speedrunner", "Complete parkour in under 30 seconds");
        achievementMgr.registerAchievement("perfectionist", "Perfectionist", "Complete parkour without falling");
    }
    
    Block createBlock(float x, float y, float z, float w, float h, float d, const std::string& type) {
        Block block;
        block.position = Vector3(x, y, z);
        block.size = Vector3(w, h, d);
        block.type = type;
        block.solid = true;
        block.destructible = (type != "bedrock");
        block.health = (type == "gold") ? 200 : 100;
        block.dropItem = (type == "stone") ? "cobblestone" : type;
        return block;
    }
    
    void registerDefaultAchievements() {
        achievementMgr.registerAchievement("welcome", "Welcome", "Welcome to IrBox!");
        achievementMgr.registerAchievement("builder", "Builder", "Place 100 blocks");
        achievementMgr.registerAchievement("miner", "Miner", "Break 100 blocks");
    }
    
    double getCurrentTime() {
        using namespace std::chrono;
        return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
    }
};

}

#endif
