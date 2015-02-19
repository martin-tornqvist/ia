#include "render.h"

#include <vector>
#include <iostream>

#include <SDL_image.h>

#include "init.h"
#include "item.h"
#include "character_lines.h"
#include "marker.h"
#include "map.h"
#include "actor.h"
#include "actor_player.h"
#include "actor_mon.h"
#include "log.h"
#include "attack.h"
#include "feature_mob.h"
#include "feature_door.h"
#include "inventory.h"
#include "utils.h"
#include "cmn_data.h"
#include "sdl_wrapper.h"
#include "text_format.h"

using namespace std;

namespace Render
{

Cell_render_data  render_array[MAP_W][MAP_H];
Cell_render_data  render_array_no_actors[MAP_W][MAP_H];
SDL_Surface*    screen_surface       = nullptr;
SDL_Surface*    main_menu_logo_surface = nullptr;

namespace
{

const size_t PIXEL_DATA_W = 400;
const size_t PIXEL_DATA_H = 400;

bool tile_pixel_data_[PIXEL_DATA_W][PIXEL_DATA_H];
bool font_pixel_data_[PIXEL_DATA_W][PIXEL_DATA_H];
bool contour_pixel_data_[PIXEL_DATA_W][PIXEL_DATA_H];

bool is_inited()
{
    return screen_surface;
}

void load_main_menu_logo()
{
    TRACE_FUNC_BEGIN;

    SDL_Surface* main_menu_logo_surface_tmp = IMG_Load(main_menu_logo_img_name.data());

    main_menu_logo_surface = SDL_Display_format_alpha(main_menu_logo_surface_tmp);

    SDL_Free_surface(main_menu_logo_surface_tmp);

    TRACE_FUNC_END;
}

Uint32 get_pixel(SDL_Surface* const surface, const int PIXEL_X, const int PIXEL_Y)
{
    int bpp = surface->format->Bytes_per_pixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8* p = (Uint8*)surface->pixels + PIXEL_Y * surface->pitch + PIXEL_X * bpp;

    switch (bpp)
    {
    case 1:   return *p;          break;
    case 2:   return *(Uint16*)p; break;
    case 3:
    {
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            return p[0] << 16 | p[1] << 8 | p[2];
        }
        else
        {
            return p[0] | p[1] << 8 | p[2] << 16;
        }
    } break;
    case 4:   return *(Uint32*)p; break;
    default:  return -1;          break;
    }
    return -1;
}

void put_pixel(SDL_Surface* const surface, const int PIXEL_X, const int PIXEL_Y,
              Uint32 pixel)
{
    int bpp = surface->format->Bytes_per_pixel;
    //Here p is the address to the pixel we want to set
    Uint8* p = (Uint8*)surface->pixels + PIXEL_Y * surface->pitch + PIXEL_X * bpp;

    switch (bpp)
    {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16*)p = pixel;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8)  & 0xff;
            p[2] = pixel & 0xff;
        }
        else //Little endian
        {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8)  & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32*)p = pixel;
        break;

    default: {} break;
    }
}

void load_font()
{
    TRACE_FUNC_BEGIN;

    SDL_Surface* font_surface_tmp = IMG_Load(Config::get_font_name().data());

    Uint32 bg_clr = SDL_Map_rGB(font_surface_tmp->format, 0, 0, 0);

    for (int x = 0; x < font_surface_tmp->w; ++x)
    {
        for (int y = 0; y < font_surface_tmp->h; ++y)
        {
            font_pixel_data_[x][y] = get_pixel(font_surface_tmp, x, y) != bg_clr;
        }
    }

    SDL_Free_surface(font_surface_tmp);

    TRACE_FUNC_END;
}

void load_tiles()
{
    TRACE_FUNC_BEGIN;

    SDL_Surface* tile_surface_tmp = IMG_Load(tiles_img_name.data());

    Uint32 img_clr = SDL_Map_rGB(tile_surface_tmp->format, 255, 255, 255);
    for (int x = 0; x < tile_surface_tmp->w; ++x)
    {
        for (int y = 0; y < tile_surface_tmp->h; ++y)
        {
            tile_pixel_data_[x][y] = get_pixel(tile_surface_tmp, x, y) == img_clr;
        }
    }

    SDL_Free_surface(tile_surface_tmp);

    TRACE_FUNC_END;
}

void load_contour(const bool base[PIXEL_DATA_W][PIXEL_DATA_H])
{
    const Pos cell_dims(Config::get_cell_w(), Config::get_cell_h());

    for (size_t px_x = 0; px_x < PIXEL_DATA_W; ++px_x)
    {
        for (size_t px_y = 0; px_y < PIXEL_DATA_H; ++px_y)
        {

            bool& cur_val  = contour_pixel_data_[px_x][px_y];
            cur_val        = false;

            //Only mark this pixel as contour if it's not marked on the base image
            if (!base[px_x][px_y])
            {
                //Position interval to check for this pixel is constrained within current image
                const Pos cur_img_px_p0((Pos(px_x, px_y) / cell_dims) * cell_dims);
                const Pos cur_img_px_p1(cur_img_px_p0 + cell_dims - 1);
                const Pos px_p0(max(cur_img_px_p0.x, int(px_x - 1)), max(cur_img_px_p0.y, int(px_y - 1)));
                const Pos px_p1(min(cur_img_px_p1.x, int(px_x + 1)), min(cur_img_px_p1.y, int(px_y + 1)));

                for (int px_check_x = px_p0.x; px_check_x <= px_p1.x; ++px_check_x)
                {
                    for (int px_check_y = px_p0.y; px_check_y <= px_p1.y; ++px_check_y)
                    {
                        if (base[px_check_x][px_check_y])
                        {
                            cur_val = true;
                            break;
                        }
                    }
                    if (cur_val) {break;}
                }
            }
        }
    }
}

