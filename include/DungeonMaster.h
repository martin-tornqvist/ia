#ifndef DUNGEON_MASTER_H
#define DUNGEON_MASTER_H

#include <iostream>
#include <math.h>

#include "CmnTypes.h"
#include "AbilityValues.h"
#include "Colors.h"
#include "Converters.h"
#include "CreateCharacter.h"
#include "Utils.h"
#include "ActorMonster.h"

namespace  DungeonMaster
{

void init();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

int getCLvl();
int getXp();
TimeData getTimeStarted();

void onMonKilled(Actor& actor);
void onMonSpotted(Actor& actor);

void winGame();

void setTimeStartedToNow();

void playerLoseXpPercent(const int PERCENT);

int getMonTotXpWorth(const ActorDataT& d);

int getXpToNextLvl();

void playerGainXp(const int XP_GAINED);

} //DungeonMaster

#endif
