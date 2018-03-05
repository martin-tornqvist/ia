#include "draw_map.hpp"

#include "io.hpp"
#include "map.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "feature_door.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static CellRenderData render_map[map_w][map_h];
static CellRenderData render_map_player_memory[map_w][map_h];

static void clear_render_map()
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        render_map[x][y] = CellRenderData();
                }
        }
}

static void set_rigids()
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        auto& cell = map::cells[x][y];

                        if (!cell.is_seen_by_player)
                        {
                                continue;
                        }

                        auto& render_data = render_map[x][y];

                        const auto* const f = cell.rigid;

                        TileId gore_tile = TileId::empty;

                        char gore_character = 0;

                        if (f->can_have_gore())
                        {
                                gore_tile = f->gore_tile();
                                gore_character = f->gore_character();
                        }

                        if (gore_tile == TileId::empty)
                        {
                                render_data.tile = f->tile();
                                render_data.character = f->character();
                                render_data.color = f->color();

                                const Color feature_color_bg = f->color_bg();

                                if (feature_color_bg != colors::black())
                                {
                                        render_data.color_bg =
                                                feature_color_bg;
                                }
                        }
                        else // Has gore
                        {
                                render_data.tile = gore_tile;
                                render_data.character = gore_character;
                                render_data.color = colors::red();
                        }
                }
        }
}

static void post_process_wall_tiles()
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < (map_h - 1); ++y)
                {
                        auto& render_data = render_map[x][y];

                        const auto tile = render_data.tile;

                        const bool is_wall_top_tile =
                                Wall::is_wall_top_tile(tile);

                        if (!is_wall_top_tile)
                        {
                                continue;
                        }

                        const Wall* wall = nullptr;

                        {
                                const auto* const f = map::cells[x][y].rigid;

                                const auto id = f->id();

                                if (id == FeatureId::wall)
                                {
                                        wall = static_cast<const Wall*>(f);
                                }
                                else if (id == FeatureId::door)
                                {
                                        auto door = static_cast<const Door*>(f);

                                        if (door->is_secret())
                                        {
                                                wall = door->mimic();
                                        }
                                }

                                if (!wall)
                                {
                                        continue;
                                }
                        }

                        if (map::cells[x][y + 1].is_explored)
                        {
                                const auto tile_below =
                                        render_map[x][y + 1].tile;

                                if (Wall::is_wall_front_tile(tile_below) ||
                                    Wall::is_wall_top_tile(tile_below) ||
                                    Door::is_tile_any_door(tile_below))
                                {
                                        render_data.tile =
                                                wall->top_wall_tile();
                                }
                                else if (wall)
                                {
                                        render_data.tile =
                                                wall->front_wall_tile();
                                }
                        }
                        else // Cell below is not explored
                        {
                                if (wall)
                                {
                                        render_data.tile =
                                                wall->front_wall_tile();
                                }
                        }
                }
        }
}

static void set_dead_actors()
{
        for (Actor* actor : game_time::actors)
        {
                const P& p(actor->pos);

                if (!map::cells[p.x][p.y].is_seen_by_player ||
                    !actor->is_corpse() ||
                    actor->data().character == 0 ||
                    actor->data().character == ' ' ||
                    actor->data().tile == TileId::empty)
                {
                        continue;
                }

                auto& render_data = render_map[p.x][p.y];

                render_data.color = actor->color();
                render_data.tile = actor->tile();
                render_data.character = actor->character();
        }
}

static void set_items()
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        const Item* const item = map::cells[x][y].item;

                        if (!map::cells[x][y].is_seen_by_player || !item)
                        {
                                continue;
                        }

                        auto& render_data = render_map[x][y];

                        render_data.color = item->color();
                        render_data.tile = item->tile();
                        render_data.character = item->character();
                }
        }
}

static void copy_seen_cells_to_player_memory()
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        if (!map::cells[x][y].is_seen_by_player)
                        {
                                continue;
                        }

                        render_map_player_memory[x][y] = render_map[x][y];
                }
        }
}

