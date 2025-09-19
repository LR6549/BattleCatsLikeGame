#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL3_image/SDL_image.h>
#include <SDL3/SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL3_rtf/SDL_rtf.h>
#include <SDL3/SDL3_net/SDL_net.h>

namespace fs = std::filesystem;
using json = nlohmann::json;
json settings;

std::string path = fs::current_path().string() + "/";

//* Window Variables
SDL_Window*     window = nullptr;
SDL_Renderer*   renderer = nullptr;
SDL_Texture*    renderTexture = nullptr;
MIX_Mixer*      mixer = nullptr;
MIX_Track*      musicTrack = nullptr;

//* Window Configs
std::string title = "BattleCatsLike";
int windowWidth  = 800;
int windowHeight = 600;

const int virtualWidth  = 1920;
const int virtualHeight = 1080;

//* Data-Maps
std::unordered_map<std::string, SDL_Texture*> textureMap;
std::unordered_map<std::string, MIX_Audio*> soundMap;
std::unordered_map<std::string, MIX_Audio*> musicMap;

//* Load JSON
json loadJson(std::string pathToJson) {
    std::ifstream inputFile(pathToJson);
    if (!inputFile.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, ("Failed to open " + pathToJson).c_str());
        exit(1);
    }
    json j;
    inputFile >> j;
    return j;
}

//* Load Music
int loadMusic() {
    std::string musicFolderPath = path + "data/music/";

    for (const auto& dir : fs::recursive_directory_iterator(musicFolderPath)) {
        if (dir.is_regular_file() && dir.path().extension() == ".mp3") {
            std::string tempPath = dir.path().string();
            std::string tempName = dir.path().stem().string();

            SDL_Log(("Music: " + tempName + " in " + tempPath).c_str());
            
            //* Load Audio from file
            MIX_Audio* audio = MIX_LoadAudio(mixer, tempPath.c_str(), true);
            if (!audio) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load music" + *SDL_GetError());
                continue;
            }
            
            musicMap[tempName] = audio;
            SDL_Log(("Loaded Music : " + tempPath + " in " + tempName).c_str());
        }
    }

    return 0;
}

//* Load sounds
int loadSounds() {
    std::string soundFolderPath = path + "data/sounds/";

    for (const auto& dir : fs::recursive_directory_iterator(soundFolderPath)) {
        if (dir.is_regular_file() && dir.path().extension() == ".mp3") {
            std::string tempPath = dir.path().string();
            std::string tempName = dir.path().stem().string();

            SDL_Log(("Sound: " + tempName + " in " + tempPath).c_str());
            
            //* Load Audio from file
            MIX_Audio* audio = MIX_LoadAudio(mixer, tempPath.c_str(), true);
            if (!audio) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load sound" + *SDL_GetError());
                continue;
            }

            soundMap[tempName] = audio;
            SDL_Log(("Loaded Sound : " + tempPath + " in " + tempName).c_str());
        }
    }

    return 0;
}

//* Load Textures
int loadTextures() {
    std::string textureFolderPath = path + "data/textures/";

    for (const auto& dir : fs::recursive_directory_iterator(textureFolderPath)) {
        if (dir.is_regular_file() && dir.path().extension() == ".png") {
            std::string tempPath = dir.path().string();
            std::string tempName = dir.path().stem().string();

            SDL_Log(("Texture: " + tempName + " in " + tempPath).c_str());

            //* load Texture from file
            SDL_Texture* tex = IMG_LoadTexture(renderer, tempPath.c_str());
            if (!tex) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture: " + *SDL_GetError());
                continue;
            }

            textureMap[tempName] = tex;
            SDL_Log(("Loaded Texture : " + tempPath + " in " + tempName).c_str());
        }
    }

    return 0;
}

void update(int deltaTime) {
    
}