void put_pixels_on_scr(const bool pixel_data[PIXEL_DATA_W][PIXEL_DATA_H],
                    const Pos& sheet_pos, const Pos& scr_pixel_pos, const Clr& clr)
{
    if (is_inited())
    {
        const int CLR_TO = SDL_Map_rGB(screen_surface->format, clr.r, clr.g, clr.b);

        SDL_Lock_surface(screen_surface);

        const int CELL_W      = Config::get_cell_w();
        const int CELL_H      = Config::get_cell_h();
        const int SHEET_PX_X0 = sheet_pos.x * CELL_W;
        const int SHEET_PX_Y0 = sheet_pos.y * CELL_H;
        const int SHEET_PX_X1 = SHEET_PX_X0 + CELL_W - 1;
        const int SHEET_PX_Y1 = SHEET_PX_Y0 + CELL_H - 1;
        const int SCR_PX_X0   = scr_pixel_pos.x;
        const int SCR_PX_Y0   = scr_pixel_pos.y;

        int scr_px_x = SCR_PX_X0;

        for (int sheet_px_x = SHEET_PX_X0; sheet_px_x <= SHEET_PX_X1; sheet_px_x++)
        {
            int scr_px_y = SCR_PX_Y0;

            for (int sheet_px_y = SHEET_PX_Y0; sheet_px_y <= SHEET_PX_Y1; sheet_px_y++)
            {
                if (pixel_data[sheet_px_x][sheet_px_y])
                {
                    put_pixel(screen_surface, scr_px_x, scr_px_y, CLR_TO);
                }
                ++scr_px_y;
            }
            ++scr_px_x;
        }

        SDL_Unlock_surface(screen_surface);
    }
}

void put_pixels_on_scr_for_tile(const Tile_id tile, const Pos& scr_pixel_pos, const Clr& clr)
{
    put_pixels_on_scr(tile_pixel_data_, Art::get_tile_pos(tile), scr_pixel_pos, clr);
}

void put_pixels_on_scr_for_glyph(const char GLYPH, const Pos& scr_pixel_pos, const Clr& clr)
{
    put_pixels_on_scr(font_pixel_data_, Art::get_glyph_pos(GLYPH), scr_pixel_pos, clr);
}

Pos get_pixel_pos_for_cell_in_panel(const Panel panel, const Pos& pos)
{
    const Pos cell_dims(Config::get_cell_w(), Config::get_cell_h());

    switch (panel)
    {
    case Panel::screen:
        return Pos(pos.x * cell_dims.x, pos.y * cell_dims.y);

    case Panel::map:
        return (pos * cell_dims) + Pos(0, Config::get_map_pixel_offset_h());

    case Panel::log:
        return pos * cell_dims;

    case Panel::char_lines:
        return (pos * cell_dims) + Pos(0, Config::get_char_lines_pixel_offset_h());
    }
    return Pos();
}

int get_lifebar_length(const Actor& actor)
{
    const int ACTOR_HP = max(0, actor.get_hp());
    const int ACTOR_HP_MAX = actor.get_hp_max(true);
    if (ACTOR_HP < ACTOR_HP_MAX)
    {
        int HP_PERCENT = (ACTOR_HP * 100) / ACTOR_HP_MAX;
        return ((Config::get_cell_w() - 2) * HP_PERCENT) / 100;
    }
    return -1;
}

void draw_life_bar(const Pos& pos, const int LENGTH)
{
    if (LENGTH >= 0)
    {
        const Pos cell_dims(Config::get_cell_w(),  Config::get_cell_h());
        const int W_GREEN   = LENGTH;
        const int W_BAR_TOT = cell_dims.x - 2;
        const int W_RED     = W_BAR_TOT - W_GREEN;
        const Pos pixel_pos =
            get_pixel_pos_for_cell_in_panel(Panel::map, pos + Pos(0, 1)) - Pos(0, 2);
        const int X0_GREEN  = pixel_pos.x + 1;
        const int X0_RED    = X0_GREEN + W_GREEN;

        if (W_GREEN > 0)
        {
            draw_line_hor(Pos(X0_GREEN, pixel_pos.y), W_GREEN, clr_green_lgt);
        }
        if (W_RED > 0)
        {
            draw_line_hor(Pos(X0_RED, pixel_pos.y), W_RED, clr_red_lgt);
        }
    }
}

void draw_excl_mark_at(const Pos& pixel_pos)
{
    draw_rectangle_solid(pixel_pos,  Pos(3, 12),     clr_black);
    draw_line_ver(pixel_pos +        Pos(1,  1), 6,  clr_magenta_lgt);
    draw_line_ver(pixel_pos +        Pos(1,  9), 2,  clr_magenta_lgt);
}

void draw_player_shock_excl_marks()
{
    const double SHOCK  = Map::player->get_perm_shock_taken_cur_turn();
    const int NR_EXCL   = SHOCK > 8 ? 3 : SHOCK > 3 ? 2 : SHOCK > 1 ? 1 : 0;

    if (NR_EXCL > 0)
    {
        const Pos& player_pos = Map::player->pos;
        const Pos pixel_pos_right = get_pixel_pos_for_cell_in_panel(Panel::map, player_pos);

        for (int i = 0; i < NR_EXCL; ++i)
        {
            draw_excl_mark_at(pixel_pos_right + Pos(i * 3, 0));
        }
    }
}

