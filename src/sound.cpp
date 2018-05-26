#include "sound.hpp"

#include <iostream>
#include <string>

#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "feature_rigid.hpp"
#include "game_time.hpp"
#include "map.hpp"
#include "map_parsing.hpp"

// -----------------------------------------------------------------------------
// Sound
// -----------------------------------------------------------------------------
Snd::Snd(
        const std::string& msg,
        const SfxId sfx,
        const IgnoreMsgIfOriginSeen ignore_msg_if_origin_seen,
        const P& origin,
        Actor* const actor_who_made_sound,
        const SndVol vol,
        const AlertsMon alerting_mon,
        const MorePromptOnMsg add_more_prompt_on_msg,
        std::shared_ptr<SndHeardEffect> snd_heard_effect) :
        msg_(msg),
        sfx_(sfx),
        is_msg_ignored_if_origin_seen_(ignore_msg_if_origin_seen),
        origin_(origin),
        actor_who_made_sound_(actor_who_made_sound),
        vol_(vol),
        is_alerting_mon_(alerting_mon),
        add_more_prompt_on_msg_(add_more_prompt_on_msg),
        snd_heard_effect_(snd_heard_effect) {}

Snd::~Snd()
{

}

void Snd::run()
{
        snd_emit::run(*this);
}

void Snd::on_heard(Actor& actor) const
{
        if (snd_heard_effect_)
        {
                snd_heard_effect_->run(actor);
        }
}

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static int nr_snd_msg_printed_current_turn_;

static bool is_snd_heard_at_range(const int range, const Snd& snd)
{
        return
                (range <= (snd.is_loud())
                 ?
                 snd_dist_loud :
                 snd_dist_normal);
}

// -----------------------------------------------------------------------------
// Sound emitting
// -----------------------------------------------------------------------------
namespace snd_emit
{

void reset_nr_snd_msg_printed_current_turn()
{
        nr_snd_msg_printed_current_turn_ = 0;
}

void run(Snd snd)
{
        ASSERT(snd.msg() != " ");

        Array2<bool> blocked(map::dims());

        map_parsers::BlocksSound()
                .run(blocked, blocked.rect());

        const P& origin = snd.origin();

        // Never block the origin - we want to be able to run the sound from
        // e.g. a closing door, after it was closed (and we don't want this to
        // depend on the floodfill algorithm, so we explicitly set the origin to
        // free here)
        blocked.at(origin.x, origin.y) = false;

        const auto flood = floodfill(
                origin,
                blocked,
                999,
                P(-1, -1),
                true);

        flood.at(origin.x, origin.y) = 0;

        for (Actor* actor : game_time::actors)
        {
                const int flood_val_at_actor = flood.at(actor->pos);

                const bool is_origin_seen_by_player =
                        map::cells.at(origin).is_seen_by_player;

                if (!is_snd_heard_at_range(flood_val_at_actor, snd))
                {
                        continue;
                }

                if (actor->is_player())
                {
                        if (is_origin_seen_by_player &&
                            snd.is_msg_ignored_if_origin_seen())
                        {
                                snd.clear_msg();
                        }

                        const P& player_pos = map::player->pos;

                        if (!snd.msg().empty())
                        {
                                // Add a direction to the message (i.e. "(NW)")
                                if (player_pos != origin)
                                {
                                        const std::string dir_str =
                                                dir_utils::compass_dir_name(
                                                        player_pos,
                                                        origin);

                                        snd.add_string("(" + dir_str + ")");
                                }
                        }

                        const int snd_max_dist =
                                snd.is_loud() ?
                                snd_dist_loud : snd_dist_normal;

                        const int pct_dist =
                                (flood_val_at_actor * 100) / snd_max_dist;

                        const P offset = (origin - player_pos).signs();

                        const Dir dir_to_origin = dir_utils::dir(offset);

                        map::player->hear_sound(snd,
                                                is_origin_seen_by_player,
                                                dir_to_origin, pct_dist);
                }
                else // Not player
                {
                        Mon* const mon = static_cast<Mon*>(actor);

                        mon->hear_sound(snd);
                }
        }
}

} // snd_emit