void drawTexture(const std::string& textureName, float x = 0, float y = 0) {
    //* check if texture Exists
    auto it = textureMap.find(textureName);
    if (it == textureMap.end()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, ("Texture not found: " + textureName).c_str());
        return;
    }
    SDL_Texture* tex = it->second;

    //* try to get texture size
    float texW = 0, texH = 0;
    if (!SDL_GetTextureSize(tex, &texW, &texH)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_GetTextureSize failed: " + *SDL_GetError());
        return;
    }

    //* set render target to renderTexture
    SDL_SetRenderTarget(renderer, renderTexture);

    //* Destination rectangle using the Texture Size
    SDL_FRect dst = {x, y, texW, texH};

    if (!SDL_RenderTexture(renderer, tex, nullptr, &dst)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_RenderTexture failed: " + *SDL_GetError());
    }
}

void playSound(const std::string& soundName) {
    //* check if sound Exists
    auto it = soundMap.find(soundName);
    if (it == soundMap.end()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, ("Texture not found: " + soundName).c_str());
        return;
    }
    MIX_Audio* audio = it->second;

    MIX_PlayAudio(mixer, audio);
}

void playMusic(const std::string& musicName) {
    //* check if sound Exists
    auto it = musicMap.find(musicName);
    if (it == musicMap.end()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, ("Texture not found: " + musicName).c_str());
        return;
    }
    MIX_Audio* audio = it->second;

    MIX_SetTrackAudio(musicTrack, audio);

    MIX_PlayTrack(musicTrack, 0);
}

void render() {
    drawTexture("mainMenuBackground");
}

//* Handle Keyboard
void handleKeyboardInput(const SDL_KeyboardEvent& key) {
    if (key.key == SDLK_ESCAPE) {
        SDL_Event e;
        e.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&e);
    }
    SDL_Log("Pressed: " + *SDL_GetScancodeName(key.scancode));
}

//* Handle Mouse
void handleMouseInput(const SDL_MouseButtonEvent& mouse) {
    if (mouse.button == SDL_BUTTON_LEFT) {
        
    }
}

void cleanUp() {
    //* Cleanup textures
    for (auto& [name, tex] : textureMap) {
        SDL_DestroyTexture(tex);
    }    
    for (auto& [name, audio] : soundMap) {
        MIX_DestroyAudio(audio);
    }
    soundMap.clear();

    for (auto& [name, music] : musicMap) {
        MIX_DestroyAudio(music);
    }
    musicMap.clear();

    //* Clean Up SDL
    SDL_DestroyTexture(renderTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    MIX_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Error: " + *SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(title.c_str(), windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed: " + *SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer failed: " + *SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    renderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, virtualWidth, virtualHeight);
    if (!renderTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateTexture failed: " + *SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!MIX_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to init SDL_mixer: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        MIX_Quit();
        return 1;
    }

    SDL_AudioSpec spec{};
    spec.freq = 44100;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;

    mixer = MIX_CreateMixer(&spec);
    if (!mixer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateMixer failed: " + *SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    musicTrack = MIX_CreateTrack(mixer);
    if(!musicTrack) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateTrack failed: " + *SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    settings = loadJson(path + "/data/config/settings.json");
    loadMusic();
    loadSounds();
    loadTextures();

    SDL_Event event;
    bool running    = true;
    int lastTicks   = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    cleanUp();
                    return 0;
                case SDL_EVENT_KEY_DOWN:
                    handleKeyboardInput(event.key);
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    handleMouseInput(event.button);
                    break;
            }
        }

        uint64_t currentTicks = SDL_GetTicks();
        float deltaTime = (currentTicks - lastTicks) / 1000.0f;
        lastTicks = currentTicks;

        update(deltaTime);

        //* Render to the renderTexture
        SDL_SetRenderTarget(renderer, renderTexture);
        SDL_SetRenderDrawColor(renderer, 20, 20, 80, 255);
        SDL_RenderClear(renderer);

        render();

        //* Scale RenderTexture to window
        SDL_SetRenderTarget(renderer, nullptr);

        int winW, winH;
        SDL_GetWindowSize(window, &winW, &winH);
        SDL_FRect dstRect = {0, 0, (float)winW, (float)winH};

        //* clear window to black before drawing
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        //* Render the offscreen texture to the window
        SDL_RenderTexture(renderer, renderTexture, nullptr, &dstRect);

        SDL_RenderPresent(renderer);

    }

    return 0;
}
