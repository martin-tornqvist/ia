#ifndef SAVE_HANDLING_HPP
#define SAVE_HANDLING_HPP

#include <string>

namespace saving
{

void init();

void save_game();
void load_game();

bool is_save_available();

//Functions called by modules when saving and loading.
void put_str(const std::string str);
void put_int(const int v);
void put_bool(const bool v);

std::string get_str();
int         get_int();
bool        get_bool();

} //saving

#endif
