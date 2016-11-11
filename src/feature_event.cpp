#include "feature_event.hpp"

#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "actor_factory.hpp"
#include "msg_log.hpp"
#include "io.hpp"
#include "feature_rigid.hpp"
#include "popup.hpp"
#include "sdl_base.hpp"
#include "init.hpp"

// -----------------------------------------------------------------------------
// Event
// -----------------------------------------------------------------------------
Event::Event(const P& feature_pos) :
    Mob(feature_pos) {}

// -----------------------------------------------------------------------------
// Wall crumble
// -----------------------------------------------------------------------------
EventWallCrumble::EventWallCrumble(const P& p,
                                   std::vector<P>& walls,
                                   std::vector<P>& inner) :
    Event           (p),
    wall_cells_     (walls),
    inner_cells_    (inner) {}

void EventWallCrumble::on_new_turn()
{
    if (!is_pos_adj(map::player->pos, pos_, true))
    {
        return;
    }

    // Check that it still makes sense to run the crumbling
    auto check_cells_have_wall = [](const std::vector<P>& cells)
    {
        for (const P& p : cells)
        {
            const auto f_id = map::cells[p.x][p.y].rigid->id();

            if (f_id != FeatureId::wall &&
                f_id != FeatureId::rubble_high)
            {
                return false;
            }
        }

        return true;
    };

    if (check_cells_have_wall(wall_cells_) &&
        check_cells_have_wall(inner_cells_))
    {
        if (map::player->prop_handler().allow_see())
        {
            msg_log::add("Suddenly, the walls collapse!",
                         clr_msg_note,
                         false,
                         MorePromptOnMsg::yes);
        }

        // Crumble
        bool done = false;

        while (!done)
        {
            for (const P& p : wall_cells_)
            {
                if (is_pos_inside(p, R(P(1, 1), P(map_w - 2, map_h - 2))))
                {
                    auto* const f = map::cells[p.x][p.y].rigid;

                    f->hit(DmgType::physical, DmgMethod::forced, nullptr);
                }
            }

            bool is_opening_made = true;

            for (const P& p : wall_cells_)
            {
                if (is_pos_adj(map::player->pos, p, true))
                {
                    Rigid* const f = map::cells[p.x][p.y].rigid;

                    if (!f->can_move_cmn())
                    {
                        is_opening_made = false;
                    }
                }
            }

            done = is_opening_made;
        }

        // Spawn things
        int         nr_mon_limit_except_adj_to_entry    = 9999;
        ActorId     mon_type                            = ActorId::zombie;
        const int   rnd                                 = rnd::range(1, 5);

        switch (rnd)
        {
        case 1:
            mon_type = ActorId::zombie;
            nr_mon_limit_except_adj_to_entry = 4;
            break;

        case 2:
            mon_type = ActorId::zombie_axe;
            nr_mon_limit_except_adj_to_entry = 3;
            break;

        case 3:
            mon_type = ActorId::bloated_zombie;
            nr_mon_limit_except_adj_to_entry = 2;
            break;

        case 4:
            mon_type = ActorId::rat;
            nr_mon_limit_except_adj_to_entry = 30;
            break;

        case 5:
            mon_type = ActorId::rat_thing;
            nr_mon_limit_except_adj_to_entry = 20;
            break;

        default:
            break;
        }

        int nr_mon_spawned = 0;

        random_shuffle(begin(inner_cells_), end(inner_cells_));

        for (const P& p : inner_cells_)
        {
            map::put(new Floor(p));

            if (rnd::one_in(5))
            {
                map::mk_gore(p);
                map::mk_blood(p);
            }

            if (nr_mon_spawned < nr_mon_limit_except_adj_to_entry ||
                is_pos_adj(p, pos_, false))
            {
                Actor*  const actor = actor_factory::mk(mon_type, p);
                Mon*    const mon   = static_cast<Mon*>(actor);
                mon->aware_counter_ = mon->data().nr_turns_aware;
                ++nr_mon_spawned;
            }
        }

        map::player->incr_shock(ShockLvl::terrifying,
                                ShockSrc::see_mon);
    }

    game_time::erase_mob(this, true);
}

// -----------------------------------------------------------------------------
// Snake emerge
// -----------------------------------------------------------------------------
EventSnakeEmerge::EventSnakeEmerge() :
    Event (P(-1, -1)) {}

bool EventSnakeEmerge::try_find_p()
{
    bool blocked[map_w][map_h];

    blocked_cells(R(0, 0, map_w - 1, map_h - 1), blocked);

    std::vector<P> p_bucket;

    to_vec(blocked, false, p_bucket);

    random_shuffle(begin(p_bucket), end(p_bucket));

    std::vector<P> emerge_bucket;

    for (const P& p : p_bucket)
    {
        emerge_p_bucket(p, blocked, emerge_bucket);

        if (emerge_bucket.size() >= min_nr_snakes_)
        {
            pos_ = p;
            return true;
        }
    }

    return false;
}

R EventSnakeEmerge::allowed_emerge_rect(const P& p) const
{
    const int max_d = allowed_emerge_dist_range.max;

    const int X0 = std::max(1,          p.x - max_d);
    const int Y0 = std::max(1,          p.y - max_d);
    const int X1 = std::min(map_w - 2,  p.x + max_d);
    const int Y1 = std::min(map_h - 2,  p.y + max_d);

    return R(X0, Y0, X1, Y1);
}

