#include "audio.hpp"

#include <time.h>

#include <SDL_mixer.h>

#include "init.hpp"
#include "map.hpp"
#include "render.hpp"
#include "utils.hpp"

namespace audio
{

namespace
{

std::vector<Mix_Chunk*> audio_chunks_;

size_t ms_at_sfx_played_[size_t(Sfx_id::END)];

int         cur_channel_            = 0;
int         seconds_at_amb_played_  = -1;

int         nr_files_loaded_        = 0;
const int   NR_FILES_TOT            = int(Sfx_id::END) - 2; //Subtracting AMB_START and AMB_END

void load_audio_file(const Sfx_id sfx, const std::string& filename)
{
    //Read events, so that we don't freeze the game while we loading sounds
    SDL_PumpEvents();

    render::clear_screen();

    const std::string file_rel_path  = "audio/" + filename;

    const std::string nr_loaded_str = to_str(nr_files_loaded_) + "/" + to_str(NR_FILES_TOT) ;

    render::draw_text("Loading audio file " + nr_loaded_str + " (" + file_rel_path + ")...",
                      Panel::screen,
                      P(0, 0),
                      clr_white);

    audio_chunks_[size_t(sfx)] = Mix_LoadWAV(file_rel_path.c_str());

    if (!audio_chunks_[size_t(sfx)])
    {
        TRACE << "Problem loading audio file with name: "   << filename         << std::endl
              << "Mix_GetError(): "                         << Mix_GetError()   << std::endl;
        assert(false);
    }

    //Draw a loading bar
    const int PCT_LOADED    = (nr_files_loaded_ * 100) / NR_FILES_TOT;
    const int BAR_W_TOT     = 32;
    const int BAR_W_L       = (BAR_W_TOT * PCT_LOADED) / 100;
    const int BAR_W_R       = BAR_W_TOT - BAR_W_L;

    const P bar_p(1, 2);

    if (BAR_W_L > 0)
    {
        const std::string bar_l_str(BAR_W_L, '#');

        render::draw_text(bar_l_str,
                          Panel::screen,
                          bar_p,
                          clr_green);
    }

    if (BAR_W_R > 0)
    {
        const std::string bar_r_str(BAR_W_R, '-');

        render::draw_text(bar_r_str,
                          Panel::screen,
                          P(bar_p.x + BAR_W_L, bar_p.y),
                          clr_gray_drk);
    }

    render::draw_text("[",
                      Panel::screen,
                      P(bar_p.x - 1, bar_p.y),
                      clr_white);

    render::draw_text("]",
                      Panel::screen,
                      P(bar_p.x + BAR_W_TOT - 1, bar_p.y),
                      clr_white);

    render::update_screen();

    ++nr_files_loaded_;
}

int next_channel(const int FROM)
{
    assert(FROM >= 0 && FROM < AUDIO_ALLOCATED_CHANNELS);

    int ret = FROM + 1;

    if (ret == AUDIO_ALLOCATED_CHANNELS)
    {
        ret = 0;
    }

    return ret;
}

int free_channel(const int FROM)
{
    assert(FROM >= 0 && FROM < AUDIO_ALLOCATED_CHANNELS);

    int ret = FROM;

    for (int i = 0; i < AUDIO_ALLOCATED_CHANNELS; ++i)
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

} //Namespace

void init()
{
    TRACE_FUNC_BEGIN;

    cleanup();

    if (config::is_audio_enabled())
    {
        audio_chunks_.resize(size_t(Sfx_id::END));

        //Monster sounds
        load_audio_file(Sfx_id::dog_snarl,              "sfx_dog_snarl.ogg");
        load_audio_file(Sfx_id::wolf_howl,              "sfx_wolf_howl.ogg");
        load_audio_file(Sfx_id::hiss,                   "sfx_hiss.ogg");
        load_audio_file(Sfx_id::zombie_growl,           "sfx_zombie_growl.ogg");
        load_audio_file(Sfx_id::ghoul_growl,            "sfx_ghoul_growl.ogg");
        load_audio_file(Sfx_id::ooze_gurgle,            "sfx_ooze_gurgle.ogg");
        load_audio_file(Sfx_id::flapping_wings,         "sfx_flapping_wings.ogg");
        load_audio_file(Sfx_id::ape,                    "sfx_ape.ogg");

        //Weapon and attack sounds
        load_audio_file(Sfx_id::hit_small,              "sfx_hit_small.ogg");
        load_audio_file(Sfx_id::hit_medium,             "sfx_hit_medium.ogg");
        load_audio_file(Sfx_id::hit_hard,               "sfx_hit_hard.ogg");
        load_audio_file(Sfx_id::hit_corpse_break,       "sfx_hit_corpse_break.ogg");
        load_audio_file(Sfx_id::miss_light,             "sfx_miss_light.ogg");
        load_audio_file(Sfx_id::miss_medium,            "sfx_miss_medium.ogg");
        load_audio_file(Sfx_id::miss_heavy,             "sfx_miss_heavy.ogg");
        load_audio_file(Sfx_id::hit_sharp,              "sfx_hit_sharp.ogg");
        load_audio_file(Sfx_id::pistol_fire,            "sfx_pistol_fire.ogg");
        load_audio_file(Sfx_id::pistol_reload,          "sfx_pistol_reload.ogg");
        load_audio_file(Sfx_id::shotgun_sawed_off_fire, "sfx_shotgun_sawed_off_fire.ogg");
        load_audio_file(Sfx_id::shotgun_pump_fire,      "sfx_shotgun_pump_fire.ogg");
        load_audio_file(Sfx_id::shotgun_reload,         "sfx_shotgun_reload.ogg");
        load_audio_file(Sfx_id::machine_gun_fire,       "sfx_machine_gun_fire.ogg");
        load_audio_file(Sfx_id::machine_gun_reload,     "sfx_machine_gun_reload.ogg");
        load_audio_file(Sfx_id::mi_go_gun_fire,         "sfx_migo_gun.ogg");
        load_audio_file(Sfx_id::spike_gun,              "sfx_spike_gun.ogg");

        //Environment sounds
        load_audio_file(Sfx_id::metal_clank,            "sfx_metal_clank.ogg");
        load_audio_file(Sfx_id::ricochet,               "sfx_ricochet.ogg");
        load_audio_file(Sfx_id::explosion,              "sfx_explosion.ogg");
        load_audio_file(Sfx_id::explosion_molotov,      "sfx_explosion_molotov.ogg");
        load_audio_file(Sfx_id::gas,                    "sfx_gas.ogg");
        load_audio_file(Sfx_id::door_open,              "sfx_door_open.ogg");
        load_audio_file(Sfx_id::door_close,             "sfx_door_close.ogg");
        load_audio_file(Sfx_id::door_bang,              "sfx_door_bang.ogg");
        load_audio_file(Sfx_id::door_break,             "sfx_door_break.ogg");
        load_audio_file(Sfx_id::tomb_open,              "sfx_tomb_open.ogg");
        load_audio_file(Sfx_id::fountain_drink,         "sfx_fountain_drink.ogg");
        load_audio_file(Sfx_id::boss_voice1,            "sfx_boss_voice1.ogg");
        load_audio_file(Sfx_id::boss_voice2,            "sfx_boss_voice2.ogg");

        //User interface sounds
        load_audio_file(Sfx_id::backpack,               "sfx_backpack.ogg");
        load_audio_file(Sfx_id::pickup,                 "sfx_pickup.ogg");
        load_audio_file(Sfx_id::lantern,                "sfx_electric_lantern.ogg");
        load_audio_file(Sfx_id::potion_quaff,           "sfx_potion_quaff.ogg");
        load_audio_file(Sfx_id::spell_generic,          "sfx_spell_generic.ogg");
        load_audio_file(Sfx_id::spell_shield_break,     "sfx_spell_shield_break.ogg");
        load_audio_file(Sfx_id::insanity_rise,          "sfx_insanity_rising.ogg");
        load_audio_file(Sfx_id::glop,                   "sfx_glop.ogg");
        load_audio_file(Sfx_id::death,                  "sfx_death.ogg");

        int a = 1;

        const int FIRST = int(Sfx_id::AMB_START) + 1;
        const int LAST  = int(Sfx_id::AMB_END)   - 1;

        for (int i = FIRST; i <= LAST; ++i)
        {
            const std::string padding_str   = (a < 10)    ? "00"  :
                                              (a < 100)   ? "0"   : "";

            const std::string idx_str       = to_str(a);

            const std::string file_name = "amb_" + padding_str + idx_str + ".ogg";

            load_audio_file(Sfx_id(i), file_name);

            ++a;
        }

        load_audio_file(Sfx_id::mus_cthulhiana_Madness,
                        "musica_cthulhiana-fragment-madness.ogg");

        assert(nr_files_loaded_ == NR_FILES_TOT);
    }

    TRACE_FUNC_END;
}

void cleanup()
{
    TRACE_FUNC_BEGIN;

    for (size_t i = 0; i < size_t(Sfx_id::END); ++i)
    {
        ms_at_sfx_played_[i] = 0;
    }

    for (Mix_Chunk* chunk : audio_chunks_)
    {
        Mix_FreeChunk(chunk);
    }

    audio_chunks_.clear();

    cur_channel_            =  0;
    seconds_at_amb_played_  = -1;

    nr_files_loaded_ = 0;

    TRACE_FUNC_END;
}

int play(const Sfx_id sfx, const int VOL_PCT_TOT, const int VOL_PCT_L)
{
    if (
        !audio_chunks_.empty()      &&
        sfx != Sfx_id::AMB_START    &&
        sfx != Sfx_id::AMB_END      &&
        sfx != Sfx_id::END          &&
        !config::is_bot_playing())
    {
        const int       FREE_CHANNEL    = free_channel(cur_channel_);
        const size_t    MS_NOW          = SDL_GetTicks();
        size_t&         ms_last         = ms_at_sfx_played_[size_t(sfx)];
        const size_t    MS_DIFF         = MS_NOW - ms_last;

        if (FREE_CHANNEL >= 0 && MS_DIFF >= MIN_MS_BETWEEN_SAME_SFX)
        {
            cur_channel_ = FREE_CHANNEL;

            const int VOL_TOT   = (255 * VOL_PCT_TOT)   / 100;
            const int VOL_L     = (VOL_PCT_L * VOL_TOT) / 100;
            const int VOL_R     = VOL_TOT - VOL_L;

            Mix_SetPanning(cur_channel_, VOL_L, VOL_R);

            Mix_PlayChannel(cur_channel_, audio_chunks_[size_t(sfx)], 0);

            ms_last = SDL_GetTicks();

            return cur_channel_;
        }
    }

    return -1;
}

void play(const Sfx_id sfx, const Dir dir, const int DISTANCE_PCT)
{
    if (!audio_chunks_.empty() && dir != Dir::END)
    {
        //The distance value is scaled down to avoid too much volume degradation
        const int VOL_PCT_TOT = 100 - ((DISTANCE_PCT * 2) / 3);

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

        play(sfx, VOL_PCT_TOT, vol_pct_l);
    }
}

void try_play_amb(const int ONE_IN_N_CHANCE_TO_PLAY)
{
    if (!audio_chunks_.empty() && rnd::one_in(ONE_IN_N_CHANCE_TO_PLAY))
    {
        const int SECONDS_NOW               = time(nullptr);
        const int TIME_REQ_BETWEEN_AMB_SFX  = 20;

        if ((SECONDS_NOW - TIME_REQ_BETWEEN_AMB_SFX) > seconds_at_amb_played_)
        {
            seconds_at_amb_played_ = SECONDS_NOW;

            const int       VOL_PCT     = rnd::one_in(5) ? rnd::range(50,  99) : 100;
            const int       FIRST_INT   = int(Sfx_id::AMB_START) + 1;
            const int       LAST_INT    = int(Sfx_id::AMB_END)   - 1;
            const Sfx_id    sfx         = Sfx_id(rnd::range(FIRST_INT, LAST_INT));

            play(sfx , VOL_PCT);
        }
    }
}

void fade_out_channel(const int CHANNEL_NR)
{
    if (!audio_chunks_.empty())
    {
        Mix_FadeOutChannel(CHANNEL_NR, 5000);
    }
}

} //Audio
