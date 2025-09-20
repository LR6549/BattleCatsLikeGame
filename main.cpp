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

//* Game STATE
enum class STATE {
    NONE,
    TITLESCREEN,
    MAINMENU,
    LOADOUTSELECT,
    UPGRADEUNIT,
    WORLDSELECT,
    LEVELSELECT,
    LEVELPLAY,
    BANNERSELECT,
    ROLLBANNER,
    STORAGEUNITS,
    STORAGEMATERIAL,
};

bool isPaused = false;

STATE currentState = STATE::TITLESCREEN;
STATE lastState    = STATE::NONE;

//* Load JSON from file and return as parsed json object
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

//* Load all music files from "data/music/" into musicMap
int loadMusic() {
    std::string musicFolderPath = path + "data/music/";

    if (!fs::exists(musicFolderPath)) {
        SDL_Log("Music folder does not exist: %s", musicFolderPath.c_str());
        return 0;
    }

    for (const auto& dir : fs::recursive_directory_iterator(musicFolderPath)) {
        if (dir.is_regular_file() && dir.path().extension() == ".mp3") {
            std::string tempPath = dir.path().string();
            std::string tempName = dir.path().stem().string();

            SDL_Log("Music: %s in %s", tempName.c_str(), tempPath.c_str());
            
            //* Load Audio from file (not streamed)
            MIX_Audio* audio = MIX_LoadAudio(mixer, tempPath.c_str(), false);
            if (!audio) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load music '%s': %s", tempPath.c_str(), SDL_GetError());
                continue;
            }
            
            musicMap[tempName] = audio;
            SDL_Log("Loaded Music : %s as %s", tempPath.c_str(), tempName.c_str());
        }
    }

    return 0;
}

//* Load all sound effects from "data/sounds/" into soundMap
int loadSounds() {
    std::string soundFolderPath = path + "data/sounds/";

    if (!fs::exists(soundFolderPath)) {
        SDL_Log("Sound folder does not exist: %s", soundFolderPath.c_str());
        return 0;
    }

    for (const auto& dir : fs::recursive_directory_iterator(soundFolderPath)) {
        if (dir.is_regular_file() && dir.path().extension() == ".mp3") {
            std::string tempPath = dir.path().string();
            std::string tempName = dir.path().stem().string();

            SDL_Log("Sound: %s in %s", tempName.c_str(), tempPath.c_str());
            
            //* Load Audio from file (not streamed)
            MIX_Audio* audio = MIX_LoadAudio(mixer, tempPath.c_str(), false);
            if (!audio) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load sound '%s': %s", tempPath.c_str(), SDL_GetError());
                continue;
            }

            soundMap[tempName] = audio;
            SDL_Log("Loaded Sound : %s as %s", tempPath.c_str(), tempName.c_str());
        }
    }

    return 0;
}

//* Load all textures from "data/textures/" into textureMap
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

//* Play a sound effect by name
void playSound(const std::string& soundName) {
    //* check if sound Exists
    auto it = soundMap.find(soundName);
    if (it == soundMap.end()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound not found: %s", soundName.c_str());
        return;
    }
    MIX_Audio* audio = it->second;

    if (!MIX_PlayAudio(mixer, audio)) {
        SDL_Log("Played sound: %s", soundName.c_str());
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_PlayAudio failed: %s", SDL_GetError());
    }
}

//* Start a new music track, fading out the old one (helper)
static void playMusic(std::string musicName) {
    //* If a track is already playing, fade it out and stop
    if (musicTrack && MIX_TrackPlaying(musicTrack)) {
        SDL_PropertiesID fadeout_opts = SDL_CreateProperties();
        SDL_SetNumberProperty(fadeout_opts, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, 3000);
        SDL_DestroyProperties(fadeout_opts);
    }

    //* check if music Exists
    auto it = musicMap.find(musicName);
    if (it == musicMap.end()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Music not found: %s", musicName.c_str());
        return;
    }
    MIX_Audio* audio = it->second;

    if (!musicTrack) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Music track not initialized");
        return;
    }

    //* Loop infinitely by default
    SDL_PropertiesID options = SDL_CreateProperties();
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, 2000);
    if (!MIX_PlayTrack(musicTrack, options)) {
        SDL_Log("Playing music: %s", musicName.c_str());
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_PlayTrack failed: %s", SDL_GetError());
    }
    SDL_DestroyProperties(options);
}

