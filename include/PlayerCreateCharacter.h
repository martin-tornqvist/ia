#ifndef PLAYER_CHARACTER_HANDLER_H
#define PLAYER_CHARACTER_HANDLER_H

#include <string>

#include "CmnTypes.h"
#include "PlayerBon.h"

using namespace std;

class MenuBrowser;

class PlayerCreateCharacter {
public:
  PlayerCreateCharacter() {}

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
};

//Helper class for PlayerCreateCharacter for input and rendering when the
//player enters a character name
class PlayerEnterName {
public:
private:
  friend class PlayerCreateCharacter;
  PlayerEnterName() {}
  void run() const;
  void draw(const string& currentString) const;
  void readKeys(string& currentString, bool& isDone) const;
};

#endif
