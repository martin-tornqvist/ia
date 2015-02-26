#include "render.hpp"

#include <vector>
#include <iostream>

#include "init.hpp"
#include "item.hpp"
#include "character_lines.hpp"
#include "marker.hpp"
#include "map.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "msg_log.hpp"
#include "attack.hpp"
#include "feature_mob.hpp"
#include "feature_door.hpp"
#include "inventory.hpp"
#include "utils.hpp"
#include "cmn_data.hpp"
#include "sdl_wrapper.hpp"
#include "text_format.hpp"

using namespace std;

namespace render
{

Cell_render_data render_array[MAP_W][MAP_H];
Cell_render_data render_array_no_actors[MAP_W][MAP_H];

namespace
{

SDL_Window*     sdl_window_         = nullptr;
SDL_Renderer*   sdl_renderer_       = nullptr;

SDL_Surface*    screen_srf_         = nullptr;
SDL_Texture*    screen_texture_     = nullptr;

SDL_Surface*    main_menu_logo_srf_ = nullptr;

const size_t PIXEL_DATA_W = 400;
const size_t PIXEL_DATA_H = 400;

bool tile_px_data_[PIXEL_DATA_W][PIXEL_DATA_H];
bool font_px_data_[PIXEL_DATA_W][PIXEL_DATA_H];
bool contour_px_data_[PIXEL_DATA_W][PIXEL_DATA_H];

bool is_inited()
{
    return sdl_window_;
}

Uint32 get_px(SDL_Surface& srf, const int PIXEL_X, const int PIXEL_Y)
{
    const int BPP = srf.format->BytesPerPixel;

    //p is the address to the pixel we want to retrieve
    Uint8* p = (Uint8*)srf.pixels + PIXEL_Y * srf.pitch + PIXEL_X * BPP;

    switch (BPP)
    {
    case 1:
        return *p;

    case 2:
        return *(Uint16*)p;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            return p[0] << 16 | p[1] << 8 | p[2];
        }
        else
        {
            return p[0] | p[1] << 8 | p[2] << 16;
        }
        break;

    case 4:
        return *(Uint32*)p;

    default:
        break;
    }

