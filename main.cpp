#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL3_image/SDL_image.h>
#include <SDL3/SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL3_rtf/SDL_rtf.h>
#include <SDL3/SDL3_net/SDL_net.h>
#include <JFLX/logging.hpp>
#include <JFLX/jsonFunctionality.hpp>
#include <JFLX/collision.hpp>
#include "buttonStruct.hpp"
#include "colorStruct.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;
json settings;

std::string path = fs::current_path().string() + "/";

//* Window Variables
SDL_Window*     window = nullptr;
SDL_Renderer*   renderer = nullptr;
SDL_Texture*    renderTexture = nullptr;
MIX_Mixer*      musicMixer = nullptr;
MIX_Mixer*      soundMixer = nullptr;
MIX_Track*      musicTrack = nullptr;

//* Fonts
TTF_Font* font              = nullptr;
TTF_Font* fontBold          = nullptr;

//* Window Configs
std::string title = "Furfront";
int windowWidth  = 960;
int windowHeight = 540;

const int virtualWidth  = 1920;
const int virtualHeight = 1080;

//* Data-Maps
std::unordered_map<std::string, SDL_Texture*> textureMap;
std::unordered_map<std::string, MIX_Audio*> soundMap;
std::unordered_map<std::string, MIX_Audio*> musicMap;

std::unordered_map<std::string, std::unique_ptr<Button>> buttonMap;

