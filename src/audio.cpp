#include "audio.hpp"

#include <time.h>

#include <SDL_mixer.h>

#include "init.hpp"
#include "map.hpp"
#include "render.hpp"

namespace audio
{

namespace
{

std::vector<Mix_Chunk*> audio_chunks_;

size_t ms_at_sfx_played_[size_t(SfxId::END)];

int         current_channel_            = 0;
int         seconds_at_amb_played_  = -1;

int         nr_files_loaded_        = 0;
const int   nr_files_tot            = int(SfxId::END) - 2; //Subtracting AMB_START and AMB_END

void load_audio_file(const SfxId sfx, const std::string& filename)
{
    //Read events, so that we don't freeze the game while we loading sounds
    SDL_PumpEvents();

    render::clear_screen();

    const std::string file_rel_path  = "audio/" + filename;

    const std::string nr_loaded_str = to_str(nr_files_loaded_) + "/" + to_str(nr_files_tot) ;

    render::draw_text("Loading audio file " + nr_loaded_str + " (" + file_rel_path + ")...",
                      Panel::screen,
                      P(0, 0),
                      clr_white);

    audio_chunks_[size_t(sfx)] = Mix_LoadWAV(file_rel_path.c_str());

    if (!audio_chunks_[size_t(sfx)])
    {
        TRACE << "Problem loading audio file with name: "   << filename         << std::endl
              << "Mix_GetError(): "                         << Mix_GetError()   << std::endl;
        ASSERT(false);
    }

    //Draw a loading bar
    const int pct_loaded    = (nr_files_loaded_ * 100) / nr_files_tot;
    const int bar_w_tot     = 32;
    const int bar_w_l       = (bar_w_tot * pct_loaded) / 100;
    const int bar_w_r       = bar_w_tot - bar_w_l;

    const P bar_p(1, 2);

    if (bar_w_l > 0)
    {
        const std::string bar_l_str(bar_w_l, '#');

        render::draw_text(bar_l_str,
                          Panel::screen,
                          bar_p,
                          clr_green);
    }

    if (bar_w_r > 0)
    {
        const std::string bar_r_str(bar_w_r, '-');

        render::draw_text(bar_r_str,
                          Panel::screen,
                          P(bar_p.x + bar_w_l, bar_p.y),
                          clr_gray_drk);
    }

    render::draw_text("[",
                      Panel::screen,
                      P(bar_p.x - 1, bar_p.y),
                      clr_white);

    render::draw_text("]",
                      Panel::screen,
                      P(bar_p.x + bar_w_tot, bar_p.y),
                      clr_white);

    render::update_screen();

    ++nr_files_loaded_;
}

int next_channel(const int from)
{
    ASSERT(from >= 0 && from < audio_allocated_channels);

    int ret = from + 1;

    if (ret == audio_allocated_channels)
    {
        ret = 0;
    }

    return ret;
}

int find_free_channel(const int from)
{
    ASSERT(from >= 0 && from < audio_allocated_channels);

    int ret = from;

    for (int i = 0; i < audio_allocated_channels; ++i)
    {
        ret = next_channel(ret);

        if (Mix_Playing(ret) == 0)
        {
            return ret;
        }
    }

    // Failed to find free channel
    return -1;
}

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;

    cleanup();

