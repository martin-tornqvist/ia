#ifndef EXPLOSION_HPP
#define EXPLOSION_HPP

#include <vector>

#include "colors.hpp"
#include "audio.hpp"
#include "global.hpp"

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
    const P& origin,
    const Expl_type expl_type,
    const Expl_src expl_src = Expl_src::misc,
    const Emit_expl_snd emit_expl_snd = Emit_expl_snd::yes,
    const int radi_change = 0,
    Prop* const prop = nullptr,
    const Clr* const clr_override = nullptr);

void run_smoke_explosion_at(const P& origin, const int radi_change = 0);

R explosion_area(const P& c, const int radi);

} //explosion

#endif