void draw_glyph_at_pixel(const char GLYPH, const Pos& pixel_pos, const Clr& clr,
                      const bool DRAW_BG_CLR, const Clr& bg_clr = clr_black)
{
    if (DRAW_BG_CLR)
    {
        const Pos cell_dims(Config::get_cell_w(), Config::get_cell_h());

        draw_rectangle_solid(pixel_pos, cell_dims, bg_clr);

        //Only draw contour if neither the foreground or background is black
        if (!Utils::is_clr_eq(clr, clr_black) && !Utils::is_clr_eq(bg_clr, clr_black))
        {
            put_pixels_on_scr(contour_pixel_data_, Art::get_glyph_pos(GLYPH), pixel_pos, clr_black);
        }
    }

    put_pixels_on_scr_for_glyph(GLYPH, pixel_pos, clr);
}

} //Namespace

void init()
{
    TRACE_FUNC_BEGIN;
    cleanup();

    TRACE << "Setting up rendering window" << endl;
    const string title = "IA " + game_version_str;
    SDL_WM_Set_caption(title.data(), nullptr);

    const int W = Config::get_screen_pixel_w();
    const int H = Config::get_screen_pixel_h();
    if (Config::is_fullscreen())
    {
        screen_surface = SDL_Set_video_mode(W, H, SCREEN_BPP,
                                         SDL_SWSURFACE | SDL_FULLSCREEN);
    }
    if (!Config::is_fullscreen() || !screen_surface)
    {
        screen_surface = SDL_Set_video_mode(W, H, SCREEN_BPP, SDL_SWSURFACE);
    }

    if (!screen_surface)
    {
        TRACE << "Failed to create screen surface" << endl;
        assert(false);
    }

    load_font();

    if (Config::is_tiles_mode())
    {
        load_tiles();
        load_main_menu_logo();
    }

    load_contour(Config::is_tiles_mode() ? tile_pixel_data_ : font_pixel_data_);

    TRACE_FUNC_END;
}

void cleanup()
{
    TRACE_FUNC_BEGIN;

    if (screen_surface)
    {
        SDL_Free_surface(screen_surface);
        screen_surface = nullptr;
    }

    if (main_menu_logo_surface)
    {
        SDL_Free_surface(main_menu_logo_surface);
        main_menu_logo_surface = nullptr;
    }

    TRACE_FUNC_END;
}

void update_screen()
{
    if (is_inited()) {SDL_Flip(screen_surface);}
}

void clear_screen()
{
    if (is_inited())
    {
        SDL_Fill_rect(screen_surface, nullptr, SDL_Map_rGB(screen_surface->format, 0, 0, 0));
    }
}

void apply_surface(const Pos& pixel_pos, SDL_Surface* const src,
                  SDL_Rect* clip)
{
    if (is_inited())
    {
        SDL_Rect offset;
        offset.x = pixel_pos.x;
        offset.y = pixel_pos.y;
        SDL_Blit_surface(src, clip, screen_surface, &offset);
    }
}

void draw_main_menu_logo(const int Y_POS)
{
    const Pos pos((Config::get_screen_pixel_w() - main_menu_logo_surface->w) / 2,
                  Config::get_cell_h() * Y_POS);
    apply_surface(pos, main_menu_logo_surface);
}

void draw_marker(const Pos& p, const vector<Pos>& trail, const int EFFECTIVE_RANGE)
{
    if (trail.size() > 2)
    {
        for (size_t i = 1; i < trail.size(); ++i)
        {
            const Pos& pos = trail[i];
            cover_cell_in_map(pos);

            Clr clr = clr_green_lgt;

            if (EFFECTIVE_RANGE != -1)
            {
                const int CHEB_DIST = Utils::king_dist(trail[0], pos);

                if (CHEB_DIST > EFFECTIVE_RANGE)
                {
                    clr = clr_orange;
                }
            }
            if (Config::is_tiles_mode())
            {
                draw_tile(Tile_id::aim_marker_trail, Panel::map, pos, clr, clr_black);
            }
            else
            {
                draw_glyph('*', Panel::map, pos, clr, true, clr_black);
            }
        }
    }

    Clr clr = clr_green_lgt;

    if (trail.size() > 2)
    {
        if (EFFECTIVE_RANGE != -1)
        {
            const int CHEB_DIST = Utils::king_dist(trail[0], p);

            if (CHEB_DIST > EFFECTIVE_RANGE)
            {
                clr = clr_orange;
            }
        }
    }

    if (Config::is_tiles_mode())
    {
        draw_tile(Tile_id::aim_marker_head, Panel::map, p, clr, clr_black);
    }
    else
    {
        draw_glyph('X', Panel::map, p, clr, true, clr_black);
    }
}

