/*-----------------------------------------
Class for bashing doors, chests(?), etc(?).
The bashed object handles success chance,
setting of traps, and various failure modes.
This class works only as a "bridge" between
basher and bashee.
------------------------------------------*/

#ifndef BASH_H
#define BASH_H

#include "FeatureDoor.h"

class Engine;

class Bash
{
   public:
      Bash(Engine* engine) : eng(engine) {}
      ~Bash() {}

      void playerBash() const;

      //void monsterBashDirection(Actor* const actorBashing, const coord dir) const;

   private:
      Engine* eng;
};

#endif
