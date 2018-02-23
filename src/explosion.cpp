#include "explosion.hpp"

#include "io.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "map_parsing.hpp"
#include "sdl_base.hpp"
#include "line_calc.hpp"
#include "actor_player.hpp"
#include "sdl_base.hpp"
#include "player_bon.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "game.hpp"
#include "property.hpp"
#include "property_factory.hpp"

namespace
{

std::vector< std::vector<P> > cells_reached(
    const R& area,
    const P& origin,
    const ExplExclCenter exclude_center,
    bool blocked[map_w][map_h])
{
    std::vector< std::vector<P> > out;

    for (int y = area.p0.y; y <= area.p1.y; ++y)
    {
        for (int x = area.p0.x; x <= area.p1.x; ++x)
        {
            const P pos(x, y);

            if (exclude_center == ExplExclCenter::yes &&
                pos == origin)
            {
                continue;
            }

            const int dist = king_dist(pos, origin);

            bool is_reached = true;

            if (dist > 1)
            {
                    const auto path =
                        line_calc::calc_new_line(
                            origin,
                            pos,
                            true,
                            999,
                            false);

                for (const P& pos_check_block : path)
                {
                    if (blocked[pos_check_block.x][pos_check_block.y])
                    {
                        is_reached = false;
                        break;
                    }
                }
            }

            if (is_reached)
            {
                if ((int)out.size() <= dist)
                {
                    out.resize(dist + 1);
                }

                out[dist].push_back(pos);
            }
        }
    }

    return out;
}

void draw(const std::vector< std::vector<P> >& pos_lists,
          bool blocked[map_w][map_h],
          const Color color_override)
{
    states::draw();

    const Color& color_inner =
        color_override.is_defined() ?
        color_override :
        colors::yellow();

    const Color& color_outer =
        color_override.is_defined() ?
        color_override :
        colors::light_red();

    const bool is_tiles = config::is_tiles_mode();

    const int nr_anim_steps = is_tiles ? 2 : 1;

    bool is_any_cell_seen_by_player = false;

    for (int i_anim = 0; i_anim < nr_anim_steps; i_anim++)
    {
        const TileId tile =
            (i_anim == 0) ?
            TileId::blast1 : TileId::blast2;

        const int nr_outer = pos_lists.size();

        for (int i_outer = 0; i_outer < nr_outer; i_outer++)
        {
            const Color& color =
                (i_outer == nr_outer - 1) ?
                color_outer : color_inner;

            const std::vector<P>& inner = pos_lists[i_outer];

            for (const P& pos : inner)
            {
                if (map::cells[pos.x][pos.y].is_seen_by_player &&
                    !blocked[pos.x][pos.y])
                {
                    is_any_cell_seen_by_player = true;

                    if (is_tiles)
                    {
                        io::draw_tile(tile,
                                      Panel::map,
                                      pos,
                                      color,
                                      colors::black());
                    }
                    else // Text mode
                    {
                        io::draw_character('*',
                                           Panel::map,
                                           pos,
                                           color,
                                           true,
                                           colors::black());
                    }
                }
            }
        }

        if (is_any_cell_seen_by_player)
        {
            io::update_screen();

            sdl_base::sleep(config::delay_explosion() / nr_anim_steps);
        }
    }
}

} // namespace


