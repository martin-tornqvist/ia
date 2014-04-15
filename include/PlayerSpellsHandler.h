#ifndef PLAYER_SPELLS_HANDLER_H
#define PLAYER_SPELLS_HANDLER_H

#include <vector>

#include "MenuBrowser.h"
#include "Spells.h"

using namespace std;

class Engine;
class Spell;

class PlayerSpellsHandler {
public:
  PlayerSpellsHandler(Engine& engine) : prevSpellCast_(NULL), eng(engine) {
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

  void addSaveLines(vector<string>& lines) const;

  void setParamsFromSaveLines(vector<string>& lines);
private:
  void draw(MenuBrowser& browser);

  void tryCast(const Spell* const spell);

  vector<Spell*>  knownSpells_;
  const Spell*    prevSpellCast_;

  Engine& eng;
};

#endif
