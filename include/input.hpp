#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>

#include "cmn_data.h"

struct Key_data
{
    Key_data() :
        key         (-1),
        sdl_key      (SDLK_LAST),
        is_shift_held (false),
        is_ctrl_held  (false) {}

    Key_data(char key_, SDLKey sdl_key_, bool is_shift_held_,  bool is_ctrl_held_) :
        key         (key_),
        sdl_key      (sdl_key_),
        is_shift_held (is_shift_held_),
        is_ctrl_held  (is_ctrl_held_) {}

    Key_data(char key_) :
        key         (key_),
        sdl_key      (SDLK_LAST),
        is_shift_held (false),
        is_ctrl_held  (false) {}

    Key_data(SDLKey sdl_key_) :
        key         (-1),
        sdl_key      (sdl_key_),
        is_shift_held (false),
        is_ctrl_held  (false) {}

    char    key;
    SDLKey  sdl_key;
    bool    is_shift_held, is_ctrl_held;
};

namespace Input
{

void init();
void cleanup();

//Reads input until a valid map mode command is executed
void map_mode_input();

Key_data get_input(const bool IS_O_RETURN = true);

void clear_events();

void set_key_repeat_delays();

void handle_map_mode_key_press(const Key_data& d);

bool is_key_held(const SDLKey key);

} //Input

#endif
