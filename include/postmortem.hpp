#ifndef POSTMORTEM_H
#define POSTMORTEM_H

#include <vector>
#include <string>

namespace postmortem
{

void init();

void save();

void load();

void add_history_event(const std::string msg);

void run(bool* const quit_game);

} //Postmortem

#endif
