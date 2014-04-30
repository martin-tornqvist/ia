#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H

#include <string>
#include <vector>
#include <iostream>

#include "Colors.h"
#include "Converters.h"
#include "CmnTypes.h"

namespace Log {

void init();

void addMsg(const std::string& text, const SDL_Color& clr = clrWhite,
            const bool INTERRUPT_PLAYER_ACTIONS = false,
            const bool ADD_MORE_PROMPT_AFTER_MSG = false);

void drawLog(const bool SHOULD_UPDATE_SCREEN);

void displayHistory();

void clearLog();

void addLineToHistory(const std::string& lineToAdd);

} //Log

#endif
