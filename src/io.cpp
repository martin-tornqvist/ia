#include "io.hpp"

#include <vector>
#include <iostream>

#include "init.hpp"
#include "item.hpp"
#include "status_lines.hpp"
#include "marker.hpp"
#include "map.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "msg_log.hpp"
#include "attack.hpp"
#include "inventory.hpp"
#include "sdl_base.hpp"
#include "text_format.hpp"

namespace io
{

namespace
{

SDL_Window* sdl_window_ = nullptr;
SDL_Renderer* sdl_renderer_ = nullptr;

// Bytes per pixel
int bpp_ = -1;

SDL_Surface* scr_srf_ = nullptr;
SDL_Texture* scr_texture_ = nullptr;

SDL_Surface* main_menu_logo_srf_ = nullptr;
SDL_Surface* skull_srf_ = nullptr;

const size_t tiles_nr_x_ = 21;
const size_t tiles_nr_y_ = 12;

const size_t font_nr_x_ = 16;
const size_t font_nr_y_ = 7;

std::vector<P> tile_px_data_[tiles_nr_x_][tiles_nr_y_];
std::vector<P> font_px_data_[font_nr_x_][font_nr_y_];

std::vector<P> tile_contour_px_data_[tiles_nr_x_][tiles_nr_y_];
std::vector<P> font_contour_px_data_[font_nr_x_][font_nr_y_];

SDL_Event sdl_event_;

// Pointer to function used for writing pixels on the screen (there are
// different variants depending on bpp)
void (*put_px_ptr_)(const SDL_Surface& srf,
                    int pixel_x,
                    int pixel_y,
                    Uint32 px) = nullptr;

bool is_inited()
{
    return sdl_window_;
}

Uint32 px(const SDL_Surface& srf,
          const int pixel_x,
          const int pixel_y)
{
    // p is the address to the pixel we want to retrieve
    Uint8* p =
        (Uint8*)srf.pixels +
        pixel_y * srf.pitch +
        pixel_x * bpp_;

    switch (bpp_)
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
        else // Little endian
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

void put_px8(const SDL_Surface& srf,
             const int pixel_x,
             const int pixel_y,
             const Uint32 px)
{
    // p is the address to the pixel we want to set
    Uint8* const p =
        (Uint8*)srf.pixels +
        (pixel_y * srf.pitch) +
        (pixel_x * bpp_);

    *p = px;
}

void put_px16(const SDL_Surface& srf,
              const int pixel_x,
              const int pixel_y,
              const Uint32 px)
{
    // p is the address to the pixel we want to set
    Uint8* const p =
        (Uint8*)srf.pixels +
        (pixel_y * srf.pitch) +
        (pixel_x * bpp_);

    *(Uint16*)p = px;
}

void put_px24(const SDL_Surface& srf,
              const int pixel_x,
              const int pixel_y,
              const Uint32 px)
{
    // p is the address to the pixel we want to set
    Uint8* const p =
        (Uint8*)srf.pixels +
        (pixel_y * srf.pitch) +
        (pixel_x * bpp_);

    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
    {
        p[0] = (px >> 16) & 0xff;
        p[1] = (px >> 8) & 0xff;
        p[2] = px & 0xff;
    }
    else // Little endian
    {
        p[0] = px & 0xff;
        p[1] = (px >> 8) & 0xff;
        p[2] = (px >> 16) & 0xff;
    }
}

void put_px32(const SDL_Surface& srf,
              const int pixel_x,
              const int pixel_y,
              const Uint32 px)
{
    // p is the address to the pixel we want to set
    Uint8* const p =
        (Uint8*)srf.pixels +
        (pixel_y * srf.pitch) +
        (pixel_x * bpp_);

    *(Uint32*)p = px;
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

    // Main menu logo
    SDL_Surface* tmp_srf = IMG_Load(logo_img_name.c_str());

    ASSERT(tmp_srf && "Failed to load main menu logo image");

    main_menu_logo_srf_ = SDL_ConvertSurface(tmp_srf, scr_srf_->format, 0);

    SDL_FreeSurface(tmp_srf);

    // Skull
    tmp_srf = IMG_Load(skull_img_name.c_str());

    ASSERT(tmp_srf && "Failed to load skull image");

    skull_srf_ = SDL_ConvertSurface(tmp_srf, scr_srf_->format, 0);

    SDL_FreeSurface(tmp_srf);

    TRACE_FUNC_END;
}

void load_font()
{
    TRACE_FUNC_BEGIN;

    SDL_Surface* const font_srf_tmp =
        IMG_Load(config::font_name().data());

    const Uint32 img_clr =
        SDL_MapRGB(font_srf_tmp->format, 255, 255, 255);

    const int cell_w = config::cell_px_w();
    const int cell_h = config::cell_px_h();

    for (int x = 0; x < (int)font_nr_x_; ++x)
    {
        for (int y = 0; y < (int)font_nr_y_; ++y)
        {
            auto& px_data = font_px_data_[x][y];

            px_data.clear();

            const int sheet_x0 = x * cell_w;
            const int sheet_y0 = y * cell_h;
            const int sheet_x1 = sheet_x0 + cell_w - 1;
            const int sheet_y1 = sheet_y0 + cell_h - 1;

            for (int sheet_x = sheet_x0; sheet_x <= sheet_x1; ++sheet_x)
            {
                for (int sheet_y = sheet_y0; sheet_y <= sheet_y1; ++sheet_y)
                {
                    const bool is_img_px =
                        px(*font_srf_tmp, sheet_x, sheet_y) == img_clr;

                    if (is_img_px)
                    {
                        const int x_relative = sheet_x - sheet_x0;
                        const int y_relative = sheet_y - sheet_y0;

                        px_data.push_back(P(x_relative, y_relative));
                    }
                }
            }
        }
    }

    SDL_FreeSurface(font_srf_tmp);

    TRACE_FUNC_END;
}

void load_tiles()
{
    TRACE_FUNC_BEGIN;

    SDL_Surface* const tile_srf_tmp =
        IMG_Load(tiles_img_name.data());

    const Uint32 img_clr =
        SDL_MapRGB(tile_srf_tmp->format, 255, 255, 255);

    const int cell_w = config::cell_px_w();
    const int cell_h = config::cell_px_h();

    for (int x = 0; x < (int)tiles_nr_x_; ++x)
    {
        for (int y = 0; y < (int)tiles_nr_y_; ++y)
        {
            auto& px_data = tile_px_data_[x][y];

            px_data.clear();

            const int sheet_x0 = x * cell_w;
            const int sheet_y0 = y * cell_h;
            const int sheet_x1 = sheet_x0 + cell_w - 1;
            const int sheet_y1 = sheet_y0 + cell_h - 1;

            for (int sheet_x = sheet_x0; sheet_x <= sheet_x1; ++sheet_x)
            {
                for (int sheet_y = sheet_y0; sheet_y <= sheet_y1; ++sheet_y)
                {
                    const bool is_img_px =
                        px(*tile_srf_tmp, sheet_x, sheet_y) == img_clr;

                    if (is_img_px)
                    {
                        const int x_relative = sheet_x - sheet_x0;
                        const int y_relative = sheet_y - sheet_y0;

                        px_data.push_back(P(x_relative, y_relative));
                    }
                }
            }
        }
    }

    SDL_FreeSurface(tile_srf_tmp);

    TRACE_FUNC_END;
}

void load_contours(const std::vector<P>* base,
                   const int nr_x,
                   const int nr_y,
                   std::vector<P>* out)
{
    TRACE_FUNC_BEGIN;

    const int cell_w = config::cell_px_w();
    const int cell_h = config::cell_px_h();

    for (int x = 0; x < nr_x; ++x)
    {
        for (int y = 0; y < nr_y; ++y)
        {
            const int offset = (x * nr_y) + y;

            const auto& base_px_data = *(base + offset);
            auto& dest_px_data = *(out + offset);

            for (const P& base_px_pos : base_px_data)
            {
                const int size = 1;

                const int px_x0 = std::max(0, base_px_pos.x - size);
                const int px_y0 = std::max(0, base_px_pos.y - size);
                const int px_x1 = std::min(cell_w - 1, base_px_pos.x + size);
                const int px_y1 = std::min(cell_h - 1, base_px_pos.y + size);

                for (int px_x = px_x0; px_x <= px_x1; ++px_x)
                {
                    for (int px_y = px_y0; px_y <= px_y1; ++px_y)
                    {
                        const P p(px_x, px_y);

                        // Only mark pixel as contour if it's not marked on the
                        // base image
                        const auto it = std::find(begin(base_px_data),
                                                  end(base_px_data),
                                                  p);

                        if (it == end(base_px_data))
                        {
                            dest_px_data.push_back(p);
                        }
                    }
                }
            }
        }
    }

    TRACE_FUNC_END;
}

void put_pixels_on_scr(const std::vector<P> px_data,
                       /*const P& sheet_pos,*/
                       const P& scr_px_pos,
                       const Clr& clr)
{
    const int px_clr = SDL_MapRGB(scr_srf_->format,
                                  clr.r,
                                  clr.g,
                                  clr.b);

    for (const P& p_relative : px_data)
    {
        const int scr_px_x = scr_px_pos.x + p_relative.x;
        const int scr_px_y = scr_px_pos.y + p_relative.y;

        put_px_ptr_(*scr_srf_,
                    scr_px_x,
                    scr_px_y,
                    px_clr);
    }
}

void put_pixels_on_scr_for_tile(const TileId tile,
                                const P& scr_px_pos,
                                const Clr& clr)
{
    const P sheet_pos(art::tile_pos(tile));

    const auto& pixel_data =
        tile_px_data_[sheet_pos.x][sheet_pos.y];

    put_pixels_on_scr(pixel_data,
                      /*art::tile_pos(tile),*/
                      scr_px_pos,
                      clr);
}

void put_pixels_on_scr_for_glyph(const char glyph,
                                 const P& scr_px_pos,
                                 const Clr& clr)
{
    const P sheet_pos(art::glyph_pos(glyph));

    const auto& pixel_data =
        font_px_data_[sheet_pos.x][sheet_pos.y];

    put_pixels_on_scr(pixel_data,
                      /*art::glyph_pos(glyph),*/
                      scr_px_pos,
                      clr);
}

/*
void draw_excl_mark_at(const P& px_pos)
{
    draw_rectangle_solid(px_pos, P(3, 12), clr_black);
    draw_line_ver(px_pos + P(1, 1), 6, clr_magenta_lgt);
    draw_line_ver(px_pos + P(1, 9), 2, clr_magenta_lgt);
}
*/

/*
void draw_player_shock_excl_marks()
{
    const double shock = map::player->perm_shock_taken_current_turn();
    const int nr_excl = shock > 8 ? 3 : shock > 3 ? 2 : shock > 1 ? 1 : 0;

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
*/

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

        // Draw contour if neither the foreground nor background is black
        if (!is_clr_equal(clr, clr_black) &&
            !is_clr_equal(bg_clr, clr_black))
        {
            const P glyph_pos(art::glyph_pos(glyph));

            const auto& contour_px_data =
                font_contour_px_data_[glyph_pos.x][glyph_pos.y];

            put_pixels_on_scr(contour_px_data,
                              px_pos,
                              clr_black);
        }
    }

