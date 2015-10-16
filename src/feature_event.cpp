#include "feature_event.hpp"

#include <cassert>

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

using namespace std;

//------------------------------------------------------------------- EVENT
Event::Event(const P& feature_pos) :
    Mob(feature_pos) {}

//------------------------------------------------------------------- WALL CRUMBLE
Event_wall_crumble::Event_wall_crumble(const P& feature_pos, vector<P>& walls,
                                       vector<P>& inner) :
    Event(feature_pos),
    wall_cells_(walls),
    inner_cells_(inner) {}

void Event_wall_crumble::on_new_turn()
{
    if (utils::is_pos_adj(map::player->pos, pos_, true))
    {
        //Check that it still makes sense to run the crumbling
        auto check_cells_have_wall = [](const vector<P>& cells)
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
                msg_log::add("Suddenly, the walls collapse!", clr_white, false,
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
                nr_mon_limit_except_adj_to_entry = 1;
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

                if (nr_mon_spawned < nr_mon_limit_except_adj_to_entry || utils::is_pos_adj(p, pos_, false))
                {
                    Actor*  const actor = actor_factory::mk(mon_type, p);
                    Mon*    const mon   = static_cast<Mon*>(actor);
                    mon->aware_counter_  = mon->data().nr_turns_aware;
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

        const string str =
            "Before me lies a twilit grotto of enormous height. An insane tangle of human "
            "bones extends for yards like a foamy sea - invariably in postures of demoniac "
            "frenzy, either fighting off some menace or clutching other forms with "
            "cannibal intent.";

        popup::show_msg(str, false, "A gruesome discovery...");

        map::player->incr_shock(100, Shock_src::misc);

        for (Actor* const actor : game_time::actors_)
        {
            if (!actor->is_player())
            {
                static_cast<Mon*>(actor)->is_roaming_allowed_ = true;
            }
        }

        game_time::erase_mob(this, true);
    }
}