static void set_light()
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        auto& render_data = render_map[x][y];

                        const auto* const f = map::cells[x][y].rigid;

                        if (!map::cells[x][y].is_seen_by_player ||
                            !map::light[x][y] ||
                            !f->is_los_passable() ||
                            f->is_bottomless())
                        {
                                continue;
                        }

                        auto& color = render_data.color;

                        color.set_r(std::min(255, color.r() + 40));
                        color.set_g(std::min(255, color.g() + 40));
                }
        }
}

static void set_mobiles()
{
        for (auto* mob : game_time::mobs)
        {
                const P& p = mob->pos();

                const TileId mob_tile = mob->tile();

                const char mob_character = mob->character();

                if (!map::cells[p.x][p.y].is_seen_by_player ||
                    mob_tile == TileId::empty ||
                    mob_character == 0 ||
                    mob_character == ' ')
                {
                        continue;
                }

                auto& render_data = render_map[p.x][p.y];

                render_data.color = mob->color();
                render_data.tile = mob_tile;
                render_data.character = mob_character;
        }
}

static void set_living_seen_monster(const Mon& mon,
                                    CellRenderData& render_data)
{
        if (mon.tile() == TileId::empty ||
            mon.character() == 0 ||
            mon.character() == ' ')
        {
                return;
        }

        render_data.color = mon.color();
        render_data.tile = mon.tile();
        render_data.character = mon.character();

        if (map::player->is_leader_of(&mon))
        {
                render_data.color_bg = colors::mon_allied_bg();
        }
        else // Player is not leader of monster
        {
                if (mon.aware_of_player_counter_ <= 0)
                {
                        render_data.color_bg =
                                colors::mon_unaware_bg();
                }
                else // Monster is aware of player
                {
                        const bool has_temporary_negative_prop =
                                mon.properties()
                                .has_temporary_negative_prop_mon();

                        if (has_temporary_negative_prop)
                        {
                                render_data.color_bg =
                                        colors::mon_temp_property_bg();
                        }
                }
        }
}

static void set_living_hidden_monster(const Mon& mon,
                                      CellRenderData& render_data)
{
        if (mon.player_aware_of_me_counter_ <= 0)
        {
                return;
        }

        const Color color_bg =
                map::player->is_leader_of(&mon) ?
                colors::mon_allied_bg() :
                colors::gray();

        render_data.tile = TileId::empty;
        render_data.character = '!';
        render_data.color = colors::black();
        render_data.color_bg = color_bg;
}

static void set_living_monsters()
{
        for (auto* actor : game_time::actors)
        {
                if (actor->is_player() || !actor->is_alive())
                {
                        continue;
                }

                const P& pos = actor->pos;

                auto& render_data = render_map[pos.x][pos.y];

                const auto* const mon = static_cast<const Mon*>(actor);

                if (map::player->can_see_actor(*actor))
                {
                        set_living_seen_monster(*mon, render_data);
                }
                else
                {
                        set_living_hidden_monster(*mon, render_data);
                }
        }
}

void set_unseen_cells_from_player_memory()
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        auto& render_data = render_map[x][y];

                        const Cell& cell = map::cells[x][y];

                        if (cell.is_seen_by_player || !cell.is_explored)
                        {
                                continue;
                        }

                        render_data = render_map_player_memory[x][y];

                        const double div = 3.0;

                        render_data.color =
                                render_data.color.fraction(div);

                        render_data.color_bg =
                                render_data.color_bg.fraction(div);
                }
        }
}

static void draw_render_map()
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        auto& render_data = render_map[x][y];

                        const P pos(x, y);

                        // NOTE: It can happen that text is drawn on the map
                        // even in tiles mode - for example exclamation marks on
                        // cells with known, unseen actors
                        if (config::is_tiles_mode() &&
                            (render_data.tile != TileId::empty))
                        {
                                io::draw_tile(
                                        render_data.tile,
                                        Panel::map,
                                        pos,
                                        render_data.color,
                                        render_data.color_bg);
                        }
                        else if ((render_data.character != 0) &&
                                 (render_data.character != ' '))
                        {
                                io::draw_character(
                                        render_data.character,
                                        Panel::map,
                                        pos,
                                        render_data.color,
                                        true,
                                        render_data.color_bg);
                        }
                }
        }
}

