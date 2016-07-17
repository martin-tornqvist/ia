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
#include "sdl_wrapper.hpp"
#include "text_format.hpp"

namespace render
{

CellRenderData render_array[map_w][map_h];
CellRenderData render_array_no_actors[map_w][map_h];

namespace
{

SDL_Window*     sdl_window_         = nullptr;
SDL_Renderer*   sdl_renderer_       = nullptr;

SDL_Surface*    scr_srf_            = nullptr;
SDL_Texture*    scr_texture_        = nullptr;

SDL_Surface*    main_menu_logo_srf_ = nullptr;
SDL_Surface*    skull_srf_          = nullptr;

const size_t pixel_data_w = 400;
const size_t pixel_data_h = 400;

bool tile_px_data_[pixel_data_w][pixel_data_h];
bool font_px_data_[pixel_data_w][pixel_data_h];

bool tile_contour_px_data_[pixel_data_w][pixel_data_h];
bool font_contour_px_data_[pixel_data_w][pixel_data_h];

bool is_inited()
{
    return sdl_window_;
}

Uint32 px(SDL_Surface& srf,
          const int pixel_x,
          const int pixel_y)
{
    const int bpp = srf.format->BytesPerPixel;

    //p is the address to the pixel we want to retrieve
    Uint8* p = (Uint8*)srf.pixels + pixel_y * srf.pitch + pixel_x * bpp;

    switch (bpp)
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

void put_px(const SDL_Surface& srf, const int pixel_x, const int pixel_y, Uint32 px)
{
    const int bpp = srf.format->BytesPerPixel;

    //p is the address to the pixel we want to set
    Uint8* const p = (Uint8*)srf.pixels + pixel_y * srf.pitch + pixel_x * bpp;

    switch (bpp)
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
    SDL_Surface* tmp_srf = IMG_Load(logo_img_name.c_str());

    ASSERT(tmp_srf && "Failed to load main menu logo image");

    main_menu_logo_srf_ = SDL_ConvertSurface(tmp_srf, scr_srf_->format, 0);

    SDL_FreeSurface(tmp_srf);

    //Skull
    tmp_srf = IMG_Load(skull_img_name.c_str());

    ASSERT(tmp_srf && "Failed to load skull image");

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
            const bool is_img_px = px(*font_srf_tmp, x, y) == img_clr;

            font_px_data_[x][y] = is_img_px;
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
            const bool is_img_px = px(*tile_srf_tmp, x, y) == img_clr;

            tile_px_data_[x][y] = is_img_px;
        }
    }

    SDL_FreeSurface(tile_srf_tmp);

    TRACE_FUNC_END;
}

void load_contour(const bool base[pixel_data_w][pixel_data_h],
                  bool out[pixel_data_w][pixel_data_h])
{
    const P cell_dims(config::cell_px_w(), config::cell_px_h());

    for (size_t px_x = 0; px_x < pixel_data_w; ++px_x)
    {
        for (size_t px_y = 0; px_y < pixel_data_h; ++px_y)
        {
            bool& current_val = out[px_x][px_y];

            current_val = false;

            //Only mark this pixel as contour if it's not marked on the base image
            if (!base[px_x][px_y])
            {
                //Position interval to check for this pixel is constrained within current image
                const P cur_img_px_p0((P(px_x, px_y) / cell_dims) * cell_dims);
                const P cur_img_px_p1(cur_img_px_p0 + cell_dims - 1);

                const P px_p0(std::max(cur_img_px_p0.x, (int)(px_x - 1)),
                              std::max(cur_img_px_p0.y, (int)(px_y - 1)));

                const P px_p1(std::min(cur_img_px_p1.x, (int)(px_x + 1)),
                              std::min(cur_img_px_p1.y, (int)(px_y + 1)));

                for (int px_check_x = px_p0.x; px_check_x <= px_p1.x; ++px_check_x)
                {
                    for (int px_check_y = px_p0.y; px_check_y <= px_p1.y; ++px_check_y)
                    {
                        if (base[px_check_x][px_check_y])
                        {
                            current_val = true;
                            break;
                        }
                    }

                    if (current_val)
                    {
                        break;
                    }
                }
            }
        }
    }
}

void put_pixels_on_scr(const bool px_data[pixel_data_w][pixel_data_h],
                       const P& sheet_pos,
                       const P& scr_px_pos,
                       const Clr& clr)
{
    if (is_inited())
    {
        const int px_clr = SDL_MapRGB(scr_srf_->format, clr.r, clr.g, clr.b);

        const int cell_w      = config::cell_px_w();
        const int cell_h      = config::cell_px_h();
        const int SHEET_PX_X0 = sheet_pos.x * cell_w;
        const int SHEET_PX_Y0 = sheet_pos.y * cell_h;
        const int SHEET_PX_X1 = SHEET_PX_X0 + cell_w - 1;
        const int SHEET_PX_Y1 = SHEET_PX_Y0 + cell_h - 1;
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
                    put_px(*scr_srf_, scr_px_x, scr_px_y, px_clr);
                }
                ++scr_px_y;
            }
            ++scr_px_x;
        }
    }
}