    put_pixels_on_scr_for_glyph(glyph,
                                px_pos,
                                clr);
}

} // namespace

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
                                       scr_px_w,
                                       scr_px_h,
                                       SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    if (!config::is_fullscreen() || !sdl_window_)
    {
        sdl_window_ = SDL_CreateWindow(title.c_str(),
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       scr_px_w,
                                       scr_px_h,
                                       SDL_WINDOW_SHOWN);
    }

    if (!sdl_window_)
    {
        TRACE << "Failed to create window" << std::endl;
        ASSERT(false);
    }

    sdl_renderer_ = SDL_CreateRenderer(sdl_window_,
                                       -1,
                                       SDL_RENDERER_ACCELERATED);

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

    bpp_ = scr_srf_->format->BytesPerPixel;

    switch (bpp_)
    {
    case 1:
        put_px_ptr_ = &put_px8;
        break;

    case 2:
        put_px_ptr_ = &put_px16;
        break;

    case 3:
        put_px_ptr_ = &put_px24;
        break;

    case 4:
        put_px_ptr_ = &put_px32;
        break;

    default:
        TRACE << "Bad bpp: " << bpp_ << std::endl;

        ASSERT(false);
        break;
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
        load_contours(
            reinterpret_cast<const std::vector<P>*>(tile_px_data_),
            tiles_nr_x_,
            tiles_nr_y_,
            reinterpret_cast<std::vector<P>*>(tile_contour_px_data_));
    }

    load_contours(
        reinterpret_cast<const std::vector<P>*>(font_px_data_),
        font_nr_x_,
        font_nr_y_,
        reinterpret_cast<std::vector<P>*>(font_contour_px_data_));

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
    if (is_inited())
    {
        const int scr_px_w = config::scr_px_w();
        const int logo_px_h = main_menu_logo_srf_->w;
        const int cell_px_h = config::cell_px_h();

        const P px_pos((scr_px_w - logo_px_h) / 2, cell_px_h * y_pos);

        blit_surface(*main_menu_logo_srf_, px_pos);
    }
}