bool EventSnakeEmerge::is_ok_feature_at(const P& p) const
{
    ASSERT(map::is_pos_inside_map(p, true));

    const FeatureId id = map::cells[p.x][p.y].rigid->id();

    return id == FeatureId::floor ||
           id == FeatureId::rubble_low;
}

void EventSnakeEmerge::emerge_p_bucket(
    const P& p,
    bool blocked[map_w][map_h],
    std::vector<P>& out) const
{
    LosResult fov[map_w][map_h];

    fov::run(p, blocked, fov);

     const R r = allowed_emerge_rect(p);

    for (int x = r.p0.x; x <= r.p1.x; ++x)
    {
        for (int y = r.p0.y; y <= r.p1.y; ++y)
        {
            const P tgt_p(x, y);

            const int min_d = allowed_emerge_dist_range.min;

            if (
                !blocked[x][y]              &&
                !fov[x][y].is_blocked_hard  &&
                king_dist(p, tgt_p) >= min_d)
            {
                out.push_back(tgt_p);
            }
        }
    }
}

void EventSnakeEmerge::blocked_cells(const R& r, bool out[map_w][map_h]) const
{
    for (int x = r.p0.x; x <= r.p1.x; ++x)
    {
        for (int y = r.p0.y; y <= r.p1.y; ++y)
        {
            const P p(x, y);

            out[x][y] = !is_ok_feature_at(p);
        }
    }

    for (Actor* const actor : game_time::actors)
    {
        const P& p = actor->pos;

        out[p.x][p.y] = true;
    }
}

void EventSnakeEmerge::on_new_turn()
{
    if (map::player->pos != pos_)
    {
        return;
    }

    const R r = allowed_emerge_rect(pos_);

    bool blocked[map_w][map_h];

    blocked_cells(r, blocked);

    std::vector<P> tgt_bucket;

    emerge_p_bucket(pos_, blocked, tgt_bucket);

    if (tgt_bucket.size() < min_nr_snakes_)
    {
        //Not possible to spawn at least minimum required number of snakes
        return;
    }

    int max_nr_snakes = min_nr_snakes_ + (map::dlvl / 4);

    // Cap max number of snakes to the size of the target bucket
    //
    // NOTE: The target bucket is at least as big as the minimum required number
    //       of snakes
    max_nr_snakes = std::min(max_nr_snakes, int(tgt_bucket.size()));

    random_shuffle(begin(tgt_bucket), end(tgt_bucket));

    std::vector<ActorId> id_bucket;

    for (ActorDataT d : actor_data::data)
    {
        if (d.is_snake)
        {
            id_bucket.push_back(d.id);
        }
    }

    const size_t    idx = rnd::range(0, id_bucket.size() - 1);
    const ActorId   id  = id_bucket[idx];

    const size_t nr_summoned = rnd::range(min_nr_snakes_, max_nr_snakes);

    std::vector<P> seen_tgt_positions;

    for (size_t i = 0; i < nr_summoned; ++i)
    {
        ASSERT(i < tgt_bucket.size());

        const P& p(tgt_bucket[i]);

        if (map::cells[p.x][p.y].is_seen_by_player)
        {
            seen_tgt_positions.push_back(p);
        }
    }

    if (!seen_tgt_positions.empty())
    {
        msg_log::add("Suddenly, vicious snakes slither up "
                     "from cracks in the floor!",
                     clr_msg_note,
                     true,
                     MorePromptOnMsg::yes);

        io::draw_blast_at_cells(seen_tgt_positions, clr_magenta);

        ShockLvl shock_lvl = ShockLvl::unsettling;

        if (insanity::has_sympt(InsSymptId::phobia_reptile_and_amph))
        {
            shock_lvl = ShockLvl::terrifying;
        }

        map::player->incr_shock(shock_lvl,
                                ShockSrc::see_mon);
    }

    for (size_t i = 0; i < nr_summoned; ++i)
    {
        const P& p(tgt_bucket[i]);

        Actor* const actor = actor_factory::mk(id, p);

        static_cast<Mon*>(actor)->become_aware_player(false);
    }

    game_time::erase_mob(this, true);
}

// -----------------------------------------------------------------------------
// Rats in the walls discovery
// -----------------------------------------------------------------------------
EventRatsInTheWallsDiscovery::EventRatsInTheWallsDiscovery(
    const P& feature_pos) :
    Event(feature_pos) {}

void EventRatsInTheWallsDiscovery::on_new_turn()
{
    // Run the event if player is at the event position or to the right of it.
    // If it's the latter case, it means the player somehow bypassed it (e.g.
    // teleport or dynamite), it should not be possible to use this as a "cheat"
    // to avoid the shock.
    if (map::player->pos == pos_ || map::player->pos.x > pos_.x)
    {
        const std::string str =
            "Before me lies a twilit grotto of enormous height. An insane "
            "tangle of human bones extends for yards like a foamy sea - "
            "invariably in postures of demoniac frenzy, either fighting off "
            "some menace or clutching other forms with cannibal intent.";

        popup::show_msg(str, "A gruesome discovery...");

        map::player->incr_shock(100, ShockSrc::misc);

        for (Actor* const actor : game_time::actors)
        {
            if (!actor->is_player())
            {
                static_cast<Mon*>(actor)->is_roaming_allowed_ = true;
            }
        }

        game_time::erase_mob(this, true);
    }
}
