#ifndef PLAYER_CHARACTER_HANDLER_H
#define PLAYER_CHARACTER_HANDLER_H

#include <string>

#include "CommonTypes.h"
#include "PlayerBonuses.h"

using namespace std;

class Engine;

const int Y0_CREATE_CHARACTER = 6;

//This class is responsible for letting the player create a character
//(background and name), and also for picking traits when gaining levels
class PlayerCreateCharacter {
public:
  PlayerCreateCharacter(Engine& engine) : eng(engine) {}

  void createCharacter() const;

  void pickTrait() const;

private:
  void drawPickTrait(const vector<Trait_t>& traitsColOne,
                     const vector<Trait_t>& traitsColTwo,
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
  void run(const Pos& pos) const;
  void draw(const string& currentString, const Pos& pos) const;
  void readKeys(string& currentString, bool& done, const Pos& pos) const;
  Engine& eng;
};

#endif