void draw_skull(const P& p)
{
    if (is_inited())
    {
        const int cell_px_w = config::cell_px_w();
        const int cell_px_h = config::cell_px_h();

        const P px_pos(p * P(cell_px_w, cell_px_h));

        blit_surface(*skull_srf_, px_pos);
    }
}

void draw_blast_at_field(const P& center_pos,
                         const int radius,
                         bool forbidden_cells[map_w][map_h],
                         const Clr& clr_inner,
                         const Clr& clr_outer)
{
    TRACE_FUNC_BEGIN;

    if (!is_inited())
    {
        TRACE_FUNC_END;

        return;
    }

    states::draw();

    bool is_any_blast_rendered = false;

    P pos;

    for (pos.y = std::max(1, center_pos.y - radius);
         pos.y <= std::min(map_h - 2, center_pos.y + radius);
         pos.y++)
    {
        for (pos.x = std::max(1, center_pos.x - radius);
             pos.x <= std::min(map_w - 2, center_pos.x + radius);
             pos.x++)
        {
            if (!forbidden_cells[pos.x][pos.y])
            {
                const bool is_outer =
                    pos.x == center_pos.x - radius ||
                    pos.x == center_pos.x + radius ||
                    pos.y == center_pos.y - radius ||
                    pos.y == center_pos.y + radius;

                const Clr clr = is_outer ? clr_outer : clr_inner;

                if (config::is_tiles_mode())
                {
                    draw_tile(TileId::blast1,
                              Panel::map,
                              pos,
                              clr,
                              clr_black);
                }
                else
                {
                    draw_glyph('*',
                               Panel::map,
                               pos,
                               clr,
                               true,
                               clr_black);
                }

                is_any_blast_rendered = true;
            }
        }
    }

    update_screen();

    if (is_any_blast_rendered)
    {
        sdl_base::sleep(config::delay_explosion() / 2);
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
                const bool is_outer =
                    pos.x == center_pos.x - radius ||
                    pos.x == center_pos.x + radius ||
                    pos.y == center_pos.y - radius ||
                    pos.y == center_pos.y + radius;

                const Clr clr =
                    is_outer ? clr_outer : clr_inner;

                if (config::is_tiles_mode())
                {
                    draw_tile(TileId::blast2,
                              Panel::map,
                              pos,
                              clr,
                              clr_black);
                }
                else // Text mode
                {
                    draw_glyph('*',
                               Panel::map,
                               pos,
                               clr,
                               true,
                               clr_black);
                }
            }
        }
    }

    update_screen();

    if (is_any_blast_rendered)
    {
        sdl_base::sleep(config::delay_explosion() / 2);
    }

    TRACE_FUNC_END;
}