static int lifebar_length(const Actor& actor)
{
        const int actor_hp = std::max(0, actor.hp());

        const int actor_hp_max = actor.hp_max(true);

        if (actor_hp < actor_hp_max)
        {
                int hp_percent = (actor_hp * 100) / actor_hp_max;

                return ((config::cell_px_w() - 2) * hp_percent) / 100;
        }

        return -1;
}

static void draw_life_bar(const Actor& actor)
{
        const int length = lifebar_length(actor);

        if (length < 0)
        {
                return;
        }

        const P cell_dims(config::cell_px_w(), config::cell_px_h());

        const int w_green = length;

        const int w_bar_tot = cell_dims.x - 2;

        const int w_red = w_bar_tot - w_green;

        const P& pos = actor.pos;

        const P px_pos =
                io::px_pos_for_cell_in_panel(
                        Panel::map,
                        pos + P(0, 1)) - P(0, 2);

        const int x0_green = px_pos.x + 1;

        const int x0_red = x0_green + w_green;

        if (w_green > 0)
        {
                io::draw_line_hor(
                        P(x0_green, px_pos.y),
                        w_green,
                        colors::light_green());
        }

        if (w_red > 0)
        {
                io::draw_line_hor(
                        P(x0_red, px_pos.y),
                        w_red,
                        colors::light_red());
        }
}

static void draw_monster_life_bars()
{
        for (auto* actor : game_time::actors)
        {
                if (actor->is_player() ||
                    !actor->is_alive() ||
                    !map::player->can_see_actor(*actor))
                {
                        continue;
                }

                draw_life_bar(*actor);
        }
}

static void draw_player_character()
{
        const P& pos = map::player->pos;

        Item* item = map::player->inv().item_in_slot(SlotId::wpn);

        const bool is_ghoul = player_bon::bg() == Bg::ghoul;

        const Color color = map::player->color();

        Color color_bg = colors::black();

        bool uses_ranged_wpn = false;

        if (item)
        {
                uses_ranged_wpn =
                        item->data().ranged.is_ranged_wpn;
        }

        const TileId tile =
                is_ghoul ? TileId::ghoul :
                uses_ranged_wpn ? TileId::player_firearm :
                TileId::player_melee;

        const char character = '@';

        auto& player_render_data = render_map[pos.x][pos.y];

        player_render_data.tile = tile;
        player_render_data.character = character;
        player_render_data.color = color;
        player_render_data.color_bg = color_bg;

        if (config::is_tiles_mode())
        {
                io::draw_tile(tile,
                              Panel::map,
                              pos,
                              color,
                              color_bg);
        }
        else // Text mode
        {
                io::draw_character(character,
                                   Panel::map,
                                   pos,
                                   color,
                                   true,
                                   color_bg);
        }

        draw_life_bar(*map::player);
}

// -----------------------------------------------------------------------------
// draw_map
// -----------------------------------------------------------------------------
namespace draw_map
{

void clear()
{
        for (int x = 0; x < map_w; ++x)
        {
                for (int y = 0; y < map_h; ++y)
                {
                        render_map[x][y] = CellRenderData();

                        render_map_player_memory[x][y] = CellRenderData();
                }
        }
}

void run()
{
        clear_render_map();

        set_unseen_cells_from_player_memory();

        set_rigids();

        if (config::is_tiles_mode() &&
            !config::is_tiles_wall_full_square())
        {
                post_process_wall_tiles();
        }

        set_dead_actors();

        set_items();

        copy_seen_cells_to_player_memory();

        set_light();

        set_mobiles();

        set_living_monsters();

        draw_render_map();

        draw_monster_life_bars();

        draw_player_character();
}

const CellRenderData& get_drawn_cell(int x, int y)
{
        return render_map[x][y];
}

const CellRenderData& get_drawn_cell_player_memory(int x, int y)
{
        return render_map_player_memory[x][y];
}

} // draw_map
