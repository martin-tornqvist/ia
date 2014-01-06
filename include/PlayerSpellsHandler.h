#ifndef PLAYER_SPELLS_HANDLER_H
#define PLAYER_SPELLS_HANDLER_H

/*
 * Responsible for providing a user interface for selecting spells to
 * cast, and for storing the player's spells
 */

#include <vector>

#include "MenuBrowser.h"
#include "Spells.h"

using namespace std;

class Engine;
class Spell;

class PlayerSpellsHandler {
public:
  PlayerSpellsHandler(Engine& engine) : eng(engine) {}
  ~PlayerSpellsHandler();

  void run();

  inline unsigned int getNrSpells() const {return knownSpells.size();}

  inline Spell* getSpellAt(const unsigned int ELEMENT) const {
    return knownSpells.at(ELEMENT);
  }

  void learnSpellIfNotKnown(Spell* const spell);
  void learnSpellIfNotKnown(const Spell_t id);

  bool isSpellLearned(const Spell_t id);

  void addSaveLines(vector<string>& lines) const;

  void setParamsFromSaveLines(vector<string>& lines);
private:
  void draw(MenuBrowser& browser);

  vector<Spell*> knownSpells;

  Engine& eng;
};

#endif