void draw_blast_at_cells(const std::vector<P>& positions, const Clr& clr)
{
    TRACE_FUNC_BEGIN;

    if (!is_inited())
    {
        TRACE_FUNC_END;

        return;
    }

    states::draw();

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

    sdl_base::sleep(config::delay_explosion() / 2);

    for (const P& pos : positions)
    {
        if (config::is_tiles_mode())
        {
            draw_tile(TileId::blast2,
                      Panel::map,
                      pos,
                      clr,
                      clr_black);
        }
        else // Text mode
        {
            draw_glyph('*',
                       Panel::map,
                       pos,
                       clr,
                       true,
                       clr_black);
        }
    }

    update_screen();

    sdl_base::sleep(config::delay_explosion() / 2);

    TRACE_FUNC_END;
}

void draw_blast_at_seen_cells(const std::vector<P>& positions,
                              const Clr& clr)
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
            io::draw_blast_at_cells(positions_with_vision, clr);
        }
    }
}

void draw_blast_at_seen_actors(const std::vector<Actor*>& actors,
                               const Clr& clr)
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
    if (!is_inited())
    {
        return;
    }

    const P px_pos = px_pos_for_cell_in_panel(panel, pos);

    const P cell_dims(config::cell_px_w(), config::cell_px_h());

    draw_rectangle_solid(px_pos, cell_dims, bg_clr);

    // Draw contour if neither the foreground nor background is black
    if (!is_clr_equal(clr, clr_black) &&
        !is_clr_equal(bg_clr, clr_black))
    {
        const P tile_pos(art::tile_pos(tile));

        const auto& contour_px_data =
            tile_contour_px_data_[tile_pos.x][tile_pos.y];

        put_pixels_on_scr(contour_px_data,
                          px_pos,
                          clr_black);
    }

    put_pixels_on_scr_for_tile(tile,
                               px_pos,
                               clr);
}

