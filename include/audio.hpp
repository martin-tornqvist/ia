#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <unordered_map>

#include "rl_utils.hpp"

enum class SfxId
{
    // Monster sounds
    dog_snarl,
    wolf_howl,
    hiss,
    zombie_growl,
    ghoul_growl,
    ooze_gurgle,
    flapping_wings,
    ape,

    // Weapon and attack sounds
    hit_small,
    hit_medium,
    hit_hard,
    hit_sharp,
    hit_corpse_break,
    miss_light,
    miss_medium,
    miss_heavy,
    pistol_fire,
    pistol_reload,
    shotgun_sawed_off_fire,
    shotgun_pump_fire,
    shotgun_reload,
    machine_gun_fire,
    machine_gun_reload,
    mi_go_gun_fire,
    spike_gun,
    bite,

    // Environment action sounds
    metal_clank,
    ricochet,
    explosion,
    explosion_molotov,
    gas,
    door_open,
    door_close,
    door_bang,
    door_break,
    tomb_open,
    fountain_drink,
    boss_voice1,
    boss_voice2,
    chains,
    glop,
    lever_pull,
    monolith,

    // User interface sounds
    backpack,
    pickup,
    lantern,
    potion_quaff,
    spell_generic,
    spell_shield_break,
    insanity_rise,
    death,
    menu_browse,
    menu_select,

    // Ambient sounds
    // TODO: This is ugly, there is no reason to enumerate the ambient sounds,
    // try to get rid of them (push the ambient audio chunks to a separate
    // vector instead of a fixed size array)
    AMB_START,
    amb001, amb002, amb003, amb004, amb005, amb006, amb007, amb008, amb009,
    amb010, amb011, amb012, amb013, amb014, amb015, amb016, amb017, amb018,
    amb019, amb020, amb021, amb022, amb023, amb024, amb025, amb026, amb027,
    amb028, amb029, amb030, amb031, amb032, amb033, amb034, amb035, amb036,
    amb037, amb038, amb039, amb040, amb041, amb042, amb043, amb044, amb045,
    amb046, amb047, amb048, amb049, amb050,

    END
};

const std::unordered_map<std::string, SfxId> str_to_sfx_id_map = {
    {"dog_snarl", SfxId::dog_snarl},
    {"wolf_howl", SfxId::wolf_howl},
    {"hiss", SfxId::hiss},
    {"zombie_growl", SfxId::zombie_growl},
    {"ghoul_growl", SfxId::ghoul_growl},
    {"ooze_gurgle", SfxId::ooze_gurgle},
    {"flapping_wings", SfxId::flapping_wings},
    {"ape", SfxId::ape},
    {"hit_small", SfxId::hit_small},
    {"hit_medium", SfxId::hit_medium},
    {"hit_hard", SfxId::hit_hard},
    {"hit_sharp", SfxId::hit_sharp},
    {"hit_corpse_break", SfxId::hit_corpse_break},
    {"miss_light", SfxId::miss_light},
    {"miss_medium", SfxId::miss_medium},
    {"miss_heavy", SfxId::miss_heavy},
    {"pistol_fire", SfxId::pistol_fire},
    {"pistol_reload", SfxId::pistol_reload},
    {"shotgun_sawed_off_fire", SfxId::shotgun_sawed_off_fire},
    {"shotgun_pump_fire", SfxId::shotgun_pump_fire},
    {"shotgun_reload", SfxId::shotgun_reload},
    {"machine_gun_fire", SfxId::machine_gun_fire},
    {"machine_gun_reload", SfxId::machine_gun_reload},
    {"mi_go_gun_fire", SfxId::mi_go_gun_fire},
    {"spike_gun", SfxId::spike_gun},
    {"bite", SfxId::bite},
    {"metal_clank", SfxId::metal_clank},
    {"ricochet", SfxId::ricochet},
    {"explosion", SfxId::explosion},
    {"explosion_molotov", SfxId::explosion_molotov},
    {"gas", SfxId::gas},
    {"door_open", SfxId::door_open},
    {"door_close", SfxId::door_close},
    {"door_bang", SfxId::door_bang},
    {"door_break", SfxId::door_break},
    {"tomb_open", SfxId::tomb_open},
    {"fountain_drink", SfxId::fountain_drink},
    {"boss_voice1", SfxId::boss_voice1},
    {"boss_voice2", SfxId::boss_voice2},
    {"chains", SfxId::chains},
    {"glop", SfxId::glop},
    {"lever_pull", SfxId::lever_pull},
    {"monolith", SfxId::monolith},
    {"backpack", SfxId::backpack},
    {"pickup", SfxId::pickup},
    {"lantern", SfxId::lantern},
    {"potion_quaff", SfxId::potion_quaff},
    {"spell_generic", SfxId::spell_generic},
    {"spell_shield_break", SfxId::spell_shield_break},
    {"insanity_rise", SfxId::insanity_rise},
    {"death", SfxId::death},
    {"menu_browse", SfxId::menu_browse},
    {"menu_select", SfxId::menu_select},
    {"", SfxId::END}
};

