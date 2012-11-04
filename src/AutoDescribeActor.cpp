#include "AutoDescribeActor.h"

#include "Engine.h"

void AutoDescribeActor::addAutoDescriptionLines(Actor* const actor, string& line) const {
  const ActorDefinition* const def = actor->getDef();

  if(def->unique == false) {
    line += " They tend to dwell " + getNormalGroupSizeStr(*def);
    line += " beneath level " + getDwellingLevelStr(*def) + ".";
    line += " They move " + getSpeedStr(*def) + ". ";
  } else {
    if(def->spawnStandardMinLevel < LAST_CAVERN_LEVEL) {
      line += " " + def->name_the + " is normally found beneath level " + getDwellingLevelStr(*def) + ". ";
    }
  }
}

string AutoDescribeActor::getNormalGroupSizeStr(const ActorDefinition& def) const {
  const int CHANCE = def.chanceToSpawnExtra;

  return CHANCE >= 100 ? "in swarms" : CHANCE >= 95 ? "in large groups" : CHANCE >= 80 ? "in small or large groups"
         : CHANCE >= 70 ? "in small groups" : CHANCE >= 30 ? "alone or in small groups" : "alone";
}

string AutoDescribeActor::getSpeedStr(const ActorDefinition& def) const {
  switch(def.speed) {
  case actorSpeed_sluggish: {
    return "at sluggish speed";
  }
  break;
  case actorSpeed_slow: {
    return "slowly";
  }
  break;
  case actorSpeed_normal: {
    return "at normal speed";
  }
  break;
  case actorSpeed_fast: {
    return "fast";
  }
  break;
  case actorSpeed_fastest: {
    return "very fast";
  }
  break;
  }
  return "";
}

string AutoDescribeActor::getDwellingLevelStr(const ActorDefinition& def) const {
  return intToString(max(1, def.spawnStandardMinLevel));
}

string AutoDescribeActor::getNrOfKillsStr(const ActorDefinition& def) const {
  const int KILLS = def.nrOfKills;
  return KILLS == 0 ? "I do not recall having dispatched any of these before" : "I have killed " + intToString(KILLS) + " of these before";
}