void draw_glyph(const char glyph,
                const Panel panel,
                const P& pos,
                const Clr& clr,
                const bool draw_bg_clr,
                const Clr& bg_clr)
{
    if (!is_inited())
    {
        return;
    }

    const P px_pos = px_pos_for_cell_in_panel(panel, pos);

    draw_glyph_at_px(glyph, px_pos, clr, draw_bg_clr, bg_clr);
}

void draw_text(const std::string& str,
               const Panel panel,
               const P& pos,
               const Clr& clr,
               const Clr& bg_clr)
{
    if (!is_inited())
    {
        return;
    }

    P px_pos = px_pos_for_cell_in_panel(panel, pos);

    if (px_pos.y < 0 || px_pos.y >= config::scr_px_h())
    {
        return;
    }

    const int cell_px_w = config::cell_px_w();
    const int cell_px_h = config::cell_px_h();
    const size_t msg_w = str.size();
    const int msg_px_w = msg_w * cell_px_w;

    draw_rectangle_solid(px_pos, {msg_px_w, cell_px_h}, bg_clr);

    const int scr_px_w = config::scr_px_w();
    const int msg_px_x1 = px_pos.x + msg_px_w - 1;
    const bool is_msg_w_fit_on_scr = msg_px_x1 < scr_px_w;

    // X position to start drawing dots instead when the message does not
    // fit on the screen horizontally.
    const int px_x_dots = scr_px_w - (cell_px_w * 3);

    for (size_t i = 0; i < msg_w; ++i)
    {
        if (px_pos.x < 0 || px_pos.x >= scr_px_w)
        {
            return;
        }

        const bool draw_dots =
            !is_msg_w_fit_on_scr &&
            (px_pos.x >= px_x_dots);

        if (draw_dots)
        {
            draw_glyph_at_px('.', px_pos, clr_gray, false);
        }
        else // Whole message fits, or we are not yet near the screen edge
        {
            draw_glyph_at_px(str[i], px_pos, clr, false);
        }

        px_pos.x += cell_px_w;
    }
}