void put_pixels_on_scr_for_tile(const TileId tile,
                                const P& scr_px_pos,
                                const Clr& clr)
{
    put_pixels_on_scr(tile_px_data_, art::tile_pos(tile), scr_px_pos, clr);
}

void put_pixels_on_scr_for_glyph(const char glyph,
                                 const P& scr_px_pos,
                                 const Clr& clr)
{
    put_pixels_on_scr(font_px_data_, art::glyph_pos(glyph), scr_px_pos, clr);
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
    const int actor_hp      = std::max(0, actor.hp());
    const int actor_hp_max  = actor.hp_max(true);

    if (actor_hp < actor_hp_max)
    {
        int HP_PERCENT = (actor_hp * 100) / actor_hp_max;
        return ((config::cell_px_w() - 2) * HP_PERCENT) / 100;
    }

    return -1;
}

void draw_life_bar(const P& pos, const int length)
{
    if (length >= 0)
    {
        const P cell_dims(config::cell_px_w(),  config::cell_px_h());
        const int w_green   = length;
        const int w_bar_tot = cell_dims.x - 2;
        const int w_red     = w_bar_tot - w_green;
        const P px_pos    = px_pos_for_cell_in_panel(Panel::map, pos + P(0, 1)) - P(0, 2);
        const int X0_GREEN  = px_pos.x + 1;
        const int X0_RED    = X0_GREEN + w_green;

        if (w_green > 0)
        {
            draw_line_hor(P(X0_GREEN, px_pos.y), w_green, clr_green_lgt);
        }

        if (w_red > 0)
        {
            draw_line_hor(P(X0_RED, px_pos.y), w_red, clr_red_lgt);
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
    const double shock  = map::player->perm_shock_taken_cur_turn();
    const int nr_excl   = shock > 8 ? 3 : shock > 3 ? 2 : shock > 1 ? 1 : 0;

    if (nr_excl > 0)
    {
        const P& player_pos = map::player->pos;
        const P px_pos_right = px_pos_for_cell_in_panel(Panel::map, player_pos);

        for (int i = 0; i < nr_excl; ++i)
        {
            draw_excl_mark_at(px_pos_right + P(i * 3, 0));
        }
    }
}

void draw_glyph_at_px(const char glyph,
                      const P& px_pos,
                      const Clr& clr,
                      const bool draw_bg_clr,
                      const Clr& bg_clr = clr_black)
{
    if (draw_bg_clr)
    {
        const P cell_dims(config::cell_px_w(), config::cell_px_h());

        draw_rectangle_solid(px_pos, cell_dims, bg_clr);

        //Only draw contour if neither the foreground or background is black
        if (
            !is_clr_equal(clr, clr_black) &&
            !is_clr_equal(bg_clr, clr_black))
        {
            put_pixels_on_scr(font_contour_px_data_,
                              art::glyph_pos(glyph),
                              px_pos,
                              clr_black);
        }
    }

    put_pixels_on_scr_for_glyph(glyph, px_pos, clr);
}

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;
    cleanup();

    TRACE << "Setting up rendering window" << std::endl;

    const std::string title = "IA " + version_str;

    const int scr_px_w = config::scr_px_w();
    const int scr_px_h = config::scr_px_h();

    if (config::is_fullscreen())
    {
        sdl_window_ = SDL_CreateWindow(title.c_str(),
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SDL_WINDOWPOS_UNDEFINED,
                                       scr_px_w, scr_px_h,
                                       SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    if (!config::is_fullscreen() || !sdl_window_)
    {
        sdl_window_ = SDL_CreateWindow(title.c_str(),
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       scr_px_w, scr_px_h,
                                       SDL_WINDOW_SHOWN);
    }

    if (!sdl_window_)
    {
        TRACE << "Failed to create window" << std::endl;
        ASSERT(false);
    }

    sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, SDL_RENDERER_ACCELERATED);

    if (!sdl_renderer_)
    {
        TRACE << "Failed to create SDL renderer" << std::endl;
        ASSERT(false);
    }

    scr_srf_ = SDL_CreateRGBSurface(0,
                                    scr_px_w, scr_px_h,
                                    screen_bpp,
                                    0x00FF0000,
                                    0x0000FF00,
                                    0x000000FF,
                                    0xFF000000);

    if (!scr_srf_)
    {
        TRACE << "Failed to create screen surface" << std::endl;
        ASSERT(false);
    }

    scr_texture_ = SDL_CreateTexture(sdl_renderer_,
                                     SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     scr_px_w,
                                     scr_px_h);

    if (!scr_texture_)
    {
        TRACE << "Failed to create screen texture" << std::endl;
        ASSERT(false);
    }

    load_font();

    if (config::is_tiles_mode())
    {
        load_tiles();
        load_pictures();
    }

    if (config::is_tiles_mode())
    {
        load_contour(tile_px_data_, tile_contour_px_data_);
    }

    load_contour(font_px_data_, font_contour_px_data_);

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
        SDL_UpdateTexture(scr_texture_,
                          nullptr,
                          scr_srf_->pixels,
                          scr_srf_->pitch);

        SDL_RenderCopy(sdl_renderer_,
                       scr_texture_,
                       nullptr,
                       nullptr);

        SDL_RenderPresent(sdl_renderer_);
    }
}

void clear_screen()
{
    if (is_inited())
    {
        SDL_FillRect(scr_srf_,
                     nullptr,
                     SDL_MapRGB(scr_srf_->format, 0, 0, 0));
    }
}

void draw_main_menu_logo(const int y_pos)
{
    const int scr_px_w  = config::scr_px_w();
    const int logo_px_h = main_menu_logo_srf_->w;
    const int cell_px_h = config::cell_px_h();

    const P px_pos((scr_px_w - logo_px_h) / 2, cell_px_h * y_pos);

    blit_surface(*main_menu_logo_srf_, px_pos);
}

void draw_skull(const P& p)
{
    const int cell_px_w = config::cell_px_w();
    const int cell_px_h = config::cell_px_h();

    const P px_pos(p * P(cell_px_w, cell_px_h));

    blit_surface(*skull_srf_, px_pos);
}

void draw_marker(const P& p,
                 const std::vector<P>& trail,
                 const int effective_range,
                 const int blocked_from_idx,
                 CellOverlay overlay[map_w][map_h])
{
    Clr clr = clr_green_lgt;

    //Tail
    for (size_t i = 1; i < trail.size(); ++i)
    {
        const P& pos = trail[i];

        const bool is_blocked = blocked_from_idx != -1 &&
                                (int)i >= blocked_from_idx;

        //NOTE: Final color is stored for drawing the head
        if (is_blocked)
        {
            clr = clr_red_lgt;
        }
        else if (effective_range != -1)
        {
            const int cheb_dist = king_dist(trail[0], pos);

            if (cheb_dist > effective_range)
            {
                clr = clr_orange;
            }
        }

        Clr clr_bg = clr_black;

        if (overlay)
        {
            clr_bg = overlay[pos.x][pos.y].clr_bg;
        }

        //Draw tail until (but not including) the head
        if (i < trail.size() - 1)
        {
            cover_cell_in_map(pos);

            //If blocked, always draw a character (more distinct)
            if (config::is_tiles_mode() && !is_blocked)
            {
                draw_tile(TileId::aim_marker_trail,
                          Panel::map,
                          pos,
                          clr,
                          clr_bg);
            }
            else //Text mode, or blocked
            {
                draw_glyph('*',
                           Panel::map,
                           pos,
                           clr,
                           true,
                           clr_bg);
            }
        }
    }

    Clr clr_bg = clr_black;

    if (overlay)
    {
        clr_bg = overlay[p.x][p.y].clr_bg;
    }

    //Head
    if (config::is_tiles_mode())
    {
        draw_tile(TileId::aim_marker_head,
                  Panel::map,
                  p,
                  clr,
                  clr_bg);
    }
    else //Text mode
    {
        draw_glyph('X',
                   Panel::map,
                   p,
                   clr,
                   true,
                   clr_bg);
    }
}

void draw_blast_at_field(const P& center_pos,
                         const int radius,
                         bool forbidden_cells[map_w][map_h],
                         const Clr& clr_inner,
                         const Clr& clr_outer)
{
    TRACE_FUNC_BEGIN;

    if (is_inited())
    {
        draw_map_state();

        bool is_any_blast_rendered = false;

        P pos;

        for (
            pos.y = std::max(1, center_pos.y - radius);
            pos.y <= std::min(map_h - 2, center_pos.y + radius);
            pos.y++)
        {
            for (
                pos.x = std::max(1, center_pos.x - radius);
                pos.x <= std::min(map_w - 2, center_pos.x + radius);
                pos.x++)
            {
                if (!forbidden_cells[pos.x][pos.y])
                {
                    const bool is_outer = pos.x == center_pos.x - radius ||
                                          pos.x == center_pos.x + radius ||
                                          pos.y == center_pos.y - radius ||
                                          pos.y == center_pos.y + radius;
                    const Clr clr = is_outer ? clr_outer : clr_inner;

                    if (config::is_tiles_mode())
                    {
                        draw_tile(TileId::blast1, Panel::map, pos, clr, clr_black);
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

        if (is_any_blast_rendered)
        {
            sdl_wrapper::sleep(config::delay_explosion() / 2);
        }

        for (
            pos.y = std::max(1, center_pos.y - radius);
            pos.y <= std::min(map_h - 2, center_pos.y + radius);
            pos.y++)
        {
            for (
                pos.x = std::max(1, center_pos.x - radius);
                pos.x <= std::min(map_w - 2, center_pos.x + radius);
                pos.x++)
            {
                if (!forbidden_cells[pos.x][pos.y])
                {
                    const bool is_outer = pos.x == center_pos.x - radius ||
                                          pos.x == center_pos.x + radius ||
                                          pos.y == center_pos.y - radius ||
                                          pos.y == center_pos.y + radius;
                    const Clr clr = is_outer ? clr_outer : clr_inner;

                    if (config::is_tiles_mode())
                    {
                        draw_tile(TileId::blast2, Panel::map, pos, clr, clr_black);
                    }
                    else
                    {
                        draw_glyph('*', Panel::map, pos, clr, true, clr_black);
                    }
                }
            }
        }

        update_screen();

        if (is_any_blast_rendered)
        {
            sdl_wrapper::sleep(config::delay_explosion() / 2);
        }

        draw_map_state();
    }

    TRACE_FUNC_END;
}

void draw_blast_at_cells(const std::vector<P>& positions, const Clr& clr)
{
    TRACE_FUNC_BEGIN;

    if (is_inited())
    {
        draw_map_state();

        for (const P& pos : positions)
        {
            if (config::is_tiles_mode())
            {
                draw_tile(TileId::blast1, Panel::map, pos, clr, clr_black);
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
                draw_tile(TileId::blast2, Panel::map, pos, clr, clr_black);
            }
            else
            {
                draw_glyph('*', Panel::map, pos, clr, true, clr_black);
            }
        }

        update_screen();
        sdl_wrapper::sleep(config::delay_explosion() / 2);
        draw_map_state();
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

void draw_tile(const TileId tile,
               const Panel panel,
               const P& pos,
               const Clr& clr,
               const Clr& bg_clr)
{
    if (is_inited())
    {
        const P px_pos = px_pos_for_cell_in_panel(panel, pos);
        const P cell_dims(config::cell_px_w(), config::cell_px_h());

        draw_rectangle_solid(px_pos, cell_dims, bg_clr);

        if (!is_clr_equal(bg_clr, clr_black))
        {
            put_pixels_on_scr(tile_contour_px_data_,
                              art::tile_pos(tile),
                              px_pos, clr_black);
        }

        put_pixels_on_scr_for_tile(tile,
                                   px_pos,
                                   clr);
    }
}

void draw_glyph(const char glyph,
                const Panel panel,
                const P& pos,
                const Clr& clr,
                const bool draw_bg_clr,
                const Clr& bg_clr)
{
    if (is_inited())
    {
        const P px_pos = px_pos_for_cell_in_panel(panel, pos);

        draw_glyph_at_px(glyph, px_pos, clr, draw_bg_clr, bg_clr);
    }
}

void draw_text(const std::string& str,
               const Panel panel,
               const P& pos,
               const Clr& clr,
               const Clr& bg_clr)
{
    if (is_inited())
    {
        P px_pos = px_pos_for_cell_in_panel(panel, pos);

        if (px_pos.y < 0 || px_pos.y >= config::scr_px_h())
        {
            return;
        }

        const int       cell_px_w   = config::cell_px_w();
        const int       cell_px_h   = config::cell_px_h();
        const size_t    msg_w       = str.size();
        const int       msg_px_w    = msg_w * cell_px_w;

        draw_rectangle_solid(px_pos, {msg_px_w, cell_px_h}, bg_clr);

        const int   scr_px_w            = config::scr_px_w();
        const int   MSG_PX_X1           = px_pos.x + msg_px_w - 1;
        const bool  is_msg_w_fit_on_scr = MSG_PX_X1 < scr_px_w;

        //X position to start drawing dots instead when the message does not fit
        //on the screen horizontally.
        const int   px_x_dots           = scr_px_w - (cell_px_w * 3);

        for (size_t i = 0; i < msg_w; ++i)
        {
            if (px_pos.x < 0 || px_pos.x >= scr_px_w)
            {
                return;
            }

            const bool draw_dots = !is_msg_w_fit_on_scr && px_pos.x >= px_x_dots;

            if (draw_dots)
            {
                draw_glyph_at_px('.', px_pos, clr_gray, false);
            }
            else //Whole message fits, or we are not yet near the screen edge
            {
                draw_glyph_at_px(str[i], px_pos, clr, false);
            }

            px_pos.x += cell_px_w;
        }
    }
}

int draw_text_center(const std::string& str, const Panel panel, const P& pos,
                     const Clr& clr, const Clr& bg_clr,
                     const bool is_pixel_pos_adj_allowed)
{
    if (!is_inited())
    {
        return 0;
    }

    const int len         = str.size();
    const int len_half    = len / 2;
    const int x_pos_left  = pos.x - len_half;

    const P cell_dims(config::cell_px_w(), config::cell_px_h());

    P px_pos = px_pos_for_cell_in_panel(panel, P(x_pos_left, pos.y));

    if (is_pixel_pos_adj_allowed)
    {
        const int pixel_x_adj = len_half * 2 == len ? cell_dims.x / 2 : 0;
        px_pos += P(pixel_x_adj, 0);
    }

    const int w_tot_pixel = len * cell_dims.x;

    SDL_Rect sdl_rect =
    {
        (Sint16)px_pos.x, (Sint16)px_pos.y,
        (Uint16)w_tot_pixel, (Uint16)cell_dims.y
    };

    SDL_FillRect(scr_srf_, &sdl_rect,
                 SDL_MapRGB(scr_srf_->format, bg_clr.r, bg_clr.g, bg_clr.b));

    for (int i = 0; i < len; ++i)
    {
        if (px_pos.x < 0 || px_pos.x >= config::scr_px_w())
        {
            return x_pos_left;
        }

        draw_glyph_at_px(str[i], px_pos, clr, false, bg_clr);
        px_pos.x += cell_dims.x;
    }

    return x_pos_left;
}

void cover_panel(const Panel panel)
{
    const int screen_pixel_w = config::scr_px_w();

    switch (panel)
    {
    case Panel::char_lines:
    {
        const P px_pos = px_pos_for_cell_in_panel(panel, P(0, 0));
        cover_area_px(px_pos, P(screen_pixel_w, config::char_lines_px_h()));
    }
    break;

    case Panel::log:
        cover_area_px(P(0, 0), P(screen_pixel_w, config::log_px_h()));
        break;

    case Panel::map:
    {
        const P px_pos = px_pos_for_cell_in_panel(panel, P(0, 0));
        cover_area_px(px_pos, P(screen_pixel_w, config::map_px_h()));
    }
    break;

    case Panel::screen:
        clear_screen();
        break;
    }
}

void cover_area(const Panel panel, const R& area)
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

void draw_line_hor(const P& px_pos, const int w, const Clr& clr)
{
    draw_rectangle_solid(px_pos, P(w, 2), clr);
}

void draw_line_ver(const P& px_pos, const int h, const Clr& clr)
{
    draw_rectangle_solid(px_pos, P(1, h), clr);
}

void draw_rectangle_solid(const P& px_pos, const P& px_dims, const Clr& clr)
{
    if (is_inited())
    {
        SDL_Rect sdl_rect =
        {
            (Sint16)px_pos.x,
            (Sint16)px_pos.y,
            (Uint16)px_dims.x,
            (Uint16)px_dims.y
        };

        SDL_FillRect(scr_srf_,
                     &sdl_rect,
                     SDL_MapRGB(scr_srf_->format, clr.r, clr.g, clr.b));
    }
}

void draw_projectiles(std::vector<Projectile*>& projectiles,
                      const bool draw_map_before)
{

    if (draw_map_before)
    {
        draw_map_state(UpdateScreen::no);
    }

    for (Projectile* p : projectiles)
    {
        if (!p->is_done_rendering && p->is_seen_by_player)
        {
            cover_cell_in_map(p->pos);

            if (config::is_tiles_mode())
            {
                if (p->tile != TileId::empty)
                {
                    draw_tile(p->tile, Panel::map, p->pos, p->clr);
                }
            }
            else
            {
                if (p->glyph != -1)
                {
                    draw_glyph(p->glyph, Panel::map, p->pos, p->clr);
                }
            }
        }
    }

    update_screen();
}

void draw_box(const R& border, const Panel panel, const Clr& clr, const bool do_cover_area)
{
    if (do_cover_area)
    {
        cover_area(panel, border);
    }

    const bool is_tile_mode = config::is_tiles_mode();

    //Vertical bars
    const int Y0_VERT = border.p0.y + 1;
    const int Y1_VERT = border.p1.y - 1;

    for (int y = Y0_VERT; y <= Y1_VERT; ++y)
    {
        if (is_tile_mode)
        {
            draw_tile(TileId::popup_ver, panel, P(border.p0.x, y), clr, clr_black);
            draw_tile(TileId::popup_ver, panel, P(border.p1.x, y), clr, clr_black);
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
        if (is_tile_mode)
        {
            draw_tile(TileId::popup_hor, panel, P(x, border.p0.y), clr, clr_black);
            draw_tile(TileId::popup_hor, panel, P(x, border.p1.y), clr, clr_black);
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
    if (is_tile_mode)
    {
        draw_tile(TileId::popup_top_l, panel, corners[0], clr, clr_black);
        draw_tile(TileId::popup_top_r, panel, corners[1], clr, clr_black);
        draw_tile(TileId::popup_btm_l, panel, corners[2], clr, clr_black);
        draw_tile(TileId::popup_btm_r, panel, corners[3], clr, clr_black);
    }
    else
    {
        draw_glyph('+', panel, corners[0], clr, true, clr_black);
        draw_glyph('+', panel, corners[1], clr, true, clr_black);
        draw_glyph('+', panel, corners[2], clr, true, clr_black);
        draw_glyph('+', panel, corners[3], clr, true, clr_black);
    }
}

void draw_descr_box(const std::vector<StrAndClr>& lines)
{
    const int DESCR_Y0  = 1;
    const int DESCR_X1  = map_w - 1;

    cover_area(Panel::screen,
               R(descr_x0 - 1, DESCR_Y0, DESCR_X1, screen_h - 1));

    const int max_w = DESCR_X1 - descr_x0 + 1;

    P p(descr_x0, DESCR_Y0);

    for (const auto& line : lines)
    {
        std::vector<std::string> formatted;
        text_format::split(line.str, max_w, formatted);

        for (const auto& line_in_formatted : formatted)
        {
            draw_text(line_in_formatted, Panel::screen, p, line.clr);
            ++p.y;
        }

        ++p.y;
    }
}

void draw_info_scr_interface(const std::string& title,
                             const InfScreenType screen_type)
{
    if (config::is_tiles_mode())
    {
        for (int x = 0; x < screen_w; ++x)
        {
            render::draw_tile(TileId::popup_hor,
                              Panel::screen,
                              P(x, 0),
                              clr_title);

            render::draw_tile(TileId::popup_hor,
                              Panel::screen,
                              P(x, screen_h - 1),
                              clr_title);
        }
    }
    else //Text mode
    {
        const std::string decoration_line(map_w, '-');

        render::draw_text(decoration_line,
                          Panel::screen,
                          P(0, 0),
                          clr_title);

        render::draw_text(decoration_line,
                          Panel::screen,
                          P(0, screen_h - 1),
                          clr_title);
    }

    const int x_label = 3;

    render::draw_text(" " + title + " ",
                      Panel::screen,
                      P(x_label, 0),
                      clr_title);

    const std::string cmd_info = screen_type == InfScreenType::scrolling ?
                                 info_scr_tip_scrollable :
                                 info_scr_tip;

    render::draw_text(" " + cmd_info + " ",
                      Panel::screen,
                      P(x_label, screen_h - 1),
                      clr_title);
}

void draw_map_state(const UpdateScreen update,
                    CellOverlay overlay[map_w][map_h])
{
    if (!is_inited())
    {
        return;
    }

    clear_screen();

    draw_map(overlay);

    character_lines::draw();

    msg_log::draw(UpdateScreen::no);

    if (update == UpdateScreen::yes)
    {
        update_screen();
    }
}

void draw_map(CellOverlay overlay[map_w][map_h])
{
    if (!is_inited())
    {
        return;
    }

    CellRenderData* render_data = nullptr;

    const bool is_tile_mode = config::is_tiles_mode();

    //---------------- INSERT RIGIDS AND BLOOD INTO ARRAY
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            //Reset render data at this position
            render_array[x][y] = CellRenderData();

            if (map::cells[x][y].is_seen_by_player)
            {
                render_data                     = &render_array[x][y];
                const auto* const   f           = map::cells[x][y].rigid;
                TileId             gore_tile   = TileId::empty;
                char                gore_glyph  = 0;

                if (f->can_have_gore())
                {
                    gore_tile  = f->gore_tile();
                    gore_glyph = f->gore_glyph();
                }

                if (gore_tile == TileId::empty)
                {
                    render_data->tile   = f->tile();
                    render_data->glyph  = f->glyph();
                    render_data->clr    = f->clr();

                    const Clr& feature_clr_bg = f->clr_bg();

                    if (!is_clr_equal(feature_clr_bg, clr_black))
                    {
                        render_data->clr_bg = feature_clr_bg;
                    }
                }
                else //Has gore
                {
                    render_data->tile  = gore_tile;
                    render_data->glyph = gore_glyph;
                    render_data->clr   = clr_red;
                }

                if (map::cells[x][y].is_lit && f->is_los_passable())
                {
                    render_data->is_marked_lit = true;
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
            actor->data().tile != TileId::empty    &&
            map::cells[p.x][p.y].is_seen_by_player)
        {
            render_data        = &render_array[p.x][p.y];
            render_data->clr   = actor->clr();
            render_data->tile  = actor->tile();
            render_data->glyph = actor->glyph();
        }
    }

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            render_data = &render_array[x][y];

            if (map::cells[x][y].is_seen_by_player)
            {
                //---------------- INSERT ITEMS INTO ARRAY
                const Item* const item = map::cells[x][y].item;

                if (item)
                {
                    render_data->clr   = item->clr();
                    render_data->tile  = item->tile();
                    render_data->glyph = item->glyph();
                }

                //Copy array to player memory (before living actors and mobile features)
                render_array_no_actors[x][y] = render_array[x][y];

                //Color cells slightly yellow when marked as lit
                if (render_data->is_marked_lit)
                {
                    render_data->clr.r = std::min(255, render_data->clr.r + 40);
                    render_data->clr.g = std::min(255, render_data->clr.g + 40);

                    render_data->is_light_fade_allowed = false;
                }
            }
        }
    }

    //---------------- INSERT MOBILE FEATURES INTO ARRAY
    for (auto* mob : game_time::mobs)
    {
        const P& p                  = mob->pos();
        const TileId   mob_tile    = mob->tile();
        const char      mob_glyph   = mob->glyph();

        if (
            mob_tile != TileId::empty && mob_glyph != ' ' &&
            map::cells[p.x][p.y].is_seen_by_player)
        {
            render_data = &render_array[p.x][p.y];
            render_data->clr   = mob->clr();
            render_data->tile  = mob_tile;
            render_data->glyph = mob_glyph;
        }
    }

    //---------------- INSERT LIVING ACTORS INTO ARRAY
    for (auto* actor : game_time::actors)
    {
        if (!actor->is_player() && actor->is_alive())
        {
            //There is a living monster here

            const P& p = actor->pos;

            render_data = &render_array[p.x][p.y];

            //There should NOT already be an actor here which is seen, or that
            //we are aware of
            ASSERT(!render_data->is_living_actor_seen_here);
            ASSERT(!render_data->is_aware_of_hostile_mon_here);
            ASSERT(!render_data->is_aware_of_allied_mon_here);

            const auto* const mon = static_cast<const Mon*>(actor);

            if (map::player->can_see_actor(*actor))
            {
                if (actor->tile() != TileId::empty && actor->glyph() != ' ')
                {
                    render_data->clr   = actor->clr();
                    render_data->tile  = actor->tile();
                    render_data->glyph = actor->glyph();

                    render_data->lifebar_length             = lifebar_length(*actor);
                    render_data->is_living_actor_seen_here  = true;
                    render_data->is_light_fade_allowed      = false;

                    if (map::player->is_leader_of(mon))
                    {
                        render_data->clr_bg = clr_allied_mon;
                    }
                    else //Player is not leader of monster
                    {
                        if (mon->aware_counter_ <= 0)
                        {
                            render_data->clr_bg = clr_blue;
                        }
                    }
                }
            }
            else //Player cannot see actor
            {
                if (mon->player_aware_of_me_counter_ > 0)
                {
                    if (map::player->is_leader_of(mon))
                    {
                        render_data->is_aware_of_allied_mon_here = true;
                    }
                    else //Player is not leader of monster
                    {
                        render_data->is_aware_of_hostile_mon_here = true;
                    }
                }
            }
        }
    }

    //---------------- DRAW THE GRID
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            CellRenderData render_data_cpy = render_array[x][y];

            const Cell& cell = map::cells[x][y];

            if (cell.is_seen_by_player)
            {
                if (render_data_cpy.is_light_fade_allowed)
                {
                    const int dist_from_player = king_dist(map::player->pos, P(x, y));

                    if (dist_from_player > 1)
                    {
                        const double div =
                            std::min(2.0, 1.0 + ((double(dist_from_player - 1) * 0.33)));

                        div_clr(render_data_cpy.clr,    div);
                        div_clr(render_data_cpy.clr_bg, div);
                    }

                    if (cell.is_dark && !cell.is_lit)
                    {
                        const double drk_div = 1.75;
                        div_clr(render_data_cpy.clr,    drk_div);
                        div_clr(render_data_cpy.clr_bg, drk_div);
                    }
                }
            }
            else if (
                cell.is_explored &&
                !render_data_cpy.is_living_actor_seen_here)
            {
                bool is_aware_of_hostile_mon_here   = render_data_cpy.is_aware_of_hostile_mon_here;
                bool is_aware_of_allied_mon_here    = render_data_cpy.is_aware_of_allied_mon_here;

                //Set render array and the temporary render data to the remembered cell
                render_array[x][y]  = cell.player_visual_memory;
                render_data_cpy     = cell.player_visual_memory;

                render_data_cpy.is_aware_of_hostile_mon_here    = is_aware_of_hostile_mon_here;
                render_data_cpy.is_aware_of_allied_mon_here     = is_aware_of_allied_mon_here;

                const double div = 5.0;
                div_clr(render_data_cpy.clr,    div);
                div_clr(render_data_cpy.clr_bg, div);
            }

            if (is_tile_mode)
            {
                //Walls are given perspective here. If the tile to be set is a (top) wall
                //tile, instead place a front wall tile on any of the current conditions:
                //(1) Cell below is explored, and its tile is not a front or top wall.
                //(2) Cell below is unexplored.
                if (
                    !render_data_cpy.is_living_actor_seen_here      &&
                    !render_data_cpy.is_aware_of_hostile_mon_here   &&
                    !render_data_cpy.is_aware_of_allied_mon_here)
                {
                    const auto tile_seen    = render_array_no_actors[x][y].tile;
                    const auto tile_mem     = cell.player_visual_memory.tile;
                    const bool is_tile_wall = cell.is_seen_by_player ?
                                              Wall::is_tile_any_wall_top(tile_seen) :
                                              Wall::is_tile_any_wall_top(tile_mem);

                    if (is_tile_wall)
                    {
                        const auto* const   f               = cell.rigid;
                        const auto          feature_id      = f->id();
                        bool                is_hidden_door  = false;

                        if (feature_id == FeatureId::door)
                        {
                            is_hidden_door = static_cast<const Door*>(f)->is_secret();
                        }

                        if (
                            y < map_h - 1 &&
                            (feature_id == FeatureId::wall || is_hidden_door))
                        {
                            if (map::cells[x][y + 1].is_explored)
                            {
                                const bool is_seen_below  =
                                    map::cells[x][y + 1].is_seen_by_player;

                                const auto tile_below_seen  =
                                    render_array_no_actors[x][y + 1].tile;

                                const auto tile_below_mem   =
                                    map::cells[x][y + 1].player_visual_memory.tile;

                                const bool tile_below_is_wall_front =
                                    is_seen_below ?
                                    Wall::is_tile_any_wall_front(tile_below_seen) :
                                    Wall::is_tile_any_wall_front(tile_below_mem);

                                const bool tile_below_is_wall_top =
                                    is_seen_below ?
                                    Wall::is_tile_any_wall_top(tile_below_seen) :
                                    Wall::is_tile_any_wall_top(tile_below_mem);

                                bool tile_below_is_revealed_door =
                                    is_seen_below ?
                                    Door::is_tile_any_door(tile_below_seen) :
                                    Door::is_tile_any_door(tile_below_mem);

                                if (
                                    tile_below_is_wall_front  ||
                                    tile_below_is_wall_top    ||
                                    tile_below_is_revealed_door)
                                {
                                    if (feature_id == FeatureId::wall)
                                    {
                                        const auto* const wall = static_cast<const Wall*>(f);

                                        render_data_cpy.tile = wall->top_wall_tile();
                                    }
                                }
                                else if (feature_id == FeatureId::wall)
                                {
                                    const auto* const wall  = static_cast<const Wall*>(f);
                                    render_data_cpy.tile    = wall->front_wall_tile();
                                }
                                else if (is_hidden_door)
                                {
                                    render_data_cpy.tile = config::is_tiles_wall_full_square() ?
                                                           TileId::wall_top :
                                                           TileId::wall_front;
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

                                render_data_cpy.tile =
                                    static_cast<const Wall*>(wall)->front_wall_tile();
                            }
                        }
                    }
                }
            }

            const P pos(x, y);

            //Draw exclamation mark if aware of monster here
            if (
                render_data_cpy.is_aware_of_hostile_mon_here ||
                render_data_cpy.is_aware_of_allied_mon_here)
            {
                //We should never see both a hostile AND an allied monster in
                //the same cell
                ASSERT(!render_data_cpy.is_aware_of_hostile_mon_here ||
                       !render_data_cpy.is_aware_of_allied_mon_here);

                const Clr clr_bg = render_data_cpy.is_aware_of_hostile_mon_here ?
                                   clr_nosf_teal_drk :
                                   clr_allied_mon;

                render_data_cpy.tile    = TileId::empty;
                render_data_cpy.glyph   = '!';
                render_data_cpy.clr     = clr_black;
                render_data_cpy.clr_bg  = clr_bg;
            }

            //Overlay?
            if (overlay)
            {
                const CellOverlay& overlay_here = overlay[pos.x][pos.y];

                if (!is_clr_equal(overlay_here.clr_bg, clr_black))
                {
                    render_data_cpy.clr_bg = overlay_here.clr_bg;
                }
            }

            bool did_draw = false;

            //Draw tile here if tile mode, and a tile has been set
            if (is_tile_mode && render_data_cpy.tile != TileId::empty)
            {
                draw_tile(render_data_cpy.tile,
                          Panel::map,
                          pos,
                          render_data_cpy.clr,
                          render_data_cpy.clr_bg);

                did_draw = true;
            }
            else /* Text mode, or no tile set */ if (render_data_cpy.glyph != ' ')
            {
                draw_glyph(render_data_cpy.glyph,
                           Panel::map,
                           pos,
                           render_data_cpy.clr,
                           true,
                           render_data_cpy.clr_bg);

                did_draw = true;
            }

            //Draw lifebar here?
            if (did_draw && render_data_cpy.lifebar_length != -1)
            {
                draw_life_bar(pos, render_data_cpy.lifebar_length);
            }

            if (!cell.is_explored)
            {
                render_array[x][y] = CellRenderData();
            }
        }
    }

    //---------------- DRAW PLAYER CHARACTER
    const P&    pos         = map::player->pos;
    Item*       item        = map::player->inv().item_in_slot(SlotId::wpn);
    const bool  is_ghoul    = player_bon::bg() == Bg::ghoul;

    Clr clr_bg = clr_black;

    //Overlay?
    if (overlay)
    {
        const CellOverlay& overlay_here = overlay[pos.x][pos.y];

        if (!is_clr_equal(overlay_here.clr_bg, clr_black))
        {
            clr_bg = overlay_here.clr_bg;
        }
    }

    if (is_tile_mode)
    {
        bool uses_ranged_wpn = false;

        if (item)
        {
            uses_ranged_wpn = item->data().ranged.is_ranged_wpn;
        }

        const TileId tile = is_ghoul           ? TileId::ghoul :
                             uses_ranged_wpn    ? TileId::player_firearm :
                             TileId::player_melee;

        draw_tile(tile,
                  Panel::map,
                  pos, map::player->clr(),
                  clr_bg);
    }
    else //Text mode
    {
        draw_glyph('@',
                   Panel::map,
                   pos,
                   map::player->clr(),
                   true,
                   clr_bg);
    }

    const int life_bar_length = lifebar_length(*map::player);

    if (life_bar_length != -1)
    {
        draw_life_bar(pos, life_bar_length);
    }

    draw_player_shock_excl_marks();
}

} //render
