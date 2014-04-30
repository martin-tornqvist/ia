#ifndef AUTO_DESCRIBE_ACTOR_H
#define AUTO_DESCRIBE_ACTOR_H

/*-----------------------------------------------------------
Purpose: Given an actor and a pointer to string, it generates
description text and adds them to the string
-------------------------------------------------------------*/

#include <vector>
#include <string>

#include "ActorData.h"

class Actor;

class AutoDescribeActor {
public:
  AutoDescribeActor() {}
  ~AutoDescribeActor() {}

  void addAutoDescriptionLines(Actor* const actor, std::string& line) const;

private:
  std::string getNormalGroupSizeStr(const ActorDataT& def) const;
  std::string getDwellingLevelStr(const ActorDataT& def) const;
  std::string getSpeedStr(const ActorDataT& def) const;
};

#endif
