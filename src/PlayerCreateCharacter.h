#ifndef PLAYER_CHARACTER_HANDLER_H
#define PLAYER_CHARACTER_HANDLER_H

#include <string>

#include "CommonTypes.h"
#include "PlayerBonuses.h"

using namespace std;

class Engine;

const int Y0_CREATE_CHARACTER = 6;

enum CharGenStep_t {
  CharGenStep_traits,
  CharGenStep_skills,
  CharGenStep_name
};

class PlayerCreateCharacter {
public:
  PlayerCreateCharacter(Engine* engine) : eng(engine) {}

  void run();

private:
  void draw(const vector<PlayerBon_t>& bonsTraits,
            const vector<PlayerBon_t>& bonsSkills,
            const MenuBrowser& browser,
            const CharGenStep_t step) const;

  Engine* eng;
};

class PlayerEnterName {
public:
private:
  friend class PlayerCreateCharacter;
  PlayerEnterName(Engine* engine) : eng(engine) {}
  void run(const Pos& pos);
  void draw(const string& currentString, const Pos& pos);
  void readKeys(string& currentString, bool& done, const Pos& pos);
  Engine* eng;
};

#endif
