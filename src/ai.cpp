#include "ai.hpp"

#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "feature_mob.hpp"
#include "feature_door.hpp"
#include "actor_mon.hpp"
#include "line_calc.hpp"
#include "map_parsing.hpp"
#include "game_time.hpp"
#include "fov.hpp"

namespace ai
{

namespace action
{

bool try_cast_random_spell(Mon& mon)
{
    if (
        !mon.is_alive()                 ||
        mon.spell_cool_down_cur_ > 0    ||
        mon.spells_known_.empty()       ||
        !mon.prop_handler().allow_cast_spell(Verbosity::silent))
    {
        return false;
    }

    std::random_shuffle(begin(mon.spells_known_), end(mon.spells_known_));

    for (Spell* const spell : mon.spells_known_)
    {
        if (spell->allow_mon_cast_now(mon))
        {
            const int CUR_SPI = mon.spi();
            const int SPELL_MAX_SPI = spell->spi_cost(false, &mon).max;

            //Cast spell if max spirit cost is lower than current spirit,
            if (SPELL_MAX_SPI < CUR_SPI)
            {
                spell->cast(&mon, true);
                return true;
            }

            //This point reached means SPI was lower than the spells potential cost
            const int CUR_HP  = mon.hp();
            const int MAX_HP  = mon.hp_max(true);

            //If monster is not allied to player, with a small chance, cast the spell
            //anyway if HP is low.
            if (
                !map::player->is_leader_of(&mon)    &&
                CUR_HP < (MAX_HP / 3)               &&
                rnd::one_in(20))
            {
                if (map::player->can_see_actor(mon))
                {
                    msg_log::add(mon.name_the() + " looks desperate.");
                }

                spell->cast(&mon, true);
                return true;
            }

            return false;
        }
    }

    return false;
}

bool handle_closed_blocking_door(Mon& mon, std::vector<P> path)
{
    if (!mon.is_alive() || path.empty())
    {
        return false;
    }

    const P& p = path.back();
    Feature* const f = map::cells[p.x][p.y].rigid;

    if (f->id() == Feature_id::door)
    {
        Door* const door = static_cast<Door*>(f);

        if (!door->can_move(mon))
        {
            if (!door->is_stuck())
            {
                if (mon.data().can_open_doors)
                {
                    door->try_open(&mon);
                    return true;
                }
                else if (mon.data().can_bash_doors)
                {
                    door->hit(Dmg_type::physical, Dmg_method::kick, &mon);
                    return true;
                }
            }
            else if (mon.data().can_bash_doors)
            {
                door->hit(Dmg_type::physical, Dmg_method::kick, &mon);
                return true;
            }
        }
    }

    return false;
}

bool handle_inventory(Mon& mon)
{
    (void)mon;
    return false;
}

//Helper function(s) for make_room_for_friend()
namespace
{

//Check if position is on a line between two points
bool is_pos_on_line(const P& p, const P& line_p0, const P& line_p1)
{
    //First, do a cheaper check and just see if we are outside a rectangle defined by the two
    //points. If we are outside this area, we can't possibly be on a line between the points.
    const R r(std::min(line_p0.x, line_p1.x),
              std::min(line_p0.y, line_p1.y),
              std::max(line_p0.x, line_p1.x),
              std::max(line_p0.y, line_p1.y));

    if (!is_pos_inside(p, r))
    {
        return false;
    }

    //OK, we could be on the line!

    std::vector<P> line;

    line_calc::calc_new_line(line_p0,
                             line_p1,
                             true,
                             9999,
                             false,
                             line);

    for (const P& pos_in_line : line)
    {
        if (p == pos_in_line)
        {
            return true;
        }
    }

    return false;
}

//Returns all free positions around the acting monster that is further from the player than the
//monster's current position
void move_bucket(Mon& mon, std::vector<P>& dirs_to_mk)
{
    dirs_to_mk.clear();

    const P& mon_p      = mon.pos;
    const P& player_p   = map::player->pos;

    bool blocked[map_w][map_h];

    const R area_to_check_blocked(mon_p - P(1, 1), mon_p + P(1, 1));

    map_parse::run(cell_check::Blocks_actor(mon, true),
                   blocked,
                   Map_parse_mode::overwrite,
                   area_to_check_blocked);

    for (const P& d : dir_utils::dir_list)
    {
        const P tgt_p = mon_p + d;

        const int CUR_TO_PLAYER_DIST = king_dist(mon_p, player_p);
        const int TGT_TO_PLAYER_DIST = king_dist(tgt_p, player_p);

        if (
            TGT_TO_PLAYER_DIST <= CUR_TO_PLAYER_DIST &&
            !blocked[tgt_p.x][tgt_p.y])
        {
            dirs_to_mk.push_back(tgt_p);
        }
    }
}

} //namespace

bool make_room_for_friend(Mon& mon)
{
    if (!mon.is_alive())
    {
        return false;
    }

    bool blocked_los[map_w][map_h];

    map_parse::run(cell_check::Blocks_los(), blocked_los);

    if (!mon.can_see_actor(*map::player, blocked_los))
    {
        return false;
    }

    const P& player_p = map::player->pos;

    //Check if there is an allied monster that we should move away for
    for (Actor* other_actor : game_time::actors)
    {
        if (
            other_actor != &mon         &&
            other_actor->is_alive()     &&
            !other_actor->is_player()   &&
            !map::player->is_leader_of(other_actor)
        )
        {
            Mon* const other_mon = static_cast<Mon*>(other_actor);

            const bool IS_OTHER_ADJ =
                is_pos_adj(mon.pos, other_mon->pos, false);

            //TODO: It's probably better to check LOS than vision here?
            //We don't want to move out of the way for a blind monster.
            const bool IS_OTHER_SEEING_PLAYER =
                other_mon->can_see_actor(*map::player, blocked_los);

            /*
             Do we have this situation?
             #####
             #.A.#
             #@#B#
             #####
             */
            const bool IS_OTHER_ADJ_WITH_NO_PLAYER_LOS =
                IS_OTHER_ADJ && !IS_OTHER_SEEING_PLAYER;

            //We consider moving out of the way if the other monster EITHER:
            // * Is seeing the player and we are blocking it, or
            // * Is adjacent to us, and is not seeing the player.
            if (
                (IS_OTHER_SEEING_PLAYER && is_pos_on_line(mon.pos, other_mon->pos, player_p)) ||
                IS_OTHER_ADJ_WITH_NO_PLAYER_LOS)
            {
                //We are blocking a friend! Try to find an adjacent free cell, which:
                // * Is NOT further away from the player than our current position, and
                // * Is not also blocking another monster

                //NOTE: We do not care whether the target cell has LOS to the
                //player or not. If we move into a cell without LOS, it will
                //appear as if we are dodging in and out of cover. It lets us
                //move towards the player with less time in the player's LOS,
                //and allows blocked ranged monsters to shoot at the player.

                //Get a list of neighbouring free cells
                std::vector<P> pos_bucket;
                move_bucket(mon, pos_bucket);

                //Sort the list by distance to player
                Is_closer_to_pos cmp(player_p);
                sort(pos_bucket.begin(), pos_bucket.end(), cmp);

                //Try to find a position not blocking a third allied monster
                for (const auto& tgt_p : pos_bucket)
                {
                    bool is_p_ok = true;

                    for (Actor* actor3 : game_time::actors)
                    {
                        //NOTE: The third actor here can include the original
                        //blocked "other" actor, since we must also check if we
                        //block that actor from the target position.
                        if (
                            actor3 != &mon          &&
                            actor3->is_alive()      &&
                            !actor3->is_player()    &&
                            !map::player->is_leader_of(actor3))
                        {
                            Mon* const mon3 = static_cast<Mon*>(actor3);

                            const bool OTHER_IS_SEEING_PLAYER =
                                mon3->can_see_actor(*map::player, blocked_los);

                            //TODO: We also need to check that we don't move
                            //into a cell which is adjacent to a third monster,
                            //who does not have LOS to player! As it is now,
                            //we may move out of the way for one such monster,
                            //only to block another in the same way!

                            if (
                                OTHER_IS_SEEING_PLAYER &&
                                is_pos_on_line(tgt_p, mon3->pos, player_p))
                            {
                                is_p_ok = false;
                                break;
                            }
                        }
                    }

                    if (is_p_ok)
                    {
                        const P offset = tgt_p - mon.pos;

                        mon.move(dir_utils::dir(offset));

                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool move_to_random_adj_cell(Mon& mon)
{
    if (
        !mon.is_alive() ||
        (!mon.is_roaming_allowed_ && mon.aware_counter_ <= 0))
    {
        return false;
    }

    bool blocked[map_w][map_h];

    cell_check::Blocks_actor cellcheck(mon, true);

    for (const P& d : dir_utils::dir_list)
    {
        const P p(mon.pos + d);

        blocked[p.x][p.y] = cellcheck.check(map::cells[p.x][p.y]);
    }

    for (Actor* actor : game_time::actors)
    {
        if (actor->is_alive())
        {
            const P& p = actor->pos;
            blocked[p.x][p.y] = true;
        }
    }

    for (Mob* mob : game_time::mobs)
    {
        const P& p = mob->pos();

        if (cellcheck.check(*mob))
        {
            blocked[p.x][p.y] = true;
        }
    }

    const R area_allowed(P(1, 1), P(map_w - 2, map_h - 2));

    //First, try the same direction as last travelled
    Dir dir = Dir::END;

    const Dir last_dir_travelled = mon.last_dir_moved_;

    if (last_dir_travelled != Dir::center && last_dir_travelled != Dir::END)
    {
        const P tgt_p(mon.pos + dir_utils::offset(last_dir_travelled));

        if (
            !blocked[tgt_p.x][tgt_p.y] &&
            is_pos_inside(tgt_p, area_allowed))
        {
            dir = last_dir_travelled;
        }
    }

    //Attempt to find a random non-blocked adjacent cell
    if (dir == Dir::END)
    {
        std::vector<Dir> dir_bucket;
        dir_bucket.clear();

        for (const P& d : dir_utils::dir_list)
        {
            const P tgt_p(mon.pos + d);

            if (
                !blocked[tgt_p.x][tgt_p.y] &&
                is_pos_inside(tgt_p, area_allowed))
            {
                dir_bucket.push_back(dir_utils::dir(d));
            }
        }

        if (!dir_bucket.empty())
        {
            dir = dir_bucket[rnd::range(0, dir_bucket.size() - 1)];
        }
    }

    //Valid direction found?
    if (dir != Dir::END)
    {
        mon.move(dir);
        return true;
    }

    return false;
}

bool move_to_tgt_simple(Mon& mon)
{
    if (!mon.is_alive() || !mon.tgt_)
    {
        return false;
    }

    if (mon.aware_counter_ > 0 || map::player->is_leader_of(&mon))
    {
        const P offset  = mon.tgt_->pos - mon.pos;
        const P signs   = offset.signs();

        const P new_pos(mon.pos + signs);

        const bool IS_BLOCKED = map_parse::cell(cell_check::Blocks_actor(mon, true),
                                                new_pos);

        if (!IS_BLOCKED)
        {
            mon.move(dir_utils::dir(signs));
            return true;
        }
    }

    return false;
}

bool step_path(Mon& mon, std::vector<P>& path)
{
    if (mon.is_alive() && !path.empty())
    {
        const P delta = path.back() - mon.pos;

        mon.move(dir_utils::dir(delta));

        return true;
    }

    return false;
}

bool step_to_lair_if_los(Mon& mon, const P& lair_p)
{
    if (mon.is_alive())
    {
        bool blocked[map_w][map_h];

        const R area_check_blocked = fov::get_fov_rect(mon.pos);

        map_parse::run(cell_check::Blocks_los(),
                       blocked,
                       Map_parse_mode::overwrite,
                       area_check_blocked);

        const Los_result los = fov::check_cell(mon.pos,
                                               lair_p,
                                               blocked);

        if (!los.is_blocked_hard)
        {
            const P d       = (lair_p - mon.pos).signs();
            const P tgt_p   = mon.pos + d;

            const bool IS_BLOCKED = map_parse::cell(cell_check::Blocks_actor(mon, true), tgt_p);

            if (IS_BLOCKED)
            {
                return false;
            }
            else //Step is not blocked
            {
                mon.move(dir_utils::dir(d));
                return true;
            }
        }
    }

    return false;
}

} //action

namespace info
{

bool look_become_player_aware(Mon& mon)
{
    if (!mon.is_alive())
    {
        return false;
    }

    const bool WAS_AWARE_BEFORE = mon.aware_counter_ > 0;

    std::vector<Actor*> seen_foes;

    mon.seen_foes(seen_foes);

    if (!seen_foes.empty() && WAS_AWARE_BEFORE)
    {
        mon.become_aware_player(false);
        return false;
    }

    for (Actor* actor : seen_foes)
    {
        if (actor->is_player())
        {
            if (mon.is_spotting_sneaking_actor(*actor))
            {
                mon.become_aware_player(true);

                if (WAS_AWARE_BEFORE)
                {
                    return false;
                }
                else //Was not aware before
                {
                    game_time::tick();
                    return true;
                }
            }
        }
        else //Other actor is monster
        {
            mon.become_aware_player(false);

            if (WAS_AWARE_BEFORE)
            {
                return false;
            }
            else //Was not aware before
            {
                game_time::tick();
                return true;
            }
        }
    }

    return false;
}

void try_set_path_to_lair_if_no_los(Mon& mon, std::vector<P>& path, const P& lair_p)
{
    if (mon.is_alive())
    {
        bool blocked[map_w][map_h];

        const R fov_lmt = fov::get_fov_rect(mon.pos);

        map_parse::run(cell_check::Blocks_los(),
                       blocked,
                       Map_parse_mode::overwrite,
                       fov_lmt);

        const Los_result los = fov::check_cell(mon.pos, lair_p, blocked);

        if (!los.is_blocked_hard)
        {
            path.clear();
            return;
        }

        map_parse::run(cell_check::Blocks_actor(mon, false),
                       blocked);

        map_parse::run(cell_check::Living_actors_adj_to_pos(mon.pos),
                       blocked,
                       Map_parse_mode::append);

        pathfind::run(mon.pos,
                      lair_p,
                      blocked,
                      path);
        
        return;
    }

    path.clear();
}

void try_set_path_to_leader(Mon& mon, std::vector<P>& path)
{
    if (mon.is_alive())
    {
        Actor* leader = mon.leader_;

        if (leader && leader->is_alive())
        {
            bool blocked[map_w][map_h];

            const R fov_lmt = fov::get_fov_rect(mon.pos);

            map_parse::run(cell_check::Blocks_los(),
                           blocked,
                           Map_parse_mode::overwrite,
                           fov_lmt);

            const Los_result los = fov::check_cell(mon.pos, leader->pos, blocked);

            if (!los.is_blocked_hard)
            {
                path.clear();
                return;
            }

            map_parse::run(cell_check::Blocks_actor(mon, false), blocked);

            map_parse::run(cell_check::Living_actors_adj_to_pos(mon.pos),
                           blocked,
                           Map_parse_mode::append);

            pathfind::run(mon.pos,
                          leader->pos,
                          blocked,
                          path);
            return;
        }
    }

    path.clear();
}

void try_set_path_to_player(Mon& mon, std::vector<P>& path)
{
    if (!mon.is_alive() || mon.aware_counter_ <= 0)
    {
        path.clear();
        return;
    }

    bool blocked[map_w][map_h] = {};

    const int X0 = 1;
    const int Y0 = 1;
    const int X1 = map_w - 1;
    const int Y1 = map_h - 1;

    //Mark blocking features in the blocking array
    for (int x = X0; x < X1; ++x)
    {
        for (int y = Y0; y < Y1; ++y)
        {
            const auto* const f = map::cells[x][y].rigid;

            if (!f->can_move(mon))
            {
                if (f->id() == Feature_id::door)
                {
                    //Mark doors as blocked depending on if the monster can bash or open doors,

                    const Actor_data_t& d = mon.data();

                    //TODO: What if there is a monster that can open doors but not bash them,
                    //and the door is stuck?

                    if (!d.can_open_doors && !d.can_bash_doors)
                    {
                        blocked[x][y] = true;
                    }
                }
                else //Not a door (e.g. a wall)
                {
                    blocked[x][y] = true;
                }
            }
        }
    }

    //If there is an unblocked LOS between the monster and the player we cancel the pathfinding.
    //The monster should not use the pathfinder to move towards the player in this case. If the
    //player is invisible for example, we *do* want pathfinding as long as the monster is aware,
    //and is around corner (they are guided by sound or something else) - but when they come into
    //LOS of an invisible player, they should not approach further.
    //This creates a pretty cool effect, where monsters appear a bit confused that they cannot see
    //anyone when they should have come into sight.
    const P& player_pos = map::player->pos;

    Los_result los_result = fov::check_cell(mon.pos, player_pos, blocked);

    if (los_result.is_blocked_hard || los_result.is_blocked_by_drk)
    {
        //Append living adjacent actors to the blocking array
        map_parse::run(cell_check::Living_actors_adj_to_pos(mon.pos),
                       blocked,
                       Map_parse_mode::append);

        //Find a path
        pathfind::run(mon.pos,
                      player_pos,
                      blocked,
                      path);
    }
}

void set_special_blocked_cells(Mon& mon, bool a[map_w][map_h])
{
    (void)mon;
    (void)a;
}

} //info

} //ai

