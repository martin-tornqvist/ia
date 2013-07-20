#ifndef DUNGEON_MASTER_H
#define DUNGEON_MASTER_H

#include <iostream>
#include <math.h>

#include "CommonTypes.h"
#include "AbilityValues.h"
#include "Colors.h"
#include "Converters.h"
#include "PlayerCreateCharacter.h"
#include "BasicUtils.h"

class Engine;

class DungeonMaster
{
public:
  DungeonMaster(Engine* engine) : eng(engine) {}
  ~DungeonMaster() {}

  void monsterKilled(Actor* monster);

  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(vector<string>& lines);

  void winGame();

  TimeData getTimeStarted() const {return timeStarted;}
  void setTimeStartedToNow();

private:
  TimeData timeStarted;

  Engine* const eng;
};

#endif
