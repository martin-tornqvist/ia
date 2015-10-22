#include "explosion.hpp"

#include "render.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "map_parsing.hpp"
#include "sdl_wrapper.hpp"
#include "line_calc.hpp"
#include "actor_player.hpp"
#include "utils.hpp"
#include "sdl_wrapper.hpp"
#include "player_bon.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "dungeon_master.hpp"

namespace
{

void draw(const std::vector< std::vector<P> >& pos_lists,
          bool blocked[MAP_W][MAP_H],
          const Clr* const clr_override)
{
    render::draw_map_and_interface();

    const Clr& clr_inner = clr_override ? *clr_override : clr_yellow;
    const Clr& clr_outer = clr_override ? *clr_override : clr_red_lgt;

    const bool IS_TILES     = config::is_tiles_mode();
    const int NR_ANIM_STEPS = IS_TILES ? 2 : 1;

    bool is_any_cell_seen_by_player = false;

    for (int i_anim = 0; i_anim < NR_ANIM_STEPS; i_anim++)
    {

        const Tile_id tile = i_anim == 0 ? Tile_id::blast1 : Tile_id::blast2;

        const int NR_OUTER = pos_lists.size();

        for (int i_outer = 0; i_outer < NR_OUTER; i_outer++)
        {
            const Clr& clr = i_outer == NR_OUTER - 1 ? clr_outer : clr_inner;
            const std::vector<P>& inner = pos_lists[i_outer];

            for (const P& pos : inner)
            {
                if (map::cells[pos.x][pos.y].is_seen_by_player && !blocked[pos.x][pos.y])
                {
                    is_any_cell_seen_by_player = true;

                    if (IS_TILES)
                    {
                        render::draw_tile(tile, Panel::map, pos, clr, clr_black);
                    }
                    else
                    {
                        render::draw_glyph('*', Panel::map, pos, clr, true, clr_black);
                    }
                }
            }
        }

        if (is_any_cell_seen_by_player)
        {
            render::update_screen();
            sdl_wrapper::sleep(config::delay_explosion() / NR_ANIM_STEPS);
        }
    }
}

void explosion_area(const P& c, const int RADI, Rect& rect_ref)
{
    rect_ref = Rect(P(std::max(c.x - RADI, 1),         std::max(c.y - RADI, 1)),
                    P(std::min(c.x + RADI, MAP_W - 2), std::min(c.y + RADI, MAP_H - 2)));
}

void cells_reached(const Rect& area, const P& origin,
                   bool blocked[MAP_W][MAP_H],
                   std::vector< std::vector<P> >& pos_list_ref)
{
    std::vector<P> line;

    for (int y = area.p0.y; y <= area.p1.y; ++y)
    {
        for (int x = area.p0.x; x <= area.p1.x; ++x)
        {
            const P pos(x, y);
            const int DIST = utils::king_dist(pos, origin);
            bool is_reached = true;

            if (DIST > 1)
            {
                line_calc::calc_new_line(origin, pos, true, 999, false, line);

                for (P& pos_check_block : line)
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
                if (int(pos_list_ref.size()) <= DIST) {pos_list_ref.resize(DIST + 1);}

                pos_list_ref[DIST].push_back(pos);
            }
        }
    }
}

} //namespace


namespace explosion
{

void run(const P& origin,
         const Expl_type expl_type,
         const Expl_src expl_src,
         const Emit_expl_snd emit_expl_snd,
         const int RADI_CHANGE,
         Prop* const prop,
         const Clr* const clr_override)
{
    Rect area;

    const int RADI = EXPLOSION_STD_RADI + RADI_CHANGE;
    explosion_area(origin, RADI, area);

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_projectiles(), blocked);

    std::vector< std::vector<P> > pos_lists;
    cells_reached(area, origin, blocked, pos_lists);

    if (emit_expl_snd == Emit_expl_snd::yes)
    {
        Snd snd("I hear an explosion!",
                Sfx_id::explosion,
                Ignore_msg_if_origin_seen::yes,
                origin,
                nullptr,
                Snd_vol::high,
                Alerts_mon::yes);

        snd_emit::run(snd);
    }

