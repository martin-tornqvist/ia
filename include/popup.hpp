#ifndef POPUP_H
#define POPUP_H

#include <vector>
#include <string>

#include "audio.hpp"

namespace popup
{

void show_msg(const std::string& msg,
              const bool DRAW_MAP_AND_INTERFACE,
              const std::string& title = "",
              const Sfx_id sfx = Sfx_id::END,
              const int W_CHANGE = 0);

int show_menu_msg(
    const std::string& msg, const bool DRAW_MAP_AND_INTERFACE,
    const std::vector<std::string>& choices, const std::string& title = "",
    const Sfx_id sfx = Sfx_id::END);

} //Popup

#endif
