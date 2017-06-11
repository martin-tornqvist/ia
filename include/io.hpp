#ifndef IO_HPP
#define IO_HPP

#include <SDL.h>
#include <SDL_video.h>
#include <SDL_image.h>

#include <vector>

#include "game_time.hpp"
#include "config.hpp"
#include "art.hpp"

struct Projectile;

enum class Panel
{
    screen,
    map,
    status_lines,
    log
};

struct InputData
{
    InputData() :
        key             (-1),
        is_shift_held   (false),
        is_ctrl_held    (false) {}

    InputData(int key,
              bool is_shift_held = false,
              bool is_ctrl_held  = false) :
        key             (key),
        is_shift_held   (is_shift_held),
        is_ctrl_held    (is_ctrl_held) {}

    int key;
    bool is_shift_held, is_ctrl_held;
};

namespace io
{

void init();
void cleanup();

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

// TODO: Perhaps centering by adjusting pixel position should not be alloed?
//       It ruins the terminal feeling a bit...
int draw_text_center(const std::string& str,
                     const Panel panel,
                     const P& pos,
                     const Clr& clr,
                     const Clr& bg_clr = clr_black,
                     const bool is_pixel_pos_adj_allowed = true);

void cover_cell_in_map(const P& pos);

void cover_panel(const Panel panel);

void cover_area(const Panel panel,
                const R& area);

void cover_area(const Panel panel,
                const P& pos,
                const P& dims);

void cover_area_px(const P& px_pos, const P& px_dims);

void draw_rectangle_solid(const P& px_pos,
                          const P& px_dims,
                          const Clr& clr);

void draw_line_hor(const P& px_pos,
                   const int w,
                   const Clr& clr);

void draw_line_ver(const P& px_pos,
                   const int h,
                   const Clr& clr);

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

// Draws a description "box" for items, spells, etc. The parameter lines may be
// empty, in which case an empty area is drawn.
void draw_descr_box(const std::vector<StrAndClr>& lines);

void draw_info_scr_interface(const std::string& title,
                             const InfScreenType screen_type);

P px_pos_for_cell_in_panel(const Panel panel, const P& pos);

// ---
//
// TODO: WTF is the difference between these two functions?
//
void flush_input();

void clear_events();
// ---

//
// TODO: "is_o_return" is very hacky...
//
InputData get(const bool is_o_return);

} // io

#endif // IO_HPP
