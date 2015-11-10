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

namespace render
{

Cell_render_data render_array[MAP_W][MAP_H];
Cell_render_data render_array_no_actors[MAP_W][MAP_H];

namespace
{

SDL_Window*     sdl_window_         = nullptr;
SDL_Renderer*   sdl_renderer_       = nullptr;

SDL_Surface*    scr_srf_            = nullptr;
SDL_Texture*    scr_texture_        = nullptr;

SDL_Surface*    main_menu_logo_srf_ = nullptr;
SDL_Surface*    skull_srf_          = nullptr;

const size_t PIXEL_DATA_W = 400;
const size_t PIXEL_DATA_H = 400;

bool tile_px_data_[PIXEL_DATA_W][PIXEL_DATA_H];
bool font_px_data_[PIXEL_DATA_W][PIXEL_DATA_H];
bool contour_px_data_[PIXEL_DATA_W][PIXEL_DATA_H];

bool is_inited()
{
    return sdl_window_;
}

void div_clr(Clr & clr, const double DIV)
{
    clr.r = double(clr.r) / DIV;
    clr.g = double(clr.g) / DIV;
    clr.b = double(clr.b) / DIV;
}

Uint32 px(SDL_Surface& srf, const int PIXEL_X, const int PIXEL_Y)
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

void blit_surface(SDL_Surface& srf, const P& px_pos)
{
    SDL_Rect dst_rect
    {
        px_pos.x, px_pos.y, srf.w, srf.h
    };

    SDL_BlitSurface(&srf, nullptr, scr_srf_, &dst_rect);
}

void load_pictures()
{
    TRACE_FUNC_BEGIN;

    //Main menu logo
    SDL_Surface* tmp_srf = IMG_Load(main_menu_logo_img_name.c_str());

    assert(tmp_srf && "Failed to load main menu logo image");

    main_menu_logo_srf_ = SDL_ConvertSurface(tmp_srf, scr_srf_->format, 0);

    SDL_FreeSurface(tmp_srf);

    //Skull
    tmp_srf = IMG_Load(skull_img_name.c_str());

    assert(tmp_srf && "Failed to load skull image");

    skull_srf_ = SDL_ConvertSurface(tmp_srf, scr_srf_->format, 0);

    SDL_FreeSurface(tmp_srf);

    TRACE_FUNC_END;
}

void load_font()
{
    TRACE_FUNC_BEGIN;

    SDL_Surface* font_srf_tmp = IMG_Load(config::font_name().data());

    Uint32 img_clr = SDL_MapRGB(font_srf_tmp->format, 255, 255, 255);

    for (int x = 0; x < font_srf_tmp->w; ++x)
    {
        for (int y = 0; y < font_srf_tmp->h; ++y)
        {
            const bool IS_IMG_PX = px(*font_srf_tmp, x, y) == img_clr;

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
            const bool IS_IMG_PX = px(*tile_srf_tmp, x, y) == img_clr;

            tile_px_data_[x][y] = IS_IMG_PX;
        }
    }

    SDL_FreeSurface(tile_srf_tmp);

    TRACE_FUNC_END;
}

void load_contour(const bool base[PIXEL_DATA_W][PIXEL_DATA_H])
{
    const P cell_dims(config::cell_px_w(), config::cell_px_h());

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
                const P cur_img_px_p0((P(px_x, px_y) / cell_dims) * cell_dims);
                const P cur_img_px_p1(cur_img_px_p0 + cell_dims - 1);

                const P px_p0(std::max(cur_img_px_p0.x, int(px_x - 1)),
                              std::max(cur_img_px_p0.y, int(px_y - 1)));

                const P px_p1(std::min(cur_img_px_p1.x, int(px_x + 1)),
                              std::min(cur_img_px_p1.y, int(px_y + 1)));

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
                       const P& sheet_pos, const P& scr_px_pos, const Clr& clr)
{
    if (is_inited())
    {
        const int PX_CLR = SDL_MapRGB(scr_srf_->format, clr.r, clr.g, clr.b);

        const int CELL_W      = config::cell_px_w();
        const int CELL_H      = config::cell_px_h();
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
                    put_px(*scr_srf_, scr_px_x, scr_px_y, PX_CLR);
                }
                ++scr_px_y;
            }
            ++scr_px_x;
        }
    }
}

