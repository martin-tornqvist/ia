#ifndef PLAYER_BON_HPP
#define PLAYER_BON_HPP

#include <string>
#include <vector>
#include <algorithm>

#include "ability_values.hpp"

#include <math.h>

struct ActorDataT;

enum class Trait
{
    // Common (except some traits can be blocked for certain backgrounds)
    adept_melee_fighter,
    expert_melee_fighter,
    adept_marksman,
    expert_marksman,
    cool_headed,
    courageous,
    dem_expert,
    dexterous,
    fearless,
    imperceptible,
    stealthy,
    mobile,
    lithe,
    observant,
    perceptive,
    vigilant,
    treasure_hunter,
    self_aware,
    healer,
    rapid_recoverer,
    survivalist,
    perseverant,
    strong_spirit,
    mighty_spirit,
    tough,
    rugged,
    strong_backed,
    undead_bane,
    elec_incl,

    // Unique for Ghoul
    ravenous,
    foul,
    toxic,
    indomitable_fury,

    // Unique for Occultist
    warlock,
    summoner,
    blood_sorc,
    seer,

    // Unique for Rogue
    vicious,

    // Unique for War veteran
    fast_shooter,
    steady_aimer,
    sharpshooter,

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

void pickable_bgs(std::vector<Bg>& bgs_out);

// Returns a list of all traits in the game, except traits which cannot be
// picked by the given background
void trait_list_for_bg(const Bg bg, std::vector<Trait>& traits_out);

void trait_prereqs(const Trait id,
                   const Bg bg,
                   std::vector<Trait>& traits_out,
                   Bg& bg_out);

bool is_prereqs_ok(const Trait id);

Bg bg();

std::string trait_title(const Trait id);
std::string trait_descr(const Trait id);

std::string bg_title(const Bg id);

// NOTE: The string vector returned is not formatted. Each line still needs to
//       be formatted by the caller. The reason for using a vector instead of a
//       string is to separate the text into paragraphs.
std::vector<std::string> bg_descr(const Bg id);

std::string all_picked_traits_titles_line();

void pick_trait(const Trait id);

void pick_bg(const Bg bg);

void set_all_traits_to_picked();

bool gets_undead_bane_bon(const ActorDataT& actor_data);

} // player_bon

#endif // PLAYER_BON_HPP