    draw(pos_lists, blocked, clr_override);

    //----------------------------------------------------------
    //Do damage, apply effect

    Actor* living_actors[MAP_W][MAP_H];
    std::vector<Actor*> corpses[MAP_W][MAP_H];

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
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

    const bool IS_DEM_EXP = player_bon::traits[size_t(Trait::dem_expert)];

    const int NR_OUTER = pos_lists.size();

    for (int cur_radi = 0; cur_radi < NR_OUTER; cur_radi++)
    {
        const std::vector<P>& positions_at_cur_radi = pos_lists[cur_radi];

        for (const P& pos : positions_at_cur_radi)
        {

            Actor* living_actor                 = living_actors[pos.x][pos.y];
            std::vector<Actor*> corpses_here    = corpses[pos.x][pos.y];

            if (expl_type == Expl_type::expl)
            {
                //Damage environment
                Cell& cell = map::cells[pos.x][pos.y];
                cell.rigid->hit(Dmg_type::physical, Dmg_method::explosion, nullptr);

                const int ROLLS = EXPL_DMG_ROLLS - cur_radi;
                const int DMG   = rnd::dice(ROLLS, EXPL_DMG_SIDES) + EXPL_DMG_PLUS;

                //Damage living actor
                if (living_actor)
                {
                    if (living_actor->is_player())
                    {
                        msg_log::add("I am hit by an explosion!", clr_msg_bad);
                    }

                    living_actor->hit(DMG, Dmg_type::physical);

                    if (living_actor->is_alive() && living_actor->is_player())
                    {
                        //Player survived being hit by an explosion, that's pretty cool!
                        dungeon_master::add_history_event("Survived an explosion.");
                    }
                }

                //Damage dead actors
                for (Actor* corpse : corpses_here) {corpse->hit(DMG, Dmg_type::physical);}

                //Add smoke
                if (rnd::fraction(6, 10)) {game_time::add_mob(new Smoke(pos, rnd::range(2, 4)));}
            }

            //Apply property
            if (prop)
            {
                bool should_apply_on_living_actor = living_actor;

                //Do not apply burning if actor is player with the demolition expert trait, and
                //intentionally throwing a Molotov
                if (
                    living_actor == map::player     &&
                    prop->id() == Prop_id::burning  &&
                    IS_DEM_EXP                      &&
                    expl_src == Expl_src::player_use_moltv_intended)
                {
                    should_apply_on_living_actor = false;
                }

                if (should_apply_on_living_actor)
                {
                    Prop_handler& prop_hlr = living_actor->prop_handler();
                    Prop* prop_cpy = prop_hlr.mk_prop(prop->id(), Prop_turns::specific,
                                                      prop->nr_turns_left());
                    prop_hlr.try_add_prop(prop_cpy);
                }

                //If property is burning, also apply it to corpses and environment
                if (prop->id() == Prop_id::burning)
                {
                    Cell& cell = map::cells[pos.x][pos.y];
                    cell.rigid->hit(Dmg_type::fire, Dmg_method::elemental, nullptr);

                    for (Actor* corpse : corpses_here)
                    {
                        Prop_handler& prop_hlr = corpse->prop_handler();
                        Prop* prop_cpy = prop_hlr.mk_prop(prop->id(), Prop_turns::specific,
                                                          prop->nr_turns_left());
                        prop_hlr.try_add_prop(prop_cpy);
                    }
                }
            }
        }
    }

    game_time::update_light_map();

    map::player->update_fov();

    render::draw_map_and_interface();

    if (prop)
    {
        delete prop;
    }
}

void run_smoke_explosion_at(const P& origin)
{
    Rect area;
    const int RADI = EXPLOSION_STD_RADI;
    explosion_area(origin, RADI, area);

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_projectiles(), blocked);

    std::vector< std::vector<P> > pos_lists;
    cells_reached(area, origin, blocked, pos_lists);

    //TODO: Sound message?
    Snd snd("", Sfx_id::END, Ignore_msg_if_origin_seen::yes, origin, nullptr,
            Snd_vol::low, Alerts_mon::yes);
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

    map::player->update_fov();

    render::draw_map_and_interface();
}

} //Explosion

