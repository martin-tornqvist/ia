#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>

#include "cmn_data.hpp"

struct Key_data
{
    Key_data() :
        key(-1),
        sdl_key(SDLK_UNKNOWN),
        is_shift_held(false),
        is_ctrl_held(false) {}

    Key_data(char key_, SDL_Keycode sdl_key_, bool is_shift_held_, bool is_ctrl_held_) :
        key(key_),
        sdl_key(sdl_key_),
        is_shift_held(is_shift_held_),
        is_ctrl_held(is_ctrl_held_) {}

    Key_data(char key_) :
        key(key_),
        sdl_key(SDLK_UNKNOWN),
        is_shift_held(false),
        is_ctrl_held(false) {}

    Key_data(SDL_Keycode sdl_key_) :
        key(-1),
        sdl_key(sdl_key_),
        is_shift_held(false),
        is_ctrl_held(false) {}

    char  key;
    SDL_Keycode sdl_key;
    bool is_shift_held, is_ctrl_held;
};

namespace input
{

void init();
void cleanup();

//Reads input until a valid map mode command is executed
void map_mode_input();

Key_data input(const bool IS_O_RETURN = true);

void clear_events();

void handle_map_mode_key_press(const Key_data& d);

} //Input

#endif
