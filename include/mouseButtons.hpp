#ifndef MOUSE
#define MOUSE

#include <iostream>
#include <unordered_map>

float mouseX;
float mouseY;

std::unordered_map<std::string, bool> mouseButtons = {
    {"left", false},
    {"holdingLeft", false},
    {"middle", false},
    {"holdingMiddle", false},
    {"right", false},
    {"holdingRight", false}
};

#endif