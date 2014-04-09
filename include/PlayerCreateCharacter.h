#ifndef PLAYER_CHARACTER_HANDLER_H
#define PLAYER_CHARACTER_HANDLER_H

#include <string>

#include "CommonTypes.h"
#include "PlayerBon.h"

using namespace std;

class Engine;
class MenuBrowser;

class PlayerCreateCharacter {
public:
  PlayerCreateCharacter(Engine& engine) : eng(engine) {}

  void createCharacter() const;

  void pickNewTrait(const bool IS_CHARACTER_CREATION) const;

private:
  void drawPickTrait(const vector<Trait>& traitsColOne,
                     const vector<Trait>& traitsColTwo,
                     const MenuBrowser& browser,
                     const bool IS_CHARACTER_CREATION) const;

  void pickBg() const;
  void drawPickBg(const vector<Bg>& bgs,
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
  void run() const;
  void draw(const string& currentString) const;
  void readKeys(string& currentString, bool& isDone) const;
  Engine& eng;
};

#endif
