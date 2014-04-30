#ifndef PLAYER_SPELLS_HANDLER_H
#define PLAYER_SPELLS_HANDLER_H

#include <vector>

#include "MenuBrowser.h"
#include "Spells.h"

class Spell;

class PlayerSpellsHandler {
public:
  PlayerSpellsHandler() : prevSpellCast_(NULL) {
    knownSpells_.resize(0);
  }
  ~PlayerSpellsHandler();

  void playerSelectSpellToCast();

  void tryCastPrevSpell();

  inline unsigned int getNrSpells() const {return knownSpells_.size();}

  inline Spell* getSpellAt(const unsigned int ELEMENT) const {
    return knownSpells_.at(ELEMENT);
  }

  void learnSpellIfNotKnown(Spell* const spell);
  void learnSpellIfNotKnown(const SpellId id);

  bool isSpellLearned(const SpellId id);

  void storeToSaveLines(std::vector<std::string>& lines) const;

  void setupFromSaveLines(std::vector<std::string>& lines);
private:
  void draw(MenuBrowser& browser);

  void tryCast(const Spell* const spell);

  std::vector<Spell*>  knownSpells_;
  const Spell*    prevSpellCast_;


};

#endif