    return -1;
}

void put_px(const SDL_Surface& srf, const int PIXEL_X, const int PIXEL_Y, Uint32 px)
{
    const int BPP = srf.format->BytesPerPixel;

    //p is the address to the pixel we want to set
    Uint8* const p = (Uint8*)srf.pixels + PIXEL_Y * srf.pitch + PIXEL_X * BPP;

    switch (BPP)
    {
    case 1:
        *p = px;
        break;

    case 2:
        *(Uint16*)p = px;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (px >> 16) & 0xff;
            p[1] = (px >> 8)  & 0xff;
            p[2] = px & 0xff;
        }
        else //Little endian
        {
            p[0] = px & 0xff;
            p[1] = (px >> 8)  & 0xff;
            p[2] = (px >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32*)p = px;
        break;

    default:
        break;
    }
}

void blit_surface(SDL_Surface& srf, const Pos& px_pos)
{
    SDL_Rect dst_rect
    {
        px_pos.x, px_pos.y, srf.w, srf.h
    };

    SDL_BlitSurface(&srf, nullptr, screen_srf_, &dst_rect);
}

void load_main_menu_logo()
{
    TRACE_FUNC_BEGIN;

    SDL_Surface* main_menu_logo_srf_tmp = IMG_Load(main_menu_logo_img_name.c_str());

    assert(main_menu_logo_srf_tmp && "Failed to load main menu logo");

    main_menu_logo_srf_ = SDL_ConvertSurface(main_menu_logo_srf_tmp, screen_srf_->format, 0);

    SDL_FreeSurface(main_menu_logo_srf_tmp);

    TRACE_FUNC_END;
}

void load_font()
{
    TRACE_FUNC_BEGIN;

    SDL_Surface* font_srf_tmp = IMG_Load(config::get_font_name().data());

    Uint32 img_clr = SDL_MapRGB(font_srf_tmp->format, 255, 255, 255);

    for (int x = 0; x < font_srf_tmp->w; ++x)
    {
        for (int y = 0; y < font_srf_tmp->h; ++y)
        {
            const bool IS_IMG_PX = get_px(*font_srf_tmp, x, y) == img_clr;

            font_px_data_[x][y] = IS_IMG_PX;
        }
    }

    SDL_FreeSurface(font_srf_tmp);

    TRACE_FUNC_END;
}

void load_tiles()
{
    TRACE_FUNC_BEGIN;

    SDL_Surface* tile_srf_tmp = IMG_Load(tiles_img_name.data());

    Uint32 img_clr = SDL_MapRGB(tile_srf_tmp->format, 255, 255, 255);

    for (int x = 0; x < tile_srf_tmp->w; ++x)
    {
        for (int y = 0; y < tile_srf_tmp->h; ++y)
        {
            const bool IS_IMG_PX = get_px(*tile_srf_tmp, x, y) == img_clr;

            tile_px_data_[x][y] = IS_IMG_PX;
        }
    }

    SDL_FreeSurface(tile_srf_tmp);

    TRACE_FUNC_END;
}

void load_contour(const bool base[PIXEL_DATA_W][PIXEL_DATA_H])
{
    const Pos cell_dims(config::get_cell_w(), config::get_cell_h());

    for (size_t px_x = 0; px_x < PIXEL_DATA_W; ++px_x)
    {
        for (size_t px_y = 0; px_y < PIXEL_DATA_H; ++px_y)
        {

            bool& cur_val  = contour_px_data_[px_x][px_y];
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

                    if (cur_val)
                    {
                        break;
                    }
                }
            }
        }
    }
}

void put_pixels_on_scr(const bool px_data[PIXEL_DATA_W][PIXEL_DATA_H],
                       const Pos& sheet_pos, const Pos& scr_px_pos, const Clr& clr)
{
    if (is_inited())
    {
        const int PX_CLR = SDL_MapRGB(screen_srf_->format, clr.r, clr.g, clr.b);

        const int CELL_W      = config::get_cell_w();
        const int CELL_H      = config::get_cell_h();
        const int SHEET_PX_X0 = sheet_pos.x * CELL_W;
        const int SHEET_PX_Y0 = sheet_pos.y * CELL_H;
        const int SHEET_PX_X1 = SHEET_PX_X0 + CELL_W - 1;
        const int SHEET_PX_Y1 = SHEET_PX_Y0 + CELL_H - 1;
        const int SCR_PX_X0   = scr_px_pos.x;
        const int SCR_PX_Y0   = scr_px_pos.y;

        int scr_px_x = SCR_PX_X0;

        for (int sheet_px_x = SHEET_PX_X0; sheet_px_x <= SHEET_PX_X1; sheet_px_x++)
        {
            int scr_px_y = SCR_PX_Y0;

            for (int sheet_px_y = SHEET_PX_Y0; sheet_px_y <= SHEET_PX_Y1; sheet_px_y++)
            {
                if (px_data[sheet_px_x][sheet_px_y])
                {
                    put_px(*screen_srf_, scr_px_x, scr_px_y, PX_CLR);
                }
                ++scr_px_y;
            }
            ++scr_px_x;
        }
    }
}

void put_pixels_on_scr_for_tile(const Tile_id tile, const Pos& scr_px_pos, const Clr& clr)
{
    put_pixels_on_scr(tile_px_data_, art::get_tile_pos(tile), scr_px_pos, clr);
}

void put_pixels_on_scr_for_glyph(const char GLYPH, const Pos& scr_px_pos, const Clr& clr)
{
    put_pixels_on_scr(font_px_data_, art::get_glyph_pos(GLYPH), scr_px_pos, clr);
}

