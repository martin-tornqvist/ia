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

namespace throwing
{

void player_throw_lit_explosive(const Pos& aim_cell)
{
    assert(map::player->active_explosive);

    std::vector<Pos> path;
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
                else //Text mode
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
    Throw_att_data att_data(&actor_throwing, tgt_cell, actor_throwing.pos, item_thrown);

    const Actor_size aim_lvl = att_data.intended_aim_lvl;

    std::vector<Pos> path;

    line_calc::calc_new_line(actor_throwing.pos, tgt_cell, false, THROW_RANGE_LMT, false, path);

    const Item_data_t& item_thrown_data = item_thrown.data();

    const std::string item_name_a = item_thrown.name(Item_ref_type::a);

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

    int         blocked_idx             = -1;
    bool        is_actor_hit            = false;
    const Clr   item_clr                = item_thrown.clr();
    int         break_item_one_in_n     = -1;

    Pos pos(-1, -1);

    for (size_t i = 1; i < path.size(); ++i)
    {
        render::draw_map_and_interface(false);

        pos.set(path[i]);

        Actor* const actor_here = utils::actor_at_pos(pos);

        if (
            actor_here &&
            (pos == tgt_cell || (actor_here->data().actor_size >= Actor_size::humanoid)))
        {
            att_data = Throw_att_data(&actor_throwing, tgt_cell, pos, item_thrown, aim_lvl);

            if (att_data.att_result >= success && !att_data.is_ethereal_defender_missed)
            {
                const bool IS_POT = item_thrown_data.type == Item_type::potion;

                if (map::player->can_see_actor(*actor_here))
                {
                    const Clr hit_clr = IS_POT ? item_clr : clr_red_lgt;

                    render::draw_blast_at_cells({pos}, hit_clr);
                }

                const Clr hit_message_clr = actor_here == map::player ? clr_msg_bad : clr_msg_good;

                const bool CAN_SEE_ACTOR = map::player->can_see_actor(*actor_here);

                const std::string defender_name = CAN_SEE_ACTOR ? actor_here->name_the() : "It";

                msg_log::add(defender_name + " is hit.", hit_message_clr);

                actor_here->hit(att_data.dmg, Dmg_type::physical);
                is_actor_hit = true;

                //If throwing a potion on an actor, let it make stuff happen
                if (IS_POT)
                {
                    Potion* const potion = static_cast<Potion*>(&item_thrown);

                    potion->on_collide(pos, actor_here);

                    delete &item_thrown;

                    game_time::tick();

                    return;
                }

                blocked_idx = i;


                if (item_thrown_data.type == Item_type::throwing_wpn)
                {
                    break_item_one_in_n = 4;
                }

                break;
            }
        }

        if (map::cells[pos.x][pos.y].is_seen_by_player)
        {
            if (config::is_tiles_mode())
            {
                render::draw_tile(item_thrown.tile(), Panel::map, pos, item_clr);
            }
            else //Text mode
            {
                render::draw_glyph(item_thrown.glyph(), Panel::map, pos, item_clr);
            }


            render::update_screen();
            sdl_wrapper::sleep(config::delay_projectile_draw());
        }

        const auto* feature_here = map::cells[pos.x][pos.y].rigid;

        if (!feature_here->is_projectile_passable())
        {
            blocked_idx = (item_thrown_data.type == Item_type::potion) ? i : (i - 1);
            break;
        }

        if (pos == tgt_cell && att_data.intended_aim_lvl == Actor_size::floor)
        {
            blocked_idx = i;
            break;
        }
    }

    //If potion, collide it on the landscape
    if (item_thrown_data.type == Item_type::potion)
    {
        if (blocked_idx >= 0)
        {
            const Clr hit_clr = item_clr;

            render::draw_blast_at_seen_cells({pos}, hit_clr);

            Potion* const potion = static_cast<Potion*>(&item_thrown);

            potion->on_collide(path[blocked_idx], nullptr);

            delete &item_thrown;

            game_time::tick();

            return;
        }
    }

    const int   FINAL_IDX   = blocked_idx == -1 ?
                              (path.size() - 1) : blocked_idx;

    const Pos   final_pos   = path[FINAL_IDX];

    if (break_item_one_in_n != -1 && rnd::one_in(break_item_one_in_n))
    {
        delete &item_thrown;
    }
    else //Thrown item not destroyed
    {
        const Matl  matl_at_drop_pos    = map::cells[final_pos.x][final_pos.y].rigid->matl();
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
                Snd snd(item_thrown_data.land_on_hard_snd_msg,
                        item_thrown_data.land_on_hard_sfx,
                        Ignore_msg_if_origin_seen::yes,
                        final_pos,
                        nullptr,
                        Snd_vol::low,
                        alerts);

                snd_emit::run(snd);
            }
        }

        item_drop::drop_item_on_map(final_pos, item_thrown);
    }

    render::draw_map_and_interface();
    game_time::tick();
}

} //Throwing
