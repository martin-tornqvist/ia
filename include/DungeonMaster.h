#ifndef DUNGEON_MASTER_H
#define DUNGEON_MASTER_H

#include <vector>
#include <string>

struct TimeData;
struct ActorDataT;
class Actor;

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
