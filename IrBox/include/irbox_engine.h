#ifndef IRBOX_ENGINE_H
#define IRBOX_ENGINE_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <random>
#include <queue>
#include <unordered_map>
#include <set>
#include <condition_variable>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
#endif

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

namespace IrBox {

struct Vector3 {
    float x, y, z;
    Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vector3 normalize() const { float l = length(); return l > 0 ? *this * (1.0f/l) : Vector3(); }
};

struct Vector2 {
    float x, y;
    Vector2(float _x = 0, float _y = 0) : x(_x), y(_y) {}
};

struct Color {
    float r, g, b, a;
    Color(float _r = 1, float _g = 1, float _b = 1, float _a = 1) : r(_r), g(_g), b(_b), a(_a) {}
};

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
};

struct Model {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    bool encrypted;
    
    Model() : position(0,0,0), rotation(0,0,0), scale(1,1,1), encrypted(false) {}
};

struct Player {
    std::string username;
    uint64_t userId;
    Vector3 position;
    Vector3 velocity;
    bool isGrounded;
    int health;
    std::string avatarModel;
    
    Player() : userId(0), position(0,0,0), velocity(0,0,0), isGrounded(false), health(100) {}
};

struct UIElement {
    enum Type { BUTTON, LABEL, PANEL, INPUT, IMAGE };
    Type type;
    Vector2 position;
    Vector2 size;
    std::string text;
    Color backgroundColor;
    Color textColor;
    bool visible;
    std::function<void()> onClick;
    
    UIElement() : type(LABEL), position(0,0), size(100,30), visible(true), 
                  backgroundColor(0,0,0,0.5f), textColor(1,1,1,1) {}
};

class CryptoEngine {
public:
    static std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data, const std::string& key) {
        std::vector<uint8_t> result(data.size());
        for(size_t i = 0; i < data.size(); ++i) {
            result[i] = data[i] ^ key[i % key.size()] ^ 0xAA;
        }
        return result;
    }
    
    static std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data, const std::string& key) {
        return encrypt(data, key);
    }
    
    static std::string obfuscate(const std::string& code) {
        std::vector<uint8_t> data(code.begin(), code.end());
        auto encrypted = encrypt(data, "IrBoxObf2024");
        std::string result;
        for(auto b : encrypted) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02X", b);
            result += hex;
        }
        return result;
    }
    
    static std::string deobfuscate(const std::string& hexData) {
        std::vector<uint8_t> data;
        for(size_t i = 0; i < hexData.length(); i += 2) {
            uint8_t byte = std::stoi(hexData.substr(i, 2), nullptr, 16);
            data.push_back(byte);
        }
        auto decrypted = decrypt(data, "IrBoxObf2024");
        return std::string(decrypted.begin(), decrypted.end());
    }
};

class LuaStateManager {
private:
    lua_State* L;
    std::mutex luaMutex;
    
public:
    LuaStateManager() {
        L = luaL_newstate();
        luaL_openlibs(L);
    }
    
    ~LuaStateManager() {
        if(L) lua_close(L);
    }
    
    lua_State* getState() { return L; }
    
    std::lock_guard<std::mutex> lock() {
        return std::lock_guard<std::mutex>(luaMutex);
    }
    
    bool loadString(const std::string& code) {
        auto guard = lock();
        if(luaL_loadstring(L, code.c_str()) || lua_pcall(L, 0, 0, 0)) {
            std::cerr << "Lua Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }
        return true;
    }
    
    bool loadEncryptedString(const std::string& encryptedHex) {
        std::string decrypted = CryptoEngine::deobfuscate(encryptedHex);
        return loadString(decrypted);
    }
};

class NetworkManager {
private:
    SOCKET sock;
    sockaddr_in serverAddr;
    bool connected;
    std::thread recvThread;
    std::atomic<bool> running;
    
public:
    NetworkManager() : sock(INVALID_SOCKET), connected(false), running(false) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
    }
    
