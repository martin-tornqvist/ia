#include "throwing.hpp"

#include <vector>
#include <cassert>

#include "item.hpp"
#include "item_potion.hpp"
#include "actor_data.hpp"
#include "actor_player.hpp"
#include "render.hpp"
#include "map.hpp"
#include "msg_log.hpp"
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

namespace throwing
{

void player_throw_lit_explosive(const Pos& aim_cell)
{
    assert(map::player->active_explosive);

    vector<Pos> path;
    line_calc::calc_new_line(map::player->pos, aim_cell, true, THROW_RANGE_LMT, false, path);

    //Remove cells after blocked cells
    for (size_t i = 1; i < path.size(); ++i)
    {
        const Pos   p = path[i];
        const auto* f = map::cells[p.x][p.y].rigid;

        if (!f->is_projectile_passable())
        {
            path.resize(i);
            break;
        }
    }

    const Pos end_pos(path.empty() ? Pos() : path.back());

    auto* const explosive = map::player->active_explosive;

    msg_log::add(explosive->str_on_player_throw());

    //Render
    if (path.size() > 1)
    {
        const auto  clr = explosive->ignited_projectile_clr();

        for (const Pos& p : path)
        {
            render::draw_map_and_interface(false);

            if (map::cells[p.x][p.y].is_seen_by_player)
            {
                if (config::is_tiles_mode())
                {
                    render::draw_tile(explosive->tile(), Panel::map, p, clr);
                }
                else //ASCII mode
                {
                    render::draw_glyph(explosive->glyph(), Panel::map, p, clr);
                }

                render::update_screen();
                sdl_wrapper::sleep(config::delay_projectile_draw());
            }
        }
    }

    if (!map::cells[end_pos.x][end_pos.y].rigid->is_bottomless())
    {
        explosive->on_thrown_ignited_landing(end_pos);
    }

    delete explosive;
    map::player->active_explosive = nullptr;

    game_time::tick();
}

void throw_item(Actor& actor_throwing, const Pos& tgt_cell, Item& item_thrown)
{
    Throw_att_data data(&actor_throwing, tgt_cell, actor_throwing.pos, item_thrown);

    const Actor_size aim_lvl = data.intended_aim_lvl;

    vector<Pos> path;
    line_calc::calc_new_line(actor_throwing.pos, tgt_cell, false, THROW_RANGE_LMT, false, path);

    const auto& item_thrown_data = item_thrown.data();

    const string item_name_a = item_thrown.name(Item_ref_type::a);

    if (&actor_throwing == map::player)
    {
        msg_log::clear();
        msg_log::add("I throw " + item_name_a + ".");
    }
    else //Monster throwing
    {
        const Pos& p = path.front();

        if (map::cells[p.x][p.y].is_seen_by_player)
        {
            msg_log::add(actor_throwing.name_the() + " throws " + item_name_a + ".");
        }
    }

    render::draw_map_and_interface(true);

    int         blocked_in_element      = -1;
    bool        is_actor_hit            = false;
    const char  glyph                   = item_thrown.glyph();
    const Clr   clr                     = item_thrown.clr();
    int         chance_to_destroy_item  = 0;

    Pos cur_pos(-1, -1);

    for (size_t i = 1; i < path.size(); ++i)
    {
        render::draw_map_and_interface(false);

        cur_pos.set(path[i]);

        Actor* const actor_here = utils::actor_at_pos(cur_pos);

        if (
            actor_here &&
            (cur_pos == tgt_cell || actor_here->data().actor_size >= Actor_size::humanoid))
        {
            data = Throw_att_data(&actor_throwing, tgt_cell, cur_pos, item_thrown, aim_lvl);

            if (data.attack_result >= success_small && !data.is_ethereal_defender_missed)
            {
                if (map::cells[cur_pos.x][cur_pos.y].is_seen_by_player)
                {
                    render::draw_glyph('*', Panel::map, cur_pos, clr_red_lgt);
                    render::update_screen();
                    sdl_wrapper::sleep(config::delay_projectile_draw() * 4);
                }

                const Clr hit_message_clr = actor_here == map::player ? clr_msg_bad : clr_msg_good;

                const bool CAN_SEE_ACTOR = map::player->can_see_actor(*actor_here, nullptr);

                const string defender_name = CAN_SEE_ACTOR ? actor_here->name_the() : "It";

                msg_log::add(defender_name + " is hit.", hit_message_clr);

                actor_here->hit(data.dmg, Dmg_type::physical);
                is_actor_hit = true;

                //If throwing a potion on an actor, let it make stuff happen
                if (item_thrown_data.type == Item_type::potion)
                {
                    static_cast<Potion*>(&item_thrown)->on_collide(cur_pos, actor_here);
                    delete &item_thrown;
                    game_time::tick();
                    return;
                }

                blocked_in_element      = i;
                chance_to_destroy_item  = 25;
                break;
            }
        }

        if (map::cells[cur_pos.x][cur_pos.y].is_seen_by_player)
        {
            render::draw_glyph(glyph, Panel::map, cur_pos, clr);
            render::update_screen();
            sdl_wrapper::sleep(config::delay_projectile_draw());
        }

        const auto* feature_here = map::cells[cur_pos.x][cur_pos.y].rigid;

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
            static_cast<Potion*>(&item_thrown)->on_collide(path[blocked_in_element], nullptr);
            delete &item_thrown;
            game_time::tick();
            return;
        }
    }

    if (rnd::percent(chance_to_destroy_item))
    {
        delete &item_thrown;
    }
    else //Thrown item not destroyed
    {
        const int   DROP_ELEMENT        = blocked_in_element == -1 ?
                                          path.size() - 1 : blocked_in_element;

        const Pos   drop_pos            = path[DROP_ELEMENT];
        const Matl  matl_at_drop_pos    = map::cells[drop_pos.x][drop_pos.y].rigid->matl();
        bool        is_noisy            = false;

        switch (matl_at_drop_pos)
        {
        case Matl::empty:
            is_noisy = false;
            break;

        case Matl::stone:
            is_noisy = true;
            break;

        case Matl::metal:
            is_noisy = true;
            break;

        case Matl::plant:
            is_noisy = false;
            break;

        case Matl::wood:
            is_noisy = true;
            break;

        case Matl::cloth:
            is_noisy = false;
            break;

        case Matl::fluid:
            is_noisy = false;
            break;
        }

        if (is_noisy)
        {
            const Alerts_mon alerts = &actor_throwing == map::player ?
                                      Alerts_mon::yes :
                                      Alerts_mon::no;

            if (!is_actor_hit)
            {
                Snd snd(item_thrown_data.land_on_hard_snd_msg, item_thrown_data.land_on_hard_sfx,
                        Ignore_msg_if_origin_seen::yes, drop_pos, nullptr, Snd_vol::low, alerts);

                snd_emit::emit_snd(snd);
            }
        }

        item_drop::drop_item_on_map(drop_pos, item_thrown);
    }

    render::draw_map_and_interface();
    game_time::tick();
}

} //Throwing
