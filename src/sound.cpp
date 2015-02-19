#include "sound.hpp"

#include <iostream>
#include <string>

#include "feature_rigid.hpp"
#include "map.hpp"
#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "game_time.hpp"
#include "map_parsing.hpp"
#include "utils.hpp"

using namespace std;

Snd::Snd(
    const std::string&          msg,
    const Sfx_id                 sfx,
    const Ignore_msg_if_origin_seen ignore_msg_if_origin_seen,
    const Pos&                  origin,
    Actor* const                actor_who_made_sound,
    const Snd_vol                vol,
    const Alerts_mon             alerting_mon) :
    msg_                        (msg),
    sfx_                        (sfx),
    is_msg_ignored_if_origin_seen_   (ignore_msg_if_origin_seen),
    origin_                     (origin),
    actor_who_made_sound_          (actor_who_made_sound),
    vol_                        (vol),
    is_alerting_mon_              (alerting_mon) {}

namespace snd_emit
{

namespace
{

int nr_snd_msg_printed_cur_turn_;

bool is_snd_heard_at_range(const int RANGE, const Snd& snd)
{
    return snd.is_loud() ? (RANGE <= SND_DIST_LOUD) : (RANGE <= SND_DIST_NORMAL);
}

} //namespace

void reset_nr_snd_msg_printed_cur_turn()
{
    nr_snd_msg_printed_cur_turn_ = 0;
}

void emit_snd(Snd snd)
{
    bool blocked[MAP_W][MAP_H];
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const auto f  = map::cells[x][y].rigid;
            blocked[x][y] = !f->is_sound_passable();
        }
    }
    int flood_fill[MAP_W][MAP_H];
    const Pos& origin = snd.get_origin();
    flood_fill::run(origin, blocked, flood_fill, 999, Pos(-1, -1), true);
    flood_fill[origin.x][origin.y] = 0;

    for (Actor* actor : game_time::actors_)
    {
        const int FLOOD_VALUE_AT_ACTOR = flood_fill[actor->pos.x][actor->pos.y];

        const bool IS_ORIGIN_SEEN_BY_PLAYER =
            map::cells[origin.x][origin.y].is_seen_by_player;

        if (is_snd_heard_at_range(FLOOD_VALUE_AT_ACTOR, snd))
        {
            if (actor->is_player())
            {
                //Various conditions may clear the sound message
                if (
                    nr_snd_msg_printed_cur_turn_ >= 1 ||
                    (IS_ORIGIN_SEEN_BY_PLAYER && snd.is_msg_ignored_if_origin_seen()))
                {
                    snd.clear_msg();
                }

                const Pos& player_pos = map::player->pos;

                if (!snd.get_msg().empty())
                {
                    //Add a direction string to the message (i.e. "(NW)", "(E)" , etc)
                    if (player_pos != origin)
                    {
                        string dir_str;
                        dir_utils::get_compass_dir_name(player_pos, origin, dir_str);
                        snd.add_string("(" + dir_str + ")");
                    }
                    ++nr_snd_msg_printed_cur_turn_;
                }

                const int SND_MAX_DISTANCE =
                    snd.is_loud() ? SND_DIST_LOUD : SND_DIST_NORMAL;
                const int PERCENT_DISTANCE =
                    (FLOOD_VALUE_AT_ACTOR * 100) / SND_MAX_DISTANCE;

                const Pos offset = (origin - player_pos).get_signs();
                const Dir dir_to_origin = dir_utils::get_dir(offset);
                map::player->hear_sound(snd, IS_ORIGIN_SEEN_BY_PLAYER, dir_to_origin,
                                        PERCENT_DISTANCE);
            }
            else //Not player
            {
                Mon* const mon = static_cast<Mon*>(actor);
                mon->hear_sound(snd);
            }
        }
    }
}

}
