#ifndef SAVE_HANDLING
#define SAVE_HANDLING

#include <string>

namespace save_handling
{

void init();

void save_game();
void load_game();

bool is_save_available();

//Functions called by modules when saving and loading. The get functions will erase the value.
void put_str(const std::string str);
void put_int(const int V);
void put_bool(const bool V);

std::string get_str();
int         get_int();
bool        get_bool();

} //Save_handling

#endif
