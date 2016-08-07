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

struct CellRenderData
{
    CellRenderData() :
        clr                             (clr_black),
        clr_bg                          (clr_black),
        tile                            (TileId::empty),
        glyph                           (' '),
        is_light_fade_allowed           (true),
        is_marked_lit                   (false),
        is_living_actor_seen_here       (false),
        is_aware_of_hostile_mon_here    (false),
        is_aware_of_allied_mon_here     (false) {}

    Clr     clr;
    Clr     clr_bg;
    TileId  tile;
    char    glyph;
    bool    is_light_fade_allowed;
    bool    is_marked_lit;
    bool    is_living_actor_seen_here;
    bool    is_aware_of_hostile_mon_here;
    bool    is_aware_of_allied_mon_here;
};

struct CellOverlay
{
    CellOverlay() :
        clr_bg(clr_black) {}

    Clr clr_bg;
};

namespace render
{

extern CellRenderData render_array[map_w][map_h];
extern CellRenderData render_array_no_actors[map_w][map_h];

void init();
void cleanup();

//Draws the whole "map state" including character lines, log, etc
void draw_map_state(const UpdateScreen update = UpdateScreen::yes,
                    CellOverlay overlay[map_w][map_h] = nullptr);

void update_screen();

void clear_screen();

void draw_tile(const TileId tile,
               const Panel panel,
               const P& pos,
               const Clr& clr,
               const Clr& bg_clr = clr_black);

void draw_glyph(const char glyph,
                const Panel panel,
                const P& pos,
                const Clr& clr,
                const bool draw_bg_clr = true,
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
                     const bool is_pixel_pos_adj_allowed = true);

void cover_cell_in_map(const P& pos);

void cover_panel(const Panel panel);

void cover_area(const Panel panel, const R& area);
void cover_area(const Panel panel, const P& pos, const P& dims);

void cover_area_px(const P& px_pos, const P& px_dims);

void draw_rectangle_solid(const P& px_pos, const P& px_dims,
                          const Clr& clr);

void draw_line_hor(const P& px_pos, const int w, const Clr& clr);

void draw_line_ver(const P& px_pos, const int h, const Clr& clr);

void draw_marker(const P& p,
                 const std::vector<P>& trail,
                 const int effective_range = -1,
                 const int blocked_from_idx = -1,
                 CellOverlay overlay[map_w][map_h] = nullptr);

void draw_blast_at_field(const P& center_pos,
                         const int radius,
                         bool forbidden_cells[map_w][map_h],
                         const Clr& clr_inner,
                         const Clr& clr_outer);

void draw_blast_at_cells(const std::vector<P>& positions,
                         const Clr& clr);

void draw_blast_at_seen_cells(const std::vector<P>& positions,
                              const Clr& clr);

void draw_blast_at_seen_actors(const std::vector<Actor*>& actors,
                               const Clr& clr);

void draw_main_menu_logo(const int y_pos);

void draw_skull(const P& p);

void draw_projectiles(std::vector<Projectile*>& projectiles,
                      const bool draw_map_before);

void draw_box(const R& area,
              const Panel panel = Panel::screen,
              const Clr& clr = clr_gray_drk,
              const bool do_cover_area = false);

//Draws a description "box" for items, spells, etc. The parameter lines may be empty,
//in which case an empty area is drawn.
void draw_descr_box(const std::vector<StrAndClr>& lines);

void draw_info_scr_interface(const std::string& title,
                             const InfScreenType screen_type);

void draw_map(CellOverlay overlay[map_w][map_h] = nullptr);

void on_toggle_fullscreen();

} //render

#endif

