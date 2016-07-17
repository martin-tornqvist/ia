#ifndef INPUT_HPP
#define INPUT_HPP

#include <SDL.h>

struct KeyData
{
    KeyData() :
        key(-1),
        sdl_key(SDLK_UNKNOWN),
        is_shift_held(false),
        is_ctrl_held(false) {}

    KeyData(char key_, SDL_Keycode sdl_key_, bool is_shift_held_, bool is_ctrl_held_) :
        key(key_),
        sdl_key(sdl_key_),
        is_shift_held(is_shift_held_),
        is_ctrl_held(is_ctrl_held_) {}

    KeyData(char key_) :
        key(key_),
        sdl_key(SDLK_UNKNOWN),
        is_shift_held(false),
        is_ctrl_held(false) {}

    KeyData(SDL_Keycode sdl_key_) :
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

KeyData input(const bool is_o_return = true);

void clear_events();

void handle_map_mode_key_press(const KeyData& d);

} //Input

#endif
