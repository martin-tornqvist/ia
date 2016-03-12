#include "feature_event.hpp"

#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "actor_factory.hpp"
#include "msg_log.hpp"
#include "render.hpp"
#include "utils.hpp"
#include "feature_rigid.hpp"
#include "popup.hpp"
#include "sdl_wrapper.hpp"
#include "init.hpp"

//------------------------------------------------------------------- EVENT
Event::Event(const P& feature_pos) :
    Mob(feature_pos) {}

//------------------------------------------------------------------- WALL CRUMBLE
Event_wall_crumble::Event_wall_crumble(const P& p,
                                       std::vector<P>& walls,
                                       std::vector<P>& inner) :
    Event           (p),
    wall_cells_     (walls),
    inner_cells_    (inner) {}

void Event_wall_crumble::on_new_turn()
{
    if (utils::is_pos_adj(map::player->pos, pos_, true))
    {
        //Check that it still makes sense to run the crumbling
        auto check_cells_have_wall = [](const std::vector<P>& cells)
        {
            for (const P& p : cells)
            {
                const auto f_id = map::cells[p.x][p.y].rigid->id();

                if (f_id != Feature_id::wall && f_id != Feature_id::rubble_high)
                {
                    return false;
                }
            }

            return true;
        };

        if (check_cells_have_wall(wall_cells_) && check_cells_have_wall(inner_cells_))
        {
            if (map::player->prop_handler().allow_see())
            {
                msg_log::add("Suddenly, the walls collapse!",
                             clr_msg_note,
                             false,
                             More_prompt_on_msg::yes);
            }

            //Crumble
            bool done = false;

            while (!done)
            {
                for (const P& p : wall_cells_)
                {
                    if (utils::is_pos_inside(p, Rect(P(1, 1), P(MAP_W - 2, MAP_H - 2))))
                    {
                        auto* const f = map::cells[p.x][p.y].rigid;
                        f->hit(Dmg_type::physical, Dmg_method::forced, nullptr);
                    }
                }

                bool is_opening_made = true;

                for (const P& p : wall_cells_)
                {
                    if (utils::is_pos_adj(map::player->pos, p, true))
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

            //Spawn things
            int         nr_mon_limit_except_adj_to_entry    = 9999;
            Actor_id    mon_type                            = Actor_id::zombie;
            const int   RND                                 = rnd::range(1, 5);

            switch (RND)
            {
            case 1:
                mon_type = Actor_id::zombie;
                nr_mon_limit_except_adj_to_entry = 4;
                break;

            case 2:
                mon_type = Actor_id::zombie_axe;
                nr_mon_limit_except_adj_to_entry = 3;
                break;

            case 3:
                mon_type = Actor_id::bloated_zombie;
                nr_mon_limit_except_adj_to_entry = 2;
                break;

            case 4:
                mon_type = Actor_id::rat;
                nr_mon_limit_except_adj_to_entry = 30;
                break;

            case 5:
                mon_type = Actor_id::rat_thing;
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

                if (
                    nr_mon_spawned < nr_mon_limit_except_adj_to_entry ||
                    utils::is_pos_adj(p, pos_, false))
                {
                    Actor*  const actor = actor_factory::mk(mon_type, p);
                    Mon*    const mon   = static_cast<Mon*>(actor);
                    mon->aware_counter_ = mon->data().nr_turns_aware;
                    ++nr_mon_spawned;
                }
            }

            map::player->update_fov();
            render::draw_map_and_interface();

            map::player->incr_shock(Shock_lvl::heavy, Shock_src::see_mon);
        }

        game_time::erase_mob(this, true);
    }
}

//------------------------------------------------------------------- SNAKE EMERGE
Event_snake_emerge::Event_snake_emerge() :
    Event (P(-1, -1)) {}

bool Event_snake_emerge::try_find_p()
{
    bool blocked[MAP_W][MAP_H];

    blocked_cells(Rect(0, 0, MAP_W - 1, MAP_H - 1), blocked);

    std::vector<P> p_bucket;

    utils::mk_vector_from_bool_map(false, blocked, p_bucket);

    random_shuffle(begin(p_bucket), end(p_bucket));

    std::vector<P> emerge_bucket;

    for (const P& p : p_bucket)
    {
        emerge_p_bucket(p, blocked, emerge_bucket);

        if (emerge_bucket.size() >= MIN_NR_SNAKES_)
        {
            pos_ = p;
            return true;
        }
    }

    return false;
}

Rect Event_snake_emerge::allowed_emerge_rect(const P& p) const
{
    const int MAX_D = allowed_emerge_dist_range.max;

    const int X0 = std::max(1,          p.x - MAX_D);
    const int Y0 = std::max(1,          p.y - MAX_D);
    const int X1 = std::min(MAP_W - 2,  p.x + MAX_D);
    const int Y1 = std::min(MAP_H - 2,  p.y + MAX_D);

    return Rect(X0, Y0, X1, Y1);
}

bool Event_snake_emerge::is_ok_feature_at(const P& p) const
{
    IA_ASSERT(utils::is_pos_inside_map(p, true));

    const Feature_id id = map::cells[p.x][p.y].rigid->id();

    return id == Feature_id::floor ||
           id == Feature_id::rubble_low;
}

void Event_snake_emerge::emerge_p_bucket(
    const P& p,
    bool blocked[MAP_W][MAP_H],
    std::vector<P>& out) const
{
    Los_result fov[MAP_W][MAP_H];

    fov::run(p, blocked, fov);

    const Rect r = allowed_emerge_rect(p);

    for (int x = r.p0.x; x <= r.p1.x; ++x)
    {
        for (int y = r.p0.y; y <= r.p1.y; ++y)
        {
            const P tgt_p(x, y);

            const int MIN_D = allowed_emerge_dist_range.min;

            if (
                !blocked[x][y]              &&
                !fov[x][y].is_blocked_hard  &&
                utils::king_dist(p, tgt_p) >= MIN_D)
            {
                out.push_back(tgt_p);
            }
        }
    }
}

void Event_snake_emerge::blocked_cells(const Rect& r, bool out[MAP_W][MAP_H]) const
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

void Event_snake_emerge::on_new_turn()
{
    if (map::player->pos != pos_)
    {
        return;
    }

    const Rect r = allowed_emerge_rect(pos_);

    bool blocked[MAP_W][MAP_H];

    blocked_cells(r, blocked);

    std::vector<P> tgt_bucket;

    emerge_p_bucket(pos_, blocked, tgt_bucket);

    if (tgt_bucket.size() < MIN_NR_SNAKES_)
    {
        //Not possible to spawn at least minimum required number of snakes
        return;
    }

    int max_nr_snakes = MIN_NR_SNAKES_ + (map::dlvl / 4);

    //Cap max number of snakes to the size of the target bucket
    //NOTE: The target bucket is at least as big as the minimum required number of snakes
    max_nr_snakes = std::min(max_nr_snakes, int(tgt_bucket.size()));

    random_shuffle(begin(tgt_bucket), end(tgt_bucket));

    std::vector<Actor_id> id_bucket;

    for (Actor_data_t d : actor_data::data)
    {
        if (d.is_snake)
        {
            id_bucket.push_back(d.id);
        }
    }

    const size_t    IDX = rnd::range(0, id_bucket.size() - 1);
    const Actor_id  id  = id_bucket[IDX];

    const size_t NR_SUMMONED = rnd::range(MIN_NR_SNAKES_, max_nr_snakes);

    std::vector<P> seen_tgt_positions;

    for (size_t i = 0; i < NR_SUMMONED; ++i)
    {
        IA_ASSERT(i < tgt_bucket.size());

        const P& p(tgt_bucket[i]);

        if (map::cells[p.x][p.y].is_seen_by_player)
        {
            seen_tgt_positions.push_back(p);
        }
    }

    if (!seen_tgt_positions.empty())
    {
        msg_log::add("Suddenly, vicious snakes slither up from cracks in the floor!",
                     clr_msg_note,
                     true,
                     More_prompt_on_msg::yes);

        render::draw_blast_at_cells(seen_tgt_positions, clr_magenta);

        map::player->incr_shock(Shock_lvl::some, Shock_src::see_mon);
    }

    for (size_t i = 0; i < NR_SUMMONED; ++i)
    {
        const P& p(tgt_bucket[i]);

        Actor* const actor = actor_factory::mk(id, p);

        static_cast<Mon*>(actor)->become_aware_player(false);
    }

    render::draw_map_and_interface(true);

    game_time::erase_mob(this, true);
}

//------------------------------------------------------------------- RITW DISCOVERY
Event_rats_in_the_walls_discovery::Event_rats_in_the_walls_discovery(const P& feature_pos) :
    Event(feature_pos) {}

void Event_rats_in_the_walls_discovery::on_new_turn()
{
    //Run the event if player is at the event position or to the right of it. If it's the
    //latter case, it means the player somehow bypassed it (e.g. teleport or dynamite),
    //it should not be possible to use this as a "cheat" to avoid the shock.
    if (map::player->pos == pos_ || map::player->pos.x > pos_.x)
    {
        map::player->update_fov();
        render::draw_map_and_interface();

        const std::string str =
            "Before me lies a twilit grotto of enormous height. An insane tangle of human "
            "bones extends for yards like a foamy sea - invariably in postures of demoniac "
            "frenzy, either fighting off some menace or clutching other forms with "
            "cannibal intent.";

        popup::show_msg(str, false, "A gruesome discovery...");

        map::player->incr_shock(100, Shock_src::misc);

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
