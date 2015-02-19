#ifndef KNOCKBACK_H
#define KNOCKBACK_H

#include "cmn_types.hpp"

class Actor;

namespace Knock_back
{

void try_knock_back(Actor&        defender,
                  const Pos&    attacked_from_pos,
                  const bool    IS_SPIKE_GUN,
                  const bool    IS_MSG_ALLOWED = true);

} //Knock_back

#endif
