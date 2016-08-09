#ifndef PLAYER_SPELLS_HPP
#define PLAYER_SPELLS_HPP

#include <vector>

#include "spells.hpp"

class Spell;

namespace player_spells
{

void init();
void cleanup();

void save();
void load();

void player_select_spell_to_cast();

void learn_spell(const SpellId id, const Verbosity verbosity);

void incr_spell_skill(const SpellId id, const Verbosity verbosity);

int spell_skill_pct(const SpellId id);

void set_spell_skill_pct(const SpellId id, const int val);

bool is_spell_learned(const SpellId id);

} //player_spells

#endif //PLAYER_SPELLS_HPP
