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

void drawTexture(const std::string& textureName, float x = 0, float y = 0, bool flipTexture = false);

void emptyFunction() {

}

struct Button {
    std::string b_buttonName;
    std::string b_textureName;

    float b_x, b_y = 0.0f;
    float b_w, b_h = 0.0f;

    Button(std::string textureName, float x, float y, std::string buttonName = "") {
        b_buttonName = buttonName;
        b_textureName = textureName;
        b_x = x;
        b_y = y;

        auto it = textureMap.find(textureName);
        if (it == textureMap.end()) {
            log("Texture not found: ", textureName, LOGTYPE::ERROR);
            return;
        }
        SDL_Texture* tex = it->second;

        SDL_GetTextureSize(tex, &b_w, &b_h);
    }

    bool isPressed(float mouseX, float mouseY) {
        bool pressed = aabbCollision(b_x, b_y, b_w, b_h, mouseX, mouseY);

        if (pressed) {
            log("Pressed Button: ", b_buttonName, LOGTYPE::INFO);
        }

        return pressed;
    }

    void render() {
        drawTexture(b_textureName, b_x, b_y);
    }
};

#endif