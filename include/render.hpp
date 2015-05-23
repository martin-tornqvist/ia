#ifndef RENDER_H
#define RENDER_H

#include <vector>

#include <SDL_video.h>
#include <SDL_image.h>

#include "cmn_data.hpp"
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

namespace render
{

extern Cell_render_data render_array[MAP_W][MAP_H];
extern Cell_render_data render_array_no_actors[MAP_W][MAP_H];

void init();
void cleanup();

void draw_map_and_interface(const bool SHOULD_UPDATE_SCREEN = true);

void update_screen();

void clear_screen();

void draw_tile(const Tile_id tile, const Panel panel, const Pos& pos,
               const Clr& clr, const Clr& bg_clr = clr_black);

void draw_glyph(const char GLYPH, const Panel panel, const Pos& pos,
                const Clr& clr, const bool DRAW_BG_CLR = true,
                const Clr& bg_clr = clr_black);

void draw_text(const std::string& str, const Panel panel, const Pos& pos,
               const Clr& clr, const Clr& bg_clr = clr_black);

int draw_text_centered(const std::string& str, const Panel panel, const Pos& pos,
                       const Clr& clr, const Clr& bg_clr = clr_black,
                       const bool IS_PIXEL_POS_ADJ_ALLOWED = true);

void cover_cell_in_map(const Pos& pos);

void cover_panel(const Panel panel);

void cover_area(const Panel panel, const Rect& area);
void cover_area(const Panel panel, const Pos& pos, const Pos& dims);

void cover_area_px(const Pos& px_pos, const Pos& px_dims);

void draw_rectangle_solid(const Pos& px_pos, const Pos& px_dims,
                          const Clr& clr);

void draw_line_hor(const Pos& px_pos, const int W, const Clr& clr);

void draw_line_ver(const Pos& px_pos, const int H, const Clr& clr);

void draw_marker(const Pos& p, const std::vector<Pos>& trail,
                 const int EFFECTIVE_RANGE = -1);

void draw_blast_at_field(const Pos& center_pos, const int RADIUS,
                         bool forbidden_cells[MAP_W][MAP_H], const Clr& clr_inner,
                         const Clr& clr_outer);

void draw_blast_at_cells(const std::vector<Pos>& positions, const Clr& clr);

void draw_blast_at_seen_cells(const std::vector<Pos>& positions, const Clr& clr);

void draw_blast_at_seen_actors(const std::vector<Actor*>& actors, const Clr& clr);

void draw_main_menu_logo(const int Y_POS);

void draw_projectiles(std::vector<Projectile*>& projectiles,
                      const bool SHOULD_DRAW_MAP_BEFORE);

void draw_popup_box(const Rect& area, const Panel panel = Panel::screen,
                    const Clr& clr = clr_popup_box, const bool COVER_AREA = false);

//Draws a description "box" for items, spells, etc. The parameter lines may be empty,
//in which case an empty area is drawn.
void draw_descr_box(const std::vector<Str_and_clr>& lines);

void draw_map();

void on_toggle_fullscreen();

} //render

#endif