void draw_blast_at_field(const Pos& center_pos, const int RADIUS,
                      bool forbidden_cells[MAP_W][MAP_H], const Clr& clr_inner,
                      const Clr& clr_outer)
{
    TRACE_FUNC_BEGIN;
    if (is_inited())
    {
        draw_map_and_interface();

        bool is_any_blast_rendered = false;

        Pos pos;

        for (
            pos.y = max(1, center_pos.y - RADIUS);
            pos.y <= min(MAP_H - 2, center_pos.y + RADIUS);
            pos.y++)
        {
            for (
                pos.x = max(1, center_pos.x - RADIUS);
                pos.x <= min(MAP_W - 2, center_pos.x + RADIUS);
                pos.x++)
            {
                if (!forbidden_cells[pos.x][pos.y])
                {
                    const bool IS_OUTER = pos.x == center_pos.x - RADIUS ||
                                          pos.x == center_pos.x + RADIUS ||
                                          pos.y == center_pos.y - RADIUS ||
                                          pos.y == center_pos.y + RADIUS;
                    const Clr clr = IS_OUTER ? clr_outer : clr_inner;
                    if (Config::is_tiles_mode())
                    {
                        draw_tile(Tile_id::blast1, Panel::map, pos, clr, clr_black);
                    }
                    else
                    {
                        draw_glyph('*', Panel::map, pos, clr, true, clr_black);
                    }
                    is_any_blast_rendered = true;
                }
            }
        }
        update_screen();
        if (is_any_blast_rendered) {Sdl_wrapper::sleep(Config::get_delay_explosion() / 2);}

        for (
            pos.y = max(1, center_pos.y - RADIUS);
            pos.y <= min(MAP_H - 2, center_pos.y + RADIUS);
            pos.y++)
        {
            for (
                pos.x = max(1, center_pos.x - RADIUS);
                pos.x <= min(MAP_W - 2, center_pos.x + RADIUS);
                pos.x++)
            {
                if (!forbidden_cells[pos.x][pos.y])
                {
                    const bool IS_OUTER = pos.x == center_pos.x - RADIUS ||
                                          pos.x == center_pos.x + RADIUS ||
                                          pos.y == center_pos.y - RADIUS ||
                                          pos.y == center_pos.y + RADIUS;
                    const Clr clr = IS_OUTER ? clr_outer : clr_inner;
                    if (Config::is_tiles_mode())
                    {
                        draw_tile(Tile_id::blast2, Panel::map, pos, clr, clr_black);
                    }
                    else
                    {
                        draw_glyph('*', Panel::map, pos, clr, true, clr_black);
                    }
                }
            }
        }
        update_screen();
        if (is_any_blast_rendered) {Sdl_wrapper::sleep(Config::get_delay_explosion() / 2);}
        draw_map_and_interface();
    }
    TRACE_FUNC_END;
}

void draw_blast_at_cells(const vector<Pos>& positions, const Clr& clr)
{
    TRACE_FUNC_BEGIN;

    if (is_inited())
    {
        draw_map_and_interface();

        for (const Pos& pos : positions)
        {
            if (Config::is_tiles_mode())
            {
                draw_tile(Tile_id::blast1, Panel::map, pos, clr, clr_black);
            }
            else
            {
                draw_glyph('*', Panel::map, pos, clr, true, clr_black);
            }
        }
        update_screen();
        Sdl_wrapper::sleep(Config::get_delay_explosion() / 2);

        for (const Pos& pos : positions)
        {
            if (Config::is_tiles_mode())
            {
                draw_tile(Tile_id::blast2, Panel::map, pos, clr, clr_black);
            }
            else
            {
                draw_glyph('*', Panel::map, pos, clr, true, clr_black);
            }
        }
        update_screen();
        Sdl_wrapper::sleep(Config::get_delay_explosion() / 2);
        draw_map_and_interface();
    }
    TRACE_FUNC_END;
}

void draw_blast_at_seen_cells(const vector<Pos>& positions, const Clr& clr)
{
    if (is_inited())
    {
        vector<Pos> positions_with_vision;

        for (const Pos& p : positions)
        {
            if (Map::cells[p.x][p.y].is_seen_by_player)
            {
                positions_with_vision.push_back(p);
            }
        }

        if (!positions_with_vision.empty())
        {
            Render::draw_blast_at_cells(positions_with_vision, clr);
        }
    }
}

void draw_blast_at_seen_actors(const std::vector<Actor*>& actors, const Clr& clr)
{
    if (is_inited())
    {
        vector<Pos> positions;
        for (Actor* const actor : actors)
        {
            positions.push_back(actor->pos);
        }
        draw_blast_at_seen_cells(positions, clr);
    }
}

void draw_tile(const Tile_id tile, const Panel panel, const Pos& pos, const Clr& clr,
              const Clr& bg_clr)
{
    if (is_inited())
    {
        const Pos pixel_pos = get_pixel_pos_for_cell_in_panel(panel, pos);
        const Pos cell_dims(Config::get_cell_w(), Config::get_cell_h());

        draw_rectangle_solid(pixel_pos, cell_dims, bg_clr);

        if (!Utils::is_clr_eq(bg_clr, clr_black))
        {
            put_pixels_on_scr(contour_pixel_data_, Art::get_tile_pos(tile), pixel_pos, clr_black);
        }

        put_pixels_on_scr_for_tile(tile, pixel_pos, clr);
    }
}

void draw_glyph(const char GLYPH, const Panel panel, const Pos& pos, const Clr& clr,
               const bool DRAW_BG_CLR, const Clr& bg_clr)
{
    if (is_inited())
    {
        const Pos pixel_pos = get_pixel_pos_for_cell_in_panel(panel, pos);
        draw_glyph_at_pixel(GLYPH, pixel_pos, clr, DRAW_BG_CLR, bg_clr);
    }
}

