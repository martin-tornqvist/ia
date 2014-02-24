#include "AutoDescribeActor.h"

#include "Engine.h"
#include "ActorData.h"
#include "Actor.h"

void AutoDescribeActor::addAutoDescriptionLines(
  Actor* const actor, string& line) const {

  const ActorData& def = actor->getData();

  if(def.isUnique) {
    if(def.spawnMinDLVL < LAST_CAVERN_LEVEL) {
      line += " " + def.name_the + " is normally found beneath level " +
              getDwellingLevelStr(def) + ". ";
    }
  } else {
    line += " They tend to dwell " + getNormalGroupSizeStr(def) + ",";
    line += " and usually stay at depths beneath level " +
            getDwellingLevelStr(def) + ".";
    line += " They appear to move " + getSpeedStr(def) + ". ";
  }
}

string AutoDescribeActor::getNormalGroupSizeStr(const ActorData& def) const {
  const MonsterGroupSize s = def.groupSize;

  return
    s == monsterGroupSize_alone ? "alone" :
    s == monsterGroupSizeFew    ? "in small groups" :
    s == monsterGroupSizeGroup  ? "in groups" :
    s == monsterGroupSizeHorde  ? "in hordes" :
    "in swarms";
}

string AutoDescribeActor::getSpeedStr(const ActorData& def) const {
  switch(def.speed) {
    case ActorSpeed::sluggish:   {return "at sluggishly";}
    case ActorSpeed::slow:       {return "slowly";}
    case ActorSpeed::normal:     {return "at normal speed";}
    case ActorSpeed::fast:       {return "fast";}
    case ActorSpeed::fastest:    {return "very fast";}
    case ActorSpeed::endOfActorSpeed: {} break;
  }
  return "";
}

string AutoDescribeActor::getDwellingLevelStr(const ActorData& def) const {
  return toString(max(1, def.spawnMinDLVL));
}

