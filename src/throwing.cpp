#include "throwing.hpp"

#include <vector>
#include <assert.h>

#include "item.hpp"
#include "item_potion.hpp"
#include "actor_data.hpp"
#include "actor_player.hpp"
#include "render.hpp"
#include "map.hpp"
#include "log.hpp"
#include "explosion.hpp"
#include "drop.hpp"
#include "inventory.hpp"
#include "item_factory.hpp"
#include "attack.hpp"
#include "line_calc.hpp"
#include "player_bon.hpp"
#include "utils.hpp"
#include "sdl_wrapper.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"

using namespace std;

namespace Throwing
{

void player_throw_lit_explosive(const Pos& aim_cell)
{
    assert(Map::player->active_explosive);

    vector<Pos> path;
    Line_calc::calc_new_line(Map::player->pos, aim_cell, true, THROW_RANGE_LMT, false, path);

    //Remove cells after blocked cells
    for (size_t i = 1; i < path.size(); ++i)
    {
        const Pos   p = path[i];
        const auto* f = Map::cells[p.x][p.y].rigid;
        if (!f->is_projectile_passable())
        {
            path.resize(i);
            break;
        }
    }

    const Pos end_pos(path.empty() ? Pos() : path.back());

    auto* const explosive = Map::player->active_explosive;

    Log::add_msg(explosive->get_str_on_player_throw());

    //Render
    if (path.size() > 1)
    {
        const auto  clr = explosive->get_ignited_projectile_clr();

        for (const Pos& p : path)
        {
            Render::draw_map_and_interface(false);
            if (Map::cells[p.x][p.y].is_seen_by_player)
            {
                if (Config::is_tiles_mode())
                {
                    Render::draw_tile(explosive->get_tile(),    Panel::map, p, clr);
                }
                else
                {
                    Render::draw_glyph(explosive->get_glyph(),  Panel::map, p, clr);
                }
                Render::update_screen();
                Sdl_wrapper::sleep(Config::get_delay_projectile_draw());
            }
        }
    }

    if (!Map::cells[end_pos.x][end_pos.y].rigid->is_bottomless())
    {
        explosive->on_thrown_ignited_landing(end_pos);
    }

    delete explosive;
    Map::player->active_explosive = nullptr;

    Game_time::tick();
}

void throw_item(Actor& actor_throwing, const Pos& tgt_cell, Item& item_thrown)
{
    Throw_att_data data(actor_throwing, item_thrown, tgt_cell, actor_throwing.pos);

    const Actor_size aim_lvl = data.intended_aim_lvl;

    vector<Pos> path;
    Line_calc::calc_new_line(actor_throwing.pos, tgt_cell, false, THROW_RANGE_LMT, false, path);

    const auto& item_thrown_data = item_thrown.get_data();

    const string item_name_a = item_thrown.get_name(Item_ref_type::a);

    if (&actor_throwing == Map::player)
    {
        Log::clear_log();
        Log::add_msg("I throw " + item_name_a + ".");
    }
    else
    {
        const Pos& p = path.front();
        if (Map::cells[p.x][p.y].is_seen_by_player)
        {
            Log::add_msg(actor_throwing.get_name_the() + " throws " + item_name_a + ".");
        }
    }
    Render::draw_map_and_interface(true);

    int         blocked_in_element    = -1;
    bool        is_actor_hit          = false;
    const char  glyph               = item_thrown.get_glyph();
    const Clr   clr                 = item_thrown.get_clr();
    int         chance_to_destroy_item = 0;

    Pos cur_pos(-1, -1);

    for (size_t i = 1; i < path.size(); ++i)
    {
        Render::draw_map_and_interface(false);

        cur_pos.set(path[i]);

        Actor* const actor_here = Utils::get_actor_at_pos(cur_pos);
        if (actor_here)
        {
            if (cur_pos == tgt_cell || actor_here->get_data().actor_size >= Actor_size::humanoid)
            {

                data = Throw_att_data(actor_throwing, item_thrown, tgt_cell, cur_pos, aim_lvl);

                if (data.attack_result >= success_small && !data.is_ethereal_defender_missed)
                {

                    if (Map::cells[cur_pos.x][cur_pos.y].is_seen_by_player)
                    {
                        Render::draw_glyph('*', Panel::map, cur_pos, clr_red_lgt);
                        Render::update_screen();
                        Sdl_wrapper::sleep(Config::get_delay_projectile_draw() * 4);
                    }
                    const Clr hit_message_clr   = actor_here == Map::player ? clr_msg_bad : clr_msg_good;
                    const bool CAN_SEE_ACTOR  = Map::player->can_see_actor(*actor_here, nullptr);
                    string defender_name       = CAN_SEE_ACTOR ? actor_here->get_name_the() : "It";

                    Log::add_msg(defender_name + " is hit.", hit_message_clr);

                    actor_here->hit(data.dmg, Dmg_type::physical);
                    is_actor_hit = true;

                    //If throwing a potion on an actor, let it make stuff happen...
                    if (item_thrown_data.type == Item_type::potion)
                    {
                        static_cast<Potion*>(&item_thrown)->collide(cur_pos, actor_here);
                        delete &item_thrown;
                        Game_time::tick();
                        return;
                    }

                    blocked_in_element = i;
                    chance_to_destroy_item = 25;
                    break;
                }
            }
        }

        if (Map::cells[cur_pos.x][cur_pos.y].is_seen_by_player)
        {
            Render::draw_glyph(glyph, Panel::map, cur_pos, clr);
            Render::update_screen();
            Sdl_wrapper::sleep(Config::get_delay_projectile_draw());
        }

        const auto* feature_here = Map::cells[cur_pos.x][cur_pos.y].rigid;
        if (!feature_here->is_projectile_passable())
        {
            blocked_in_element = item_thrown_data.type == Item_type::potion ? i : i - 1;
            break;
        }

        if (cur_pos == tgt_cell && data.intended_aim_lvl == Actor_size::floor)
        {
            blocked_in_element = i;
            break;
        }
    }

    //If potion, collide it on the landscape
    if (item_thrown_data.type == Item_type::potion)
    {
        if (blocked_in_element >= 0)
        {
            static_cast<Potion*>(&item_thrown)->collide(path[blocked_in_element], nullptr);
            delete &item_thrown;
            Game_time::tick();
            return;
        }
    }

    if (Rnd::percent() < chance_to_destroy_item)
    {
        delete &item_thrown;
    }
    else
    {
        const int DROP_ELEMENT = blocked_in_element == -1 ?
                                 path.size() - 1 : blocked_in_element;
        const Pos drop_pos = path[DROP_ELEMENT];
        const Matl matl_at_drop_pos =
            Map::cells[drop_pos.x][drop_pos.y].rigid->get_matl();

        bool is_noisy = false;

        switch (matl_at_drop_pos)
        {
        case Matl::empty:   is_noisy = false;  break;
        case Matl::stone:   is_noisy = true;   break;
        case Matl::metal:   is_noisy = true;   break;
        case Matl::plant:   is_noisy = false;  break;
        case Matl::wood:    is_noisy = true;   break;
        case Matl::cloth:   is_noisy = false;  break;
        case Matl::fluid:   is_noisy = false;  break;
        }

        if (is_noisy)
        {
            const Alerts_mon alerts = &actor_throwing == Map::player ?
                                     Alerts_mon::yes :
                                     Alerts_mon::no;
            if (!is_actor_hit)
            {
                Snd snd(item_thrown_data.land_on_hard_snd_msg, item_thrown_data.land_on_hard_sfx,
                        Ignore_msg_if_origin_seen::yes, drop_pos, nullptr, Snd_vol::low, alerts);

                Snd_emit::emit_snd(snd);
            }
        }
        Item_drop::drop_item_on_map(drop_pos, item_thrown);
    }

    Render::draw_map_and_interface();
    Game_time::tick();
}

} //Throwing