    ~NetworkManager() {
        disconnect();
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    bool connect(const std::string& host, int port) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock == INVALID_SOCKET) return false;
        
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        
        struct hostent* server = gethostbyname(host.c_str());
        if(!server) return false;
        
        memcpy(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);
        
        if(::connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0) {
            connected = true;
            running = true;
            recvThread = std::thread(&NetworkManager::receiveLoop, this);
            return true;
        }
        return false;
    }
    
    void disconnect() {
        running = false;
        if(recvThread.joinable()) recvThread.join();
        if(sock != INVALID_SOCKET) {
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            sock = INVALID_SOCKET;
        }
        connected = false;
    }
    
    bool send(const std::vector<uint8_t>& data) {
        if(!connected) return false;
        int sent = ::send(sock, (const char*)data.data(), data.size(), 0);
        return sent == (int)data.size();
    }
    
    bool sendString(const std::string& str) {
        std::vector<uint8_t> data(str.begin(), str.end());
        return send(data);
    }
    
    void receiveLoop() {
        char buffer[4096];
        while(running && connected) {
            int bytes = recv(sock, buffer, sizeof(buffer)-1, 0);
            if(bytes > 0) {
                std::string msg(buffer, bytes);
                handleMessage(msg);
            } else if(bytes <= 0) {
                break;
            }
        }
    }
    
    virtual void handleMessage(const std::string& msg) {
        // Override in derived class
    }
    
    bool isConnected() const { return connected; }
};

class ModelRenderer {
private:
    std::map<std::string, Model> models;
    std::string encryptionKey;
    
public:
    ModelRenderer() : encryptionKey("IrBoxModelKey2024") {}
    
    bool loadModel(const std::string& filename, const std::string& name) {
        std::ifstream file(filename, std::ios::binary);
        if(!file.is_open()) return false;
        
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
        
        auto decrypted = CryptoEngine::decrypt(data, encryptionKey);
        
        Model model;
        model.name = name;
        model.encrypted = true;
        
        size_t idx = 0;
        uint32_t vertexCount = *(uint32_t*)&decrypted[idx]; idx += 4;
        uint32_t indexCount = *(uint32_t*)&decrypted[idx]; idx += 4;
        
        for(uint32_t i = 0; i < vertexCount; ++i) {
            Vertex v;
            v.position = Vector3(*(float*)&decrypted[idx], *(float*)&decrypted[idx+4], *(float*)&decrypted[idx+8]); idx += 12;
            v.normal = Vector3(*(float*)&decrypted[idx], *(float*)&decrypted[idx+4], *(float*)&decrypted[idx+8]); idx += 12;
            v.texCoord = Vector2(*(float*)&decrypted[idx], *(float*)&decrypted[idx+4]); idx += 8;
            model.vertices.push_back(v);
        }
        
        for(uint32_t i = 0; i < indexCount; ++i) {
            model.indices.push_back(*(uint32_t*)&decrypted[idx]); idx += 4;
        }
        
        models[name] = model;
        return true;
    }
    
    bool saveModel(const Model& model, const std::string& filename) {
        std::vector<uint8_t> data;
        
        uint32_t vertexCount = model.vertices.size();
        uint32_t indexCount = model.indices.size();
        
        data.insert(data.end(), (uint8_t*)&vertexCount, (uint8_t*)&vertexCount + 4);
        data.insert(data.end(), (uint8_t*)&indexCount, (uint8_t*)&indexCount + 4);
        
        for(const auto& v : model.vertices) {
            data.insert(data.end(), (uint8_t*)&v.position.x, (uint8_t*)&v.position.x + 12);
            data.insert(data.end(), (uint8_t*)&v.normal.x, (uint8_t*)&v.normal.x + 12);
            data.insert(data.end(), (uint8_t*)&v.texCoord.x, (uint8_t*)&v.texCoord.x + 8);
        }
        
        for(uint32_t idx : model.indices) {
            data.insert(data.end(), (uint8_t*)&idx, (uint8_t*)&idx + 4);
        }
        
        auto encrypted = CryptoEngine::encrypt(data, encryptionKey);
        
        std::ofstream file(filename, std::ios::binary);
        if(!file.is_open()) return false;
        file.write((const char*)encrypted.data(), encrypted.size());
        return true;
    }
    
