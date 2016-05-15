#ifndef INIT_HPP
#define INIT_HPP

namespace init
{

extern bool is_cheat_vision_enabled;
extern bool quit_to_main_menu;

void init_io();
void cleanup_io();

void init_game();
void cleanup_game();

void init_session();
void cleanup_session();

} //init

#endif
