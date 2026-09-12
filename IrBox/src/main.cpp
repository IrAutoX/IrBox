#include "irbox_engine.h"
#include <iostream>

using namespace IrBox;

int main(int argc, char* argv[]) {
    IrBoxEngine* engine = IrBoxEngine::getInstance();
    
    if(!engine->initialize(800, 600, "IrBox - Copyright DeathAmir And IrAutoX")) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }
    
    std::cout << "IrBox Engine v" << engine->getVersion() << std::endl;
    std::cout << engine->getCopyright() << std::endl;
    
    engine->scenes.registerScene("parkour", [engine]() {
        engine->setupParkourScene();
    });
    
    engine->run();
    
    return 0;
}