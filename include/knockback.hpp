#ifndef KNOCKBACK_HPP
#define KNOCKBACK_HPP

#include "rl_utils.hpp"

class Actor;

namespace knock_back
{

void try_knock_back(Actor& defender,
                    const P& attacked_from_pos,
                    const bool is_spike_gun,
                    const Verbosity verbosity = Verbosity::verbose,
                    const int paralyze_extra_turns = 0);

} // knock_back

#endif
