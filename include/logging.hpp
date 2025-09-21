#ifndef LOGGING
#define LOGGING

#include <iostream>
#include <string>
#include <SDL3/SDL.h>

enum class LOGTYPE {
    NONE,
    ERROR,
    WARNING,
    INFO,
    SUCCESS
};

//* Returns a string for the corresponding error type
std::string logTypeToColor(LOGTYPE type) {
    switch (type) {
        case LOGTYPE::NONE:    return "\033[0m";
        case LOGTYPE::ERROR:   return "\033[91;1;4m";
        case LOGTYPE::WARNING: return "\033[33;1;4m";
        case LOGTYPE::INFO:    return "\033[93;1;4m";
        case LOGTYPE::SUCCESS: return "\033[92;1;4m";
    }
    return "\033[0m";
}

//* A wrapper for SDL_Log but with messageInfo highlighted in color showing degree
void log(std::string messageInfo, std::string messageContent = "", LOGTYPE logType = LOGTYPE::NONE) {
    std::string logColor = logTypeToColor(logType);

    std::string msg = logColor + messageInfo + "\033[0m" + messageContent;

    SDL_Log(msg.c_str());
}

#endif