const std::unordered_map<SfxId, std::string> sfx_id_to_str_map = {
    {SfxId::dog_snarl, "dog_snarl"},
    {SfxId::wolf_howl, "wolf_howl"},
    {SfxId::hiss, "hiss"},
    {SfxId::zombie_growl, "zombie_growl"},
    {SfxId::ghoul_growl, "ghoul_growl"},
    {SfxId::ooze_gurgle, "ooze_gurgle"},
    {SfxId::flapping_wings, "flapping_wings"},
    {SfxId::ape, "ape"},
    {SfxId::hit_small, "hit_small"},
    {SfxId::hit_medium, "hit_medium"},
    {SfxId::hit_hard, "hit_hard"},
    {SfxId::hit_sharp, "hit_sharp"},
    {SfxId::hit_corpse_break, "hit_corpse_break"},
    {SfxId::miss_light, "miss_light"},
    {SfxId::miss_medium, "miss_medium"},
    {SfxId::miss_heavy, "miss_heavy"},
    {SfxId::pistol_fire, "pistol_fire"},
    {SfxId::pistol_reload, "pistol_reload"},
    {SfxId::shotgun_sawed_off_fire, "shotgun_sawed_off_fire"},
    {SfxId::shotgun_pump_fire, "shotgun_pump_fire"},
    {SfxId::shotgun_reload, "shotgun_reload"},
    {SfxId::machine_gun_fire, "machine_gun_fire"},
    {SfxId::machine_gun_reload, "machine_gun_reload"},
    {SfxId::mi_go_gun_fire, "mi_go_gun_fire"},
    {SfxId::spike_gun, "spike_gun"},
    {SfxId::bite, "bite"},
    {SfxId::metal_clank, "metal_clank"},
    {SfxId::ricochet, "ricochet"},
    {SfxId::explosion, "explosion"},
    {SfxId::explosion_molotov, "explosion_molotov"},
    {SfxId::gas, "gas"},
    {SfxId::door_open, "door_open"},
    {SfxId::door_close, "door_close"},
    {SfxId::door_bang, "door_bang"},
    {SfxId::door_break, "door_break"},
    {SfxId::tomb_open, "tomb_open"},
    {SfxId::fountain_drink, "fountain_drink"},
    {SfxId::boss_voice1, "boss_voice1"},
    {SfxId::boss_voice2, "boss_voice2"},
    {SfxId::chains, "chains"},
    {SfxId::glop, "glop"},
    {SfxId::lever_pull, "lever_pull"},
    {SfxId::monolith, "monolith"},
    {SfxId::backpack, "backpack"},
    {SfxId::pickup, "pickup"},
    {SfxId::lantern, "lantern"},
    {SfxId::potion_quaff, "potion_quaff"},
    {SfxId::spell_generic, "spell_generic"},
    {SfxId::spell_shield_break, "spell_shield_break"},
    {SfxId::insanity_rise, "insanity_rise"},
    {SfxId::death, "death"},
    {SfxId::menu_browse, "menu_browse"},
    {SfxId::menu_select, "menu_select"},
    {SfxId::END, ""},
};

enum class MusId
{
    cthulhiana_madness,
    END
};

namespace audio
{

void init();
void cleanup();

void play(const SfxId sfx,
          const int vol_percent_tot = 100,
          const int vol_percent_l = 50);

void play(const SfxId sfx,
          const Dir dir,
          const int distance_percent);

void try_play_amb(const int one_in_n_chance_to_play);

// Plays music if not already playing any music
void play_music(const MusId sfx);

void fade_out_music();

}

#endif // AUDIO_HPP