    if (config::is_audio_enabled())
    {
        audio_chunks_.resize(size_t(SfxId::END));

        //Monster sounds
        load_audio_file(SfxId::dog_snarl,              "sfx_dog_snarl.ogg");
        load_audio_file(SfxId::wolf_howl,              "sfx_wolf_howl.ogg");
        load_audio_file(SfxId::hiss,                   "sfx_hiss.ogg");
        load_audio_file(SfxId::zombie_growl,           "sfx_zombie_growl.ogg");
        load_audio_file(SfxId::ghoul_growl,            "sfx_ghoul_growl.ogg");
        load_audio_file(SfxId::ooze_gurgle,            "sfx_ooze_gurgle.ogg");
        load_audio_file(SfxId::flapping_wings,         "sfx_flapping_wings.ogg");
        load_audio_file(SfxId::ape,                    "sfx_ape.ogg");

        //Weapon and attack sounds
        load_audio_file(SfxId::hit_small,              "sfx_hit_small.ogg");
        load_audio_file(SfxId::hit_medium,             "sfx_hit_medium.ogg");
        load_audio_file(SfxId::hit_hard,               "sfx_hit_hard.ogg");
        load_audio_file(SfxId::hit_corpse_break,       "sfx_hit_corpse_break.ogg");
        load_audio_file(SfxId::miss_light,             "sfx_miss_light.ogg");
        load_audio_file(SfxId::miss_medium,            "sfx_miss_medium.ogg");
        load_audio_file(SfxId::miss_heavy,             "sfx_miss_heavy.ogg");
        load_audio_file(SfxId::hit_sharp,              "sfx_hit_sharp.ogg");
        load_audio_file(SfxId::pistol_fire,            "sfx_pistol_fire.ogg");
        load_audio_file(SfxId::pistol_reload,          "sfx_pistol_reload.ogg");
        load_audio_file(SfxId::shotgun_sawed_off_fire, "sfx_shotgun_sawed_off_fire.ogg");
        load_audio_file(SfxId::shotgun_pump_fire,      "sfx_shotgun_pump_fire.ogg");
        load_audio_file(SfxId::shotgun_reload,         "sfx_shotgun_reload.ogg");
        load_audio_file(SfxId::machine_gun_fire,       "sfx_machine_gun_fire.ogg");
        load_audio_file(SfxId::machine_gun_reload,     "sfx_machine_gun_reload.ogg");
        load_audio_file(SfxId::mi_go_gun_fire,         "sfx_migo_gun.ogg");
        load_audio_file(SfxId::spike_gun,              "sfx_spike_gun.ogg");
        load_audio_file(SfxId::bite,                   "sfx_bite.ogg");

        //Environment sounds
        load_audio_file(SfxId::metal_clank,            "sfx_metal_clank.ogg");
        load_audio_file(SfxId::ricochet,               "sfx_ricochet.ogg");
        load_audio_file(SfxId::explosion,              "sfx_explosion.ogg");
        load_audio_file(SfxId::explosion_molotov,      "sfx_explosion_molotov.ogg");
        load_audio_file(SfxId::gas,                    "sfx_gas.ogg");
        load_audio_file(SfxId::door_open,              "sfx_door_open.ogg");
        load_audio_file(SfxId::door_close,             "sfx_door_close.ogg");
        load_audio_file(SfxId::door_bang,              "sfx_door_bang.ogg");
        load_audio_file(SfxId::door_break,             "sfx_door_break.ogg");
        load_audio_file(SfxId::tomb_open,              "sfx_tomb_open.ogg");
        load_audio_file(SfxId::fountain_drink,         "sfx_fountain_drink.ogg");
        load_audio_file(SfxId::boss_voice1,            "sfx_boss_voice1.ogg");
        load_audio_file(SfxId::boss_voice2,            "sfx_boss_voice2.ogg");

        //User interface sounds
        load_audio_file(SfxId::backpack,               "sfx_backpack.ogg");
        load_audio_file(SfxId::pickup,                 "sfx_pickup.ogg");
        load_audio_file(SfxId::lantern,                "sfx_electric_lantern.ogg");
        load_audio_file(SfxId::potion_quaff,           "sfx_potion_quaff.ogg");
        load_audio_file(SfxId::spell_generic,          "sfx_spell_generic.ogg");
        load_audio_file(SfxId::spell_shield_break,     "sfx_spell_shield_break.ogg");
        load_audio_file(SfxId::insanity_rise,          "sfx_insanity_rising.ogg");
        load_audio_file(SfxId::glop,                   "sfx_glop.ogg");
        load_audio_file(SfxId::death,                  "sfx_death.ogg");

        int a = 1;

        const int first = int(SfxId::AMB_START) + 1;
        const int last  = int(SfxId::AMB_END)   - 1;

        for (int i = first; i <= last; ++i)
        {
            const std::string padding_str   = (a < 10)    ? "00"  :
                                              (a < 100)   ? "0"   : "";

            const std::string idx_str       = to_str(a);

            const std::string file_name = "amb_" + padding_str + idx_str + ".ogg";

            load_audio_file(SfxId(i), file_name);

            ++a;
        }

        load_audio_file(SfxId::mus_cthulhiana_Madness,
                        "musica_cthulhiana-fragment-madness.ogg");

        ASSERT(nr_files_loaded_ == nr_files_tot);
    }

