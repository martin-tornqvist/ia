#ifndef EXPLOSION_HPP
#define EXPLOSION_HPP

#include <vector>

#include "colors.hpp"
#include "audio.hpp"
#include "global.hpp"

class Prop;

enum class ExplType
{
    expl,
    apply_prop
};

enum class ExplSrc
{
    misc,
    player_use_moltv_intended
};

enum class EmitExplSnd
{
    no,
    yes
};

enum class ExplExclCenter
{
    no,
    yes
};

enum class ExplIsGas
{
    no,
    yes
};

namespace explosion
{

//
// NOTE: If "emit_expl_sound" is set to "no", this typically means that the
//       caller should emit a custom sound before running the explosion (e.g.
//       molotov explosion sound).
//
void run(const P& origin,
         const ExplType expl_type,
         const ExplSrc expl_src = ExplSrc::misc,
         const EmitExplSnd emit_expl_snd = EmitExplSnd::yes,
         const int radi_change = 0,
         const ExplExclCenter exclude_center = ExplExclCenter::no,
         std::vector<Prop*> properties_applied = {},
         const Clr* const clr_override = nullptr,
         const ExplIsGas is_gas = ExplIsGas::no);

void run_smoke_explosion_at(const P& origin,
                            const int radi_change = 0);

R explosion_area(const P& c,
                 const int radi);

} // explosion

#endif
