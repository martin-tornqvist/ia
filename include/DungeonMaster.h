#ifndef DUNGEON_MASTER_H
#define DUNGEON_MASTER_H

#include <iostream>
#include <math.h>

#include "CmnTypes.h"
#include "AbilityValues.h"
#include "Colors.h"
#include "Converters.h"
#include "PlayerCreateCharacter.h"
#include "Utils.h"
#include "ActorMonster.h"

class DungeonMaster {
public:
  DungeonMaster() : clvl(1), xp(0) {initXpArray();}
  ~DungeonMaster() {}

  void onMonsterKilled(Actor& actor);
  void onMonsterSpotted(Actor& actor);

  void storeToSaveLines(vector<string>& lines) const;
  void setupFromSaveLines(vector<string>& lines);

  void winGame();

  TimeData getTimeStarted() const {return timeStarted;}
  void setTimeStartedToNow();

  void playerLoseXpPercent(const int PERCENT);

  int getMonsterTotXpWorth(const ActorDataT& d) const;

  inline int getCLvl()  const {return clvl;}
  inline int getXp()    const {return xp;}

  int getXpToNextLvl() const;

  void playerGainXp(int XP_GAINED);

private:
  void playerGainLvl();

  void initXpArray();

  int xpForLvl[PLAYER_MAX_CLVL + 1];

  int clvl, xp;

  TimeData timeStarted;
};

#endif