    TRACE_FUNC_END;
}

void cleanup()
{
    TRACE_FUNC_BEGIN;

    for (size_t i = 0; i < size_t(SfxId::END); ++i)
    {
        ms_at_sfx_played_[i] = 0;
    }

    for (Mix_Chunk* chunk : audio_chunks_)
    {
        Mix_FreeChunk(chunk);
    }

    audio_chunks_.clear();

    current_channel_            =  0;
    seconds_at_amb_played_  = -1;

    nr_files_loaded_ = 0;

    TRACE_FUNC_END;
}

int play(const SfxId sfx, const int vol_pct_tot, const int vol_pct_l)
{
    if (
        !audio_chunks_.empty()      &&
        sfx != SfxId::AMB_START    &&
        sfx != SfxId::AMB_END      &&
        sfx != SfxId::END          &&
        !config::is_bot_playing())
    {
        const int       free_channel    = find_free_channel(current_channel_);
        const size_t    ms_now          = SDL_GetTicks();
        size_t&         ms_last         = ms_at_sfx_played_[size_t(sfx)];
        const size_t    ms_diff         = ms_now - ms_last;

        if (free_channel >= 0 && ms_diff >= min_ms_between_same_sfx)
        {
            current_channel_ = free_channel;

            const int vol_tot   = (255 * vol_pct_tot)   / 100;
            const int vol_l     = (vol_pct_l * vol_tot) / 100;
            const int vol_r     = vol_tot - vol_l;

            Mix_SetPanning(current_channel_, vol_l, vol_r);

            Mix_PlayChannel(current_channel_, audio_chunks_[size_t(sfx)], 0);

            ms_last = SDL_GetTicks();

            return current_channel_;
        }
    }

    return -1;
}

void play(const SfxId sfx, const Dir dir, const int distance_pct)
{
    if (!audio_chunks_.empty() && dir != Dir::END)
    {
        //The distance value is scaled down to avoid too much volume degradation
        const int vol_pct_tot = 100 - ((distance_pct * 2) / 3);

        int vol_pct_l = 0;

        switch (dir)
        {
        case Dir::left:
            vol_pct_l = 85;
            break;

        case Dir::up_left:
            vol_pct_l = 75;
            break;

        case Dir::down_left:
            vol_pct_l = 75;
            break;

        case Dir::up:
            vol_pct_l = 50;
            break;

        case Dir::center:
            vol_pct_l = 50;
            break;

        case Dir::down:
            vol_pct_l = 50;
            break;

        case Dir::up_right:
            vol_pct_l = 25;
            break;

        case Dir::down_right:
            vol_pct_l = 25;
            break;

        case Dir::right:
            vol_pct_l = 15;
            break;

        case Dir::END:
            vol_pct_l = 50;
            break;
        }

        play(sfx, vol_pct_tot, vol_pct_l);
    }
}

void try_play_amb(const int one_in_n_chance_to_play)
{
    if (!audio_chunks_.empty() && rnd::one_in(one_in_n_chance_to_play))
    {
        const int seconds_now               = time(nullptr);
        const int time_req_between_amb_sfx  = 20;

        if ((seconds_now - time_req_between_amb_sfx) > seconds_at_amb_played_)
        {
            seconds_at_amb_played_ = seconds_now;

            const int       vol_pct     = rnd::range(25, 100);
            const int       first_int   = (int)SfxId::AMB_START + 1;
            const int       last_int    = (int)SfxId::AMB_END   - 1;
            const SfxId     sfx         = (SfxId)rnd::range(first_int, last_int);

            play(sfx , vol_pct);
        }
    }
}

void fade_out_channel(const int channel_nr)
{
    if (!audio_chunks_.empty())
    {
        Mix_FadeOutChannel(channel_nr, 5000);
    }
}

} //Audio
