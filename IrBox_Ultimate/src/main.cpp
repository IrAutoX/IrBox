#include <iostream>
#include "irbox_engine.h"

int main(int argc, char* argv[]) {
    IrBox::IrBoxEngine engine;
    
    if(!engine.initialize()) {
        std::cerr << "[Error] Failed to initialize engine" << std::endl;
        return 1;
    }
    
    bool serverMode = false;
    int port = 7777;
    
    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if(arg == "--server") {
            serverMode = true;
        } else if(arg == "--port" && i+1 < argc) {
            port = std::stoi(argv[++i]);
        } else if(arg == "--connect" && i+2 < argc) {
            std::string addr = argv[++i];
            int srvPort = std::stoi(argv[++i]);
            engine.connectToServer(addr, srvPort);
        }
    }
    
    if(serverMode) {
        std::cout << "[Main] Starting in server mode on port " << port << std::endl;
        engine.startLocalServer(port);
        
        std::cout << "[Main] Server running. Press Enter to stop." << std::endl;
        std::cin.get();
        
        engine.shutdown();
    } else {
        std::cout << "[Main] Starting in client mode" << std::endl;
        engine.run();
    }
    
    return 0;
}
