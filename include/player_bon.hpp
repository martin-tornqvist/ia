#ifndef PLAYER_BON_HPP
#define PLAYER_BON_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <math.h>

#include "global.hpp"

struct ActorData;

enum class Trait
{
        // Common (except some traits can be blocked for certain backgrounds)
        melee_fighter,
        marksman,
        cool_headed,
        dexterous,
        fearless,
        stealthy,
        silent,
        vigilant,
        treasure_hunter,
        self_aware,
        healer,
        rapid_recoverer, // TODO: Should probably be a Rogue ability instead
        survivalist,
        strong_spirit,
        tough,
        thick_skinned,
        resistant,
        strong_backed,
        undead_bane,
        elec_incl,

        // Unique for Ghoul
        ravenous,
        foul,
        toxic,
        indomitable_fury,

        // Unique for Occultist
        lesser_invoc,
        greater_invoc,
        lesser_summoning,
        greater_summoning,
        lesser_clairv,
        greater_clairv,
        lesser_ench,
        greater_ench,
        lesser_alter,
        greater_alter,
        absorb,

        // Unique for Rogue
        vicious,

        // Unique for War veteran
        fast_shooter,
        steady_aimer,

        END
};

enum class Bg
{
        ghoul,
        occultist,
        rogue,
        war_vet,
        END
};

namespace player_bon
{

extern bool traits[(size_t)Trait::END];

void init();

void save();

void load();

std::vector<Bg> pickable_bgs();

void unpicked_traits_for_bg(const Bg bg,
                            std::vector<Trait>& traits_can_be_picked_out,
                            std::vector<Trait>& traits_prereqs_not_met_out);

void trait_prereqs(const Trait id,
                   const Bg bg,
                   std::vector<Trait>& traits_out,
                   Bg& bg_out,
                   int& clvl_out);

Bg bg();

bool has_trait(const Trait id);

std::string trait_title(const Trait id);
std::string trait_descr(const Trait id);

std::string bg_title(const Bg id);

// NOTE: The string vector returned is not formatted. Each line still needs to
// be formatted by the caller. The reason for using a vector instead of a string
// is to separate the text into paragraphs.
std::vector<ColoredString> bg_descr(const Bg id);

std::string all_picked_traits_titles_line();

void pick_trait(const Trait id);

void pick_bg(const Bg bg);

void set_all_traits_to_picked();

bool gets_undead_bane_bon(const ActorData& actor_data);

} // player_bon

#endif // PLAYER_BON_HPP