//* Game STATE
enum class STATE {
    NONE,
    TITLESCREEN,
    MAINMENU,
    SETTINGS,
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

STATE currentState  = STATE::TITLESCREEN;
STATE lastState     = STATE::NONE;

float scaleMousePositionFactorX = 2.0f;
float scaleMousePositionFactorY = 2.0f;

//* Returns the name of the given State
std::string getStateName(STATE& pState) {
    switch(pState) {
        case STATE::TITLESCREEN: {
            return "title";
        }
        case STATE::MAINMENU: {
            return "menu";
        }
        case STATE::SETTINGS: {
            return "settings";
        }
        case STATE::LOADOUTSELECT: {
            return "loadout";
        }
        case STATE::UPGRADEUNIT: {
            return "upgrade";
        }
        case STATE::WORLDSELECT: {
            return "world";
        }
        case STATE::LEVELSELECT: {
            return "level";
        }
        case STATE::LEVELPLAY: {
            return "play";
        }
        case STATE::BANNERSELECT: {
            return "banner";
        }
        case STATE::ROLLBANNER: {
            return "roll";
            break;
        }
        case STATE::STORAGEUNITS: {
            return "sUnit";
        }
        case STATE::STORAGEMATERIAL: {
            return "sMaterial";
        }
        default: {
            return "";
        }
    }
}

//* Puts all buttons in the button map
void setUpButtons() {
    //*  buttonMap.emplace("mapKey", std::make_unique<Button>("textureName", 0.0f, 0.0f, "ButtonName"));

    //* TITLE SCREEN
    buttonMap.emplace("TitleScreenPlay",     std::make_unique<Button>("titleScreenPlayBTN", 737.0f, 420.0f, "TitleScreenPlay"));
    buttonMap.emplace("TitleScreenSettings", std::make_unique<Button>("titleScreenSettingsBTN", 737.0f, 585.0f, "TitleScreenSettings"));
    buttonMap.emplace("TitleScreenQuit",     std::make_unique<Button>("titleScreenQuitBTN", 737.0f, 740.0f, "TitleScreenQuit"));

    //* MAIN MENU


    //* SETTINGS
    buttonMap.emplace("SettingsMusicPlus",  std::make_unique<Button>("SettingsMusicPlusBTN", 407.0f, 418.0f, "SettingsMusicPlus"));
    buttonMap.emplace("SettingsMusicMinus", std::make_unique<Button>("SettingsMusicMinusBTN", 196.0f, 418.0f, "SettingsMusicMinus"));
    buttonMap.emplace("SettingsSFXPlus",    std::make_unique<Button>("SettingsSFXPlusBTN", 407.0f, 629.0f, "SettingsSFXPlus"));
    buttonMap.emplace("SettingsSFXMinus",   std::make_unique<Button>("SettingsSFXMinusBTN", 196.0f, 629.0f, "SettingsSFXMinus"));

}

TTF_Font* loadFont(const std::string& pathToFont) {
    TTF_Font* tempFont = TTF_OpenFont(pathToFont.c_str(), 32);
    if (!tempFont) {
        JFLX::log("Failed to load font ", (pathToFont + ", " + SDL_GetError()), JFLX::LOGTYPE::ERROR);
        exit(1);
    }
    return tempFont;
}

//* Load all music files from "data/music/" into musicMap
int loadMusic() {
    std::string musicFolderPath = path + "data/music/";

    if (!fs::exists(musicFolderPath)) {
        JFLX::log("Music folder does not exist: ", musicFolderPath, JFLX::LOGTYPE::ERROR);
        return 0;
    }

    for (const auto& dir : fs::recursive_directory_iterator(musicFolderPath)) {
        if (dir.is_regular_file() && dir.path().extension() == ".mp3") {
            std::string tempPath = dir.path().string();
            std::string tempName = dir.path().stem().string();

            JFLX::log("Music:", (tempName + " in " + tempPath), JFLX::LOGTYPE::INFO);
            
            //* Load Audio from file (not streamed)
            MIX_Audio* audio = MIX_LoadAudio(musicMixer, tempPath.c_str(), false);
            if (!audio) {
                JFLX::log("Failed to load music from: ", (tempPath + "; " + SDL_GetError()), JFLX::LOGTYPE::ERROR);
                continue;
            }
            
            musicMap[tempName] = audio;
            JFLX::log("Loaded Music: ", (tempPath + " as " + tempName), JFLX::LOGTYPE::SUCCESS);
        }
    }

    return 0;
}

//* Load all sound effects from "data/sounds/" into soundMap
int loadSounds() {
    std::string soundFolderPath = path + "data/sounds/";

    if (!fs::exists(soundFolderPath)) {
        JFLX::log("Sound folder does not exist: ", soundFolderPath.c_str());
        return 0;
    }

    for (const auto& dir : fs::recursive_directory_iterator(soundFolderPath)) {
        if (dir.is_regular_file() && dir.path().extension() == ".mp3") {
            std::string tempPath = dir.path().string();
            std::string tempName = dir.path().stem().string();

            JFLX::log("Sound: ", (tempName + " in " + tempPath), JFLX::LOGTYPE::INFO);
            
            //* Load Audio from file (not streamed)
            MIX_Audio* audio = MIX_LoadAudio(musicMixer, tempPath.c_str(), false);
            if (!audio) {
                JFLX::log("Failed to load sound from: ", (tempPath + "; " + SDL_GetError()), JFLX::LOGTYPE::ERROR);
                continue;
            }

            soundMap[tempName] = audio;
            JFLX::log("Loaded Sound: ", (tempPath + " as " + tempName), JFLX::LOGTYPE::SUCCESS);
        }
    }

    return 0;
}

//* Load all textures from "data/textures/" into textureMap
int loadTextures() {
    std::string textureFolderPath = path + "data/textures/";

    if (!fs::exists(textureFolderPath)) {
        JFLX::log("Texture folder does not exist: ", textureFolderPath, JFLX::LOGTYPE::INFO);
        return 0;
    }

    for (const auto& dir : fs::recursive_directory_iterator(textureFolderPath)) {
        if (dir.is_regular_file() && dir.path().extension() == ".png") {
            std::string tempPath = dir.path().string();
            std::string tempName = dir.path().stem().string();

            JFLX::log("Texture: ", (tempName + " in " + tempPath), JFLX::LOGTYPE::INFO);

            //* load Texture from file
            SDL_Texture* tex = IMG_LoadTexture(renderer, tempPath.c_str());
            if (!tex) {
                JFLX::log("Failed to load texture: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
                continue;
            }

            textureMap[tempName] = tex;
            JFLX::log("Loaded Texture: ", (tempPath + " in " + tempName), JFLX::LOGTYPE::SUCCESS);
        }
    }

    return 0;
}

//* Updates the mouse Scales if window size changed!
void updateMouseScale() {
    int currentWindowW, currentWindowH;
    SDL_GetWindowSize(window, &currentWindowW, &currentWindowH);

    if (currentWindowW != windowWidth || currentWindowH != windowHeight) {
        //! Change to Log
        // std::cout << "Window Resolution Was Changed! From: "
        //           << widthOld << "," << heightOld
        //           << "; To: "
        //           << windowW << "," << windowH << "\n";
        
        float scaleX = static_cast<float>(currentWindowW) / 960.0f;
        float scaleY = static_cast<float>(currentWindowH) / 540.0f;
        scaleMousePositionFactorX = 2.0f / scaleX;
        scaleMousePositionFactorY = 2.0f / scaleY;
        
        //! Change to Log
        // std::cout << "New Scales: " << scaleX << "," << scaleY
        //           << " for mouse: " << scaleXMouse << "," << scaleYMouse << "\n";
        
        windowWidth = currentWindowW;
        windowHeight = currentWindowH;
    }
}

//* Updates the music and sound mixer gain based on the settings
void updateMixerGain() {
    //* Updating music Mixer Gain
    MIX_SetMasterGain(musicMixer, settings["volume"]["music"].get<int>()/100.0f);
    //* Updating sound Mixer Gain
    MIX_SetMasterGain(soundMixer, settings["volume"]["sfx"].get<int>()/100.0f);
}

//* Play a sound effect by name
void playSound(const std::string& soundName) {
    //* check if sound Exists
    auto it = soundMap.find(soundName);
    if (it == soundMap.end()) {
        JFLX::log("Sound not found: ", soundName.c_str());
        return;
    }
    MIX_Audio* audio = it->second;

    if (!MIX_PlayAudio(soundMixer, audio)) {
        JFLX::log("Played sound: ", soundName.c_str());
    } else {
        JFLX::log("MIX_PlayAudio failed: ", SDL_GetError());
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
        JFLX::log("Music not found: ", musicName.c_str());
        return;
    }
    MIX_Audio* audio = it->second;

    if (!musicTrack) {
        JFLX::log("Music track not initialized", "", JFLX::LOGTYPE::ERROR);
        return;
    }

    if (MIX_GetTrackAudio(musicTrack) == audio) {
        JFLX::log("Already Playing Music: ", ("The Music Called to play was already playing [" + musicName + "]"), JFLX::LOGTYPE::INFO);
        return;
    }

    //* assign audio to track
    MIX_SetTrackAudio(musicTrack, audio);

    //* Loop infinitely by default
    SDL_PropertiesID options = SDL_CreateProperties();
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, 2000);

    if (MIX_PlayTrack(musicTrack, options)) {
        if (MIX_TrackPlaying(musicTrack)) {
            JFLX::log("Playing music: ", musicName, JFLX::LOGTYPE::SUCCESS);
        } else {
            JFLX::log("Track Not Playing: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        }
    } else {
        JFLX::log("MIX_PlayTrack failed: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
    }
    SDL_DestroyProperties(options);
}

//* Initialisation of a new game State
void initState() {
    if (lastState != currentState) {
        switch (currentState) {
            case STATE::TITLESCREEN: {
                playMusic("baseTheme");
                // TODO: title screen logic
                break;
            }
            case STATE::MAINMENU: {
                playMusic("baseTheme");
                // TODO: main menu logic
                break;
            }
            case STATE::SETTINGS: {
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

    switch (currentState) {
        case STATE::TITLESCREEN: {
            // TODO: title screen logic
            break;
        }
        case STATE::MAINMENU: {
            // TODO: main menu logic
            break;
        }
        case STATE::SETTINGS: {
            // TODO: main menu logic
            break;
        }
        case STATE::LOADOUTSELECT: {
            // TODO: loadout select logic
            break;
        }
        case STATE::UPGRADEUNIT: {
            // TODO: upgrade unit logic
            break;
        }
        case STATE::WORLDSELECT: {
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
            // TODO: banner select logic
            break;
        }
        case STATE::ROLLBANNER: {
            playMusic("Silent");
            // TODO: roll banner logic
            break;
        }
        case STATE::STORAGEUNITS: {
            // TODO: storage units logic
            break;
        }
        case STATE::STORAGEMATERIAL: {
            // TODO: storage material logic
            break;
        }
        default: {
            // Optional: handle unknown/invalid state
            break;
        }
    }
}

//* Draw a texture by name at given coordinates
void drawTexture(const std::string& textureName, float x = 0, float y = 0, bool flipTexture = false) {
    //* check if texture Exists
    auto it = textureMap.find(textureName);
    if (it == textureMap.end()) {
        JFLX::log("Texture not found: ", textureName, JFLX::LOGTYPE::ERROR);
        return;
    }
    SDL_Texture* tex = it->second;

    //* try to get texture size
    float texW = 0, texH = 0;
    if (!SDL_GetTextureSize(tex, &texW, &texH)) {
        JFLX::log("SDL_GetTextureSize failed: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        return;
    }

    //* set render target to renderTexture
    SDL_SetRenderTarget(renderer, renderTexture);

    //* Destination rectangle using the Texture Size
    SDL_FRect dst = {x, y, texW, texH};

    SDL_FlipMode flipOrientation = flipTexture ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (!SDL_RenderTextureRotated(renderer, tex, nullptr, &dst, 0.0, nullptr, flipOrientation)) {
        JFLX::log("SDL_RenderTexture failed: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
    }
}

//* Draw a Text at a given location | Orientations: -1 = left, 0 = center, 1 = right
void drawText(std::string text, int fontSize, float x = 0, float y = 0, Color color = COLORS::WHITE, TTF_Font* fontPtr = font, int orientation = 0, bool outline = false) {
    if (!fontPtr) {
        JFLX::log("Font Not Loaded: ", "", JFLX::LOGTYPE::ERROR);
        return;
    }

    SDL_Color sdlColor = {color.c_r, color.c_g, color.c_b, color.c_a };

    // Render text to a surface
    SDL_Surface* surface = TTF_RenderText_Blended(fontPtr, text.c_str(), text.size(), sdlColor);
    if (!surface) {
        JFLX::log("Failed to render text: ", SDL_GetError(),  JFLX::LOGTYPE::ERROR);
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (!texture) {
        JFLX::log("Failed to create text texture: ", SDL_GetError(),  JFLX::LOGTYPE::ERROR);
        return;
    }

    float textW, textH;
    SDL_GetTextureSize(texture, &textW, &textH);

    // Position based on orientation
    SDL_FRect dstRect;
    dstRect.w = (float)textW;
    dstRect.h = (float)textH;

    if (orientation == 0)       dstRect.x = x - (textW / 2.0f);
    else if (orientation == 1)  dstRect.x = x - textW;
    else                        dstRect.x = x;

    dstRect.y = y;

    // Shadow first 
    if (outline) {
        SDL_Surface* outlineSurface = TTF_RenderText_Blended(font, text.c_str(), text.size(), {0, 0, 0, 255});
        SDL_Texture* outlineTexture = SDL_CreateTextureFromSurface(renderer, outlineSurface);
        SDL_DestroySurface(outlineSurface);

        if (outlineTexture) {
            SDL_FRect outlineRect = dstRect;

            //* Offset in relation der fontSize
            float offset = fontSize / 6.5f;

            //* Liste aller Richtungen der Textur
            const SDL_FPoint offsets[] = {
                {-offset, -offset}, {0, -offset}, {offset, -offset},
                {-offset,  0},                  {offset,  0},
                {-offset,  offset}, {0,  offset}, {offset,  offset}
            };

            for (auto& o : offsets) {
                outlineRect.x = dstRect.x + o.x;
                outlineRect.y = dstRect.y + o.y;
                SDL_RenderTexture(renderer, outlineTexture, nullptr, &outlineRect);
            }

            SDL_DestroyTexture(outlineTexture);
        }
    }


    //* Draw FG text
    SDL_RenderTexture(renderer, texture, nullptr, &dstRect);

    SDL_DestroyTexture(texture);
}

//* Render function (main drawing function for a frame)
void render() {
    switch (currentState) {
        case STATE::TITLESCREEN: {
            // TODO: title screen logic
            drawTexture("mainMenuBackground");
            buttonMap.at("TitleScreenPlay")->render();
            buttonMap.at("TitleScreenSettings")->render();
            buttonMap.at("TitleScreenQuit")->render();
            break;
        }
        case STATE::MAINMENU: {
            // TODO: main menu logic
            break;
        }
        case STATE::SETTINGS: {
            // TODO: main menu logic
            drawTexture("settingsBackground");

            //* Music Settings
            buttonMap.at("SettingsMusicPlus")->render();
            buttonMap.at("SettingsMusicMinus")->render();
            drawText((std::to_string(settings["volume"]["music"].get<int>()) + "%"), 32, 370, 417, COLORS::SETTINGSCOLOR, fontBold, 1, true);

            //* SFX Settings
            buttonMap.at("SettingsSFXPlus")->render();
            buttonMap.at("SettingsSFXMinus")->render();
            drawText((std::to_string(settings["volume"]["sfx"].get<int>()) + "%"),   32, 370, 627, COLORS::SETTINGSCOLOR, fontBold, 1, true);

            break;
        }
        case STATE::LOADOUTSELECT: {
            // TODO: loadout select logic
            break;
        }
        case STATE::UPGRADEUNIT: {
            // TODO: upgrade unit logic
            break;
        }
        case STATE::WORLDSELECT: {
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
            // TODO: banner select logic
            break;
        }
        case STATE::ROLLBANNER: {
            // TODO: roll banner logic
            break;
        }
        case STATE::STORAGEUNITS: {
            // TODO: storage units logic
            break;
        }
        case STATE::STORAGEMATERIAL: {
            // TODO: storage material logic
            break;
        }
        default: {
            // Optional: handle unknown/invalid state
            break;
        }
    }
    
    if (isPaused) {
        drawTexture("PAUSED", 0, 0, true);
    }
}

//* Handle keyboard input events
void handleKeyboardInput(const SDL_KeyboardEvent& key) {
    JFLX::log("Pressed: ", SDL_GetScancodeName(key.scancode), JFLX::LOGTYPE::INFO);

    switch (currentState) {
        case STATE::TITLESCREEN: {
            // TODO: title screen logic
            
            break;
        }
        case STATE::MAINMENU: {
            // TODO: main menu logic
            if (key.key == SDLK_ESCAPE) {
                currentState = STATE::TITLESCREEN;
            }
            break;
        }
        case STATE::SETTINGS: {
            // TODO: main menu logic
            if (key.key == SDLK_ESCAPE) {
                currentState = STATE::TITLESCREEN;
            }
            break;
        }
        case STATE::LOADOUTSELECT: {
            // TODO: loadout select logic
            if (key.key == SDLK_ESCAPE) {
                currentState = STATE::MAINMENU;
            }
            break;
        }
        case STATE::UPGRADEUNIT: {
            // TODO: upgrade unit logic
            if (key.key == SDLK_ESCAPE) {
                currentState = STATE::MAINMENU;
            }
            break;
        }
        case STATE::WORLDSELECT: {
            // TODO: world select logic
            if (key.key == SDLK_ESCAPE) {
                currentState = STATE::MAINMENU;
            }
            break;
        }
        case STATE::LEVELSELECT: {
            // TODO: level select logic
            if (key.key == SDLK_ESCAPE) {
                currentState = STATE::WORLDSELECT;
            }
            break;
        }
        case STATE::LEVELPLAY: {
            // TODO: level play logic
            
            if (key.key == SDLK_ESCAPE) {
                isPaused = !isPaused;
            }
            break;
        }
        case STATE::BANNERSELECT: {
            // TODO: banner select logic
            if (key.key == SDLK_ESCAPE) {
                currentState = STATE::MAINMENU;
            }
            break;
        }
        case STATE::ROLLBANNER: {
            // TODO: roll banner logic
            break;
        }
        case STATE::STORAGEUNITS: {
            // TODO: storage units logic
            if (key.key == SDLK_ESCAPE) {
                currentState = STATE::MAINMENU;
            }
            break;
        }
        case STATE::STORAGEMATERIAL: {
            // TODO: storage material logic
            if (key.key == SDLK_ESCAPE) {
                currentState = STATE::MAINMENU;
            }
            break;
        }
        default: {
            // Optional: handle unknown/invalid state
            break;
        }
    }
}

//* Handle mouse input events
void handleMouseInput(const SDL_MouseButtonEvent& mouse) {
    if (mouse.button == SDL_BUTTON_LEFT) {
        bool updateVolume = false;

        float mouseX = mouse.x * scaleMousePositionFactorX;
        float mouseY = mouse.y * scaleMousePositionFactorY;

        JFLX::log("Left Click at:", (std::to_string(mouseX) + ", " + std::to_string(mouseY)), JFLX::LOGTYPE::INFO);

        switch (currentState) {
            case STATE::TITLESCREEN: {
                // TODO: title screen logic
                if (buttonMap.at("TitleScreenPlay")->isPressed(mouseX, mouseY)) {
                    currentState = STATE::MAINMENU;
                } else if (buttonMap.at("TitleScreenSettings")->isPressed(mouseX, mouseY)) {
                    currentState = STATE::SETTINGS;
                } else if (buttonMap.at("TitleScreenQuit")->isPressed(mouseX, mouseY)) {
                    SDL_Event e;
                    e.type = SDL_EVENT_QUIT;
                    SDL_PushEvent(&e);
                }
                break;
            }
            case STATE::MAINMENU: {
                // TODO: main menu logic
                break;
            }
            case STATE::SETTINGS: {
                // TODO: main menu logic
                if (buttonMap.at("SettingsMusicPlus")->isPressed(mouseX, mouseY)) {
                    if (settings["volume"]["music"].get<int>() >= 200) {
                        settings["volume"]["music"] = 200;
                    } else {
                        settings["volume"]["music"] = settings["volume"]["music"].get<int>() + 5;
                    }
                } else if (buttonMap.at("SettingsMusicMinus")->isPressed(mouseX, mouseY)) {
                    if (settings["volume"]["music"].get<int>() <= 0) {
                        settings["volume"]["music"] = 0;
                    } else {
                        settings["volume"]["music"] = settings["volume"]["music"].get<int>() - 5;
                    }
                } else if (buttonMap.at("SettingsSFXPlus")->isPressed(mouseX, mouseY)) {
                    if (settings["volume"]["sfx"].get<int>() >= 200) {
                        settings["volume"]["sfx"] = 200;
                    } else {
                        settings["volume"]["sfx"] = settings["volume"]["sfx"].get<int>() + 5;
                    }
                } else if (buttonMap.at("SettingsSFXMinus")->isPressed(mouseX, mouseY)) {
                    if (settings["volume"]["sfx"].get<int>() <= 0) {
                        settings["volume"]["sfx"] = 0;
                    } else {
                        settings["volume"]["sfx"] = settings["volume"]["sfx"].get<int>() - 5;
                    }
                }
                buttonMap.at("SettingsMusicPlus")->render();
                buttonMap.at("SettingsMusicMinus")->render();
                buttonMap.at("SettingsSFXPlus")->render();
                buttonMap.at("SettingsSFXMinus")->render();
                break;
            }
            case STATE::LOADOUTSELECT: {
                // TODO: loadout select logic
                break;
            }
            case STATE::UPGRADEUNIT: {
                // TODO: upgrade unit logic
                break;
            }
            case STATE::WORLDSELECT: {
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
                // TODO: banner select logic
                break;
            }
            case STATE::ROLLBANNER: {
                // TODO: roll banner logic
                break;
            }
            case STATE::STORAGEUNITS: {
                // TODO: storage units logic
                break;
            }
            case STATE::STORAGEMATERIAL: {
                // TODO: storage material logic
                break;
            }
            default: {
                // Optional: handle unknown/invalid state
                break;
            }
        }
        if (updateVolume) {
            updateMixerGain();
        }
    }
}

//* Cleanup all loaded resources and shutdown SDL properly
void cleanUp() {
    JFLX::log("Running CleanUp: ", "", JFLX::LOGTYPE::SUCCESS);

    //* Save Json's
    if (JFLX::saveJson((path+"/data/config/settings.json"), settings)) {
        JFLX::log("Saved Json", "Successfully Saved Settings.json", JFLX::LOGTYPE::SUCCESS);
    }

    //* Cleanup textures
    for (auto& [name, tex] : textureMap) {
        SDL_DestroyTexture(tex);
    }    
    textureMap.clear();

    //* Cleanup music track and musicMixer
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

    if (musicMixer) {
        MIX_DestroyMixer(musicMixer);
        musicMixer = nullptr;
    }

    //* Cleanup Fonts
    TTF_CloseFont(font);
    TTF_CloseFont(fontBold);


    //* Destroy rendering objects and quit SDL subsystems
    if (renderTexture) SDL_DestroyTexture(renderTexture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    MIX_Quit();
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        JFLX::log("SDL_Init Error: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        cleanUp();
        return 1;
    }

    window = SDL_CreateWindow(title.c_str(), windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    if (!window) {
        JFLX::log("SDL_CreateWindow failed: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        cleanUp();
        return 1;
    }

    // create a renderer (index -1 = first, flags = accelerated+vsync recommended)
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        JFLX::log("SDL_CreateRenderer failed: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        cleanUp();
        return 1;
    }
    
    renderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, virtualWidth, virtualHeight);
    if (!renderTexture) {
        JFLX::log("SDL_CreateTexture failed: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        cleanUp();
        return 1;
    }

    if (!MIX_Init()) {
        JFLX::log("Failed to init SDL_mixer: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        cleanUp();
        return 1;
    }

    // Create a musicMixer attached to the default playback device (let SDL decide format)
    musicMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!musicMixer) {
        JFLX::log("MIX_CreateMixerDevice failed: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        cleanUp();
        return 1;
    }

    musicTrack = MIX_CreateTrack(musicMixer);
    if (!musicTrack) {
        JFLX::log("MIX_CreateTrack failed: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        cleanUp();
        return 1;
    }

    if (!TTF_Init()) {
        JFLX::log("TTF_Init failed: ", SDL_GetError(), JFLX::LOGTYPE::ERROR);
        cleanUp();
        return 1;
    }
    std::string fontPath = path + "data/fonts/winterLemon.ttf";
    font            = loadFont(fontPath);
    fontBold        = loadFont(fontPath);
    if (!font || !fontBold) {
        JFLX::log("Could not load Font: ", (fontPath), JFLX::LOGTYPE::ERROR);
        cleanUp();
        return -1;
    }
    TTF_SetFontStyle(fontBold, TTF_STYLE_BOLD);
    
    //* load config, assets and general set up calls
    if (!JFLX::loadJson(path + "/data/config/settings.json", settings)) {
        cleanUp();
        return 1;
    }
    loadMusic();
    loadSounds();
    loadTextures();
    setUpButtons();
    
    SDL_Event event;
    bool running    = true;
    uint64_t lastTicks   = SDL_GetTicks();

    while (running) {
        updateMouseScale();

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
                case SDL_EVENT_MOUSE_BUTTON_UP: {
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

    cleanUp();

    return 0;
}

//! Current Bugs/Errors:
/*
! 
*/
