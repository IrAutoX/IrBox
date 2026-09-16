#include <iostream>
#include "irbox_engine.h"

int main(int argc, char* argv[]) {
    std::cout << "[Studio] IrBox Studio - Copyright (c) DeathAmir And IrAutoX" << std::endl;
    
    IrBox::IrBoxEngine engine;
    
    if(!engine.initialize()) {
        std::cerr << "[Studio Error] Failed to initialize engine" << std::endl;
        return 1;
    }
    
    std::string scriptPath = "";
    std::string scriptName = "";
    bool runServer = false;
    int port = 7777;
    
    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if(arg == "--run" && i+1 < argc) {
            scriptPath = argv[++i];
        } else if(arg == "--name" && i+1 < argc) {
            scriptName = argv[++i];
        } else if(arg == "--server") {
            runServer = true;
        } else if(arg == "--port" && i+1 < argc) {
            port = std::stoi(argv[++i]);
        } else if(arg == "--encrypt-model" && i+3 < argc) {
            std::string input = argv[++i];
            std::string output = argv[++i];
            std::string key = argv[++i];
            engine.encryptAndSaveModel(input, output, key);
            return 0;
        } else if(arg == "--obfuscate" && i+2 < argc) {
            std::string input = argv[++i];
            std::string output = argv[++i];
            
            std::ifstream file(input);
            if(file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
                std::string obfuscated = IrBox::CryptoEngine::encrypt(content, "IrBoxObf2024");
                
                std::ofstream outFile(output);
                outFile << obfuscated;
                outFile.close();
                
                std::cout << "[Studio] Script obfuscated: " << output << std::endl;
            }
            return 0;
        }
    }
    
    if(runServer) {
        std::cout << "[Studio] Starting local server on port " << port << std::endl;
        if(engine.startLocalServer(port)) {
            std::cout << "[Studio] Server started successfully" << std::endl;
            
            if(!scriptPath.empty() && !scriptName.empty()) {
                engine.uploadScript(scriptPath, scriptName);
            }
            
            std::cout << "[Studio] Server running. Press Enter to stop." << std::endl;
            std::cin.get();
            
            engine.shutdown();
        } else {
            std::cerr << "[Studio Error] Failed to start server" << std::endl;
            return 1;
        }
    } else if(!scriptPath.empty()) {
        std::cout << "[Studio] Running script: " << scriptPath << std::endl;
        
        lua_State* L = engine.getLuaState();
        if(L) {
            std::ifstream file(scriptPath);
            if(file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
                
                if(luaL_loadstring(L, content.c_str()) || lua_pcall(L, 0, 0, 0)) {
                    const char* err = lua_tostring(L, -1);
                    std::cerr << "[Studio Lua Error] " << err << std::endl;
                    lua_pop(L, 1);
                } else {
                    std::cout << "[Studio] Script executed successfully" << std::endl;
                }
            } else {
                std::cerr << "[Studio Error] Cannot open script file: " << scriptPath << std::endl;
                return 1;
            }
        }
    } else {
        std::cout << "\n[Studio] Usage:" << std::endl;
        std::cout << "  irbox_studio --run <script.lua> [--name <name>] [--server] [--port <port>]" << std::endl;
        std::cout << "  irbox_studio --encrypt-model <input> <output> <key>" << std::endl;
        std::cout << "  irbox_studio --obfuscate <input.lua> <output.ibx>" << std::endl;
        std::cout << "\n[Studio] Interactive mode..." << std::endl;
        std::cout << "> ";
        
        std::string line;
        while(std::getline(std::cin, line)) {
            if(line == "exit" || line == "quit") break;
            
            if(line.find("run ") == 0) {
                std::string path = line.substr(4);
                std::ifstream file(path);
                if(file.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(file)),
                                        std::istreambuf_iterator<char>());
                    
                    lua_State* L = engine.getLuaState();
                    if(luaL_loadstring(L, content.c_str()) || lua_pcall(L, 0, 0, 0)) {
                        const char* err = lua_tostring(L, -1);
                        std::cout << "[Error] " << err << std::endl;
                        lua_pop(L, 1);
                    }
                }
            } else if(line.find("upload ") == 0) {
                size_t space = line.find(' ', 7);
                if(space != std::string::npos) {
                    std::string path = line.substr(7, space-7);
                    std::string name = line.substr(space+1);
                    engine.uploadScript(path, name);
                }
            } else if(line == "help") {
                std::cout << "Commands: run <file>, upload <file> <name>, exit" << std::endl;
            }
            
            std::cout << "> ";
        }
    }
    
    return 0;
}
