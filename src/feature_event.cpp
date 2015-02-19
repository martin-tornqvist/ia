#include "feature_event.h"

#include <assert.h>

#include "actor_player.h"
#include "actor_mon.h"
#include "map.h"
#include "actor_factory.h"
#include "log.h"
#include "render.h"
#include "utils.h"
#include "feature_rigid.h"
#include "popup.h"
#include "sdl_wrapper.h"

using namespace std;

//------------------------------------------------------------------- EVENT
Event::Event(const Pos& pos) :
    Mob(pos) {}

//------------------------------------------------------------------- WALL CRUMBLE
Event_wall_crumble::Event_wall_crumble(const Pos& pos, vector<Pos>& walls,
                                   vector<Pos>& inner) :
    Event       (pos),
    wall_cells_  (walls),
    inner_cells_ (inner) {}

void Event_wall_crumble::on_new_turn()
{
    if (Utils::is_pos_adj(Map::player->pos, pos_, true))
    {
        //Check that it still makes sense to run the crumbling
        auto check_cells_have_wall = [](const vector<Pos>& cells)
        {
            for (const Pos& p : cells)
            {
                const auto f_id = Map::cells[p.x][p.y].rigid->get_id();
                if (f_id != Feature_id::wall && f_id != Feature_id::rubble_high)
                {
                    return false;
                }
            }
            return true;
        };

        if (check_cells_have_wall(wall_cells_) && check_cells_have_wall(inner_cells_))
        {
            if (Map::player->get_prop_handler().allow_see())
            {
                Log::add_msg("Suddenly, the walls collapse!", clr_white, false, true);
            }

            //Crumble
            bool done = false;
            while (!done)
            {
                for (const Pos& p : wall_cells_)
                {
                    if (Utils::is_pos_inside(p, Rect(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2))))
                    {
                        auto* const f = Map::cells[p.x][p.y].rigid;
                        f->hit(Dmg_type::physical, Dmg_method::forced, nullptr);
                    }
                }

                bool is_opening_made = true;
                for (const Pos& p : wall_cells_)
                {
                    if (Utils::is_pos_adj(Map::player->pos, p, true))
                    {
                        Rigid* const f = Map::cells[p.x][p.y].rigid;
                        if (!f->can_move_cmn())
                        {
                            is_opening_made = false;
                        }
                    }
                }

                done = is_opening_made;
            }

            //Spawn things
            int       nr_mon_limit_except_adj_to_entry  = 9999;
            Actor_id   mon_type                     = Actor_id::zombie;
            const int RND                         = Rnd::range(1, 5);

            switch (RND)
            {
            case 1:
            {
                mon_type = Actor_id::zombie;
                nr_mon_limit_except_adj_to_entry = 4;
            } break;

            case 2:
            {
                mon_type = Actor_id::zombie_axe;
                nr_mon_limit_except_adj_to_entry = 3;
            } break;

            case 3:
            {
                mon_type = Actor_id::bloated_zombie;
                nr_mon_limit_except_adj_to_entry = 1;
            } break;

            case 4:
            {
                mon_type = Actor_id::rat;
                nr_mon_limit_except_adj_to_entry = 30;
            } break;

            case 5:
            {
                mon_type = Actor_id::rat_thing;
                nr_mon_limit_except_adj_to_entry = 20;
            } break;

            default: {} break;
            }
            int nr_mon_spawned = 0;

            random_shuffle(begin(inner_cells_), end(inner_cells_));

            for (const Pos& p : inner_cells_)
            {
                Map::put(new Floor(p));

                if (Rnd::one_in(5))
                {
                    Map::mk_gore(p);
                    Map::mk_blood(p);
                }

                if (nr_mon_spawned < nr_mon_limit_except_adj_to_entry || Utils::is_pos_adj(p, pos_, false))
                {
                    Actor*  const actor = Actor_factory::mk(mon_type, p);
                    Mon*    const mon   = static_cast<Mon*>(actor);
                    mon->aware_counter_  = mon->get_data().nr_turns_aware;
                    ++nr_mon_spawned;
                }
            }

            Map::player->update_fov();
            Render::draw_map_and_interface();

            Map::player->incr_shock(Shock_lvl::heavy, Shock_src::see_mon);
        }
        Game_time::erase_mob(this, true);
    }
}

//------------------------------------------------------------------- RITW DISCOVERY
Event_rats_in_the_walls_discovery::Event_rats_in_the_walls_discovery(const Pos& pos) :
    Event(pos) {}

void Event_rats_in_the_walls_discovery::on_new_turn()
{
    //Run the event if player is at the event position or to the right of it. If it's the
    //latter case, it means the player somehow bypassed it (e.g. teleport or dynamite),
    //it should not be possible to use this as a "cheat" to avoid the shock.
    if (Map::player->pos == pos_ || Map::player->pos.x > pos_.x)
    {
        Map::player->update_fov();
        Render::draw_map_and_interface();

        const string str =
            "Before me lies a twilit grotto of enormous height. An insane tangle of human "
            "bones extends for yards like a foamy sea - invariably in postures of demoniac "
            "frenzy, either fighting off some menace or clutching other forms with "
            "cannibal intent.";

        Popup::show_msg(str, false, "A gruesome discovery...");

        Map::player->incr_shock(100, Shock_src::misc);

        for (Actor* const actor : Game_time::actors_)
        {
            if (!actor->is_player())
            {
                static_cast<Mon*>(actor)->is_roaming_allowed_ = true;
            }
        }

        Game_time::erase_mob(this, true);
    }
}
