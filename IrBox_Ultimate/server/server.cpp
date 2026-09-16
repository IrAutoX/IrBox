#include "irbox_engine.h"

int main(int argc, char* argv[]) {
    irbox::LuaStateManager luaState;
    irbox::NetworkServer server;
    irbox::Database db("server.db");
    
    printf("[SERVER] IrBox Server v%s\n", IRBOX_VERSION);
    printf("[SERVER] %s\n", IRBOX_COPYRIGHT);
    
    if(!server.initialize(7777, &luaState)) {
        printf("[ERROR] Failed to start server\n");
        return 1;
    }
    
    db.set("server_name", "IrBox Official Server");
    db.set("max_players", "64");
    db.save();
    
    printf("[SERVER] Server running on port 7777\n");
    printf("[SERVER] Admin key: %s\n", IRBOX_ADMIN_KEY);
    
    bool running = true;
    while(running) {
        server.update();
        
        char input[256];
        printf("> ");
        if(fgets(input, sizeof(input), stdin)) {
            input[strcspn(input, "\n")] = 0;
            
            if(strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
                running = false;
            } else if(strncmp(input, "admin ", 6) == 0) {
                std::string key = input + 6;
                if(server.isAdmin(key)) {
                    printf("[ADMIN] Access granted!\n");
                } else {
                    printf("[ADMIN] Invalid key!\n");
                }
            } else if(strncmp(input, "exec ", 5) == 0) {
                std::string script = input + 5;
                std::string obfuscated = luaState.obfuscateScript(script);
                printf("[SCRIPT] Executing (obfuscated): %s...\n", obfuscated.substr(0, 50).c_str());
                luaState.loadScript("console_script", obfuscated);
            } else if(strcmp(input, "status") == 0) {
                printf("[STATUS] Server is running\n");
                printf("[STATUS] Database entries: loaded\n");
            } else if(strcmp(input, "help") == 0) {
                printf("Commands: quit, exit, admin <key>, exec <script>, status, help\n");
            }
        }
        
        SDL_Delay(10);
    }
    
    server.shutdown();
    db.save();
    printf("[SERVER] Server shutdown complete\n");
    
    return 0;
}