int draw_text_center(const std::string& str,
                     const Panel panel,
                     const P& pos,
                     const Clr& clr,
                     const Clr& bg_clr,
                     const bool is_pixel_pos_adj_allowed)
{
    if (!is_inited())
    {
        return 0;
    }

    const int len = str.size();
    const int len_half = len / 2;
    const int x_pos_left = pos.x - len_half;

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
        (Sint16)px_pos.x,
        (Sint16)px_pos.y,
        (Uint16)w_tot_pixel,
        (Uint16)cell_dims.y
    };

    SDL_FillRect(scr_srf_,
                 &sdl_rect,
                 SDL_MapRGB(scr_srf_->format,
                            bg_clr.r,
                            bg_clr.g,
                            bg_clr.b));

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
    if (!is_inited())
    {
        return;
    }

    const int screen_pixel_w = config::scr_px_w();

    switch (panel)
    {
    case Panel::status_lines:
    {
        const P px_pos = px_pos_for_cell_in_panel(panel, P(0, 0));
        cover_area_px(px_pos, P(screen_pixel_w, config::stat_lines_px_h()));
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
        states::draw();
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

void draw_box(const R& border,
              const Panel panel,
              const Clr& clr,
              const bool do_cover_area)
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
            draw_tile(TileId::popup_ver,
                      panel,
                      P(border.p0.x, y),
                      clr,
                      clr_black);

            draw_tile(TileId::popup_ver,
                      panel,
                      P(border.p1.x, y),
                      clr,
                      clr_black);
        }
        else
        {
            draw_glyph('|', panel,
                       P(border.p0.x, y),
                       clr,
                       true,
                       clr_black);

            draw_glyph('|',
                       panel,
                       P(border.p1.x, y),
                       clr,
                       true,
                       clr_black);
        }
    }

    // Horizontal bars
    const int X0_VERT = border.p0.x + 1;
    const int X1_VERT = border.p1.x - 1;

    for (int x = X0_VERT; x <= X1_VERT; ++x)
    {
        if (is_tile_mode)
        {
            draw_tile(TileId::popup_hor,
                      panel,
                      P(x, border.p0.y),
                      clr,
                      clr_black);

            draw_tile(TileId::popup_hor,
                      panel,
                      P(x, border.p1.y),
                      clr,
                      clr_black);
        }
        else // Text mode
        {
            draw_glyph('-',
                       panel,
                       P(x, border.p0.y),
                       clr,
                       true,
                       clr_black);

            draw_glyph('-',
                       panel,
                       P(x, border.p1.y),
                       clr,
                       true,
                       clr_black);
        }
    }

    const std::vector<P> corners
    {
        {border.p0.x, border.p0.y}, //Top left
        {border.p1.x, border.p0.y}, //Top right
        {border.p0.x, border.p1.y}, //Btm left
        {border.p1.x, border.p1.y}  //Brm right
    };

    // Corners
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
    const int descr_y0 = 1;
    const int descr_x1 = map_w - 1;

    cover_area(Panel::screen,
               R(descr_x0 - 1, descr_y0, descr_x1, screen_h - 1));

    const int max_w = descr_x1 - descr_x0 + 1;

    P p(descr_x0, descr_y0);

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
            io::draw_tile(TileId::popup_hor,
                              Panel::screen,
                              P(x, 0),
                              clr_title);

            io::draw_tile(TileId::popup_hor,
                              Panel::screen,
                              P(x, screen_h - 1),
                              clr_title);
        }
    }
    else // Text mode
    {
        const std::string decoration_line(map_w, '-');

        io::draw_text(decoration_line,
                          Panel::screen,
                          P(0, 0),
                          clr_title);

        io::draw_text(decoration_line,
                          Panel::screen,
                          P(0, screen_h - 1),
                          clr_title);
    }

    const int x_label = 3;

    io::draw_text(" " + title + " ",
                      Panel::screen,
                      P(x_label, 0),
                      clr_title);

    const std::string cmd_info =
        screen_type == InfScreenType::scrolling ?
        info_scr_tip_scrollable :
        info_scr_tip;

    io::draw_text(" " + cmd_info + " ",
                      Panel::screen,
                      P(x_label, screen_h - 1),
                      clr_title);
}