Pos get_px_pos_for_cell_in_panel(const Panel panel, const Pos& pos)
{
    const Pos cell_dims(config::get_cell_w(), config::get_cell_h());

    switch (panel)
    {
    case Panel::screen:
        return Pos(pos.x * cell_dims.x, pos.y * cell_dims.y);

    case Panel::map:
        return (pos * cell_dims) + Pos(0, config::get_map_px_offset_h());

    case Panel::log:
        return pos * cell_dims;

    case Panel::char_lines:
        return (pos * cell_dims) + Pos(0, config::get_char_lines_px_offset_h());
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
        return ((config::get_cell_w() - 2) * HP_PERCENT) / 100;
    }

    return -1;
}

void draw_life_bar(const Pos& pos, const int LENGTH)
{
    if (LENGTH >= 0)
    {
        const Pos cell_dims(config::get_cell_w(),  config::get_cell_h());
        const int W_GREEN   = LENGTH;
        const int W_BAR_TOT = cell_dims.x - 2;
        const int W_RED     = W_BAR_TOT - W_GREEN;
        const Pos px_pos    = get_px_pos_for_cell_in_panel(Panel::map, pos + Pos(0, 1)) - Pos(0, 2);
        const int X0_GREEN  = px_pos.x + 1;
        const int X0_RED    = X0_GREEN + W_GREEN;

        if (W_GREEN > 0)
        {
            draw_line_hor(Pos(X0_GREEN, px_pos.y), W_GREEN, clr_green_lgt);
        }

        if (W_RED > 0)
        {
            draw_line_hor(Pos(X0_RED, px_pos.y), W_RED, clr_red_lgt);
        }
    }
}

void draw_excl_mark_at(const Pos& px_pos)
{
    draw_rectangle_solid(px_pos, Pos(3, 12), clr_black);
    draw_line_ver(px_pos + Pos(1,  1), 6, clr_magenta_lgt);
    draw_line_ver(px_pos + Pos(1,  9), 2, clr_magenta_lgt);
}

void draw_player_shock_excl_marks()
{
    const double SHOCK  = map::player->get_perm_shock_taken_cur_turn();
    const int NR_EXCL   = SHOCK > 8 ? 3 : SHOCK > 3 ? 2 : SHOCK > 1 ? 1 : 0;

    if (NR_EXCL > 0)
    {
        const Pos& player_pos = map::player->pos;
        const Pos px_pos_right = get_px_pos_for_cell_in_panel(Panel::map, player_pos);

        for (int i = 0; i < NR_EXCL; ++i)
        {
            draw_excl_mark_at(px_pos_right + Pos(i * 3, 0));
        }
    }
}

void draw_glyph_at_px(const char GLYPH, const Pos& px_pos, const Clr& clr,
                      const bool DRAW_BG_CLR, const Clr& bg_clr = clr_black)
{
    if (DRAW_BG_CLR)
    {
        const Pos cell_dims(config::get_cell_w(), config::get_cell_h());

        draw_rectangle_solid(px_pos, cell_dims, bg_clr);

        //Only draw contour if neither the foreground or background is black
        if (!utils::is_clr_eq(clr, clr_black) && !utils::is_clr_eq(bg_clr, clr_black))
        {
            put_pixels_on_scr(contour_px_data_, art::get_glyph_pos(GLYPH), px_pos, clr_black);
        }
    }

    put_pixels_on_scr_for_glyph(GLYPH, px_pos, clr);
}

} //Namespace

