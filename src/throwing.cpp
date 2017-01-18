#include "throwing.hpp"

#include <vector>

#include "init.hpp"
#include "item.hpp"
#include "item_potion.hpp"
#include "actor_data.hpp"
#include "actor_player.hpp"
#include "io.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "explosion.hpp"
#include "drop.hpp"
#include "inventory.hpp"
#include "item_factory.hpp"
#include "attack.hpp"
#include "line_calc.hpp"
#include "player_bon.hpp"
#include "sdl_base.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"

namespace throwing
{

void player_throw_lit_explosive(const P& aim_cell)
{
    ASSERT(map::player->active_explosive);

    std::vector<P> path;

    line_calc::calc_new_line(map::player->pos,
                             aim_cell,
                             true,
                             throw_range_lmt,
                             false,
                             path);

    // Remove cells after blocked cells
    for (size_t i = 1; i < path.size(); ++i)
    {
        const P   p = path[i];
        const auto* f = map::cells[p.x][p.y].rigid;

        if (!f->is_projectile_passable())
        {
            path.resize(i);
            break;
        }
    }

    const P end_pos(path.empty() ? P() : path.back());

    auto* const explosive = map::player->active_explosive;

    msg_log::add(explosive->str_on_player_throw());

    // Render
    if (path.size() > 1)
    {
        const auto  clr = explosive->ignited_projectile_clr();

        for (const P& p : path)
        {
            states::draw();

            if (map::cells[p.x][p.y].is_seen_by_player)
            {
                if (config::is_tiles_mode())
                {
                    io::draw_tile(explosive->tile(), Panel::map, p, clr);
                }
                else // Text mode
                {
                    io::draw_glyph(explosive->glyph(), Panel::map, p, clr);
                }

                io::update_screen();

                sdl_base::sleep(config::delay_projectile_draw());
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

void throw_item(Actor& actor_throwing,
                const P& tgt_cell,
                Item& item_thrown)
{
    int speed_pct_diff = 0;

    if (&actor_throwing == map::player)
    {
        if (player_bon::traits[(size_t)Trait::adept_marksman])
        {
            speed_pct_diff += 25;
        }

        if (player_bon::traits[(size_t)Trait::expert_marksman])
        {
            speed_pct_diff += 25;
        }
    }

    ThrowAttData att_data(&actor_throwing,
                          tgt_cell,
                          actor_throwing.pos,
                          item_thrown);

    const ActorSize aim_lvl = att_data.intended_aim_lvl;

    std::vector<P> path;

    line_calc::calc_new_line(actor_throwing.pos,
                             tgt_cell,
                             false,
                             throw_range_lmt,
                             false,
                             path);

    const ItemDataT& item_thrown_data = item_thrown.data();

    const std::string item_name_a = item_thrown.name(ItemRefType::a);

    if (&actor_throwing == map::player)
    {
        msg_log::clear();
        msg_log::add("I throw " + item_name_a + ".");
    }
    else //Monster throwing
    {
        const P& p = path.front();

        if (map::cells[p.x][p.y].is_seen_by_player)
        {
            msg_log::add(actor_throwing.name_the() +
                         " throws " +
                         item_name_a + ".");
        }
    }

    states::draw();

    int blocked_idx = -1;

    bool is_actor_hit = false;

    const Clr item_clr = item_thrown.clr();

    int break_item_one_in_n = -1;

    P pos(-1, -1);

    for (size_t i = 1; i < path.size(); ++i)
    {
        states::draw();

        pos.set(path[i]);

        Actor* const actor_here = map::actor_at_pos(pos);

        if (actor_here &&
            (pos == tgt_cell ||
             (actor_here->data().actor_size >= ActorSize::humanoid)))
        {
            att_data = ThrowAttData(&actor_throwing,
                                    tgt_cell,
                                    pos,
                                    item_thrown,
                                    aim_lvl);

            if (att_data.att_result >= ActionResult::success)
            {
                const bool is_pot =
                    item_thrown_data.type == ItemType::potion;

                if (map::player->can_see_actor(*actor_here))
                {
                    const Clr hit_clr = is_pot ? item_clr : clr_red_lgt;

                    io::draw_blast_at_cells({pos}, hit_clr);
                }

                const Clr hit_message_clr =
                    actor_here == map::player ?
                    clr_msg_bad : clr_msg_good;

                const bool can_see_actor =
                    map::player->can_see_actor(*actor_here);

                const std::string defender_name =
                    can_see_actor ?
                    actor_here->name_the() : "It";

                msg_log::add(defender_name + " is hit.", hit_message_clr);

                actor_here->hit(att_data.dmg, DmgType::physical);
                is_actor_hit = true;

                // If throwing a potion on an actor, let it make stuff happen
                if (is_pot)
                {
                    Potion* const potion = static_cast<Potion*>(&item_thrown);

                    potion->on_collide(pos, actor_here);

                    delete &item_thrown;

                    game_time::tick(speed_pct_diff);

                    return;
                }

                blocked_idx = i;

                if (item_thrown_data.type == ItemType::throwing_wpn)
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
                io::draw_tile(item_thrown.tile(),
                                  Panel::map,
                                  pos,
                                  item_clr);
            }
            else //Text mode
            {
                io::draw_glyph(item_thrown.glyph(),
                                   Panel::map,
                                   pos,
                                   item_clr);
            }


            io::update_screen();

            sdl_base::sleep(config::delay_projectile_draw());
        }

        const auto* feature_here = map::cells[pos.x][pos.y].rigid;

        if (!feature_here->is_projectile_passable())
        {
            blocked_idx =
                (item_thrown_data.type == ItemType::potion) ?
                i :
                (i - 1);
            break;
        }

        if (pos == tgt_cell &&
            att_data.intended_aim_lvl == ActorSize::floor)
        {
            blocked_idx = i;
            break;
        }
    }

    //If potion, collide it on the landscape
    if (item_thrown_data.type == ItemType::potion)
    {
        if (blocked_idx >= 0)
        {
            const Clr hit_clr = item_clr;

            io::draw_blast_at_seen_cells({pos}, hit_clr);

            Potion* const potion = static_cast<Potion*>(&item_thrown);

            potion->on_collide(path[blocked_idx], nullptr);

            delete &item_thrown;

            game_time::tick(speed_pct_diff);

            return;
        }
    }

    const int final_idx =
        (blocked_idx == -1) ?
        (path.size() - 1) : blocked_idx;

    const P final_pos = path[final_idx];

    if (break_item_one_in_n != -1 &&
        rnd::one_in(break_item_one_in_n))
    {
        delete &item_thrown;
    }
    else //Thrown item not destroyed
    {
        const Matl matl_at_drop_pos =
            map::cells[final_pos.x][final_pos.y].rigid->matl();

        bool is_noisy = false;

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
            const AlertsMon alerts = &actor_throwing == map::player ?
                                      AlertsMon::yes :
                                      AlertsMon::no;

            if (!is_actor_hit)
            {
                Snd snd(item_thrown_data.land_on_hard_snd_msg,
                        item_thrown_data.land_on_hard_sfx,
                        IgnoreMsgIfOriginSeen::yes,
                        final_pos,
                        nullptr,
                        SndVol::low,
                        alerts);

                snd_emit::run(snd);
            }
        }

        item_drop::drop_item_on_map(final_pos, item_thrown);
    }

    game_time::tick(speed_pct_diff);
}

} // throwing