P px_pos_for_cell_in_panel(const Panel panel, const P& pos)
{
    const P cell_dims(config::cell_px_w(),
                      config::cell_px_h());

    switch (panel)
    {
    case Panel::screen:
        return P(pos.x * cell_dims.x,
                 pos.y * cell_dims.y);

    case Panel::map:
        return (pos * cell_dims) + P(0, config::map_px_offset_h());

    case Panel::log:
        return pos * cell_dims;

    case Panel::status_lines:
        return (pos * cell_dims) + P(0, config::stat_lines_px_offset_h());
    }

    return P();
}

void flush_input()
{
    SDL_PumpEvents();
}

void clear_events()
{
    if (is_inited())
    {
        while (SDL_PollEvent(&sdl_event_)) {}
    }
}

InputData get(const bool is_o_return)
{
    InputData ret = InputData();

    if (!is_inited())
    {
        return ret;
    }

    SDL_StartTextInput();

    bool is_done = false;

    while (!is_done)
    {
        sdl_base::sleep(1);

        const bool did_poll_event = SDL_PollEvent(&sdl_event_);

        if (!did_poll_event)
        {
            continue;
        }

        switch (sdl_event_.type)
        {

        case SDL_WINDOWEVENT:
        {
            switch (sdl_event_.window.event)
            {
            case SDL_WINDOWEVENT_FOCUS_GAINED:
            case SDL_WINDOWEVENT_RESTORED:
            {
                io::update_screen();
            }
            break;

            default:
                break;
            }
        }
        break;

        case SDL_QUIT:
            ret = InputData(SDLK_ESCAPE);
            is_done = true;
            break;

        case SDL_KEYDOWN:
        {
            const int key = sdl_event_.key.keysym.sym;

            // Do not return shift, control or alt as separate key events
            if (
                key == SDLK_LSHIFT ||
                key == SDLK_RSHIFT ||
                key == SDLK_LCTRL ||
                key == SDLK_RCTRL ||
                key == SDLK_LALT ||
                key == SDLK_RALT)
            {
                continue;
            }

            Uint16 mod = SDL_GetModState();

            const bool is_shift_held = mod & KMOD_SHIFT;
            const bool is_ctrl_held = mod & KMOD_CTRL;
            const bool is_alt_held = mod & KMOD_ALT;

            ret = InputData(key,
                            is_shift_held,
                            is_ctrl_held);

            if (key >= SDLK_F1 && key <= SDLK_F9)
            {
                // F keys
                is_done = true;
            }
            else // Not an F key
            {
                switch (key)
                {
                case SDLK_RETURN:
                case SDLK_RETURN2:
                case SDLK_KP_ENTER:
                    if (is_alt_held)
                    {
                        config::toggle_fullscreen();
                        clear_events();
                        continue;
                    }
                    else // Alt is not held
                    {
                        ret.key = SDLK_RETURN;
                        is_done = true;
                    }
                    break;

                case SDLK_SPACE:
                case SDLK_BACKSPACE:
                case SDLK_TAB:
                case SDLK_PAGEUP:
                case SDLK_PAGEDOWN:
                case SDLK_END:
                case SDLK_HOME:
                case SDLK_INSERT:
                case SDLK_DELETE:
                case SDLK_LEFT:
                case SDLK_RIGHT:
                case SDLK_UP:
                case SDLK_DOWN:
                case SDLK_ESCAPE:
                    is_done = true;
                    break;

                case SDLK_MENU:
                case SDLK_PAUSE:
                default:
                    break;
                }
            }
        }
        break;

        case SDL_TEXTINPUT:
        {
            const char c = sdl_event_.text.text[0];

            if ((c == 'o' || c == 'O') &&
                is_o_return)
            {
                const bool is_shift_held = c == 'O';

                ret = InputData(SDLK_RETURN,
                                is_shift_held,
                                false);

                is_done = true;
            }
            else if (c >= 33 && c < 126)
            {
                // ASCII char entered
                // (Decimal unicode '!' = 33, '~' = 126)

                clear_events();

                ret = InputData((int)c);

                is_done = true;
            }
            else
            {
                continue;
            }
        }
        break;

        default:
            break;

        } // Event type switch
    } // While

    SDL_StopTextInput();

    return ret;
}

} // io