void draw_text(const string& str, const Panel panel, const Pos& pos, const Clr& clr,
              const Clr& bg_clr)
{
    if (is_inited())
    {
        Pos pixel_pos = get_pixel_pos_for_cell_in_panel(panel, pos);

        if (pixel_pos.y < 0 || pixel_pos.y >= Config::get_screen_pixel_h())
        {
            return;
        }

        const Pos cell_dims(Config::get_cell_w(), Config::get_cell_h());
        const int LEN = str.size();
        draw_rectangle_solid(pixel_pos, Pos(cell_dims.x * LEN, cell_dims.y), bg_clr);

        for (int i = 0; i < LEN; ++i)
        {
            if (pixel_pos.x < 0 || pixel_pos.x >= Config::get_screen_pixel_w())
            {
                return;
            }
            draw_glyph_at_pixel(str[i], pixel_pos, clr, false);
            pixel_pos.x += cell_dims.x;
        }
    }
}

int draw_text_centered(const string& str, const Panel panel, const Pos& pos,
                     const Clr& clr, const Clr& bg_clr,
                     const bool IS_PIXEL_POS_ADJ_ALLOWED)
{
    const int LEN         = str.size();
    const int LEN_HALF    = LEN / 2;
    const int X_POS_LEFT  = pos.x - LEN_HALF;

    const Pos cell_dims(Config::get_cell_w(), Config::get_cell_h());

    Pos pixel_pos = get_pixel_pos_for_cell_in_panel(panel, Pos(X_POS_LEFT, pos.y));

    if (IS_PIXEL_POS_ADJ_ALLOWED)
    {
        const int PIXEL_X_ADJ = LEN_HALF * 2 == LEN ? cell_dims.x / 2 : 0;
        pixel_pos += Pos(PIXEL_X_ADJ, 0);
    }

    const int W_TOT_PIXEL = LEN * cell_dims.x;

    SDL_Rect sdl_rect =
    {
        (Sint16)pixel_pos.x, (Sint16)pixel_pos.y,
        (Uint16)W_TOT_PIXEL, (Uint16)cell_dims.y
    };
    SDL_Fill_rect(screen_surface, &sdl_rect, SDL_Map_rGB(screen_surface->format,
                 bg_clr.r, bg_clr.g, bg_clr.b));

    for (int i = 0; i < LEN; ++i)
    {
        if (pixel_pos.x < 0 || pixel_pos.x >= Config::get_screen_pixel_w())
        {
            return X_POS_LEFT;
        }
        draw_glyph_at_pixel(str[i], pixel_pos, clr, false, bg_clr);
        pixel_pos.x += cell_dims.x;
    }
    return X_POS_LEFT;
}

void cover_panel(const Panel panel)
{
    const int SCREEN_PIXEL_W = Config::get_screen_pixel_w();

    switch (panel)
    {
    case Panel::char_lines:
    {
        const Pos pixel_pos = get_pixel_pos_for_cell_in_panel(panel, Pos(0, 0));
        cover_area_pixel(pixel_pos, Pos(SCREEN_PIXEL_W, Config::get_char_lines_pixel_h()));
    } break;

    case Panel::log:
    {
        cover_area_pixel(Pos(0, 0), Pos(SCREEN_PIXEL_W, Config::get_log_pixel_h()));
    } break;

    case Panel::map:
    {
        const Pos pixel_pos = get_pixel_pos_for_cell_in_panel(panel, Pos(0, 0));
        cover_area_pixel(pixel_pos, Pos(SCREEN_PIXEL_W, Config::get_map_pixel_h()));
    } break;

    case Panel::screen: {clear_screen();} break;
    }
}

void cover_area(const Panel panel, const Rect& area)
{
    cover_area(panel, area.p0, area.p1 - area.p0 + 1);
}

void cover_area(const Panel panel, const Pos& pos, const Pos& dims)
{
    const Pos pixel_pos = get_pixel_pos_for_cell_in_panel(panel, pos);
    const Pos cell_dims(Config::get_cell_w(), Config::get_cell_h());
    cover_area_pixel(pixel_pos, dims * cell_dims);
}

void cover_area_pixel(const Pos& pixel_pos, const Pos& pixel_dims)
{
    draw_rectangle_solid(pixel_pos, pixel_dims, clr_black);
}

void cover_cell_in_map(const Pos& pos)
{
    const Pos cell_dims(Config::get_cell_w(), Config::get_cell_h());
    Pos pixel_pos = get_pixel_pos_for_cell_in_panel(Panel::map, pos);
    cover_area_pixel(pixel_pos, cell_dims);
}

void draw_line_hor(const Pos& pixel_pos, const int W, const Clr& clr)
{
    draw_rectangle_solid(pixel_pos, Pos(W, 2), clr);
}

void draw_line_ver(const Pos& pixel_pos, const int H, const Clr& clr)
{
    draw_rectangle_solid(pixel_pos, Pos(1, H), clr);
}

void draw_rectangle_solid(const Pos& pixel_pos, const Pos& pixel_dims, const Clr& clr)
{
    if (is_inited())
    {
        SDL_Rect sdl_rect = {(Sint16)pixel_pos.x, (Sint16)pixel_pos.y,
                            (Uint16)pixel_dims.x, (Uint16)pixel_dims.y
                           };

        SDL_Fill_rect(screen_surface, &sdl_rect,
                     SDL_Map_rGB(screen_surface->format, clr.r, clr.g, clr.b));
    }
}

