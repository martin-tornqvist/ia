/*-----------------------------------------
Class for jamming doors, chests(?), etc(?).
------------------------------------------*/

#ifndef JAM_WITH_SPIKE_H
#define JAM_WITH_SPIKE_H

#include "FeatureDoor.h"

class Engine;

class JamWithSpike
{
   public:
      JamWithSpike(Engine* engine) : eng(engine) {}
      ~JamWithSpike() {}

      void playerJam() const;

   private:
      Engine* eng;
};

#endif
