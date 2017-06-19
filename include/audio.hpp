#ifndef AUDIO_HPP
#define AUDIO_HPP

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
    hit_small, hit_medium, hit_hard, hit_corpse_break,
    miss_light, miss_medium, miss_heavy,
    hit_sharp,
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
    door_open, door_close, door_bang, door_break,
    tomb_open,
    fountain_drink,
    boss_voice1, boss_voice2,
    chains,
    glop,
    lever_pull,

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
    AMB_START,
    amb001, amb002, amb003, amb004, amb005, amb006, amb007, amb008, amb009,
    amb010, amb011, amb012, amb013, amb014, amb015, amb016, amb017, amb018,
    amb019, amb020, amb021, amb022, amb023, amb024, amb025, amb026, amb027,
    amb028, amb029, amb030, amb031, amb032, amb033, amb034, amb035, amb036,
    amb037, amb038, amb039, amb040, amb041, amb042, amb043, amb044, amb045,
    amb046, amb047, amb048,

    END
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
