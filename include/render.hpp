#ifndef RENDER_HPP
#define RENDER_HPP

#include <vector>

#include <SDL_video.h>
#include <SDL_image.h>

#include "game_time.hpp"
#include "config.hpp"
#include "art.hpp"

struct Projectile;

enum class Panel
{
    screen,
    map,
    char_lines,
    log
};

struct Cell_render_data
{
    Cell_render_data() :
        clr                             (clr_black),
        clr_bg                          (clr_black),
        tile                            (Tile_id::empty),
        glyph                           (' '),
        lifebar_length                  (-1),
        is_light_fade_allowed           (true),
        is_marked_lit                   (false),
        is_living_actor_seen_here       (false),
        is_aware_of_hostile_mon_here    (false),
        is_aware_of_allied_mon_here     (false) {}

    Clr     clr;
    Clr     clr_bg;
    Tile_id tile;
    char    glyph;
    int     lifebar_length;
    bool    is_light_fade_allowed;
    bool    is_marked_lit;
    bool    is_living_actor_seen_here;
    bool    is_aware_of_hostile_mon_here;
    bool    is_aware_of_allied_mon_here;
};

struct Cell_overlay
{
    Cell_overlay() :
        clr_bg(clr_black) {}

    Clr clr_bg;
};

namespace render
{

extern Cell_render_data render_array[map_w][map_h];
extern Cell_render_data render_array_no_actors[map_w][map_h];

void init();
void cleanup();

//Draws the whole "map state" including character lines, log, etc
void draw_map_state(const Update_screen update = Update_screen::yes,
                    Cell_overlay overlay[map_w][map_h] = nullptr);

void update_screen();

void clear_screen();

void draw_tile(const Tile_id tile,
               const Panel panel,
               const P& pos,
               const Clr& clr,
               const Clr& bg_clr = clr_black);

void draw_glyph(const char GLYPH,
                const Panel panel,
                const P& pos,
                const Clr& clr,
                const bool DRAW_BG_CLR = true,
                const Clr& bg_clr = clr_black);

void draw_text(const std::string& str,
               const Panel panel,
               const P& pos,
               const Clr& clr,
               const Clr& bg_clr = clr_black);

int draw_text_center(const std::string& str,
                     const Panel panel,
                     const P& pos,
                     const Clr& clr,
                     const Clr& bg_clr = clr_black,
                     const bool IS_PIXEL_POS_ADJ_ALLOWED = true);

void cover_cell_in_map(const P& pos);

void cover_panel(const Panel panel);

void cover_area(const Panel panel, const R& area);
void cover_area(const Panel panel, const P& pos, const P& dims);

void cover_area_px(const P& px_pos, const P& px_dims);

void draw_rectangle_solid(const P& px_pos, const P& px_dims,
                          const Clr& clr);

void draw_line_hor(const P& px_pos, const int W, const Clr& clr);

void draw_line_ver(const P& px_pos, const int H, const Clr& clr);

void draw_marker(const P& p,
                 const std::vector<P>& trail,
                 const int EFFECTIVE_RANGE = -1,
                 const int BLOCKED_FROM_IDX = -1,
                 Cell_overlay overlay[map_w][map_h] = nullptr);

void draw_blast_at_field(const P& center_pos,
                         const int RADIUS,
                         bool forbidden_cells[map_w][map_h],
                         const Clr& clr_inner,
                         const Clr& clr_outer);

void draw_blast_at_cells(const std::vector<P>& positions,
                         const Clr& clr);

void draw_blast_at_seen_cells(const std::vector<P>& positions,
                              const Clr& clr);

void draw_blast_at_seen_actors(const std::vector<Actor*>& actors,
                               const Clr& clr);

void draw_main_menu_logo(const int Y_POS);

void draw_skull(const P& p);

void draw_projectiles(std::vector<Projectile*>& projectiles,
                      const bool DRAW_MAP_BEFORE);

void draw_box(const R& area,
              const Panel panel = Panel::screen,
              const Clr& clr = clr_gray_drk,
              const bool COVER_AREA = false);

//Draws a description "box" for items, spells, etc. The parameter lines may be empty,
//in which case an empty area is drawn.
void draw_descr_box(const std::vector<Str_and_clr>& lines);

void draw_info_scr_interface(const std::string& title,
                             const Inf_screen_type screen_type);

void draw_map(Cell_overlay overlay[map_w][map_h] = nullptr);

void on_toggle_fullscreen();

} //render

#endif

