#include "AutoDescribeActor.h"

#include "Engine.h"

void AutoDescribeActor::addAutoDescriptionLines(
  Actor* const actor, string& line) const {

  const ActorData* const def = actor->getData();

  if(def->isUnique) {
    if(def->spawnMinDLVL < LAST_CAVERN_LEVEL) {
      line += " " + def->name_the + " is normally found beneath level " +
              getDwellingLevelStr(*def) + ". ";
    }
  } else {
    line += " They tend to dwell " + getNormalGroupSizeStr(*def) + ",";
    line += " and usually stay at depths beneath level " +
            getDwellingLevelStr(*def) + ".";
    line += " They move " + getSpeedStr(*def) + ". ";
  }
}

string AutoDescribeActor::getNormalGroupSizeStr(const ActorData& def) const {
  const MonsterGroupSize_t s = def.groupSize;

  return
    s == monsterGroupSize_alone ? "alone" :
    s == monsterGroupSize_few ? "in groups of a few" :
    s == monsterGroupSize_group ? "in groups" :
    s == monsterGroupSize_horde ? "in hordes" :
    "in swarms";
}

string AutoDescribeActor::getSpeedStr(const ActorData& def) const {
  switch(def.speed) {
    case actorSpeed_sluggish:   {return "at sluggish speed";} break;
    case actorSpeed_slow:       {return "slowly";}            break;
    case actorSpeed_normal:     {return "at normal speed";}   break;
    case actorSpeed_fast:       {return "fast";}              break;
    case actorSpeed_fastest:    {return "very fast";}       break;
  }
  return "";
}

string AutoDescribeActor::getDwellingLevelStr(const ActorData& def) const {
  return toString(max(1, def.spawnMinDLVL));
}

string AutoDescribeActor::getNrOfKillsStr(const ActorData& def) const {
  const int KILLS = def.nrOfKills;
  return KILLS == 0 ?
         "I do not recall having dispatched any of these before" :
         "I have killed " + toString(KILLS) + " of these before";
}

