#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <vector>

#include "cmn_data.hpp"
#include "cmn_types.hpp"
#include "colors.hpp"
#include "audio.hpp"

class Prop;

enum class Expl_type
{
    expl,
    apply_prop
};

enum class Expl_src
{
    misc,
    player_use_moltv_intended
};

enum class Emit_expl_snd
{
    no,
    yes
};

namespace explosion
{

//NOTE: If "emit_expl_sound" is set to "no", this typically means that the caller should
//      emit a custom sound before running the explosion (e.g. molotov explosion sound).
void run(
    const Pos& origin,
    const Expl_type expl_type,
    const Expl_src expl_src = Expl_src::misc,
    const Emit_expl_snd emit_expl_snd = Emit_expl_snd::yes,
    const int RADI_CHANGE = 0,
    Prop* const prop = nullptr,
    const Clr* const clr_override = nullptr);

void run_smoke_explosion_at(const Pos& origin);

} //Explosion

#endif
