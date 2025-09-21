#ifndef BUTTONSTRUCT
#define BUTTONSTRUCT

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <SDL3/SDL.h>
#include "logging.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;
json settings;

void playSound(const std::string& soundName);

void drawTexture(const std::string& textureName, float x = 0, float y = 0);



#endif