void init()
{
    TRACE_FUNC_BEGIN;
    cleanup();

    TRACE << "Setting up rendering window" << endl;

    const string title = "IA " + game_version_str;

    const int SCR_PX_W = config::get_screen_px_w();
    const int SCR_PX_H = config::get_screen_px_h();

    if (config::is_fullscreen())
    {
        sdl_window_ = SDL_CreateWindow(title.c_str(),
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SCR_PX_W, SCR_PX_H,
                                       SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    if (!config::is_fullscreen() || !sdl_window_)
    {
        sdl_window_ = SDL_CreateWindow(title.c_str(),
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SCR_PX_W, SCR_PX_H,
                                       SDL_WINDOW_SHOWN);
    }

    if (!sdl_window_)
    {
        TRACE << "Failed to create window" << endl;
        assert(false);
    }

    sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, SDL_RENDERER_ACCELERATED);

    if (!sdl_renderer_)
    {
        TRACE << "Failed to create SDL renderer" << endl;
        assert(false);
    }

    screen_srf_ = SDL_CreateRGBSurface(0,
                                       SCR_PX_W, SCR_PX_H,
                                       SCREEN_BPP,
                                       0x00FF0000,
                                       0x0000FF00,
                                       0x000000FF,
                                       0xFF000000);

    if (!screen_srf_)
    {
        TRACE << "Failed to create screen surface" << endl;
        assert(false);
    }

    screen_texture_ = SDL_CreateTexture(sdl_renderer_,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        SCR_PX_W, SCR_PX_H);

    if (!screen_texture_)
    {
        TRACE << "Failed to create screen texture" << endl;
        assert(false);
    }

    load_font();

    if (config::is_tiles_mode())
    {
        load_tiles();
        load_main_menu_logo();
    }

    load_contour(config::is_tiles_mode() ? tile_px_data_ : font_px_data_);

    TRACE_FUNC_END;
}

void cleanup()
{
    TRACE_FUNC_BEGIN;

    if (sdl_renderer_)
    {
        SDL_DestroyRenderer(sdl_renderer_);
        sdl_renderer_ = nullptr;
    }

    if (sdl_window_)
    {
        SDL_DestroyWindow(sdl_window_);
        sdl_window_ = nullptr;
    }

    if (screen_texture_)
    {
        SDL_DestroyTexture(screen_texture_);
        screen_texture_ = nullptr;
    }

    if (screen_srf_)
    {
        SDL_FreeSurface(screen_srf_);
        screen_srf_ = nullptr;
    }

    if (main_menu_logo_srf_)
    {
        SDL_FreeSurface(main_menu_logo_srf_);
        main_menu_logo_srf_ = nullptr;
    }

    TRACE_FUNC_END;
}

void on_toggle_fullscreen()
{
    if (config::is_fullscreen())
    {
        SDL_SetWindowFullscreen(sdl_window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else
    {
        SDL_SetWindowFullscreen(sdl_window_, SDL_WINDOW_SHOWN);
    }

    update_screen();
}

void update_screen()
{
    if (is_inited())
    {
        SDL_UpdateTexture(screen_texture_, nullptr, screen_srf_->pixels, screen_srf_->pitch);
        SDL_RenderCopy(sdl_renderer_, screen_texture_, nullptr, nullptr);
        SDL_RenderPresent(sdl_renderer_);
    }
}

void clear_screen()
{
    if (is_inited())
    {
        SDL_FillRect(screen_srf_, nullptr, SDL_MapRGB(screen_srf_->format, 0, 0, 0));
    }
}

void draw_main_menu_logo(const int Y_POS)
{
    const int SCR_PX_W  = config::get_screen_px_w();
    const int LOGO_PX_H = main_menu_logo_srf_->w;
    const int CELL_PX_H = config::get_cell_h();

    const Pos px_pos((SCR_PX_W - LOGO_PX_H) / 2, CELL_PX_H * Y_POS);

    blit_surface(*main_menu_logo_srf_, px_pos);
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
                const int CHEB_DIST = utils::king_dist(trail[0], pos);

                if (CHEB_DIST > EFFECTIVE_RANGE)
                {
                    clr = clr_orange;
                }
            }

            if (config::is_tiles_mode())
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
            const int CHEB_DIST = utils::king_dist(trail[0], p);

            if (CHEB_DIST > EFFECTIVE_RANGE)
            {
                clr = clr_orange;
            }
        }
    }

    if (config::is_tiles_mode())
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

                    if (config::is_tiles_mode())
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

        if (is_any_blast_rendered) {sdl_wrapper::sleep(config::get_delay_explosion() / 2);}

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

                    if (config::is_tiles_mode())
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

        if (is_any_blast_rendered) {sdl_wrapper::sleep(config::get_delay_explosion() / 2);}

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
            if (config::is_tiles_mode())
            {
                draw_tile(Tile_id::blast1, Panel::map, pos, clr, clr_black);
            }
            else
            {
                draw_glyph('*', Panel::map, pos, clr, true, clr_black);
            }
        }

        update_screen();
        sdl_wrapper::sleep(config::get_delay_explosion() / 2);

        for (const Pos& pos : positions)
        {
            if (config::is_tiles_mode())
            {
                draw_tile(Tile_id::blast2, Panel::map, pos, clr, clr_black);
            }
            else
            {
                draw_glyph('*', Panel::map, pos, clr, true, clr_black);
            }
        }

        update_screen();
        sdl_wrapper::sleep(config::get_delay_explosion() / 2);
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
            if (map::cells[p.x][p.y].is_seen_by_player)
            {
                positions_with_vision.push_back(p);
            }
        }

        if (!positions_with_vision.empty())
        {
            render::draw_blast_at_cells(positions_with_vision, clr);
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
        const Pos px_pos = get_px_pos_for_cell_in_panel(panel, pos);
        const Pos cell_dims(config::get_cell_w(), config::get_cell_h());

        draw_rectangle_solid(px_pos, cell_dims, bg_clr);

        if (!utils::is_clr_eq(bg_clr, clr_black))
        {
            put_pixels_on_scr(contour_px_data_, art::get_tile_pos(tile), px_pos, clr_black);
        }

        put_pixels_on_scr_for_tile(tile, px_pos, clr);
    }
}

