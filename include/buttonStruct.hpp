#ifndef BUTTONSTRUCT
#define BUTTONSTRUCT

#include <string>
#include <SDL3/SDL.h>
#include <JFLX/logging.hpp>
#include <JFLX/collision.hpp>

extern std::unordered_map<std::string, SDL_Texture*> textureMap;

void playSound(const std::string& soundName);

void drawTexture(const std::string& textureName, float x, float y, bool flipTexture);

void emptyFunction() {

}

struct Button {
    std::string b_buttonName;
    std::string b_textureName;

    bool hovered = false;

    float b_x = 0.0f, b_y = 0.0f;
    float b_w = 0.0f, b_h = 0.0f;

    int tick = 0;

    Button(std::string textureName, float x, float y, std::string buttonName = "") {
        b_buttonName = buttonName;
        b_textureName = textureName;
        b_x = x;
        b_y = y;

        //* check for Textures in map
        auto it = textureMap.find(textureName);
        if (it == textureMap.end()) {
            JFLX::log("Texture not found: ", textureName, JFLX::LOGTYPE::ERROR);
            return;
        }
        SDL_Texture* tex = it->second;

        if (!SDL_GetTextureSize(tex, &b_w, &b_h)) {
            JFLX::log("ERROR Button Texture Size: ", "Could not get Button Texture Size!", JFLX::LOGTYPE::ERROR);
        } else {
            JFLX::log("BUTTON SIZE: ", (std::to_string(b_w) + ", " + std::to_string(b_h)), JFLX::LOGTYPE::INFO);
        }

        it = textureMap.find((textureName+"H"));
        if (it == textureMap.end()) {
            JFLX::log("Texture not found: ", textureName+"H", JFLX::LOGTYPE::ERROR);
            return;
        }
    }

    bool isHovered(float mouseX, float mouseY) {
        tick ++;
        hovered = JFLX::aabb::inBounds(b_x, b_y, b_w, b_h, mouseX, mouseY);

        if (hovered && tick > 60) {
            tick = 0;
            JFLX::log("Hovered Button: ", b_buttonName, JFLX::LOGTYPE::INFO);
        }

        return hovered;
    }

    void render() {
        if (hovered) {
            drawTexture((b_textureName+"H"), b_x, b_y, false);
        } else {
            drawTexture(b_textureName, b_x, b_y, false);
        }
    }
};

#endif
