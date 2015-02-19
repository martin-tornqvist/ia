#ifndef AUDIO_H
#define AUDIO_H

#include "cmn_types.hpp"

enum class Sfx_id
{
    //Monster sounds
    dog_snarl,
    wolf_howl,
    zombie_growl,
    ghoul_growl,
    ooze_gurgle,
    flapping_wings,
    ape,

    //Weapon and attack sounds
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

    //Environment action sounds
    metal_clank,
    ricochet,
    explosion,
    explosion_molotov,
    door_open, door_close, door_bang, door_break,
    tomb_open,
    fountain_drink,
    boss_voice1, boss_voice2,

    //User interface sounds
    backpack,
    pickup,
    electric_lantern,
    potion_quaff,
    spell_generic,
    insanity_rise,
    glop,
    death,

    //Ambient sounds
    AMB_START,
    amb001, amb002, amb003, amb004, amb005, amb006, amb007, amb008, amb009, amb010,
    amb011, amb012, amb013, amb014, amb015, amb016, amb017, amb018, amb019, amb020,
    amb021, amb022, amb023, amb024, amb025, amb026, amb027, amb028, amb029, amb030,
    amb031, amb032, amb033, amb034, amb035, amb036, amb037,
    AMB_END,

    mus_cthulhiana_Madness,

    END
};

namespace Audio
{

void init();
void cleanup();

//Returns channel playing on
int play(const Sfx_id sfx, const int VOL_PERCENT_TOT = 100,
         const int VOL_PERCENT_L = 50);

void play(const Sfx_id sfx, const Dir dir, const int DISTANCE_PERCENT);

void try_play_amb(const int ONE_IN_N_CHANCE_TO_PLAY);

void fade_out_channel(const int CHANNEL_NR);

}

#endif
