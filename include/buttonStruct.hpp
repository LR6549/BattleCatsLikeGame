#ifndef BUTTONSTRUCT
#define BUTTONSTRUCT

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <SDL3/SDL.h>
#include "logging.hpp"
#include "aabbCollision.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;
json settings;

std::unordered_map<std::string, SDL_Texture*> textureMap;

void playSound(const std::string& soundName);

void drawTexture(const std::string& textureName, float x = 0, float y = 0);

void emptyFunction() {

}

struct Button {
    std::string textureName;
    float x, y = 0.0f;
    float w, h = 0.0f;

    Button(std::string textureName, float x, float y) {
        auto it = textureMap.find(textureName);
        if (it == textureMap.end()) {
            log("Texture not found: ", textureName, LOGTYPE::ERROR);
            return;
        }
        SDL_Texture* tex = it->second;

        SDL_GetTextureSize(tex, &w, &h);
    }

    bool isPressed(float mouseX, float mouseY) {
        return (x, y, w, h, mouseX, mouseY);
    }

    void render() {
        drawTexture(textureName, x, y);
    }
};

#endif