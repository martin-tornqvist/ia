#ifndef IO_HPP
#define IO_HPP

#include <SDL.h>
#include <SDL_video.h>
#include <SDL_image.h>

#include <vector>

#include "game_time.hpp"
#include "config.hpp"
#include "gfx.hpp"

enum class Panel
{
        screen,
        map,
        status_lines,
        log
};

struct CellRenderData
{
        CellRenderData& operator=(const CellRenderData&) = default;

        TileId tile = TileId::empty;
        char character = 0;
        Color color = colors::black();
        Color color_bg = colors::black();
};

struct InputData
{
        InputData() :
                key(-1),
                is_shift_held(false),
                is_ctrl_held(false) {}

        InputData(int key,
                  bool is_shift_held = false,
                  bool is_ctrl_held  = false) :
                key(key),
                is_shift_held(is_shift_held),
                is_ctrl_held(is_ctrl_held) {}

        int key;
        bool is_shift_held, is_ctrl_held;
};

namespace io
{

void init();
void cleanup();

void update_screen();

void clear_screen();

void draw_symbol(
        const TileId tile,
        const char character,
        const Panel panel,
        const P& pos,
        const Color& color,
        const Color& color_bg = colors::black());

void draw_tile(
        const TileId tile,
        const Panel panel,
        const P& pos,
        const Color& color,
        const Color& color_bg = colors::black());

void draw_character(
        const char character,
        const Panel panel,
        const P& pos,
        const Color& color,
        const Color& color_bg = colors::black());

void draw_text(
        const std::string& str,
        const Panel panel,
        const P& pos,
        const Color& color,
        const Color& color_bg = colors::black());

int draw_text_center(
        const std::string& str,
        const Panel panel,
        const P& pos,
        const Color& color,
        const Color& color_bg = colors::black(),
        const bool is_pixel_pos_adj_allowed = true);

void cover_cell_in_map(const P& pos);

void cover_panel(const Panel panel);

void cover_area(const Panel panel, const R& area);

void cover_area(const Panel panel, const P& pos, const P& dims);

void cover_area_px(const P& px_pos, const P& px_dims);

void draw_rectangle_solid(
        const P& px_pos,
        const P& px_dims,
        const Color& color);

void draw_line_hor(
        const P& px_pos,
        const int w,
        const Color& color);

void draw_line_ver(
        const P& px_pos,
        const int h,
        const Color& color);

void draw_blast_at_field(
        const P& center_pos,
        const int radius,
        bool forbidden_cells[map_w][map_h],
        const Color& color_inner,
        const Color& color_outer);

void draw_blast_at_cells(
        const std::vector<P>& positions,
        const Color& color);

void draw_blast_at_seen_cells(
        const std::vector<P>& positions,
        const Color& color);

void draw_blast_at_seen_actors(
        const std::vector<Actor*>& actors,
        const Color& color);

void draw_main_menu_logo(const int y_pos);

void draw_skull(const P& p);

void draw_box(
        const R& area,
        const Panel panel = Panel::screen,
        const Color& color = colors::dark_gray(),
        const bool do_cover_area = false);

// Draws a description "box" for items, spells, etc. The parameter lines may be
// empty, in which case an empty area is drawn.
void draw_descr_box(const std::vector<ColoredString>& lines);

void draw_info_scr_interface(
        const std::string& title,
        const InfScreenType screen_type);

P px_pos_for_cell_in_panel(const Panel panel, const P& pos);

// ----------------------------------------
// TODO: WTF is the difference between these two functions?
void flush_input();
void clear_events();
// ----------------------------------------

// TODO: "is_o_return" is very hacky...
InputData get(const bool is_o_return);

} // io

#endif // IO_HPP
