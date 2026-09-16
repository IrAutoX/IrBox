#include "irbox_engine.h"

int main(int argc, char* argv[]) {
    irbox::GameEngine engine;
    
    if(!engine.initialize()) {
        printf("[ERROR] Failed to initialize engine\n");
        return 1;
    }
    
    printf("[IRBOX] Starting IrBox Engine v%s\n", IRBOX_VERSION);
    printf("[IRBOX] %s\n", IRBOX_COPYRIGHT);
    
    engine.run();
    engine.shutdown();
    
    return 0;
}
