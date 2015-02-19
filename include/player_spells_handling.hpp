#ifndef PLAYER_SPELLS_HANDLING_H
#define PLAYER_SPELLS_HANDLING_H

#include <vector>

#include "menu_browser.h"
#include "spells.h"

class Spell;

namespace Player_spells_handling
{

void init();
void cleanup();

void store_to_save_lines(std::vector<std::string>& lines);
void setup_from_save_lines(std::vector<std::string>& lines);

void player_select_spell_to_cast();

void try_cast_prev_spell();

void learn_spell_if_not_known(Spell* const spell);
void learn_spell_if_not_known(const Spell_id id);

bool is_spell_learned(const Spell_id id);

} //Player_spells_handling

#endif
