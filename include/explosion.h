#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <vector>

#include "Cmn_data.h"
#include "Cmn_types.h"
#include "Colors.h"
#include "Audio.h"

class Prop;

enum class Expl_type {expl, apply_prop};
enum class Expl_src  {misc, player_use_moltv_intended};

namespace Explosion
{

void run_explosion_at(
    const Pos&      origin,
    const Expl_type  expl_type,
    const Expl_src   expl_src         = Expl_src::misc,
    const int       RADI_CHANGE     = 0,
    const Sfx_id     sfx             = Sfx_id::explosion,
    Prop* const     prop            = nullptr,
    const Clr* const clr_override    = nullptr);

void run_smoke_explosion_at(const Pos& origin);

} //Explosion

#endif