void draw_projectiles(vector<Projectile*>& projectiles,
                     const bool SHOULD_DRAW_MAP_BEFORE)
{

    if (SHOULD_DRAW_MAP_BEFORE) {draw_map_and_interface(false);}

    for (Projectile* p : projectiles)
    {
        if (!p->is_done_rendering && p->is_visible_to_player)
        {
            cover_cell_in_map(p->pos);
            if (Config::is_tiles_mode())
            {
                if (p->tile != Tile_id::empty)
                {
                    draw_tile(p->tile, Panel::map, p->pos, p->clr);
                }
            }
            else
            {
                if (p->glyph != -1) {draw_glyph(p->glyph, Panel::map, p->pos, p->clr);}
            }
        }
    }

    update_screen();
}

void draw_popup_box(const Rect& border, const Panel panel, const Clr& clr,
                  const bool COVER_AREA)
{
    if (COVER_AREA) {cover_area(panel, border);}

    const bool IS_TILES = Config::is_tiles_mode();

    //Vertical bars
    const int Y0_VERT = border.p0.y + 1;
    const int Y1_VERT = border.p1.y - 1;
    for (int y = Y0_VERT; y <= Y1_VERT; ++y)
    {
        if (IS_TILES)
        {
            draw_tile(Tile_id::popup_ver, panel, Pos(border.p0.x, y), clr, clr_black);
            draw_tile(Tile_id::popup_ver, panel, Pos(border.p1.x, y), clr, clr_black);
        }
        else
        {
            draw_glyph('|', panel, Pos(border.p0.x, y), clr, true, clr_black);
            draw_glyph('|', panel, Pos(border.p1.x, y), clr, true, clr_black);
        }
    }

    //Horizontal bars
    const int X0_VERT = border.p0.x + 1;
    const int X1_VERT = border.p1.x - 1;
    for (int x = X0_VERT; x <= X1_VERT; ++x)
    {
        if (IS_TILES)
        {
            draw_tile(Tile_id::popup_hor, panel, Pos(x, border.p0.y), clr, clr_black);
            draw_tile(Tile_id::popup_hor, panel, Pos(x, border.p1.y), clr, clr_black);
        }
        else
        {
            draw_glyph('-', panel, Pos(x, border.p0.y), clr, true, clr_black);
            draw_glyph('-', panel, Pos(x, border.p1.y), clr, true, clr_black);
        }
    }

    const vector<Pos> corners
    {
        {border.p0.x, border.p0.y}, //Top left
        {border.p1.x, border.p0.y}, //Top right
        {border.p0.x, border.p1.y}, //Btm left
        {border.p1.x, border.p1.y}  //Brm right
    };

    //Corners
    if (IS_TILES)
    {
        draw_tile(Tile_id::popup_top_l, panel, corners[0], clr, clr_black);
        draw_tile(Tile_id::popup_top_r, panel, corners[1], clr, clr_black);
        draw_tile(Tile_id::popup_btm_l, panel, corners[2], clr, clr_black);
        draw_tile(Tile_id::popup_btm_r, panel, corners[3], clr, clr_black);
    }
    else
    {
        draw_glyph('+', panel, corners[0], clr, true, clr_black);
        draw_glyph('+', panel, corners[1], clr, true, clr_black);
        draw_glyph('+', panel, corners[2], clr, true, clr_black);
        draw_glyph('+', panel, corners[3], clr, true, clr_black);
    }
}

void draw_descr_box(const std::vector<Str_and_clr>& lines)
{
    const int DESCR_Y0  = 2;
    const int DESCR_X1  = MAP_W - 1;
    cover_area(Panel::screen, Rect(DESCR_X0, DESCR_Y0, DESCR_X1, SCREEN_H - 1));

    const int MAX_W = DESCR_X1 - DESCR_X0 + 1;

    Pos p(DESCR_X0, DESCR_Y0);

    for (const auto& line : lines)
    {
        vector<string> formatted;
        Text_format::line_to_lines(line.str, MAX_W, formatted);
        for (const auto& line_in_formatted : formatted)
        {
            draw_text(line_in_formatted, Panel::screen, p, line.clr);
            ++p.y;
        }
        ++p.y;
    }
}

void draw_map_and_interface(const bool SHOULD_UPDATE_SCREEN)
{
    if (is_inited())
    {
        clear_screen();

        draw_map();

        Character_lines::draw();

        Log::draw_log(false);

        if (SHOULD_UPDATE_SCREEN)
        {
            update_screen();
        }
    }
}