    Model* getModel(const std::string& name) {
        auto it = models.find(name);
        return it != models.end() ? &it->second : nullptr;
    }
};

class UIManager {
private:
    std::vector<UIElement> elements;
    bool rtlSupport;
    std::string currentFont;
    
public:
    UIManager() : rtlSupport(true), currentFont("default") {}
    
    void enableRTL(bool enable) { rtlSupport = enable; }
    void setFont(const std::string& fontName) { currentFont = fontName; }
    
    UIElement& createButton(const std::string& text, float x, float y, float w, float h, std::function<void()> callback) {
        UIElement elem;
        elem.type = UIElement::BUTTON;
        elem.text = text;
        elem.position = Vector2(x, y);
        elem.size = Vector2(w, h);
        elem.onClick = callback;
        elem.backgroundColor = Color(0.2f, 0.6f, 1.0f, 0.8f);
        elements.push_back(elem);
        return elements.back();
    }
    
    UIElement& createLabel(const std::string& text, float x, float y, float w, float h) {
        UIElement elem;
        elem.type = UIElement::LABEL;
        elem.text = text;
        elem.position = Vector2(x, y);
        elem.size = Vector2(w, h);
        elem.textColor = Color(1, 1, 1, 1);
        elements.push_back(elem);
        return elements.back();
    }
    
    UIElement& createPanel(float x, float y, float w, float h, Color bg) {
        UIElement elem;
        elem.type = UIElement::PANEL;
        elem.position = Vector2(x, y);
        elem.size = Vector2(w, h);
        elem.backgroundColor = bg;
        elements.push_back(elem);
        return elements.back();
    }
    
    void render() {
        for(auto& elem : elements) {
            if(!elem.visible) continue;
            renderElement(elem);
        }
    }
    
    void renderElement(UIElement& elem) {
        switch(elem.type) {
            case UIElement::BUTTON:
                drawRect(elem.position.x, elem.position.y, elem.size.x, elem.size.y, elem.backgroundColor);
                drawText(elem.text, elem.position.x + elem.size.x/2, elem.position.y + elem.size.y/2, elem.textColor);
                break;
            case UIElement::LABEL:
                drawText(elem.text, elem.position.x, elem.position.y, elem.textColor);
                break;
            case UIElement::PANEL:
                drawRect(elem.position.x, elem.position.y, elem.size.x, elem.size.y, elem.backgroundColor);
                break;
            default:
                break;
        }
    }
    
    void drawRect(float x, float y, float w, float h, Color c) {
    }
    
    void drawText(const std::string& text, float x, float y, Color c) {
    }
    
    void handleMouseClick(float mx, float my) {
        for(auto& elem : elements) {
            if(elem.type == UIElement::BUTTON && elem.visible) {
                if(mx >= elem.position.x && mx <= elem.position.x + elem.size.x &&
                   my >= elem.position.y && my <= elem.position.y + elem.size.y) {
                    if(elem.onClick) elem.onClick();
                }
            }
        }
    }
};

class PhysicsEngine {
public:
    static Vector3 applyGravity(const Vector3& pos, const Vector3& vel, float dt, float gravity = -9.81f) {
        Vector3 newVel = vel + Vector3(0, gravity * dt, 0);
        Vector3 newPos = pos + newVel * dt;
        return newPos;
    }
    
    static bool checkCollision(const Vector3& pos, float radius, const Vector3& planeNormal, float planeDist) {
        float dist = pos.x * planeNormal.x + pos.y * planeNormal.y + pos.z * planeNormal.z - planeDist;
        return dist <= radius;
    }
    
    static Vector3 resolveCollision(const Vector3& pos, const Vector3& vel, const Vector3& normal, float restitution = 0.5f) {
        float dot = vel.x * normal.x + vel.y * normal.y + vel.z * normal.z;
        Vector3 reflected = vel - normal * (1 + restitution) * dot;
        return reflected;
    }
};

