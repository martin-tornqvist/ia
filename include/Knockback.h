#ifndef KNOCKBACK_H
#define KNOCKBACK_H

#include "CmnTypes.h"

class Actor;

namespace KnockBack
{

void tryKnockBack(Actor& defender, const Pos& attackedFromPos,
                  const bool IS_SPIKE_GUN,
                  const bool IS_MSG_ALLOWED = true);

} //KnockBack

#endif