void draw_map()
{
    if (!is_inited()) {return;}

    Cell_render_data* cur_drw = nullptr;
    Cell_render_data  tmp_drw;

    const bool IS_TILES = Config::is_tiles_mode();

    //---------------- INSERT RIGIDS AND BLOOD INTO ARRAY
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (Map::cells[x][y].is_seen_by_player)
            {

                render_array[x][y]           = Cell_render_data();
                cur_drw                      = &render_array[x][y];
                const auto* const f         = Map::cells[x][y].rigid;
                Tile_id            gore_tile  = Tile_id::empty;
                char              gore_glyph = 0;

                if (f->can_have_gore())
                {
                    gore_tile  = f->get_gore_tile();
                    gore_glyph = f->get_gore_glyph();
                }
                if (gore_tile == Tile_id::empty)
                {
                    cur_drw->tile  = f->get_tile();
                    cur_drw->glyph = f->get_glyph();
                    cur_drw->clr   = f->get_clr();
                    const Clr& feature_clr_bg = f->get_clr_bg();
                    if (!Utils::is_clr_eq(feature_clr_bg, clr_black))
                    {
                        cur_drw->clr_bg = feature_clr_bg;
                    }
                }
                else
                {
                    cur_drw->tile  = gore_tile;
                    cur_drw->glyph = gore_glyph;
                    cur_drw->clr   = clr_red;
                }
                if (Map::cells[x][y].is_lit && f->can_move_cmn())
                {
                    cur_drw->is_marked_lit = true;
                }
            }
        }
    }

    //---------------- INSERT DEAD ACTORS INTO ARRAY
    for (Actor* actor : Game_time::actors_)
    {
        const Pos& p(actor->pos);
        if (
            actor->is_corpse()                       &&
            actor->get_data().glyph != ' '           &&
            actor->get_data().tile != Tile_id::empty  &&
            Map::cells[p.x][p.y].is_seen_by_player)
        {
            cur_drw        = &render_array[p.x][p.y];
            cur_drw->clr   = actor->get_clr();
            cur_drw->tile  = actor->get_tile();
            cur_drw->glyph = actor->get_glyph();
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            cur_drw = &render_array[x][y];
            if (Map::cells[x][y].is_seen_by_player)
            {
                //---------------- INSERT ITEMS INTO ARRAY
                const Item* const item = Map::cells[x][y].item;
                if (item)
                {
                    cur_drw->clr   = item->get_clr();
                    cur_drw->tile  = item->get_tile();
                    cur_drw->glyph = item->get_glyph();
                }

                //COPY ARRAY TO PLAYER MEMORY (BEFORE LIVING ACTORS AND MOBILE FEATURES)
                render_array_no_actors[x][y] = render_array[x][y];

                //COLOR CELLS MARKED AS LIT YELLOW
                if (cur_drw->is_marked_lit)
                {
                    cur_drw->clr.r = min(255, cur_drw->clr.r + 40);
                    cur_drw->clr.g = min(255, cur_drw->clr.g + 40);
                    cur_drw->clr.b = min(255, cur_drw->clr.b + 20);
                }
            }
        }
    }

    //---------------- INSERT MOBILE FEATURES INTO ARRAY
    for (auto* mob : Game_time::mobs_)
    {
        const Pos& p            = mob->get_pos();
        const Tile_id  mob_tile   = mob->get_tile();
        const char    mob_glyph  = mob->get_glyph();
        if (
            mob_tile != Tile_id::empty && mob_glyph != ' ' &&
            Map::cells[p.x][p.y].is_seen_by_player)
        {
            cur_drw = &render_array[p.x][p.y];
            cur_drw->clr   = mob->get_clr();
            cur_drw->tile  = mob_tile;
            cur_drw->glyph = mob_glyph;
        }
    }

    //---------------- INSERT LIVING ACTORS INTO ARRAY
    for (auto* actor : Game_time::actors_)
    {
        if (actor != Map::player)
        {
            if (actor->is_alive())
            {
                const Pos& p  = actor->pos;
                cur_drw        = &render_array[p.x][p.y];

                const auto* const mon = static_cast<const Mon*>(actor);

                if (Map::player->can_see_actor(*actor, nullptr))
                {
                    if (actor->get_tile()  != Tile_id::empty && actor->get_glyph() != ' ')
                    {
                        cur_drw->clr   = actor->get_clr();
                        cur_drw->tile  = actor->get_tile();
                        cur_drw->glyph = actor->get_glyph();

                        cur_drw->lifebar_length         = get_lifebar_length(*actor);
                        cur_drw->is_living_actor_seen_here = true;
                        cur_drw->is_light_fade_allowed    = false;

                        if (Map::player->is_leader_of(mon))
                        {
                            cur_drw->clr_bg = clr_green;
                        }
                        else
                        {
                            if (mon->aware_counter_ <= 0) {cur_drw->clr_bg = clr_blue;}
                        }
                    }
                }
                else //Player is not seeing actor
                {
                    if (mon->player_aware_of_me_counter_ > 0 || Map::player->is_leader_of(mon))
                    {
                        cur_drw->is_aware_of_mon_here  = true;
                    }
                }
            }
        }
    }

    //---------------- DRAW THE GRID
    auto div_clr = [](Clr & clr, const double DIV)
    {
        clr.r = double(clr.r) / DIV;
        clr.g = double(clr.g) / DIV;
        clr.b = double(clr.b) / DIV;
    };

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {

            tmp_drw = render_array[x][y];

            const Cell& cell = Map::cells[x][y];

            if (cell.is_seen_by_player)
            {
                if (tmp_drw.is_light_fade_allowed)
                {
                    const int DIST_FROM_PLAYER =
                        Utils::king_dist(Map::player->pos, Pos(x, y));

                    if (DIST_FROM_PLAYER > 1)
                    {
                        const double DIV =
                            min(2.0, 1.0 + (double(DIST_FROM_PLAYER - 1) * 0.33));
                        div_clr(tmp_drw.clr,    DIV);
                        div_clr(tmp_drw.clr_bg,  DIV);
                    }

                    if (cell.is_dark && !cell.is_lit)
                    {
                        const double DRK_DIV = 1.75;
                        div_clr(tmp_drw.clr,    DRK_DIV);
                        div_clr(tmp_drw.clr_bg,  DRK_DIV);
                    }
                }
            }
            else if (cell.is_explored)
            {
                bool is_aware_of_mon_here   = tmp_drw.is_aware_of_mon_here;
                render_array[x][y]       = cell.player_visual_memory;
                tmp_drw                  = render_array[x][y];
                tmp_drw.is_aware_of_mon_here = is_aware_of_mon_here;

                const double DIV = 5.0;
                div_clr(tmp_drw.clr,    DIV);
                div_clr(tmp_drw.clr_bg,  DIV);
            }

            if (IS_TILES)
            {
                //Walls are given perspective here. If the tile to be set is a (top) wall
                //tile, instead place a front wall tile on any of the current conditions:
                //(1) Cell below is explored, and its tile is not a front or top wall.
                //(2) Cell below is unexplored.
                if (!tmp_drw.is_living_actor_seen_here && !tmp_drw.is_aware_of_mon_here)
                {
                    const auto tile_seen     = render_array_no_actors[x][y].tile;
                    const auto tile_mem      = cell.player_visual_memory.tile;
                    const bool IS_TILE_WALL = cell.is_seen_by_player ?
                                              Wall::is_tile_any_wall_top(tile_seen) :
                                              Wall::is_tile_any_wall_top(tile_mem);
                    if (IS_TILE_WALL)
                    {
                        const auto* const f   = cell.rigid;
                        const auto feature_id  = f->get_id();
                        bool is_hidden_door     = false;
                        if (feature_id == Feature_id::door)
                        {
                            is_hidden_door = static_cast<const Door*>(f)->is_secret();
                        }
                        if (
                            y < MAP_H - 1 &&
                            (feature_id == Feature_id::wall || is_hidden_door))
                        {
                            if (Map::cells[x][y + 1].is_explored)
                            {
                                const bool IS_SEEN_BELOW  =
                                    Map::cells[x][y + 1].is_seen_by_player;

                                const auto tile_below_seen  =
                                    render_array_no_actors[x][y + 1].tile;

                                const auto tile_below_mem   =
                                    Map::cells[x][y + 1].player_visual_memory.tile;

                                const bool TILE_BELOW_IS_WALL_FRONT =
                                    IS_SEEN_BELOW ?
                                    Wall::is_tile_any_wall_front(tile_below_seen) :
                                    Wall::is_tile_any_wall_front(tile_below_mem);

                                const bool TILE_BELOW_IS_WALL_TOP =
                                    IS_SEEN_BELOW ?
                                    Wall::is_tile_any_wall_top(tile_below_seen) :
                                    Wall::is_tile_any_wall_top(tile_below_mem);

                                bool tile_below_is_revealed_door =
                                    IS_SEEN_BELOW ?
                                    Door::is_tile_any_door(tile_below_seen) :
                                    Door::is_tile_any_door(tile_below_mem);

                                if (
                                    TILE_BELOW_IS_WALL_FRONT  ||
                                    TILE_BELOW_IS_WALL_TOP    ||
                                    tile_below_is_revealed_door)
                                {
                                    if (feature_id == Feature_id::wall)
                                    {
                                        const auto* const wall =
                                            static_cast<const Wall*>(f);
                                        tmp_drw.tile = wall->get_top_wall_tile();
                                    }
                                }
                                else if (feature_id == Feature_id::wall)
                                {
                                    const auto* const wall  = static_cast<const Wall*>(f);
                                    tmp_drw.tile             = wall->get_front_wall_tile();
                                }
                                else if (is_hidden_door)
                                {
                                    tmp_drw.tile = Config::is_tiles_wall_full_square() ?
                                                  Tile_id::wall_top :
                                                  Tile_id::wall_front;
                                }
                            }
                            else //Cell below is not explored
                            {
                                const Rigid* wall = nullptr;
                                if (is_hidden_door)
                                {
                                    wall = static_cast<const Door*>(f)->get_mimic();
                                }
                                else
                                {
                                    wall = f;
                                }
                                tmp_drw.tile =
                                    static_cast<const Wall*>(wall)->get_front_wall_tile();
                            }
                        }
                    }
                }
            }

            Pos pos(x, y);

            if (tmp_drw.is_aware_of_mon_here)
            {
                draw_glyph('!', Panel::map, pos, clr_black, true, clr_nosf_teal_drk);
            }
            else if (tmp_drw.tile != Tile_id::empty && tmp_drw.glyph != ' ')
            {
                if (IS_TILES)
                {
                    draw_tile(tmp_drw.tile, Panel::map, pos, tmp_drw.clr, tmp_drw.clr_bg);
                }
                else
                {
                    draw_glyph(tmp_drw.glyph, Panel::map, pos, tmp_drw.clr, true,
                              tmp_drw.clr_bg);
                }

                if (tmp_drw.lifebar_length != -1)
                {
                    draw_life_bar(pos, tmp_drw.lifebar_length);
                }
            }

            if (!cell.is_explored) {render_array[x][y] = Cell_render_data();}
        }
    }

    //---------------- DRAW PLAYER CHARACTER
    bool        is_ranged_wpn = false;
    const Pos&  pos         = Map::player->pos;
    Item*       item        = Map::player->get_inv().get_item_in_slot(Slot_id::wielded);
    if (item)
    {
        is_ranged_wpn = item->get_data().ranged.is_ranged_wpn;
    }
    if (IS_TILES)
    {
        const Tile_id tile = is_ranged_wpn ? Tile_id::player_firearm : Tile_id::player_melee;
        draw_tile(tile, Panel::map, pos, Map::player->get_clr(), clr_black);
    }
    else
    {
        draw_glyph('@', Panel::map, pos, Map::player->get_clr(), true, clr_black);
    }

    const int LIFE_BAR_LENGTH = get_lifebar_length(*Map::player);

    if (LIFE_BAR_LENGTH != -1)
    {
        draw_life_bar(pos, LIFE_BAR_LENGTH);
    }
    draw_player_shock_excl_marks();
}

} //Render
