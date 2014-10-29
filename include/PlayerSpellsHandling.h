#ifndef PLAYER_SPELLS_HANDLING_H
#define PLAYER_SPELLS_HANDLING_H

#include <vector>

#include "MenuBrowser.h"
#include "Spells.h"

class Spell;

namespace PlayerSpellsHandling {

void init();
void cleanup();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

void playerSelectSpellToCast();

void tryCastPrevSpell();

void learnSpellIfNotKnown(Spell* const spell);
void learnSpellIfNotKnown(const SpellId id);

bool isSpellLearned(const SpellId id);

} //PlayerSpellsHandling

#endif
