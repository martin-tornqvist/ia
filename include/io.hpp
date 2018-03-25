#ifndef IO_HPP
#define IO_HPP

#include <SDL.h>
#include <SDL_video.h>
#include <SDL_image.h>

#include <vector>

#include "game_time.hpp"
#include "config.hpp"
#include "gfx.hpp"
#include "panel.hpp"

struct CellRenderData
{
        CellRenderData& operator=(const CellRenderData&) = default;

        TileId tile = TileId::END;
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

struct PxPos
{
        PxPos() {}

        PxPos(int x, int y) :
                value(P(x, y)) {}

        P value = P();
};

struct PxRect
{
        PxRect() {}

        PxRect(int x0, int y0, int x1, int y1) :
                value(x0, y0, x1, y1) {}

        R value = R();
};

namespace io
{

void init();
void cleanup();

void update_screen();

void clear_screen();

void on_fullscreen_toggled();

P min_screen_gui_dims();

// Scale from gui/map cell coordinate(s) to pixel coordinate(s)
int gui_to_px_coords_x(const int value);
int gui_to_px_coords_y(const int value);

int map_to_px_coords_x(const int value);
int map_to_px_coords_y(const int value);

PxPos gui_to_px_coords(const P pos);
PxPos gui_to_px_coords(const int x, const int y);

PxPos map_to_px_coords(const P pos);
PxPos map_to_px_coords(const int x, const int y);

P px_to_gui_coords(const PxPos px_pos);

P px_to_map_coords(const PxPos px_pos);

P gui_to_map_coords(const P gui_pos);

// Returns a screen pixel position, relative to a cell position in a panel
PxPos gui_to_px_coords(const Panel panel, const P offset);
PxPos map_to_px_coords(const Panel panel, const P offset);

void draw_symbol(
        const TileId tile,
        const char character,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg = true,
        const Color& color_bg = colors::black());

void draw_tile(
        const TileId tile,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg = true,
        const Color& color_bg = colors::black());

void draw_character(
        const char character,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg = true,
        const Color& color_bg = colors::black());

void draw_text(
        const std::string& str,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg = true,
        const Color& color_bg = colors::black());

void draw_text_center(
        const std::string& str,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg = true,
        const Color& color_bg = colors::black(),
        const bool is_pixel_pos_adj_allowed = true);

void draw_text_right(
        const std::string& str,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg = true,
        const Color& color_bg = colors::black());

void cover_cell(const Panel panel, const P offset);

void cover_panel(
        const Panel panel,
        const Color& color = colors::black());

void cover_area(
        const Panel panel,
        const R area,
        const Color& color = colors::black());

void cover_area(
        const Panel panel,
        const P offset,
        const P dims,
        const Color& color = colors::black());

void draw_rectangle_solid(
        const PxPos px_pos,
        const PxPos px_dims,
        const Color& color);

void draw_blast_at_field(
        const P center_pos,
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

void draw_main_menu_logo();

void draw_skull(const P pos);

// TODO: Perhaps add an option to draw a background color inside the box
void draw_box(
        const R& area,
        const Color& color = colors::dark_gray_brown());

// Draws a description "box" for items, spells, etc. The parameter lines may be
// empty, in which case an empty area is drawn.
void draw_descr_box(const std::vector<ColoredString>& lines);

// ----------------------------------------
// TODO: WTF is the difference between these two functions?
void flush_input();
void clear_events();
// ----------------------------------------

// TODO: "is_o_return" is very hacky...
InputData get(const bool is_o_return);

} // io

#endif // IO_HPP