void put_pixels_on_scr_for_tile(const Tile_id tile, const P& scr_px_pos, const Clr& clr)
{
    put_pixels_on_scr(tile_px_data_, art::tile_pos(tile), scr_px_pos, clr);
}

void put_pixels_on_scr_for_glyph(const char GLYPH, const P& scr_px_pos, const Clr& clr)
{
    put_pixels_on_scr(font_px_data_, art::glyph_pos(GLYPH), scr_px_pos, clr);
}

P px_pos_for_cell_in_panel(const Panel panel, const P& pos)
{
    const P cell_dims(config::cell_px_w(), config::cell_px_h());

    switch (panel)
    {
    case Panel::screen:
        return P(pos.x * cell_dims.x, pos.y * cell_dims.y);

    case Panel::map:
        return (pos * cell_dims) + P(0, config::map_px_offset_h());

    case Panel::log:
        return pos * cell_dims;

    case Panel::char_lines:
        return (pos * cell_dims) + P(0, config::char_lines_px_offset_h());
    }

    return P();
}

int lifebar_length(const Actor& actor)
{
    const int ACTOR_HP      = std::max(0, actor.hp());
    const int ACTOR_HP_MAX  = actor.hp_max(true);

    if (ACTOR_HP < ACTOR_HP_MAX)
    {
        int HP_PERCENT = (ACTOR_HP * 100) / ACTOR_HP_MAX;
        return ((config::cell_px_w() - 2) * HP_PERCENT) / 100;
    }

    return -1;
}

void draw_life_bar(const P& pos, const int LENGTH)
{
    if (LENGTH >= 0)
    {
        const P cell_dims(config::cell_px_w(),  config::cell_px_h());
        const int W_GREEN   = LENGTH;
        const int W_BAR_TOT = cell_dims.x - 2;
        const int W_RED     = W_BAR_TOT - W_GREEN;
        const P px_pos    = px_pos_for_cell_in_panel(Panel::map, pos + P(0, 1)) - P(0, 2);
        const int X0_GREEN  = px_pos.x + 1;
        const int X0_RED    = X0_GREEN + W_GREEN;

        if (W_GREEN > 0)
        {
            draw_line_hor(P(X0_GREEN, px_pos.y), W_GREEN, clr_green_lgt);
        }

        if (W_RED > 0)
        {
            draw_line_hor(P(X0_RED, px_pos.y), W_RED, clr_red_lgt);
        }
    }
}

void draw_excl_mark_at(const P& px_pos)
{
    draw_rectangle_solid(px_pos, P(3, 12), clr_black);
    draw_line_ver(px_pos + P(1,  1), 6, clr_magenta_lgt);
    draw_line_ver(px_pos + P(1,  9), 2, clr_magenta_lgt);
}

void draw_player_shock_excl_marks()
{
    const double SHOCK  = map::player->perm_shock_taken_cur_turn();
    const int NR_EXCL   = SHOCK > 8 ? 3 : SHOCK > 3 ? 2 : SHOCK > 1 ? 1 : 0;

    if (NR_EXCL > 0)
    {
        const P& player_pos = map::player->pos;
        const P px_pos_right = px_pos_for_cell_in_panel(Panel::map, player_pos);

        for (int i = 0; i < NR_EXCL; ++i)
        {
            draw_excl_mark_at(px_pos_right + P(i * 3, 0));
        }
    }
}

void draw_glyph_at_px(const char GLYPH, const P& px_pos, const Clr& clr,
                      const bool DRAW_BG_CLR, const Clr& bg_clr = clr_black)
{
    if (DRAW_BG_CLR)
    {
        const P cell_dims(config::cell_px_w(), config::cell_px_h());

        draw_rectangle_solid(px_pos, cell_dims, bg_clr);

        //Only draw contour if neither the foreground or background is black
        if (!utils::is_clr_eq(clr, clr_black) && !utils::is_clr_eq(bg_clr, clr_black))
        {
            put_pixels_on_scr(contour_px_data_, art::glyph_pos(GLYPH), px_pos, clr_black);
        }
    }

    put_pixels_on_scr_for_glyph(GLYPH, px_pos, clr);
}

} //Namespace

