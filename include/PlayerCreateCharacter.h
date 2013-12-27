#ifndef PLAYER_CHARACTER_HANDLER_H
#define PLAYER_CHARACTER_HANDLER_H

#include <string>

#include "CommonTypes.h"
#include "PlayerBonuses.h"

using namespace std;

class Engine;

//This class is responsible for letting the player create a character
//(bg, traits and name), and also for picking traits when gaining levels
class PlayerCreateCharacter {
public:
  PlayerCreateCharacter(Engine& engine) : eng(engine) {}

  void createCharacter() const;

  void pickNewTrait(const bool IS_CHARACTER_CREATION) const;

private:
  void drawPickTrait(const vector<Trait_t>& traitsColOne,
                     const vector<Trait_t>& traitsColTwo,
                     const MenuBrowser& browser,
                     const bool IS_CHARACTER_CREATION) const;

  void pickBg() const;
  void drawPickBg(const vector<Bg_t>& bgs,
                  const MenuBrowser& browser) const;

  Engine& eng;
};

//Helper class for PlayerCreateCharacter for input and rendering when the
//player enters a character name
class PlayerEnterName {
public:
private:
  friend class PlayerCreateCharacter;
  PlayerEnterName(Engine& engine) : eng(engine) {}
  void run(const int Y0_TITLE) const;
  void draw(const string& currentString, const int Y0_TITLE) const;
  void readKeys(string& currentString, bool& isDone, const int Y0_TITLE) const;
  Engine& eng;
};

#endif
