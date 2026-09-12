#include "irbox_engine.h"
#include <iostream>
#include <csignal>

using namespace IrBox;

static std::atomic<bool> serverRunning(true);

void signalHandler(int signum) {
    serverRunning = false;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    if(argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    GameServer server;
    server.setAdminKey("IrBoxAdmin2024");
    
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "IrBox Server v1.0.0" << std::endl;
    std::cout << "Copyright DeathAmir And IrAutoX" << std::endl;
    std::cout << "Server starting on port " << port << std::endl;
    
    std::string script = R"(
print("Server script loaded")
function onPlayerJoin(userId, username)
    print("Player joined: " .. username)
end
function onPlayerLeave(userId)
    print("Player left: " .. userId)
end
)";
    
    auto obfuscated = CryptoEngine::obfuscate(script);
    std::cout << "Obfuscated script length: " << obfuscated.length() << std::endl;
    
    while(serverRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Server shutting down..." << std::endl;
    return 0;
}