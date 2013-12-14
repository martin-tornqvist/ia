#ifndef AUTO_DESCRIBE_ACTOR_H
#define AUTO_DESCRIBE_ACTOR_H

/*-----------------------------------------------------------
Purpose: Given an actor and a pointer to string, it generates
description text and adds them to the string
-------------------------------------------------------------*/

#include <vector>
#include <string>

#include "ActorData.h"

using namespace std;

class Engine;
class Actor;

class AutoDescribeActor {
public:
  AutoDescribeActor(Engine& engine) : eng(engine) {}
  ~AutoDescribeActor() {}

  void addAutoDescriptionLines(Actor* const actor, string& line) const;

private:
  string getNormalGroupSizeStr(const ActorData& def) const;
  string getDwellingLevelStr(const ActorData& def) const;
  string getNrOfKillsStr(const ActorData& def) const;
  string getSpeedStr(const ActorData& def) const;

  Engine& eng;
};

#endif