//* Initialisation of a new game State
void initState() {
    if (lastState != currentState) {
        switch (currentState) {
            case STATE::TITLESCREEN: {
                playMusic("baseTheme");
                break;
            }
            case STATE::MAINMENU: {
                playMusic("baseTheme");
                // TODO: main menu logic
                break;
            }
            case STATE::LOADOUTSELECT: {
                playMusic("baseTheme");
                // TODO: loadout select logic
                break;
            }
            case STATE::UPGRADEUNIT: {
                playMusic("baseTheme");
                // TODO: upgrade unit logic
                break;
            }
            case STATE::WORLDSELECT: {
                playMusic("baseTheme");
                // TODO: world select logic
                break;
            }
            case STATE::LEVELSELECT: {
                // TODO: level select logic
                break;
            }
            case STATE::LEVELPLAY: {
                // TODO: level play logic
                break;
            }
            case STATE::BANNERSELECT: {
                playMusic("baseTheme");
                // TODO: banner select logic
                break;
            }
            case STATE::ROLLBANNER: {
                playMusic("Silent");
                // TODO: roll banner logic
                break;
            }
            case STATE::STORAGEUNITS: {
                playMusic("baseTheme");
                // TODO: storage units logic
                break;
            }
            case STATE::STORAGEMATERIAL: {
                playMusic("baseTheme");
                // TODO: storage material logic
                break;
            }
            default: {
                // Optional: handle unknown/invalid state
                break;
            }
        }
        lastState = currentState;
    }
}

//* Update function (game logic per frame)
void update(int deltaTime) {
    initState();
}

//* Draw a texture by name at given coordinates
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

//* Render function (main drawing function for a frame)
void render() {
    drawTexture("mainMenuBackground");
}

//* Handle keyboard input events
void handleKeyboardInput(const SDL_KeyboardEvent& key) {
    if (key.key == SDLK_ESCAPE) {
        SDL_Event e;
        e.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&e);
    }
    SDL_Log("Pressed: " + *SDL_GetScancodeName(key.scancode));
}

//* Handle mouse input events
void handleMouseInput(const SDL_MouseButtonEvent& mouse) {
    if (mouse.button == SDL_BUTTON_LEFT) {
        
    }
}

//* Cleanup all loaded resources and shutdown SDL properly
void cleanUp() {
    //* Cleanup textures
    for (auto& [name, tex] : textureMap) {
        SDL_DestroyTexture(tex);
    }    
    textureMap.clear();

    //* Cleanup music track and mixer
    for (auto& [name, audio] : soundMap) {
        MIX_DestroyAudio(audio);
    }

    soundMap.clear();
    for (auto& [name, music] : musicMap) {
        MIX_DestroyAudio(music);
    }
    musicMap.clear();

    if (musicTrack) {
        MIX_DestroyTrack(musicTrack);
        musicTrack = nullptr;
    }

    if (mixer) {
        MIX_DestroyMixer(mixer);
        mixer = nullptr;
    }

    //* Destroy rendering objects and quit SDL subsystems
    if (renderTexture) SDL_DestroyTexture(renderTexture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    MIX_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Error: %s", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(title.c_str(), windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // create a renderer (index -1 = first, flags = accelerated+vsync recommended)
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    renderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, virtualWidth, virtualHeight);
    if (!renderTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateTexture failed: %s", SDL_GetError());
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
        return 1;
    }

    // Create a mixer attached to the default playback device (let SDL decide format)
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!mixer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateMixerDevice failed: %s", SDL_GetError());
        MIX_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    musicTrack = MIX_CreateTrack(mixer);
    if (!musicTrack) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "MIX_CreateTrack failed: %s", SDL_GetError());
        if (mixer) MIX_DestroyMixer(mixer);
        MIX_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // load config, assets
    settings = loadJson(path + "/data/config/settings.json");
    loadMusic();
    loadSounds();
    loadTextures();

    SDL_Event event;
    bool running    = true;
    uint64_t lastTicks   = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    cleanUp();
                    return 0;
                }
                case SDL_EVENT_KEY_DOWN: {
                    handleKeyboardInput(event.key);
                    break;
                }
                case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                    handleMouseInput(event.button);
                    break;
                }
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

        if (!isPaused) {
            update(deltaTime);
        }

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
