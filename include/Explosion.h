#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <vector>

#include "CommonData.h"
#include "CommonTypes.h"
#include "Colors.h"
#include "AudioIds.h"

class Engine;
class Prop;

namespace Explosion {

void runExplosionAt(
  const Pos& origin,
  Engine& eng,
  const int RADI_CHANGE               = 0,
  const SfxId sfx                     = SfxId::explosion,
  const bool SHOULD_DO_EXPLOSION_DMG  = true,
  Prop* const prop                    = NULL,
  const bool SHOULD_OVERRIDE_CLR      = false,
  const SDL_Color& clrOverride        = clrBlack);

void runSmokeExplosionAt(const Pos& origin, Engine& eng);

} //Explosion

#endif