namespace explosion
{

void run(const P& origin,
         const ExplType expl_type,
         const EmitExplSnd emit_expl_snd,
         const int radi_change,
         const ExplExclCenter exclude_center,
         std::vector<Prop*> properties_applied,
         const Color color_override,
         const ExplIsGas is_gas)
{
    const int radi = expl_std_radi + radi_change;

    const R area = explosion_area(origin, radi);

    bool blocked[map_w][map_h];

    map_parsers::BlocksProjectiles().
        run(blocked,
            MapParseMode::overwrite,
            area);

    auto pos_lists = cells_reached(area,
                                   origin,
                                   exclude_center,
                                   blocked);

    if (emit_expl_snd == EmitExplSnd::yes)
    {
        Snd snd("I hear an explosion!",
                SfxId::explosion,
                IgnoreMsgIfOriginSeen::yes,
                origin,
                nullptr,
                SndVol::high,
                AlertsMon::yes);

        snd_emit::run(snd);
    }

    draw(pos_lists, blocked, color_override);

    // Do damage, apply effect
    Actor* living_actors[map_w][map_h];

    std::vector<Actor*> corpses[map_w][map_h];

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            living_actors[x][y] = nullptr;
            corpses[x][y].clear();
        }
    }

    for (Actor* actor : game_time::actors)
    {
        const P& pos = actor->pos;

        if (actor->is_alive())
        {
            living_actors[pos.x][pos.y] = actor;
        }
        else if (actor->is_corpse())
        {
            corpses[pos.x][pos.y].push_back(actor);
        }
    }

    const int nr_outer = pos_lists.size();

    for (int radi = 0; radi < nr_outer; ++radi)
    {
        const std::vector<P>& positions_at_radi = pos_lists[radi];

        for (const P& pos : positions_at_radi)
        {
            Actor* living_actor = living_actors[pos.x][pos.y];

            std::vector<Actor*> corpses_here = corpses[pos.x][pos.y];

            if (expl_type == ExplType::expl)
            {
                const int rolls = expl_dmg_rolls - radi;

                const int dmg =
                    rnd::dice(rolls, expl_dmg_sides) +
                    expl_dmg_plus;

                // Damage environment
                Cell& cell = map::cells[pos.x][pos.y];

                cell.rigid->hit(dmg,
                                DmgType::physical,
                                DmgMethod::explosion,
                                nullptr);

                // Damage living actor
                if (living_actor)
                {
                    if (living_actor->is_player())
                    {
                        msg_log::add("I am hit by an explosion!",
                                     colors::msg_bad());
                    }

                    living_actor->hit(dmg, DmgType::physical);

                    if (living_actor->is_alive() && living_actor->is_player())
                    {
                        // Player survived being hit by an explosion, that's
                        // pretty cool!
                        game::add_history_event("Survived an explosion.");
                    }
                }

                // Damage dead actors
                for (Actor* corpse : corpses_here)
                {
                    corpse->hit(dmg, DmgType::physical);
                }

                // Add smoke
                if (rnd::fraction(6, 10))
                {
                    game_time::add_mob(new Smoke(pos, rnd::range(2, 4)));
                }
            }

            // Apply properties
            for (auto* prop : properties_applied)
            {
                bool should_apply_on_living_actor = true;

                if (living_actor)
                {
                    if (is_gas == ExplIsGas::yes)
                    {
                        if (living_actor->has_prop(PropId::r_breath))
                        {
                            should_apply_on_living_actor = false;
                        }

                        if (living_actor == map::player)
                        {
                            // Do not apply effect if wearing Gas Mask, and this
                            // is a gas explosion
                            const Item* const head_item =
                                map::player->inv().item_in_slot(SlotId::head);

                            if ((is_gas == ExplIsGas::yes) &&
                                head_item &&
                                (head_item->id() == ItemId::gas_mask))
                            {
                                should_apply_on_living_actor = false;
                            }
                        }
                    }
                }
                else // No living actor here
                {
                    should_apply_on_living_actor = false;
                }

                if (should_apply_on_living_actor)
                {
                    Prop* const prop_cpy = property_factory::make(prop->id());

                    prop_cpy->set_duration(prop->nr_turns_left());

                    living_actor->apply_prop(prop_cpy);
                }

                // If property is burning, also apply it to corpses and the
                // environment
                if (prop->id() == PropId::burning)
                {
                    Cell& cell = map::cells[pos.x][pos.y];

                    cell.rigid->hit(1, // Doesn't matter
                                    DmgType::fire,
                                    DmgMethod::elemental,
                                    nullptr);

                    for (Actor* corpse : corpses_here)
                    {
                        Prop* const prop_cpy = property_factory::make(prop->id());

                        prop_cpy->set_duration(prop->nr_turns_left());

                        corpse->properties().apply(prop_cpy);
                    }
                }
            }
        }
    }

    for (auto* prop : properties_applied)
    {
        delete prop;
    }

    map::update_vision();

} // run

void run_smoke_explosion_at(const P& origin, const int radi_change)
{
    const int radi = expl_std_radi + radi_change;

    const R area = explosion_area(origin, radi);

    bool blocked[map_w][map_h];

    map_parsers::BlocksProjectiles()
        .run(blocked,
             MapParseMode::overwrite,
             area);

    auto pos_lists = cells_reached(area,
                                   origin,
                                   ExplExclCenter::no,
                                   blocked);

    // TODO: Sound message?
    Snd snd("",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            origin,
            nullptr,
            SndVol::low,
            AlertsMon::yes);

    snd_emit::run(snd);

    for (const std::vector<P>& inner : pos_lists)
    {
        for (const P& pos : inner)
        {
            if (!blocked[pos.x][pos.y])
            {
                game_time::add_mob(new Smoke(pos, rnd::range(25, 30)));
            }
        }
    }

    map::update_vision();
}

R explosion_area(const P& c, const int radi)
{
    return R(P(std::max(c.x - radi, 1),
               std::max(c.y - radi, 1)),
             P(std::min(c.x + radi, map_w - 2),
               std::min(c.y + radi, map_h - 2)));
}

} // explosion
