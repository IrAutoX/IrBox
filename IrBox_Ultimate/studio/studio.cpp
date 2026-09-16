#include "irbox_engine.h"

namespace studio {

class Project {
private:
    std::string name;
    std::string author;
    std::vector<std::string> scripts;
    std::vector<std::string> models;
    
public:
    Project(const std::string& n, const std::string& a) : name(n), author(a) {}
    
    void addScript(const std::string& script) {
        scripts.push_back(script);
        printf("[STUDIO] Added script: %s\n", script.c_str());
    }
    
    void addModel(const std::string& model) {
        models.push_back(model);
        printf("[STUDIO] Added model: %s\n", model.c_str());
    }
    
    bool save(const std::string& filename) {
        std::ofstream file(filename);
        if(!file) return false;
        
        file << "name=" << name << "\n";
        file << "author=" << author << "\n";
        file << "scripts=" << scripts.size() << "\n";
        for(const auto& s : scripts) file << "script=" << s << "\n";
        file << "models=" << models.size() << "\n";
        for(const auto& m : models) file << "model=" << m << "\n";
        
        return true;
    }
    
    const std::string& getName() const { return name; }
};

class StudioApp {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    Project* currentProject;
    
public:
    StudioApp() : window(nullptr), renderer(nullptr), running(false), currentProject(nullptr) {}
    
    bool initialize() {
        if(SDL_Init(SDL_INIT_VIDEO) < 0) return false;
        window = SDL_CreateWindow("IrBox Studio - DeathAmir And IrAutoX", 1024, 768, SDL_WINDOW_OPENGL);
        if(!window) return false;
        renderer = SDL_CreateRenderer(window, nullptr);
        if(!renderer) return false;
        running = true;
        return true;
    }
    
    void render() {
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 0, 102, 255, 255);
        SDL_Rect header = {0, 0, 1024, 50};
        SDL_RenderFillRect(renderer, &header);
        
        drawText("IrBox Studio", 20, 15, Color(1,1,1,1));
        drawText("Project Editor", 200, 15, Color(0.8f,0.8f,0.8f,1));
        
        drawPanel(20, 70, 300, 600, "Explorer");
        drawPanel(340, 70, 664, 400, "Viewport");
        drawPanel(340, 490, 664, 180, "Properties");
        drawPanel(20, 690, 984, 58, "Console");
        
        drawButton("New Project", 750, 15, 120, 30);
        drawButton("Open", 880, 15, 60, 30);
        drawButton("Save", 950, 15, 60, 30);
        
        if(currentProject) {
            char projText[128];
            sprintf(projText, "Project: %s by %s", currentProject->getName().c_str(), "User");
            drawText(projText, 350, 85, Color(0.5f,0.8f,0.5f,1));
        }
    }
    
    void drawPanel(int x, int y, int w, int h, const char* title) {
        SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderFillRect(renderer, &rect);
        
        SDL_SetRenderDrawColor(renderer, 0, 102, 255, 255);
        SDL_Rect border = {x-1, y-1, w+2, h+2};
        SDL_RenderDrawRect(renderer, &border);
        
        drawText(title, x + 10, y + 10, Color(1,1,1,1));
    }
    
    void drawButton(const char* text, int x, int y, int w, int h) {
        SDL_SetRenderDrawColor(renderer, 0, 102, 255, 200);
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderFillRect(renderer, &rect);
        drawText(text, x + w/2 - strlen(text)*4, y + h/2 - 6, Color(1,1,1,1));
    }
    
    void drawText(const char* text, int x, int y, Color color) {
        SDL_SetRenderDrawColor(renderer, (int)(color.r*255), (int)(color.g*255), (int)(color.b*255), (int)(color.a*255));
    }
    
    void handleEvents() {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                int mx = event.button.x, my = event.button.y;
                if(mx > 750 && mx < 870 && my > 15 && my < 45) {
                    createNewProject();
                }
            }
        }
    }
    
    void createNewProject() {
        if(currentProject) delete currentProject;
        currentProject = new Project("NewGame", "Developer");
        currentProject->addScript("main.lua");
        currentProject->addModel("player.ibx");
        printf("[STUDIO] Created new project\n");
    }
    
    void run() {
        while(running) {
            handleEvents();
            render();
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    }
    
    void shutdown() {
        if(currentProject) delete currentProject;
        if(renderer) SDL_DestroyRenderer(renderer);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

}

int main(int argc, char* argv[]) {
    studio::StudioApp app;
    
    if(!app.initialize()) {
        printf("[ERROR] Failed to initialize studio\n");
        return 1;
    }
    
    printf("[STUDIO] IrBox Studio v%s\n", IRBOX_VERSION);
    printf("[STUDIO] %s\n", IRBOX_COPYRIGHT);
    
    app.run();
    app.shutdown();
    
    return 0;
}
