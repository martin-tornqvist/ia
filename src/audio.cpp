#include "audio.hpp"

#include <time.h>

#include <SDL_mixer.h>

#include "init.hpp"
#include "map.hpp"
#include "render.hpp"
#include "utils.hpp"

using namespace std;

namespace audio
{

namespace
{

vector<Mix_Chunk*> audio_chunks;

int cur_channel_     = 0;
int time_at_last_amb_  = -1;

void load_audio_file(const Sfx_id sfx, const string& filename)
{
    Render::clear_screen();

    const string file_rel_path  = "audio/" + filename;

    Render::draw_text("Loading " + file_rel_path + "...", Panel::screen, Pos(0, 0),
                     clr_white);

    Render::update_screen();

    audio_chunks[int(sfx)] = Mix_Load_wAV((file_rel_path).data());

    if (!audio_chunks[int(sfx)])
    {
        TRACE << "Problem loading audio file with name: " << filename       << endl
              << "Mix_Get_error(): "                       << Mix_Get_error() << endl;
        assert(false);
    }
}

int get_next_channel(const int FROM)
{
    assert(FROM >= 0 && FROM < AUDIO_ALLOCATED_CHANNELS);

    int ret = FROM + 1;

    if (ret == AUDIO_ALLOCATED_CHANNELS)
    {
        ret = 0;
    }

    return ret;
}

int get_free_channel(const int FROM)
{
    assert(FROM >= 0 && FROM < AUDIO_ALLOCATED_CHANNELS);

    int ret = FROM;

    for (int i = 0; i < AUDIO_ALLOCATED_CHANNELS; ++i)
    {
        ret = get_next_channel(ret);

        if (Mix_Playing(ret) == 0)
        {
            return ret;
        }
    }

    // Failed to find free channel
    return -1;
}

} //Namespace

void init()
{
    cleanup();

    if (Config::is_audio_enabled())
    {
        audio_chunks.resize(int(Sfx_id::END));

        //Monster sounds
        load_audio_file(Sfx_id::dog_snarl,              "sfx_dog_snarl.ogg");
        load_audio_file(Sfx_id::wolf_howl,              "sfx_wolf_howl.ogg");
        load_audio_file(Sfx_id::zombie_growl,           "sfx_zombie_growl.ogg");
        load_audio_file(Sfx_id::ghoul_growl,            "sfx_ghoul_growl.ogg");
        load_audio_file(Sfx_id::ooze_gurgle,            "sfx_ooze_gurgle.ogg");
        load_audio_file(Sfx_id::flapping_wings,         "sfx_flapping_wings.ogg");
        load_audio_file(Sfx_id::ape,                   "sfx_ape.ogg");

        //Weapon and attack sounds
        load_audio_file(Sfx_id::hit_small,              "sfx_hit_small.ogg");
        load_audio_file(Sfx_id::hit_medium,             "sfx_hit_medium.ogg");
        load_audio_file(Sfx_id::hit_hard,               "sfx_hit_hard.ogg");
        load_audio_file(Sfx_id::hit_corpse_break,        "sfx_hit_corpse_break.ogg");
        load_audio_file(Sfx_id::miss_light,             "sfx_miss_light.ogg");
        load_audio_file(Sfx_id::miss_medium,            "sfx_miss_medium.ogg");
        load_audio_file(Sfx_id::miss_heavy,             "sfx_miss_heavy.ogg");
        load_audio_file(Sfx_id::hit_sharp,              "sfx_hit_sharp.ogg");
        load_audio_file(Sfx_id::pistol_fire,            "sfx_pistol_fire.ogg");
        load_audio_file(Sfx_id::pistol_reload,          "sfx_pistol_reload.ogg");
        load_audio_file(Sfx_id::shotgun_sawed_off_fire,   "sfx_shotgun_sawed_off_fire.ogg");
        load_audio_file(Sfx_id::shotgun_pump_fire,       "sfx_shotgun_pump_fire.ogg");
        load_audio_file(Sfx_id::shotgun_reload,         "sfx_shotgun_reload.ogg");
        load_audio_file(Sfx_id::machine_gun_fire,        "sfx_machine_gun_fire.ogg");
        load_audio_file(Sfx_id::machine_gun_reload,      "sfx_machine_gun_reload.ogg");
        load_audio_file(Sfx_id::mi_go_gun_fire,           "sfx_migo_gun.ogg");
        load_audio_file(Sfx_id::spike_gun,              "sfx_spike_gun.ogg");

        //Environment sounds
        load_audio_file(Sfx_id::metal_clank,            "sfx_metal_clank.ogg");
        load_audio_file(Sfx_id::ricochet,              "sfx_ricochet.ogg");
        load_audio_file(Sfx_id::explosion,             "sfx_explosion.ogg");
        load_audio_file(Sfx_id::explosion_molotov,      "sfx_explosion_molotov.ogg");
        load_audio_file(Sfx_id::door_open,              "sfx_door_open.ogg");
        load_audio_file(Sfx_id::door_close,             "sfx_door_close.ogg");
        load_audio_file(Sfx_id::door_bang,              "sfx_door_bang.ogg");
        load_audio_file(Sfx_id::door_break,             "sfx_door_break.ogg");
        load_audio_file(Sfx_id::tomb_open,              "sfx_tomb_open.ogg");
        load_audio_file(Sfx_id::fountain_drink,         "sfx_fountain_drink.ogg");
        load_audio_file(Sfx_id::boss_voice1,            "sfx_boss_voice1.ogg");
        load_audio_file(Sfx_id::boss_voice2,            "sfx_boss_voice2.ogg");

        //User interface sounds
        load_audio_file(Sfx_id::backpack,              "sfx_backpack.ogg");
        load_audio_file(Sfx_id::pickup,                "sfx_pickup.ogg");
        load_audio_file(Sfx_id::electric_lantern,       "sfx_electric_lantern.ogg");
        load_audio_file(Sfx_id::potion_quaff,           "sfx_potion_quaff.ogg");
        load_audio_file(Sfx_id::spell_generic,          "sfx_spell_generic.ogg");
        load_audio_file(Sfx_id::insanity_rise,          "sfx_insanity_rising.ogg");
        load_audio_file(Sfx_id::glop,                  "sfx_glop.ogg");
        load_audio_file(Sfx_id::death,                 "sfx_death.ogg");

        int a = 1;
        const int FIRST = int(Sfx_id::AMB_START) + 1;
        const int LAST  = int(Sfx_id::AMB_END)   - 1;
        for (int i = FIRST; i <= LAST; ++i)
        {
            const string index_str = to_str(a);
            const string index_str_padded =
                a < 10  ? "00" + index_str : a < 100 ? "0"  + index_str : index_str;
            load_audio_file(Sfx_id(i), "amb_" + index_str_padded + ".ogg");
            a++;
        }

        load_audio_file(Sfx_id::mus_cthulhiana_Madness,
                      "Musica_Cthulhiana-Fragment-Madness.ogg");
    }
}

void cleanup()
{
    for (Mix_Chunk* chunk : audio_chunks) {Mix_Free_chunk(chunk);}
    audio_chunks.clear();

    cur_channel_     =  0;
    time_at_last_amb_  = -1;
}

int play(const Sfx_id sfx, const int VOL_PERCENT_TOT, const int VOL_PERCENT_L)
{
    if (
        !audio_chunks.empty()    &&
        sfx != Sfx_id::AMB_START &&
        sfx != Sfx_id::AMB_END   &&
        sfx != Sfx_id::END       &&
        !Config::is_bot_playing())
    {
        const int FREE_CHANNEL = get_free_channel(cur_channel_);

        if (FREE_CHANNEL >= 0)
        {
            cur_channel_ = FREE_CHANNEL;

            const int VOL_TOT   = (255 * VOL_PERCENT_TOT)   / 100;
            const int VOL_L     = (VOL_PERCENT_L * VOL_TOT) / 100;
            const int VOL_R     = VOL_TOT - VOL_L;

            Mix_Set_panning(cur_channel_, VOL_L, VOL_R);

            Mix_Play_channel(cur_channel_, audio_chunks[int(sfx)], 0);

            return cur_channel_;
        }
    }

    return -1;
}

void play(const Sfx_id sfx, const Dir dir, const int DISTANCE_PERCENT)
{
    if (!audio_chunks.empty() && dir != Dir::END)
    {
        //The distance value is scaled down to avoid too much volume degradation
        const int VOL_PERCENT_TOT = 100 - ((DISTANCE_PERCENT * 2) / 3);

        int vol_percent_l = 0;
        switch (dir)
        {
        case Dir::left:       vol_percent_l = 85;  break;
        case Dir::up_left:     vol_percent_l = 75;  break;
        case Dir::down_left:   vol_percent_l = 75;  break;
        case Dir::up:         vol_percent_l = 50;  break;
        case Dir::center:     vol_percent_l = 50;  break;
        case Dir::down:       vol_percent_l = 50;  break;
        case Dir::up_right:    vol_percent_l = 25;  break;
        case Dir::down_right:  vol_percent_l = 25;  break;
        case Dir::right:      vol_percent_l = 15;  break;
        case Dir::END:        vol_percent_l = 50;  break;
        }
        play(sfx, VOL_PERCENT_TOT, vol_percent_l);
    }
}

void try_play_amb(const int ONE_IN_N_CHANCE_TO_PLAY)
{
    if (!audio_chunks.empty() && Rnd::one_in(ONE_IN_N_CHANCE_TO_PLAY))
    {
        const int TIME_NOW                  = time(nullptr);
        const int TIME_REQ_BETWEEN_AMB_SFX  = 20;

        if ((TIME_NOW - TIME_REQ_BETWEEN_AMB_SFX) > time_at_last_amb_)
        {
            time_at_last_amb_          = TIME_NOW;
            const int   VOL_PERCENT = Rnd::one_in(5) ? Rnd::range(50,  99) : 100;
            const int   FIRST_INT   = int(Sfx_id::AMB_START) + 1;
            const int   LAST_INT    = int(Sfx_id::AMB_END)   - 1;
            const Sfx_id sfx         = Sfx_id(Rnd::range(FIRST_INT, LAST_INT));
            play(sfx , VOL_PERCENT);
        }
    }
}

void fade_out_channel(const int CHANNEL_NR)
{
    if (!audio_chunks.empty())
    {
        Mix_Fade_out_channel(CHANNEL_NR, 5000);
    }
}

} //Audio
