#ifndef PLAYER_ALLOC_BONUS_H
#define PLAYER_ALLOC_BONUS_H

#include "PlayerBonuses.h"
#include "MenuBrowser.h"

class Engine;

class Skills;

class PlayerAllocBonus {
public:
  PlayerAllocBonus(Engine* engine) : eng(engine) {
  }

  void run();

private:
  void draw(const vector<PlayerBonuses_t>& bonusesToChooseFrom, const MenuBrowser& browser) const;

  Engine* eng;
};

#endif