class GameServer : public NetworkManager {
private:
    std::map<uint64_t, Player> players;
    std::mutex playerMutex;
    LuaStateManager serverLua;
    std::string adminKey;
    
public:
    GameServer() : adminKey("IrBoxAdmin2024") {}
    
    void setAdminKey(const std::string& key) { adminKey = key; }
    
    void handleMessage(const std::string& msg) override {
        if(msg.find("ADMIN:") == 0) {
            std::string key = msg.substr(6, 32);
            if(key == adminKey) {
                executeAdminCommand(msg.substr(39));
            }
            return;
        }
        
        if(msg.find("SCRIPT:") == 0) {
            std::string script = msg.substr(7);
            serverLua.loadString(script);
            return;
        }
        
        if(msg.find("PLAYER_POS:") == 0) {
            std::istringstream iss(msg.substr(11));
            uint64_t userId;
            float x, y, z;
            char comma;
            iss >> userId >> comma >> x >> comma >> y >> comma >> z;
            
            std::lock_guard<std::mutex> lock(playerMutex);
            if(players.find(userId) != players.end()) {
                players[userId].position = Vector3(x, y, z);
            }
        }
    }
    
    void executeAdminCommand(const std::string& cmd) {
        if(cmd.find("KICK ") == 0) {
            uint64_t userId = std::stoull(cmd.substr(5));
            std::lock_guard<std::mutex> lock(playerMutex);
            players.erase(userId);
        } else if(cmd.find("BROADCAST ") == 0) {
            std::string message = cmd.substr(10);
            sendString("MSG:" + message);
        }
    }
    
    void addPlayer(uint64_t userId, const std::string& username) {
        std::lock_guard<std::mutex> lock(playerMutex);
        Player p;
        p.userId = userId;
        p.username = username;
        players[userId] = p;
    }
    
    std::string getPlayerList() {
        std::lock_guard<std::mutex> lock(playerMutex);
        std::string list;
        for(const auto& pair : players) {
            list += pair.second.username + ",";
        }
        return list;
    }
};

class ClientNetwork : public NetworkManager {
private:
    LuaStateManager clientLua;
    std::function<void(const std::string&)> onMessage;
    
public:
    void handleMessage(const std::string& msg) override {
        if(msg.find("SCRIPT:") == 0) {
            std::string encryptedScript = msg.substr(7);
            clientLua.loadEncryptedString(encryptedScript);
            return;
        }
        
        if(msg.find("MSG:") == 0) {
            if(onMessage) onMessage(msg.substr(4));
            return;
        }
        
        if(msg.find("MODEL:") == 0) {
            std::string modelData = msg.substr(6);
            handleModelDownload(modelData);
            return;
        }
    }
    
    void handleModelDownload(const std::string& modelData) {
    }
    
    void setOnMessage(std::function<void(const std::string&)> cb) {
        onMessage = cb;
    }
};

class Renderer {
private:
    void* window;
    void* glContext;
    int width, height;
    bool initialized;
    
public:
    Renderer() : window(nullptr), glContext(nullptr), width(800), height(600), initialized(false) {}
    
    bool initialize(int w, int h, const std::string& title) {
        width = w;
        height = h;
        initialized = true;
        return true;
    }
    
    void beginFrame() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    void endFrame() {
    }
    
    void renderModel(const Model& model) {
    }
    
    void shutdown() {
        initialized = false;
    }
    