void init()
{
    TRACE_FUNC_BEGIN;
    cleanup();

    TRACE << "Setting up rendering window" << std::endl;

    const std::string title = "IA " + version_str;

    const int SCR_PX_W = config::scr_px_w();
    const int SCR_PX_H = config::scr_px_h();

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
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       SCR_PX_W, SCR_PX_H,
                                       SDL_WINDOW_SHOWN);
    }

    if (!sdl_window_)
    {
        TRACE << "Failed to create window" << std::endl;
        assert(false);
    }

    sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, SDL_RENDERER_ACCELERATED);

    if (!sdl_renderer_)
    {
        TRACE << "Failed to create SDL renderer" << std::endl;
        assert(false);
    }

    scr_srf_ = SDL_CreateRGBSurface(0,
                                    SCR_PX_W, SCR_PX_H,
                                    SCREEN_BPP,
                                    0x00FF0000,
                                    0x0000FF00,
                                    0x000000FF,
                                    0xFF000000);

    if (!scr_srf_)
    {
        TRACE << "Failed to create screen surface" << std::endl;
        assert(false);
    }

    scr_texture_ = SDL_CreateTexture(sdl_renderer_,
                                     SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     SCR_PX_W, SCR_PX_H);

    if (!scr_texture_)
    {
        TRACE << "Failed to create screen texture" << std::endl;
        assert(false);
    }

    load_font();

    if (config::is_tiles_mode())
    {
        load_tiles();
        load_pictures();
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

    if (scr_texture_)
    {
        SDL_DestroyTexture(scr_texture_);
        scr_texture_ = nullptr;
    }

    if (scr_srf_)
    {
        SDL_FreeSurface(scr_srf_);
        scr_srf_ = nullptr;
    }

    if (main_menu_logo_srf_)
    {
        SDL_FreeSurface(main_menu_logo_srf_);
        main_menu_logo_srf_ = nullptr;
    }

    if (skull_srf_)
    {
        SDL_FreeSurface(skull_srf_);
        skull_srf_ = nullptr;
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
        SDL_UpdateTexture(scr_texture_, nullptr, scr_srf_->pixels, scr_srf_->pitch);
        SDL_RenderCopy(sdl_renderer_, scr_texture_, nullptr, nullptr);
        SDL_RenderPresent(sdl_renderer_);
    }
}

void clear_screen()
{
    if (is_inited())
    {
        SDL_FillRect(scr_srf_, nullptr, SDL_MapRGB(scr_srf_->format, 0, 0, 0));
    }
}

void draw_main_menu_logo(const int Y_POS)
{
    const int SCR_PX_W  = config::scr_px_w();
    const int LOGO_PX_H = main_menu_logo_srf_->w;
    const int CELL_PX_H = config::cell_px_h();

    const P px_pos((SCR_PX_W - LOGO_PX_H) / 2, CELL_PX_H * Y_POS);

    blit_surface(*main_menu_logo_srf_, px_pos);
}

void draw_skull(const P& p)
{
    const int CELL_PX_W = config::cell_px_w();
    const int CELL_PX_H = config::cell_px_h();

    const P px_pos(p * P(CELL_PX_W, CELL_PX_H));

    blit_surface(*skull_srf_, px_pos);
}