void draw_glyph(const char GLYPH, const Panel panel, const Pos& pos, const Clr& clr,
                const bool DRAW_BG_CLR, const Clr& bg_clr)
{
    if (is_inited())
    {
        const Pos px_pos = get_px_pos_for_cell_in_panel(panel, pos);
        draw_glyph_at_px(GLYPH, px_pos, clr, DRAW_BG_CLR, bg_clr);
    }
}

void draw_text(const string& str, const Panel panel, const Pos& pos, const Clr& clr,
               const Clr& bg_clr)
{
    if (is_inited())
    {
        Pos px_pos = get_px_pos_for_cell_in_panel(panel, pos);

        if (px_pos.y < 0 || px_pos.y >= config::get_screen_px_h())
        {
            return;
        }

        const Pos cell_dims(config::get_cell_w(), config::get_cell_h());
        const int LEN = str.size();
        draw_rectangle_solid(px_pos, Pos(cell_dims.x * LEN, cell_dims.y), bg_clr);

        for (int i = 0; i < LEN; ++i)
        {
            if (px_pos.x < 0 || px_pos.x >= config::get_screen_px_w())
            {
                return;
            }

            draw_glyph_at_px(str[i], px_pos, clr, false);
            px_pos.x += cell_dims.x;
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

    const Pos cell_dims(config::get_cell_w(), config::get_cell_h());

    Pos px_pos = get_px_pos_for_cell_in_panel(panel, Pos(X_POS_LEFT, pos.y));

    if (IS_PIXEL_POS_ADJ_ALLOWED)
    {
        const int PIXEL_X_ADJ = LEN_HALF * 2 == LEN ? cell_dims.x / 2 : 0;
        px_pos += Pos(PIXEL_X_ADJ, 0);
    }

    const int W_TOT_PIXEL = LEN * cell_dims.x;

    SDL_Rect sdl_rect =
    {
        (Sint16)px_pos.x, (Sint16)px_pos.y,
        (Uint16)W_TOT_PIXEL, (Uint16)cell_dims.y
    };

    SDL_FillRect(screen_srf_, &sdl_rect, SDL_MapRGB(screen_srf_->format, bg_clr.r, bg_clr.g, bg_clr.b));

    for (int i = 0; i < LEN; ++i)
    {
        if (px_pos.x < 0 || px_pos.x >= config::get_screen_px_w())
        {
            return X_POS_LEFT;
        }

        draw_glyph_at_px(str[i], px_pos, clr, false, bg_clr);
        px_pos.x += cell_dims.x;
    }

    return X_POS_LEFT;
}

void cover_panel(const Panel panel)
{
    const int SCREEN_PIXEL_W = config::get_screen_px_w();

    switch (panel)
    {
    case Panel::char_lines:
    {
        const Pos px_pos = get_px_pos_for_cell_in_panel(panel, Pos(0, 0));
        cover_area_px(px_pos, Pos(SCREEN_PIXEL_W, config::get_char_lines_px_h()));
    } break;

    case Panel::log:
    {
        cover_area_px(Pos(0, 0), Pos(SCREEN_PIXEL_W, config::get_log_px_h()));
    } break;

    case Panel::map:
    {
        const Pos px_pos = get_px_pos_for_cell_in_panel(panel, Pos(0, 0));
        cover_area_px(px_pos, Pos(SCREEN_PIXEL_W, config::get_map_px_h()));
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
    const Pos px_pos = get_px_pos_for_cell_in_panel(panel, pos);
    const Pos cell_dims(config::get_cell_w(), config::get_cell_h());
    cover_area_px(px_pos, dims * cell_dims);
}

void cover_area_px(const Pos& px_pos, const Pos& px_dims)
{
    draw_rectangle_solid(px_pos, px_dims, clr_black);
}

void cover_cell_in_map(const Pos& pos)
{
    const Pos cell_dims(config::get_cell_w(), config::get_cell_h());
    Pos px_pos = get_px_pos_for_cell_in_panel(Panel::map, pos);
    cover_area_px(px_pos, cell_dims);
}

void draw_line_hor(const Pos& px_pos, const int W, const Clr& clr)
{
    draw_rectangle_solid(px_pos, Pos(W, 2), clr);
}

void draw_line_ver(const Pos& px_pos, const int H, const Clr& clr)
{
    draw_rectangle_solid(px_pos, Pos(1, H), clr);
}

void draw_rectangle_solid(const Pos& px_pos, const Pos& px_dims, const Clr& clr)
{
    if (is_inited())
    {
        SDL_Rect sdl_rect =
        {
            (Sint16)px_pos.x, (Sint16)px_pos.y,
            (Uint16)px_dims.x, (Uint16)px_dims.y
        };

        SDL_FillRect(screen_srf_, &sdl_rect, SDL_MapRGB(screen_srf_->format, clr.r, clr.g, clr.b));
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

            if (config::is_tiles_mode())
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

void draw_popup_box(const Rect& border, const Panel panel, const Clr& clr, const bool COVER_AREA)
{
    if (COVER_AREA)
    {
        cover_area(panel, border);
    }

    const bool IS_TILES = config::is_tiles_mode();

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
        text_format::line_to_lines(line.str, MAX_W, formatted);

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

        character_lines::draw();

        msg_log::draw(false);

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

    const bool IS_TILES = config::is_tiles_mode();

    //---------------- INSERT RIGIDS AND BLOOD INTO ARRAY
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (map::cells[x][y].is_seen_by_player)
            {

                render_array[x][y]           = Cell_render_data();
                cur_drw                      = &render_array[x][y];
                const auto* const f         = map::cells[x][y].rigid;
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

                    if (!utils::is_clr_eq(feature_clr_bg, clr_black))
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

                if (map::cells[x][y].is_lit && f->can_move_cmn())
                {
                    cur_drw->is_marked_lit = true;
                }
            }
        }
    }

    //---------------- INSERT DEAD ACTORS INTO ARRAY
    for (Actor* actor : game_time::actors_)
    {
        const Pos& p(actor->pos);

        if (
            actor->is_corpse()                       &&
            actor->get_data().glyph != ' '           &&
            actor->get_data().tile != Tile_id::empty  &&
            map::cells[p.x][p.y].is_seen_by_player)
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

            if (map::cells[x][y].is_seen_by_player)
            {
                //---------------- INSERT ITEMS INTO ARRAY
                const Item* const item = map::cells[x][y].item;

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
    for (auto* mob : game_time::mobs_)
    {
        const Pos& p            = mob->get_pos();
        const Tile_id  mob_tile   = mob->get_tile();
        const char    mob_glyph  = mob->get_glyph();

        if (
            mob_tile != Tile_id::empty && mob_glyph != ' ' &&
            map::cells[p.x][p.y].is_seen_by_player)
        {
            cur_drw = &render_array[p.x][p.y];
            cur_drw->clr   = mob->get_clr();
            cur_drw->tile  = mob_tile;
            cur_drw->glyph = mob_glyph;
        }
    }

    //---------------- INSERT LIVING ACTORS INTO ARRAY
    for (auto* actor : game_time::actors_)
    {
        if (actor != map::player)
        {
            if (actor->is_alive())
            {
                const Pos& p  = actor->pos;
                cur_drw        = &render_array[p.x][p.y];

                const auto* const mon = static_cast<const Mon*>(actor);

                if (map::player->can_see_actor(*actor, nullptr))
                {
                    if (actor->get_tile()  != Tile_id::empty && actor->get_glyph() != ' ')
                    {
                        cur_drw->clr   = actor->get_clr();
                        cur_drw->tile  = actor->get_tile();
                        cur_drw->glyph = actor->get_glyph();

                        cur_drw->lifebar_length         = get_lifebar_length(*actor);
                        cur_drw->is_living_actor_seen_here = true;
                        cur_drw->is_light_fade_allowed    = false;

                        if (map::player->is_leader_of(mon))
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
                    if (mon->player_aware_of_me_counter_ > 0 || map::player->is_leader_of(mon))
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

            const Cell& cell = map::cells[x][y];

            if (cell.is_seen_by_player)
            {
                if (tmp_drw.is_light_fade_allowed)
                {
                    const int DIST_FROM_PLAYER =
                        utils::king_dist(map::player->pos, Pos(x, y));

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
                            if (map::cells[x][y + 1].is_explored)
                            {
                                const bool IS_SEEN_BELOW  =
                                    map::cells[x][y + 1].is_seen_by_player;

                                const auto tile_below_seen  =
                                    render_array_no_actors[x][y + 1].tile;

                                const auto tile_below_mem   =
                                    map::cells[x][y + 1].player_visual_memory.tile;

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
                                    tmp_drw.tile = config::is_tiles_wall_full_square() ?
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
    const Pos&  pos         = map::player->pos;
    Item*       item        = map::player->get_inv().get_item_in_slot(Slot_id::wielded);

    if (item)
    {
        is_ranged_wpn = item->get_data().ranged.is_ranged_wpn;
    }

    if (IS_TILES)
    {
        const Tile_id tile = is_ranged_wpn ? Tile_id::playerFirearm : Tile_id::player_melee;
        draw_tile(tile, Panel::map, pos, map::player->get_clr(), clr_black);
    }
    else
    {
        draw_glyph('@', Panel::map, pos, map::player->get_clr(), true, clr_black);
    }

    const int LIFE_BAR_LENGTH = get_lifebar_length(*map::player);

    if (LIFE_BAR_LENGTH != -1)
    {
        draw_life_bar(pos, LIFE_BAR_LENGTH);
    }

    draw_player_shock_excl_marks();
}

} //render
