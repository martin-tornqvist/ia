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
#include "text_format.hpp"

namespace ai
{

namespace action
{

bool try_cast_random_spell(Mon& mon)
{
    const auto& prop_handler = mon.prop_handler();

    if (!mon.is_alive() ||
        mon.spells_known_.empty() ||
        !prop_handler.allow_cast_intr_spell_absolute(Verbosity::silent))
    {
        return false;
    }

    rnd::shuffle(mon.spells_known_);

    for (Spell* const spell : mon.spells_known_)
    {
        int& current_cooldown =
            mon.spell_cooldowns_[(size_t)spell->id()];

        if ((current_cooldown <= 0) &&
            spell->allow_mon_cast_now(mon))
        {
            const int current_spi = mon.spi();
            const int spell_max_spi = spell->spi_cost(&mon).max;

            const int current_hp = mon.hp();
            const int max_hp = mon.hp_max(true);

            const bool has_spi = spell_max_spi < current_spi;

            const bool is_hostile_player = !map::player->is_leader_of(&mon);

            const bool is_low_hp = current_hp < (max_hp / 3);

            // Only cast the spell if monster has enough spirit - or sometimes
            // try anyway if the monster has low HP and is hostile to the player
            if (has_spi ||
                (is_hostile_player &&
                 is_low_hp &&
                 rnd::one_in(20)))
            {
                if (!has_spi &&
                    map::player->can_see_actor(mon))
                {
                    const std::string mon_name_the =
                        text_format::first_to_upper(mon.name_the());

                    msg_log::add(mon_name_the + " looks desperate.");
                }

                current_cooldown = spell->mon_cooldown();

                const auto skill = mon.spell_skill(spell->id());

                spell->cast(&mon,
                            skill,
                            IsIntrinsic::yes);

                return true;
            }
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

    if (f->id() == FeatureId::door)
    {
        Door* const door = static_cast<Door*>(f);

        if (!door->can_move(mon))
        {
            const bool is_stuck = door->is_stuck();

            const bool mon_can_bash = mon.data().can_bash_doors;

            const bool mon_can_open = mon.data().can_open_doors;

            // Open the door?
            if (mon_can_open && !is_stuck)
            {
                door->try_open(&mon);
                return true;
            }

            // Bash the door?
            if (mon_can_bash &&
                (is_stuck || !mon_can_open))
            {
                // When bashing doors, give the bashing monster some bonus
                // awareness time (because monsters trying to bash down doors is
                // a pretty central part of the game, and they should not give
                // up so easily)
                if (rnd::fraction(3, 5))
                {
                    ++mon.aware_of_player_counter_;
                }

                if (map::player->can_see_actor(mon))
                {
                    const std::string mon_name_the =
                        text_format::first_to_upper(
                            mon.name_the());

                    const std::string door_name = door->base_name_short();

                    msg_log::add(mon_name_the +
                                 " bashes at the " +
                                 door_name +
                                 "!");
                }

                door->hit(1, // Doesn't matter
                          DmgType::physical,
                          DmgMethod::blunt,
                          &mon);

                game_time::tick();

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

// Helper function(s) for make_room_for_friend()
namespace
{

// Check if position is on a line between two points
bool is_pos_on_line(const P& p,
                    const P& line_p0,
                    const P& line_p1)
{
    // First, do a cheaper check and just see if we are outside a rectangle
    // defined by the two points. If we are outside this area, we can't
    // possibly be on a line between the points.
     const R r(std::min(line_p0.x, line_p1.x),
              std::min(line_p0.y, line_p1.y),
              std::max(line_p0.x, line_p1.x),
              std::max(line_p0.y, line_p1.y));

    if (!is_pos_inside(p, r))
    {
        return false;
    }

    // OK, we could be on the line!

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

// Returns all free positions around the acting monster that is further from the
// player than the monster's current position
void move_bucket(Mon& mon, std::vector<P>& dirs_to_mk)
{
    dirs_to_mk.clear();

    const P& mon_p = mon.pos;
    const P& player_p = map::player->pos;

    bool blocked[map_w][map_h];

    const R area_to_check_blocked(mon_p - P(1, 1), mon_p + P(1, 1));

    map_parsers::BlocksActor(mon, ParseActors::yes)
        .run(blocked,
             MapParseMode::overwrite,
             area_to_check_blocked);

    for (const P& d : dir_utils::dir_list)
    {
        const P tgt_p = mon_p + d;

        const int current_dist_to_player = king_dist(mon_p, player_p);

        const int tgt_dist_to_player = king_dist(tgt_p, player_p);

        if (tgt_dist_to_player <= current_dist_to_player &&
            !blocked[tgt_p.x][tgt_p.y])
        {
            dirs_to_mk.push_back(tgt_p);
        }
    }
}

} // namespace

bool make_room_for_friend(Mon& mon)
{
    if (!mon.is_alive())
    {
        return false;
    }

    bool blocked_los[map_w][map_h];

    map_parsers::BlocksLos()
        .run(blocked_los);

    if (!mon.can_see_actor(*map::player, blocked_los))
    {
        return false;
    }

    const P& player_p = map::player->pos;

    // Check if there is an allied monster that we should move away for
    for (Actor* other_actor : game_time::actors)
    {
        if (other_actor != &mon &&
            other_actor->is_alive() &&
            !other_actor->is_player() &&
            !map::player->is_leader_of(other_actor))
        {
            Mon* const other_mon = static_cast<Mon*>(other_actor);

            const bool is_other_adj =
                is_pos_adj(mon.pos, other_mon->pos, false);

            // TODO: It's probably better to check LOS than vision here? We
            //       don't want to move out of the way for a blind monster.
            const bool is_other_seeing_player =
                other_mon->can_see_actor(*map::player, blocked_los);

            /*
             Do we have this situation?
             #####
             #.A.#
             #@#B#
             #####
             */
            const bool is_other_adj_with_no_player_los =
                is_other_adj && !is_other_seeing_player;

            // We consider moving out of the way if the other monster EITHER:
            //  * Is seeing the player and we are blocking it, OR
            //  * Is adjacent to us, and is not seeing the player.
            const bool is_between =
                is_pos_on_line(mon.pos, other_mon->pos, player_p);

            if ((is_other_seeing_player && is_between) ||
                is_other_adj_with_no_player_los)
            {
                // We are blocking a friend! Try to find an adjacent free cell,
                // which:
                // * Is NOT further away from the player than our current
                //   position, and
                // * Is not also blocking another monster

                // NOTE: We do not care whether the target cell has LOS to the
                //       player or not. If we move into a cell without LOS, it
                //       will appear as if we are dodging in and out of cover.
                //       It lets us move towards the player with less time in
                //       the player's LOS, and allows blocked ranged monsters to
                //       shoot at the player.

                // Get a list of neighbouring free cells
                std::vector<P> pos_bucket;
                move_bucket(mon, pos_bucket);

                // Sort the list by distance to player
                IsCloserToPos cmp(player_p);
                sort(pos_bucket.begin(), pos_bucket.end(), cmp);

                // Try to find a position not blocking a third allied monster
                for (const auto& tgt_p : pos_bucket)
                {
                    bool is_p_ok = true;

                    for (Actor* actor3 : game_time::actors)
                    {
                        // NOTE: The third actor here can include the original
                        //       blocked "other" actor, since we must also check
                        //       if we block that actor from the target position
                        if (actor3 != &mon &&
                            actor3->is_alive() &&
                            !actor3->is_player() &&
                            !map::player->is_leader_of(actor3))
                        {
                            Mon* const mon3 = static_cast<Mon*>(actor3);

                            const bool other_is_seeing_player =
                                mon3->can_see_actor(*map::player, blocked_los);

                            // TODO: We also need to check that we don't move
                            //       into a cell which is adjacent to a third
                            //       monster, who does not have LOS to player!
                            //       As it is now, we may move out of the way
                            //       for one such monster, only to block
                            //       another in the same way!

                            if (other_is_seeing_player &&
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
    if (!mon.is_alive() ||
        (!mon.is_roaming_allowed_ && (mon.aware_of_player_counter_ <= 0)))
    {
        return false;
    }

    bool blocked[map_w][map_h];

    const R parse_area(mon.pos - 1, mon.pos + 1);

    map_parsers::BlocksActor(mon, ParseActors::yes)
        .run(blocked,
             MapParseMode::overwrite,
             parse_area);

    const R area_allowed(P(1, 1), P(map_w - 2, map_h - 2));

    // First, try the same direction as last travelled
    Dir dir = Dir::END;

    const Dir last_dir_travelled = mon.last_dir_moved_;

    if (last_dir_travelled != Dir::center &&
        last_dir_travelled != Dir::END)
    {
        const P tgt_p(mon.pos + dir_utils::offset(last_dir_travelled));

        if (!blocked[tgt_p.x][tgt_p.y] &&
            is_pos_inside(tgt_p, area_allowed))
        {
            dir = last_dir_travelled;
        }
    }

    // Attempt to find a random non-blocked adjacent cell
    if (dir == Dir::END)
    {
        std::vector<Dir> dir_bucket;
        dir_bucket.clear();

        for (const P& d : dir_utils::dir_list)
        {
            const P tgt_p(mon.pos + d);

            if (!blocked[tgt_p.x][tgt_p.y] &&
                is_pos_inside(tgt_p, area_allowed))
            {
                dir_bucket.push_back(dir_utils::dir(d));
            }
        }

        if (!dir_bucket.empty())
        {
            const size_t idx = rnd::range(0, dir_bucket.size() - 1);

            dir = dir_bucket[idx];
        }
    }

    // Valid direction found?
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

    if (mon.aware_of_player_counter_ > 0 ||
        map::player->is_leader_of(&mon))
    {
        const P offset = mon.tgt_->pos - mon.pos;
        const P signs = offset.signs();

        const P new_pos(mon.pos + signs);

        const bool is_blocked =
            map_parsers::BlocksActor(mon, ParseActors::yes)
            .cell(new_pos);

        if (!is_blocked)
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

        map_parsers::BlocksLos()
            .run(blocked,
                 MapParseMode::overwrite,
                 area_check_blocked);

        const LosResult los = fov::check_cell(mon.pos,
                                               lair_p,
                                               blocked);

        if (!los.is_blocked_hard)
        {
            const P d = (lair_p - mon.pos).signs();

            const P tgt_p = mon.pos + d;

            const bool is_blocked =
                map_parsers::BlocksActor(mon, ParseActors::yes)
                .cell(tgt_p);

            if (is_blocked)
            {
                return false;
            }
            else // Step is not blocked
            {
                mon.move(dir_utils::dir(d));
                return true;
            }
        }
    }

    return false;
}

} // action

namespace info
{

bool look(Mon& mon)
{
    if (!mon.is_alive())
    {
        return false;
    }

    const bool was_aware_before = mon.aware_of_player_counter_ > 0;

    auto seen_foes = mon.seen_foes();

    if (seen_foes.empty())
    {
        return false;
    }

    if (was_aware_before)
    {
        mon.become_aware_player(false);

        return false;
    }

    for (Actor* actor : seen_foes)
    {
        if (actor->is_player())
        {
            const auto result = actor->roll_sneak(mon);

            std::cout << mon.name_a() << ": " << (int)result << std::endl;

            const bool is_non_critical_fail =
                (result == ActionResult::fail) ||
                (result == ActionResult::fail_big);

            // Become aware if we got a critical fail, OR if we got a
            // non-critical (normal/big) fail, and we were already wary
            const bool become_aware =
                (result == ActionResult::fail_critical) ||
                (is_non_critical_fail &&
                 mon.wary_of_player_counter_);

            if (become_aware)
            {
                mon.become_aware_player(true);
            }
            // Not aware, just become wary if non-critical fail
            else if (is_non_critical_fail)
            {
                mon.become_wary_player();
            }
        }
        else // Other actor is monster
        {
            mon.become_aware_player(false);
        }

        // Did the monster become aware?
        if (mon.aware_of_player_counter_ > 0)
        {
            return true;
        }
    }

    return false;
}

void find_path_to_lair_if_no_los(Mon& mon,
                                 std::vector<P>& path,
                                 const P& lair_p)
{
    if (mon.is_alive())
    {
        bool blocked[map_w][map_h];

         const R fov_lmt = fov::get_fov_rect(mon.pos);

        map_parsers::BlocksLos()
            .run(blocked,
                 MapParseMode::overwrite,
                 fov_lmt);

        const LosResult los = fov::check_cell(mon.pos,
                                              lair_p,
                                              blocked);

        if (!los.is_blocked_hard)
        {
            path.clear();
            return;
        }

        map_parsers::BlocksActor(mon, ParseActors::no)
            .run(blocked);

        map_parsers::LivingActorsAdjToPos(mon.pos)
            .run(blocked,
                 MapParseMode::append);

        pathfind(mon.pos,
                 lair_p,
                 blocked,
                 path);

        return;
    }

    path.clear();
}

void find_path_to_leader(Mon& mon, std::vector<P>& path)
{
    if (!mon.is_alive())
    {
        return;
    }

    Actor* leader = mon.leader_;

    if (!leader || !leader->is_alive())
    {
        return;
    }

    bool blocked[map_w][map_h];

    const R fov_lmt = fov::get_fov_rect(mon.pos);

    map_parsers::BlocksLos()
        .run(blocked,
             MapParseMode::overwrite,
             fov_lmt);

    const LosResult los =
        fov::check_cell(mon.pos,
                        leader->pos,
                        blocked);

    if (!los.is_blocked_hard)
    {
        path.clear();
        return;
    }

    map_parsers::BlocksActor(mon, ParseActors::no)
        .run(blocked);

    map_parsers::LivingActorsAdjToPos(mon.pos)
        .run(blocked,
             MapParseMode::append);

    pathfind(mon.pos,
             leader->pos,
             blocked,
             path);
    return;

    path.clear();
}

void find_path_to_player(Mon& mon, std::vector<P>& path)
{
    if (!mon.is_alive() || mon.aware_of_player_counter_ <= 0)
    {
        path.clear();
        return;
    }

    //
    // Monsters should not pathfind to the player if there is LOS, but they
    // cannot see the player (e.g. the player is invisible).
    //
    // If the player is invisible for example, we want pathfinding as long as
    // the monster is aware and is around a corner (i.e. they are guided by
    // sound or something else) - but when they come into LOS of an invisible
    // player, they should not approach further.
    //
    // This creates a pretty cool effect, where monsters appear a bit confused
    // that they cannot see anyone when they should have come into sight.
    //
    bool blocked[map_w][map_h];

    const int los_x0 = std::min(map::player->pos.x, mon.pos.x);
    const int los_y0 = std::min(map::player->pos.y, mon.pos.y);
    const int los_x1 = std::max(map::player->pos.x, mon.pos.x);
    const int los_y1 = std::max(map::player->pos.y, mon.pos.y);

    map_parsers::BlocksLos()
        .run(blocked,
             MapParseMode::overwrite,
             R(los_x0, los_y0, los_x1, los_y1));

    const bool is_seeing_player =
        mon.can_see_actor(*map::player, blocked);

    if (!is_seeing_player)
    {
        LosResult los_result =
            fov::check_cell(mon.pos,
                            map::player->pos,
                            blocked);

        if (!los_result.is_blocked_hard &&
            !los_result.is_blocked_by_drk)
        {
            return;
        }
    }

    // Monster does not have LOS to player - alright, let's go!

    const int x0 = 1;
    const int y0 = 1;
    const int x1 = map_w - 1;
    const int y1 = map_h - 1;

    for (int x = x0; x < x1; ++x)
    {
        for (int y = y0; y < y1; ++y)
        {
            blocked[x][y] = false;

            const auto* const f = map::cells[x][y].rigid;

            if (f->can_move(mon))
            {
                continue;
            }

            // Doors are only blocked if the monster cannot open or bash
            if (f->id() == FeatureId::door)
            {
                const auto* const door = static_cast<const Door*>(f);

                // Metal doors are always blocking
                if (door->type() == DoorType::metal)
                {
                    blocked[x][y] = true;

                    continue;
                }

                // Not a metal door

                //
                // TODO: What if there is a monster that can open
                //       doors but not bash, and the door is stuck?
                //

                // Consider non-metal doors as free if monster can open or bash
                const ActorDataT& d = mon.data();

                if (d.can_open_doors ||
                    d.can_bash_doors)
                {
                    continue;
                }
            }

            // Not a door (e.g. a wall)
            blocked[x][y] = true;
        }
    }

    // Append living adjacent actors to the blocking array
    map_parsers::LivingActorsAdjToPos(mon.pos)
        .run(blocked,
             MapParseMode::append);

    // Find a path
    pathfind(mon.pos,
             map::player->pos,
             blocked,
             path);
}

void set_special_blocked_cells(Mon& mon, bool a[map_w][map_h])
{
    (void)mon;
    (void)a;
}

} // info

} // ai
