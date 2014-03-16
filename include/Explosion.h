#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <vector>

#include "CommonData.h"
#include "CommonTypes.h"
#include "Colors.h"
#include "AudioIds.h"

class Engine;
class Prop;

enum class ExplType {expl, applyProp};
enum class ExplSrc  {misc, playerUseMoltvIntended};

namespace Explosion {

void runExplosionAt(
  const Pos& origin,
  Engine& eng,
  const ExplType  explType,
  const ExplSrc   explSrc             = ExplSrc::misc,
  const int RADI_CHANGE               = 0,
  const SfxId sfx                     = SfxId::explosion,
  Prop* const prop                    = NULL,
  const SDL_Color* const clrOverride  = NULL);

void runSmokeExplosionAt(const Pos& origin, Engine& eng);

} //Explosion

#endif
