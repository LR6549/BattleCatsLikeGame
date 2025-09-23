#ifndef BUTTONSTRUCT
#define BUTTONSTRUCT

#include <string>
#include <SDL3/SDL.h>
#include "logging.hpp"
#include "aabbCollision.hpp"

extern std::unordered_map<std::string, SDL_Texture*> textureMap;

void playSound(const std::string& soundName);

void drawTexture(const std::string& textureName, float x, float y, bool flipTexture);

void emptyFunction() {

}

struct Button {
    std::string b_buttonName;
    std::string b_textureName;

    float b_x = 0.0f, b_y = 0.0f;
    float b_w = 0.0f, b_h = 0.0f;

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

        if (!SDL_GetTextureSize(tex, &b_w, &b_h)) {
            log("ERROR Button Texture Size: ", "Could not get Button Texture Size!", LOGTYPE::ERROR);
        } else {
            log("BUTTON SIZE: ", (std::to_string(b_w) + ", " + std::to_string(b_h)), LOGTYPE::INFO);
        }
    }

    bool isPressed(float mouseX, float mouseY) {
        bool pressed = aabbCollision(b_x, b_y, b_w, b_h, mouseX, mouseY);

        if (pressed) {
            log("Pressed Button: ", b_buttonName, LOGTYPE::INFO);
        }

        return pressed;
    }

    void render() {
        drawTexture(b_textureName, b_x, b_y, false);
    }
};

#endif
