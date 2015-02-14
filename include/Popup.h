#ifndef POPUP_H
#define POPUP_H

#include <vector>
#include <string>

#include "Audio.h"

namespace Popup
{

void showMsg(const std::string& msg,
             const bool         DRAW_MAP_AND_INTERFACE,
             const std::string& title = "",
             const SfxId        sfx = SfxId::END,
             const int          W_CHANGE = 0);

int showMenuMsg(
    const std::string& msg, const bool DRAW_MAP_AND_INTERFACE,
    const std::vector<std::string>& choices, const std::string& title = "",
    const SfxId sfx = SfxId::END);

} //Popup

#endif
