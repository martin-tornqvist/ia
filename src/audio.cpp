#include "audio.hpp"

#include <time.h>

#include <SDL_mixer.h>

#include "init.hpp"
#include "map.hpp"
#include "io.hpp"

namespace audio
{

namespace
{

std::vector<Mix_Chunk*> audio_chunks_;

std::vector<Mix_Music*> mus_chunks_;

size_t ms_at_sfx_played_[(size_t)SfxId::END];

int current_channel_ = 0;
int seconds_at_amb_played_ = -1;

int nr_files_loaded_ = 0;

void load(const SfxId sfx, const std::string& filename)
{
    // Sound already loaded?
    if (audio_chunks_[(size_t)sfx])
    {
        return;
    }

    // Read events, so that we don't freeze the game while we loading sounds
    SDL_PumpEvents();

    const std::string file_rel_path = "res/audio/" + filename;

    audio_chunks_[(size_t)sfx] = Mix_LoadWAV(file_rel_path.c_str());

    if (!audio_chunks_[(size_t)sfx])
    {
        TRACE << "Problem loading audio file with name: "
              << filename << std::endl
              << "Mix_GetError(): "
              << Mix_GetError()   << std::endl;
        ASSERT(false);
    }

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

std::string amb_sfx_filename(const SfxId sfx)
{
    const int amb_nr = (int)sfx - (int)SfxId::AMB_START;

    const std::string padding_str =
        (amb_nr < 10) ? "00" :
        (amb_nr < 100) ? "0" : "";

    const std::string idx_str = std::to_string(amb_nr);

    return
        "amb_" +
        padding_str +
        idx_str +
        ".ogg";
}

} // namespace

void init()
{
    TRACE_FUNC_BEGIN;

    cleanup();

    if (!config::is_audio_enabled())
    {
        TRACE_FUNC_END;

        return;
    }

    audio_chunks_.resize((size_t)SfxId::END);

    for (size_t i = 0; i < audio_chunks_.size(); ++i)
    {
        audio_chunks_[i] = nullptr;
    }

    //
    // Pre-load the action sound effects (ambient sounds are loaded on demand)
    //

    //
    // Monster sounds
    //
    load(SfxId::dog_snarl, "sfx_dog_snarl.ogg");
    load(SfxId::wolf_howl, "sfx_wolf_howl.ogg");
    load(SfxId::hiss, "sfx_hiss.ogg");
    load(SfxId::zombie_growl, "sfx_zombie_growl.ogg");
    load(SfxId::ghoul_growl, "sfx_ghoul_growl.ogg");
    load(SfxId::ooze_gurgle, "sfx_ooze_gurgle.ogg");
    load(SfxId::flapping_wings, "sfx_flapping_wings.ogg");
    load(SfxId::ape, "sfx_ape.ogg");

    //
    // Weapon and attack sounds
    //
    load(SfxId::hit_small, "sfx_hit_small.ogg");
    load(SfxId::hit_medium, "sfx_hit_medium.ogg");
    load(SfxId::hit_hard, "sfx_hit_hard.ogg");
    load(SfxId::hit_corpse_break, "sfx_hit_corpse_break.ogg");
    load(SfxId::miss_light, "sfx_miss_light.ogg");
    load(SfxId::miss_medium, "sfx_miss_medium.ogg");
    load(SfxId::miss_heavy, "sfx_miss_heavy.ogg");
    load(SfxId::hit_sharp, "sfx_hit_sharp.ogg");
    load(SfxId::pistol_fire, "sfx_pistol_fire.ogg");
    load(SfxId::pistol_reload, "sfx_pistol_reload.ogg");
    load(SfxId::shotgun_sawed_off_fire, "sfx_shotgun_sawed_off_fire.ogg");
    load(SfxId::shotgun_pump_fire, "sfx_shotgun_pump_fire.ogg");
    load(SfxId::shotgun_reload, "sfx_shotgun_reload.ogg");
    load(SfxId::machine_gun_fire, "sfx_machine_gun_fire.ogg");
    load(SfxId::machine_gun_reload, "sfx_machine_gun_reload.ogg");
    load(SfxId::mi_go_gun_fire, "sfx_migo_gun.ogg");
    load(SfxId::spike_gun, "sfx_spike_gun.ogg");
    load(SfxId::bite, "sfx_bite.ogg");

    //
    // Environment sounds
    //
    load(SfxId::metal_clank, "sfx_metal_clank.ogg");
    load(SfxId::ricochet, "sfx_ricochet.ogg");
    load(SfxId::explosion, "sfx_explosion.ogg");
    load(SfxId::explosion_molotov, "sfx_explosion_molotov.ogg");
    load(SfxId::gas, "sfx_gas.ogg");
    load(SfxId::door_open, "sfx_door_open.ogg");
    load(SfxId::door_close, "sfx_door_close.ogg");
    load(SfxId::door_bang, "sfx_door_bang.ogg");
    load(SfxId::door_break, "sfx_door_break.ogg");
    load(SfxId::tomb_open, "sfx_tomb_open.ogg");
    load(SfxId::fountain_drink, "sfx_fountain_drink.ogg");
    load(SfxId::boss_voice1, "sfx_boss_voice1.ogg");
    load(SfxId::boss_voice2, "sfx_boss_voice2.ogg");
    load(SfxId::chains, "sfx_chains.ogg");
    load(SfxId::glop, "sfx_glop.ogg");
    load(SfxId::lever_pull, "sfx_lever_pull.ogg");
    load(SfxId::monolith, "sfx_monolith.ogg");

    //
    // User interface sounds
    //
    load(SfxId::backpack, "sfx_backpack.ogg");
    load(SfxId::pickup, "sfx_pickup.ogg");
    load(SfxId::lantern, "sfx_electric_lantern.ogg");
    load(SfxId::potion_quaff, "sfx_potion_quaff.ogg");
    load(SfxId::spell_generic, "sfx_spell_generic.ogg");
    load(SfxId::spell_shield_break, "sfx_spell_shield_break.ogg");
    load(SfxId::insanity_rise, "sfx_insanity_rising.ogg");
    load(SfxId::death, "sfx_death.ogg");
    load(SfxId::menu_browse, "sfx_menu_browse.ogg");
    load(SfxId::menu_select, "sfx_menu_select.ogg");

    ASSERT(nr_files_loaded_ == (int)SfxId::AMB_START);

    //
    // Load music
    //
    mus_chunks_.resize((size_t)MusId::END);

    mus_chunks_[(size_t)MusId::cthulhiana_madness] =
        Mix_LoadMUS("res/audio/musica_cthulhiana-fragment-madness.ogg");

    TRACE_FUNC_END;
}

void cleanup()
{
    TRACE_FUNC_BEGIN;

    for (size_t i = 0; i < (size_t)SfxId::END; ++i)
    {
        ms_at_sfx_played_[i] = 0;
    }

    for (Mix_Chunk* chunk : audio_chunks_)
    {
        Mix_FreeChunk(chunk);
    }

    audio_chunks_.clear();

    for (Mix_Music* chunk : mus_chunks_)
    {
        Mix_FreeMusic(chunk);
    }

    mus_chunks_.clear();

    current_channel_ =  0;
    seconds_at_amb_played_ = -1;

    // nr_files_loaded_ = 0;

    TRACE_FUNC_END;
}

void play(const SfxId sfx,
          const int vol_pct_tot,
          const int vol_pct_l)
{
    if (audio_chunks_.empty() ||
        (sfx == SfxId::AMB_START) ||
        (sfx == SfxId::END) ||
        config::is_bot_playing())
    {
        return;
    }

    // Is this an ambient sound which has not yet been loaded?
    if (((int)sfx > (int)SfxId::AMB_START) &&
        !audio_chunks_[(size_t)sfx])
    {
        load(sfx, amb_sfx_filename(sfx));
    }

    const int free_channel = find_free_channel(current_channel_);

    const size_t ms_now = SDL_GetTicks();

    size_t& ms_last = ms_at_sfx_played_[(size_t)sfx];

    const size_t ms_diff = ms_now - ms_last;

    if ((free_channel >= 0) &&
        (ms_diff >= min_ms_between_same_sfx))
    {
        current_channel_ = free_channel;

        const int vol_tot = (255 * vol_pct_tot) / 100;
        const int vol_l = (vol_pct_l * vol_tot) / 100;
        const int vol_r = vol_tot - vol_l;

        Mix_SetPanning(current_channel_,
                       vol_l,
                       vol_r);

        Mix_PlayChannel(current_channel_,
                        audio_chunks_[(size_t)sfx],
                        0);

        ms_last = SDL_GetTicks();
    }
}

void play(const SfxId sfx,
          const Dir dir,
          const int distance_pct)
{
    if (!audio_chunks_.empty() &&
        (dir != Dir::END))
    {
        // The distance value is scaled down to avoid too much volume reduction
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

        play(sfx,
             vol_pct_tot,
             vol_pct_l);
    }
}

void try_play_amb(const int one_in_n_chance_to_play)
{
    //
    // NOTE: The ambient sound effect will be loaded by play(), if not already
    //       loaded (only the action sound effects are pre-loaded)
    //

    if (!audio_chunks_.empty() &&
        rnd::one_in(one_in_n_chance_to_play))
    {
        const int seconds_now = time(nullptr);
        const int time_req_between_amb_sfx = 25;

        if ((seconds_now - time_req_between_amb_sfx) > seconds_at_amb_played_)
        {
            seconds_at_amb_played_ = seconds_now;

            const int vol_pct = rnd::range(15, 100);

            const int first_int = (int)SfxId::AMB_START + 1;

            const int last_int = (int)SfxId::END - 1;

            const SfxId sfx = (SfxId)rnd::range(first_int, last_int);

            play(sfx , vol_pct);
        }
    }
}

void play_music(const MusId mus)
{
    // Only play if not already playing music
    if (!mus_chunks_.empty() &&
        !Mix_PlayingMusic())
    {
        auto* const chunk = mus_chunks_[(size_t)mus];

        // Loop forever
        Mix_PlayMusic(chunk, -1);
    }
}

void fade_out_music()
{
    if (!mus_chunks_.empty())
    {
        Mix_FadeOutMusic(2000);
    }
}

} // audio