void draw_marker(const P& p, const std::vector<P>& trail, const int EFFECTIVE_RANGE)
{
    if (trail.size() > 2)
    {
        for (size_t i = 1; i < trail.size(); ++i)
        {
            const P& pos = trail[i];
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

void draw_blast_at_field(const P& center_pos,
                         const int RADIUS,
                         bool forbidden_cells[MAP_W][MAP_H],
                         const Clr& clr_inner,
                         const Clr& clr_outer)
{
    TRACE_FUNC_BEGIN;

    if (is_inited())
    {
        draw_map_and_interface();

        bool is_any_blast_rendered = false;

        P pos;

        for (
            pos.y = std::max(1, center_pos.y - RADIUS);
            pos.y <= std::min(MAP_H - 2, center_pos.y + RADIUS);
            pos.y++)
        {
            for (
                pos.x = std::max(1, center_pos.x - RADIUS);
                pos.x <= std::min(MAP_W - 2, center_pos.x + RADIUS);
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

        if (is_any_blast_rendered) {sdl_wrapper::sleep(config::delay_explosion() / 2);}

        for (
            pos.y = std::max(1, center_pos.y - RADIUS);
            pos.y <= std::min(MAP_H - 2, center_pos.y + RADIUS);
            pos.y++)
        {
            for (
                pos.x = std::max(1, center_pos.x - RADIUS);
                pos.x <= std::min(MAP_W - 2, center_pos.x + RADIUS);
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

        if (is_any_blast_rendered) {sdl_wrapper::sleep(config::delay_explosion() / 2);}

        draw_map_and_interface();
    }

    TRACE_FUNC_END;
}

void draw_blast_at_cells(const std::vector<P>& positions, const Clr& clr)
{
    TRACE_FUNC_BEGIN;

    if (is_inited())
    {
        draw_map_and_interface();

        for (const P& pos : positions)
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
        sdl_wrapper::sleep(config::delay_explosion() / 2);

        for (const P& pos : positions)
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
        sdl_wrapper::sleep(config::delay_explosion() / 2);
        draw_map_and_interface();
    }

    TRACE_FUNC_END;
}

void draw_blast_at_seen_cells(const std::vector<P>& positions, const Clr& clr)
{
    if (is_inited())
    {
        std::vector<P> positions_with_vision;

        for (const P& p : positions)
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
        std::vector<P> positions;

        for (Actor* const actor : actors)
        {
            positions.push_back(actor->pos);
        }

        draw_blast_at_seen_cells(positions, clr);
    }
}

void draw_tile(const Tile_id tile, const Panel panel, const P& pos, const Clr& clr,
               const Clr& bg_clr)
{
    if (is_inited())
    {
        const P px_pos = px_pos_for_cell_in_panel(panel, pos);
        const P cell_dims(config::cell_px_w(), config::cell_px_h());

        draw_rectangle_solid(px_pos, cell_dims, bg_clr);

        if (!utils::is_clr_eq(bg_clr, clr_black))
        {
            put_pixels_on_scr(contour_px_data_, art::tile_pos(tile), px_pos, clr_black);
        }

        put_pixels_on_scr_for_tile(tile, px_pos, clr);
    }
}

void draw_glyph(const char GLYPH, const Panel panel, const P& pos, const Clr& clr,
                const bool DRAW_BG_CLR, const Clr& bg_clr)
{
    if (is_inited())
    {
        const P px_pos = px_pos_for_cell_in_panel(panel, pos);
        draw_glyph_at_px(GLYPH, px_pos, clr, DRAW_BG_CLR, bg_clr);
    }
}

void draw_text(const std::string& str, const Panel panel, const P& pos, const Clr& clr,
               const Clr& bg_clr)
{
    if (is_inited())
    {
        P px_pos = px_pos_for_cell_in_panel(panel, pos);

        if (px_pos.y < 0 || px_pos.y >= config::scr_px_h())
        {
            return;
        }

        const int       CELL_PX_W   = config::cell_px_w();
        const int       CELL_PX_H   = config::cell_px_h();
        const size_t    MSG_W       = str.size();
        const int       MSG_PX_W    = MSG_W * CELL_PX_W;

        draw_rectangle_solid(px_pos, {MSG_PX_W, CELL_PX_H}, bg_clr);

        const int   SCR_PX_W            = config::scr_px_w();
        const int   MSG_PX_X1           = px_pos.x + MSG_PX_W - 1;
        const bool  IS_MSG_W_FIT_ON_SCR = MSG_PX_X1 < SCR_PX_W;

        //X position to start drawing dots instead when the message does not fit
        //on the screen horizontally.
        const int   PX_X_DOTS           = SCR_PX_W - (CELL_PX_W * 3);

        for (size_t i = 0; i < MSG_W; ++i)
        {
            if (px_pos.x < 0 || px_pos.x >= SCR_PX_W)
            {
                return;
            }

            const bool DRAW_DOTS = !IS_MSG_W_FIT_ON_SCR && px_pos.x >= PX_X_DOTS;

            if (DRAW_DOTS)
            {
                draw_glyph_at_px('.', px_pos, clr_gray, false);
            }
            else //Whole message fits, or we are not yet near the screen edge
            {
                draw_glyph_at_px(str[i], px_pos, clr, false);
            }

            px_pos.x += CELL_PX_W;
        }
    }
}

int draw_text_center(const std::string& str, const Panel panel, const P& pos,
                     const Clr& clr, const Clr& bg_clr,
                     const bool IS_PIXEL_POS_ADJ_ALLOWED)
{
    if (!is_inited())
    {
        return 0;
    }

    const int LEN         = str.size();
    const int LEN_HALF    = LEN / 2;
    const int X_POS_LEFT  = pos.x - LEN_HALF;

    const P cell_dims(config::cell_px_w(), config::cell_px_h());

    P px_pos = px_pos_for_cell_in_panel(panel, P(X_POS_LEFT, pos.y));

    if (IS_PIXEL_POS_ADJ_ALLOWED)
    {
        const int PIXEL_X_ADJ = LEN_HALF * 2 == LEN ? cell_dims.x / 2 : 0;
        px_pos += P(PIXEL_X_ADJ, 0);
    }

    const int W_TOT_PIXEL = LEN * cell_dims.x;

    SDL_Rect sdl_rect =
    {
        (Sint16)px_pos.x, (Sint16)px_pos.y,
        (Uint16)W_TOT_PIXEL, (Uint16)cell_dims.y
    };

    SDL_FillRect(scr_srf_, &sdl_rect,
                 SDL_MapRGB(scr_srf_->format, bg_clr.r, bg_clr.g, bg_clr.b));

    for (int i = 0; i < LEN; ++i)
    {
        if (px_pos.x < 0 || px_pos.x >= config::scr_px_w())
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
    const int SCREEN_PIXEL_W = config::scr_px_w();

    switch (panel)
    {
    case Panel::char_lines:
    {
        const P px_pos = px_pos_for_cell_in_panel(panel, P(0, 0));
        cover_area_px(px_pos, P(SCREEN_PIXEL_W, config::char_lines_px_h()));
    }
    break;

    case Panel::log:
        cover_area_px(P(0, 0), P(SCREEN_PIXEL_W, config::log_px_h()));
        break;

    case Panel::map:
    {
        const P px_pos = px_pos_for_cell_in_panel(panel, P(0, 0));
        cover_area_px(px_pos, P(SCREEN_PIXEL_W, config::map_px_h()));
    }
    break;

    case Panel::screen:
        clear_screen();
        break;
    }
}

void cover_area(const Panel panel, const Rect& area)
{
    cover_area(panel, area.p0, area.p1 - area.p0 + 1);
}

void cover_area(const Panel panel, const P& pos, const P& dims)
{
    const P px_pos = px_pos_for_cell_in_panel(panel, pos);
    const P cell_dims(config::cell_px_w(), config::cell_px_h());
    cover_area_px(px_pos, dims * cell_dims);
}

void cover_area_px(const P& px_pos, const P& px_dims)
{
    draw_rectangle_solid(px_pos, px_dims, clr_black);
}

void cover_cell_in_map(const P& pos)
{
    const P cell_dims(config::cell_px_w(), config::cell_px_h());
    P px_pos = px_pos_for_cell_in_panel(Panel::map, pos);
    cover_area_px(px_pos, cell_dims);
}

void draw_line_hor(const P& px_pos, const int W, const Clr& clr)
{
    draw_rectangle_solid(px_pos, P(W, 2), clr);
}

void draw_line_ver(const P& px_pos, const int H, const Clr& clr)
{
    draw_rectangle_solid(px_pos, P(1, H), clr);
}

void draw_rectangle_solid(const P& px_pos, const P& px_dims, const Clr& clr)
{
    if (is_inited())
    {
        SDL_Rect sdl_rect =
        {
            (Sint16)px_pos.x, (Sint16)px_pos.y,
            (Uint16)px_dims.x, (Uint16)px_dims.y
        };

        SDL_FillRect(scr_srf_,
                     &sdl_rect,
                     SDL_MapRGB(scr_srf_->format, clr.r, clr.g, clr.b));
    }
}

void draw_projectiles(std::vector<Projectile*>& projectiles,
                      const bool SHOULD_DRAW_MAP_BEFORE)
{

    if (SHOULD_DRAW_MAP_BEFORE)
    {
        draw_map_and_interface(false);
    }

    for (Projectile* p : projectiles)
    {
        if (!p->is_done_rendering && p->is_seen_by_player)
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

void draw_box(const Rect& border, const Panel panel, const Clr& clr, const bool COVER_AREA)
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
            draw_tile(Tile_id::popup_ver, panel, P(border.p0.x, y), clr, clr_black);
            draw_tile(Tile_id::popup_ver, panel, P(border.p1.x, y), clr, clr_black);
        }
        else
        {
            draw_glyph('|', panel, P(border.p0.x, y), clr, true, clr_black);
            draw_glyph('|', panel, P(border.p1.x, y), clr, true, clr_black);
        }
    }

    //Horizontal bars
    const int X0_VERT = border.p0.x + 1;
    const int X1_VERT = border.p1.x - 1;

    for (int x = X0_VERT; x <= X1_VERT; ++x)
    {
        if (IS_TILES)
        {
            draw_tile(Tile_id::popup_hor, panel, P(x, border.p0.y), clr, clr_black);
            draw_tile(Tile_id::popup_hor, panel, P(x, border.p1.y), clr, clr_black);
        }
        else
        {
            draw_glyph('-', panel, P(x, border.p0.y), clr, true, clr_black);
            draw_glyph('-', panel, P(x, border.p1.y), clr, true, clr_black);
        }
    }

    const std::vector<P> corners
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
    const int DESCR_Y0  = 1;
    const int DESCR_X1  = MAP_W - 1;

    cover_area(Panel::screen, Rect(DESCR_X0 - 1, DESCR_Y0, DESCR_X1, SCREEN_H - 1));

    const int MAX_W = DESCR_X1 - DESCR_X0 + 1;

    P p(DESCR_X0, DESCR_Y0);

    for (const auto& line : lines)
    {
        std::vector<std::string> formatted;
        text_format::split(line.str, MAX_W, formatted);

        for (const auto& line_in_formatted : formatted)
        {
            draw_text(line_in_formatted, Panel::screen, p, line.clr);
            ++p.y;
        }

        ++p.y;
    }
}

void draw_info_scr_interface(const std::string& title)
{
    if (config::is_tiles_mode())
    {
        for (int x = 0; x < SCREEN_W; ++x)
        {
            render::draw_tile(Tile_id::popup_hor,
                              Panel::screen,
                              P(x, 0),
                              clr_title);

            render::draw_tile(Tile_id::popup_hor,
                              Panel::screen,
                              P(x, SCREEN_H - 1),
                              clr_title);
        }
    }
    else //Text mode
    {
        const std::string decoration_line(MAP_W, '-');

        render::draw_text(decoration_line,
                          Panel::screen,
                          P(0, 0),
                          clr_title);

        render::draw_text(decoration_line,
                          Panel::screen,
                          P(0, SCREEN_H - 1),
                          clr_title);
    }

    const int X_LABEL = 3;

    render::draw_text(" " + title + " ",
                      Panel::screen,
                      P(X_LABEL, 0),
                      clr_title);

    render::draw_text(" " + info_scr_cmd_info + " ",
                      Panel::screen,
                      P(X_LABEL, SCREEN_H - 1),
                      clr_title);
}

void draw_map_and_interface(const bool SHOULD_UPDATE_SCREEN)
{
    if (!is_inited())
    {
        return;
    }

    clear_screen();

    draw_map();

    character_lines::draw();

    msg_log::draw(false);

    if (SHOULD_UPDATE_SCREEN)
    {
        update_screen();
    }
}

void draw_map()
{
    if (!is_inited())
    {
        return;
    }

    Cell_render_data* cur_render_data = nullptr;

    const bool IS_TILES = config::is_tiles_mode();

    //---------------- INSERT RIGIDS AND BLOOD INTO ARRAY
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            render_array[x][y] = Cell_render_data();

            if (map::cells[x][y].is_seen_by_player)
            {
                cur_render_data                 = &render_array[x][y];
                const auto* const   f           = map::cells[x][y].rigid;
                Tile_id             gore_tile   = Tile_id::empty;
                char                gore_glyph  = 0;

                if (f->can_have_gore())
                {
                    gore_tile  = f->gore_tile();
                    gore_glyph = f->gore_glyph();
                }

                if (gore_tile == Tile_id::empty)
                {
                    cur_render_data->tile       = f->tile();
                    cur_render_data->glyph      = f->glyph();
                    cur_render_data->clr        = f->clr();
                    const Clr& feature_clr_bg   = f->clr_bg();

                    if (!utils::is_clr_eq(feature_clr_bg, clr_black))
                    {
                        cur_render_data->clr_bg = feature_clr_bg;
                    }
                }
                else //Has gore
                {
                    cur_render_data->tile  = gore_tile;
                    cur_render_data->glyph = gore_glyph;
                    cur_render_data->clr   = clr_red;
                }

                if (map::cells[x][y].is_lit && f->is_los_passable())
                {
                    cur_render_data->is_marked_lit = true;
                }
            }
        }
    }

    //---------------- INSERT DEAD ACTORS INTO ARRAY
    for (Actor* actor : game_time::actors)
    {
        const P& p(actor->pos);

        if (
            actor->is_corpse()                      &&
            actor->data().glyph != ' '              &&
            actor->data().tile != Tile_id::empty    &&
            map::cells[p.x][p.y].is_seen_by_player)
        {
            cur_render_data        = &render_array[p.x][p.y];
            cur_render_data->clr   = actor->clr();
            cur_render_data->tile  = actor->tile();
            cur_render_data->glyph = actor->glyph();
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            cur_render_data = &render_array[x][y];

            if (map::cells[x][y].is_seen_by_player)
            {
                //---------------- INSERT ITEMS INTO ARRAY
                const Item* const item = map::cells[x][y].item;

                if (item)
                {
                    cur_render_data->clr   = item->clr();
                    cur_render_data->tile  = item->tile();
                    cur_render_data->glyph = item->glyph();
                }

                //Copy array to player memory (before living actors and mobile features)
                render_array_no_actors[x][y] = render_array[x][y];

                //Color cells slightly yellow when marked as lit
                if (cur_render_data->is_marked_lit)
                {
                    cur_render_data->clr.r = std::min(255, cur_render_data->clr.r + 40);
                    cur_render_data->clr.g = std::min(255, cur_render_data->clr.g + 40);

                    cur_render_data->is_light_fade_allowed = false;
                }
            }
        }
    }

    //---------------- INSERT MOBILE FEATURES INTO ARRAY
    for (auto* mob : game_time::mobs)
    {
        const P& p            = mob->pos();
        const Tile_id  mob_tile   = mob->tile();
        const char    mob_glyph  = mob->glyph();

        if (
            mob_tile != Tile_id::empty && mob_glyph != ' ' &&
            map::cells[p.x][p.y].is_seen_by_player)
        {
            cur_render_data = &render_array[p.x][p.y];
            cur_render_data->clr   = mob->clr();
            cur_render_data->tile  = mob_tile;
            cur_render_data->glyph = mob_glyph;
        }
    }

    //---------------- INSERT LIVING ACTORS INTO ARRAY
    for (auto* actor : game_time::actors)
    {
        if (!actor->is_player() && actor->is_alive())
        {
            const P& p = actor->pos;

            cur_render_data = &render_array[p.x][p.y];

            const auto* const mon = static_cast<const Mon*>(actor);

            if (map::player->can_see_actor(*actor))
            {
                if (actor->tile() != Tile_id::empty && actor->glyph() != ' ')
                {
                    cur_render_data->clr   = actor->clr();
                    cur_render_data->tile  = actor->tile();
                    cur_render_data->glyph = actor->glyph();

                    cur_render_data->lifebar_length             = lifebar_length(*actor);
                    cur_render_data->is_living_actor_seen_here  = true;
                    cur_render_data->is_light_fade_allowed      = false;

                    if (map::player->is_leader_of(mon))
                    {
                        cur_render_data->clr_bg = clr_green;
                    }
                    else //Player is not leader of monster
                    {
                        if (mon->aware_counter_ <= 0)
                        {
                            cur_render_data->clr_bg = clr_blue;
                        }
                    }
                }
            }
            else //Player cannot see actor
            {
                if (mon->player_aware_of_me_counter_ > 0 || map::player->is_leader_of(mon))
                {
                    cur_render_data->is_aware_of_mon_here  = true;
                }
            }
        }
    }

    //---------------- DRAW THE GRID
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Cell_render_data tmp_render_data = render_array[x][y];

            const Cell& cell = map::cells[x][y];

            if (cell.is_seen_by_player)
            {
                if (tmp_render_data.is_light_fade_allowed)
                {
                    const int DIST_FROM_PLAYER =
                        utils::king_dist(map::player->pos, P(x, y));

                    if (DIST_FROM_PLAYER > 1)
                    {
                        const double DIV =
                            std::min(2.0, 1.0 + ((double(DIST_FROM_PLAYER - 1) * 0.33)));

                        div_clr(tmp_render_data.clr,    DIV);
                        div_clr(tmp_render_data.clr_bg, DIV);
                    }

                    if (cell.is_dark && !cell.is_lit)
                    {
                        const double DRK_DIV = 1.75;
                        div_clr(tmp_render_data.clr,    DRK_DIV);
                        div_clr(tmp_render_data.clr_bg, DRK_DIV);
                    }
                }
            }
            else if (cell.is_explored && !tmp_render_data.is_living_actor_seen_here)
            {
                bool is_aware_of_mon_here               = tmp_render_data.is_aware_of_mon_here;

                //Set render array and the temporary render data to the remembered cell
                render_array[x][y]                      = cell.player_visual_memory;
                tmp_render_data                         = cell.player_visual_memory;

                tmp_render_data.is_aware_of_mon_here    = is_aware_of_mon_here;

                const double DIV = 5.0;
                div_clr(tmp_render_data.clr,    DIV);
                div_clr(tmp_render_data.clr_bg, DIV);
            }

            if (IS_TILES)
            {
                //Walls are given perspective here. If the tile to be set is a (top) wall
                //tile, instead place a front wall tile on any of the current conditions:
                //(1) Cell below is explored, and its tile is not a front or top wall.
                //(2) Cell below is unexplored.
                if (
                    !tmp_render_data.is_living_actor_seen_here &&
                    !tmp_render_data.is_aware_of_mon_here)
                {
                    const auto tile_seen    = render_array_no_actors[x][y].tile;
                    const auto tile_mem     = cell.player_visual_memory.tile;
                    const bool IS_TILE_WALL = cell.is_seen_by_player ?
                                              Wall::is_tile_any_wall_top(tile_seen) :
                                              Wall::is_tile_any_wall_top(tile_mem);

                    if (IS_TILE_WALL)
                    {
                        const auto* const   f               = cell.rigid;
                        const auto          feature_id      = f->id();
                        bool                is_hidden_door  = false;

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
                                        const auto* const wall = static_cast<const Wall*>(f);

                                        tmp_render_data.tile = wall->top_wall_tile();
                                    }
                                }
                                else if (feature_id == Feature_id::wall)
                                {
                                    const auto* const wall  = static_cast<const Wall*>(f);
                                    tmp_render_data.tile    = wall->front_wall_tile();
                                }
                                else if (is_hidden_door)
                                {
                                    tmp_render_data.tile = config::is_tiles_wall_full_square() ?
                                                           Tile_id::wall_top :
                                                           Tile_id::wall_front;
                                }
                            }
                            else //Cell below is not explored
                            {
                                const Rigid* wall = nullptr;

                                if (is_hidden_door)
                                {
                                    wall = static_cast<const Door*>(f)->mimic();
                                }
                                else
                                {
                                    wall = f;
                                }

                                tmp_render_data.tile =
                                    static_cast<const Wall*>(wall)->front_wall_tile();
                            }
                        }
                    }
                }
            }

            const P pos(x, y);

            if (tmp_render_data.is_aware_of_mon_here)
            {
                draw_glyph('!', Panel::map, pos, clr_black, true, clr_nosf_teal_drk);
            }
            else if (tmp_render_data.tile != Tile_id::empty && tmp_render_data.glyph != ' ')
            {
                if (IS_TILES)
                {
                    draw_tile(tmp_render_data.tile,
                              Panel::map,
                              pos,
                              tmp_render_data.clr,
                              tmp_render_data.clr_bg);
                }
                else //Text mode
                {
                    draw_glyph(tmp_render_data.glyph,
                               Panel::map,
                               pos,
                               tmp_render_data.clr,
                               true,
                               tmp_render_data.clr_bg);
                }

                if (tmp_render_data.lifebar_length != -1)
                {
                    draw_life_bar(pos, tmp_render_data.lifebar_length);
                }
            }

            if (!cell.is_explored)
            {
                render_array[x][y] = Cell_render_data();
            }
        }
    }

    //---------------- DRAW PLAYER CHARACTER
    const P&  pos         = map::player->pos;
    Item*       item        = map::player->inv().item_in_slot(Slot_id::wpn);
    const bool  IS_GHOUL    = player_bon::bg() == Bg::ghoul;

    if (IS_TILES)
    {
        bool uses_ranged_wpn = false;

        if (item)
        {
            uses_ranged_wpn = item->data().ranged.is_ranged_wpn;
        }

        const Tile_id tile = IS_GHOUL           ? Tile_id::ghoul :
                             uses_ranged_wpn    ? Tile_id::player_firearm :
                             Tile_id::player_melee;

        draw_tile(tile, Panel::map, pos, map::player->clr(), clr_black);
    }
    else //Text mode
    {
        draw_glyph('@', Panel::map, pos, map::player->clr(), true, clr_black);
    }

    const int LIFE_BAR_LENGTH = lifebar_length(*map::player);

    if (LIFE_BAR_LENGTH != -1)
    {
        draw_life_bar(pos, LIFE_BAR_LENGTH);
    }

    draw_player_shock_excl_marks();
}

} //render