    bool isInitialized() const { return initialized; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

class InputManager {
private:
    std::map<int, bool> keys;
    std::map<int, bool> mouseButtons;
    float mouseX, mouseY;
    
public:
    InputManager() : mouseX(0), mouseY(0) {}
    
    bool isKeyDown(int key) { return keys[key]; }
    bool isMouseButtonDown(int button) { return mouseButtons[button]; }
    float getMouseX() const { return mouseX; }
    float getMouseY() const { return mouseY; }
    
    void setKeyState(int key, bool pressed) { keys[key] = pressed; }
    void setMouseButtonState(int button, bool pressed) { mouseButtons[button] = pressed; }
    void setMousePosition(float x, float y) { mouseX = x; mouseY = y; }
};

class AudioEngine {
public:
    void playSound(const std::string& filename) {}
    void stopSound(const std::string& filename) {}
    void setVolume(float volume) {}
};

class FontManager {
private:
    std::map<std::string, void*> fonts;
    std::string currentFont;
    bool rtlEnabled;
    
public:
    FontManager() : rtlEnabled(true), currentFont("default") {}
    
    bool loadFont(const std::string& name, const std::string& filename) {
        return true;
    }
    
    void setFont(const std::string& name) {
        currentFont = name;
    }
    
    void enableRTL(bool enable) {
        rtlEnabled = enable;
    }
    
    std::string processRTLText(const std::string& text) {
        if(!rtlEnabled) return text;
        std::string result = text;
        std::reverse(result.begin(), result.end());
        return result;
    }
};

class SceneManager {
private:
    std::string currentScene;
    std::map<std::string, std::function<void()>> scenes;
    
public:
    void registerScene(const std::string& name, std::function<void()> initFunc) {
        scenes[name] = initFunc;
    }
    
    void loadScene(const std::string& name) {
        currentScene = name;
        auto it = scenes.find(name);
        if(it != scenes.end()) {
            it->second();
        }
    }
    
    std::string getCurrentScene() const { return currentScene; }
};

class IrBoxEngine {
private:
    static IrBoxEngine* instance;
    
    Renderer renderer;
    InputManager input;
    UIManager ui;
    LuaStateManager lua;
    ModelRenderer models;
    FontManager fonts;
    SceneManager scenes;
    AudioEngine audio;
    ClientNetwork network;
    
    std::string version;
    bool running;
    double deltaTime;
    double lastFrameTime;
    
    Player localPlayer;
    std::vector<Player> remotePlayers;
    
    IrBoxEngine() : version("1.0.0"), running(false), deltaTime(0), lastFrameTime(0) {
        instance = this;
    }
    
public:
    static IrBoxEngine* getInstance() {
        if(!instance) instance = new IrBoxEngine();
        return instance;
    }
    
    bool initialize(int width, int height, const std::string& title) {
        if(!renderer.initialize(width, height, title)) {
            return false;
        }
        
        fonts.loadFont("default", "assets/fonts/default.ttf");
        fonts.loadFont("persian", "assets/fonts/persian.ttf");
        fonts.enableRTL(true);
        
        setupMainMenu();
        
        return true;
    }
    
    void setupMainMenu() {
        ui.createPanel(0, 0, 800, 600, Color(0.1f, 0.1f, 0.2f, 1.0f));
        
        ui.createLabel("IrBox", 350, 50, 100, 50);
        ui.createLabel("Copyright DeathAmir And IrAutoX", 250, 100, 300, 30);
        
        ui.createButton("Play Parkour", 300, 200, 200, 50, [this]() {
            scenes.loadScene("parkour");
        });
        
        ui.createButton("Profile", 300, 270, 200, 50, [this]() {
            showProfile();
        });
        
        ui.createButton("Settings", 300, 340, 200, 50, [this]() {
            showSettings();
        });
        
        ui.createButton("Exit", 300, 410, 200, 50, [this]() {
            running = false;
        });
        
        ui.setFont("persian");
        ui.enableRTL(true);
    }
    
    void showProfile() {
        ui.createPanel(200, 100, 400, 400, Color(0.2f, 0.2f, 0.3f, 0.95f));
        ui.createLabel("Player Profile", 250, 120, 300, 40);
        ui.createLabel("Username: " + localPlayer.username, 220, 180, 360, 30);
        ui.createLabel("ID: " + std::to_string(localPlayer.userId), 220, 220, 360, 30);
        ui.createButton("Back", 350, 450, 100, 40, []() {});
    }
    
    void showSettings() {
        ui.createPanel(200, 100, 400, 400, Color(0.2f, 0.2f, 0.3f, 0.95f));
        ui.createLabel("Settings", 350, 120, 100, 40);
        ui.createButton("Graphics", 250, 180, 300, 40, []() {});
        ui.createButton("Audio", 250, 240, 300, 40, []() {});
        ui.createButton("Controls", 250, 300, 300, 40, []() {});
        ui.createButton("Back", 350, 450, 100, 40, []() {});
    }
    
    void setupParkourScene() {
        ui.elements.clear();
        
        localPlayer.position = Vector3(0, 0, 0);
        localPlayer.velocity = Vector3(0, 0, 0);
        localPlayer.isGrounded = true;
        
        Model platform;
        platform.position = Vector3(0, -1, 0);
        platform.scale = Vector3(10, 1, 10);
        models.saveModel(platform, "assets/models/platform.ibx");
        
        Model obstacle1;
        obstacle1.position = Vector3(5, 0, 0);
        obstacle1.scale = Vector3(2, 2, 2);
        models.saveModel(obstacle1, "assets/models/obstacle1.ibx");
        
        Model obstacle2;
        obstacle2.position = Vector3(-5, 1, 5);
        obstacle2.scale = Vector3(2, 2, 2);
        models.saveModel(obstacle2, "assets/models/obstacle2.ibx");
        
        ui.createLabel("Parkour Level 1", 10, 10, 200, 30);
        ui.createLabel("Use WASD to move, SPACE to jump", 10, 40, 300, 20);
    }
    
    void updateParkour(double dt) {
        float speed = 5.0f;
        float jumpForce = 8.0f;
        float gravity = -20.0f;
        
        Vector3 moveDir(0, 0, 0);
        
        if(input.isKeyDown('W')) moveDir.z -= 1;
        if(input.isKeyDown('S')) moveDir.z += 1;
        if(input.isKeyDown('A')) moveDir.x -= 1;
        if(input.isKeyDown('D')) moveDir.x += 1;
        
        if(moveDir.length() > 0) {
            moveDir = moveDir.normalize() * speed * dt;
        }
        
        localPlayer.position = localPlayer.position + moveDir;
        
        if(input.isKeyDown(' ') && localPlayer.isGrounded) {
            localPlayer.velocity.y = jumpForce;
            localPlayer.isGrounded = false;
        }
        
        localPlayer.velocity.y += gravity * dt;
        localPlayer.position.y += localPlayer.velocity.y * dt;
        
        if(localPlayer.position.y <= 0) {
            localPlayer.position.y = 0;
            localPlayer.velocity.y = 0;
            localPlayer.isGrounded = true;
        }
        
        if(localPlayer.position.y < -10) {
            localPlayer.position = Vector3(0, 0, 0);
            localPlayer.velocity = Vector3(0, 0, 0);
        }
        
        if(network.isConnected()) {
            std::ostringstream oss;
            oss << "PLAYER_POS:" << localPlayer.userId << ","
                << localPlayer.position.x << ","
                << localPlayer.position.y << ","
                << localPlayer.position.z;
            network.sendString(oss.str());
        }
    }
    
    void run() {
        running = true;
        lastFrameTime = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
        
        while(running && renderer.isInitialized()) {
            double currentTime = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
            deltaTime = currentTime - lastFrameTime;
            lastFrameTime = currentTime;
            
            renderer.beginFrame();
            
            if(scenes.getCurrentScene() == "parkour") {
                updateParkour(deltaTime);
            }
            
            ui.render();
            
            renderer.endFrame();
        }
        
        renderer.shutdown();
    }
    
    void connectToServer(const std::string& host, int port) {
        network.connect(host, port);
    }
    
    void disconnectFromServer() {
        network.disconnect();
    }
    
    LuaStateManager* getLuaState() { return &lua; }
    ModelRenderer* getModelRenderer() { return &models; }
    UIManager* getUI() { return &ui; }
    FontManager* getFontManager() { return &fonts; }
    InputManager* getInput() { return &input; }
    
    std::string getVersion() const { return version; }
    std::string getCopyright() const { return "Copyright DeathAmir And IrAutoX"; }
};

IrBoxEngine* IrBoxEngine::instance = nullptr;

}

#endif