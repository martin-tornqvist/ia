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
#include "config.hpp"
#include "colors.hpp"
#include "rl_utils.hpp"
#include "viewport.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static SDL_Window* sdl_window_ = nullptr;
static SDL_Renderer* sdl_renderer_ = nullptr;

// Bytes per pixel
static int bpp_ = -1;

static SDL_Surface* screen_srf_ = nullptr;
static SDL_Texture* screen_texture_ = nullptr;

static SDL_Surface* main_menu_logo_srf_ = nullptr;
static SDL_Surface* skull_srf_ = nullptr;

static const size_t font_nr_x_ = 16;
static const size_t font_nr_y_ = 7;

static std::vector<P> tile_px_data_[(size_t)TileId::END];
static std::vector<P> font_px_data_[font_nr_x_][font_nr_y_];

static std::vector<P> tile_contour_px_data_[(size_t)TileId::END];
static std::vector<P> font_contour_px_data_[font_nr_x_][font_nr_y_];

static const SDL_Color sdl_color_black = {0, 0, 0, 0};

static SDL_Event sdl_event_;

static bool is_inited()
{
        return sdl_window_;
}

static void init_screen_surface(const PxPos dims)
{
        TRACE << "Initializing screen surface" << std::endl;

        if (screen_srf_)
        {
                SDL_FreeSurface(screen_srf_);
        }

        screen_srf_ = SDL_CreateRGBSurface(
                0,
                dims.value.x,
                dims.value.y,
                screen_bpp,
                0x00FF0000,
                0x0000FF00,
                0x000000FF,
                0xFF000000);

        if (!screen_srf_)
        {
                TRACE_ERROR_RELEASE << "Failed to create screen surface"
                                    << std::endl
                                    << SDL_GetError()
                                    << std::endl;

                PANIC;
        }
}

static void init_window(const PxPos dims)
{
        TRACE << "Initializing window" << std::endl;

        const std::string title = "IA " + version_str;

        if (sdl_window_)
        {
                SDL_DestroyWindow(sdl_window_);
        }

        if (config::is_fullscreen())
        {
                TRACE << "Fullscreen mode" << std::endl;

                sdl_window_ = SDL_CreateWindow(
                        title.c_str(),
                        SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED,
                        dims.value.x,
                        dims.value.y,
                        SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        else // Windowed mode
        {
                TRACE << "Windowed mode" << std::endl;

                sdl_window_ = SDL_CreateWindow(
                        title.c_str(),
                        SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED,
                        dims.value.x,
                        dims.value.y,
                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        }

        if (!sdl_window_)
        {
                TRACE << "Failed to create window"
                      << std::endl
                      << SDL_GetError()
                      << std::endl;
        }
}

static void init_renderer(const PxPos dims)
{
        TRACE << "Initializing renderer" << std::endl;

        if (sdl_renderer_)
        {
                SDL_DestroyRenderer(sdl_renderer_);
        }

        sdl_renderer_ = SDL_CreateRenderer(
                sdl_window_,
                -1,
                SDL_RENDERER_SOFTWARE);

        if (!sdl_renderer_)
        {
                TRACE << "Failed to create SDL renderer"
                      << std::endl
                      << SDL_GetError()
                      << std::endl;

                PANIC;
        }

        SDL_RenderSetLogicalSize(
                sdl_renderer_,
                dims.value.x,
                dims.value.y);

        // SDL_RenderSetIntegerScale(sdl_renderer_, SDL_TRUE);

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
}

static void init_screen_texture(const PxPos dims)
{
        TRACE << "Initializing screen texture" << std::endl;

        if (screen_texture_)
        {
                SDL_DestroyTexture(screen_texture_);
        }

        screen_texture_ = SDL_CreateTexture(
                sdl_renderer_,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
                dims.value.x,
                dims.value.y);

        if (!screen_texture_)
        {
                TRACE_ERROR_RELEASE << "Failed to create screen texture"
                                    << std::endl
                                    << SDL_GetError()
                                    << std::endl;

                PANIC;
        }
}

static PxPos get_resolution()
{
        // TODO: Make an option for this (native resolution or stretched when
        // running fullscreen)
        if (config::is_fullscreen())
        {
                const P min_gui_dims = io::min_screen_gui_dims();

                return io::gui_to_px_coords(min_gui_dims);
        }

        SDL_DisplayMode dm;

        if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
        {
                TRACE_ERROR_RELEASE << "Failed to read native resolution"
                                    << std::endl
                                    << SDL_GetError()
                                    << std::endl;

                PANIC;
        }

        return PxPos(dm.w, dm.h);
}

static bool is_window_maximized()
{
        // TODO: This does not seem to work very well:
        // * The flag is sometimes not set when maximizing the window
        // * The flag sometimes gets "stuck" after the window is restored from
        //   being maximized. The flag is only cleared again after tabbing to
        //   another window and back again, or after minimizing and restoring
        //   the window
        //
        // Is this an SDL bug?
        //
        return SDL_GetWindowFlags(sdl_window_) & SDL_WINDOW_MAXIMIZED;
}

// Pointer to function used for writing pixels on the screen (there are
// different variants depending on bpp)
static void (*put_px_ptr_)(
        const SDL_Surface& srf,
        int pixel_x,
        int pixel_y,
        Uint32 px) = nullptr;

static Uint32 px(const SDL_Surface& srf,
                 const int pixel_x,
                 const int pixel_y)
{
        // p is the address to the pixel we want to retrieve
        Uint8* p =
                (Uint8*)srf.pixels +
                (pixel_y * srf.pitch) +
                (pixel_x * bpp_);

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
                return *(uint32_t*)p;

        default:
                break;
        }

        return -1;
}

static void put_px8(
        const SDL_Surface& srf,
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

static void put_px16(
        const SDL_Surface& srf,
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

static void put_px24(
        const SDL_Surface& srf,
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

static void put_px32(
        const SDL_Surface& srf,
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

static void blit_surface(SDL_Surface& srf, const PxPos pos)
{
        SDL_Rect dst_rect;

        dst_rect.x = pos.value.x;
        dst_rect.y = pos.value.y;
        dst_rect.w = srf.w;
        dst_rect.h = srf.h;

        SDL_BlitSurface(&srf, nullptr, screen_srf_, &dst_rect);
}

static void load_contour(const std::vector<P>& source_px_data,
                         std::vector<P>& dest_px_data,
                         const PxPos cell_px_dims)
{
        for (const P source_px_pos : source_px_data)
        {
                const int size = 1;

                const int px_x0 =
                        std::max(0, source_px_pos.x - size);

                const int px_y0 =
                        std::max(0, source_px_pos.y - size);

                const int px_x1 =
                        std::min(cell_px_dims.value.x - 1,
                                 source_px_pos.x + size);

                const int px_y1 =
                        std::min(cell_px_dims.value.y - 1,
                                 source_px_pos.y + size);

                for (int px_x = px_x0; px_x <= px_x1; ++px_x)
                {
                        for (int px_y = px_y0; px_y <= px_y1; ++px_y)
                        {
                                const P p(px_x, px_y);

                                // Only mark pixel as contour if it's not marked
                                // on the source
                                const auto it =
                                        std::find(begin(source_px_data),
                                                  end(source_px_data),
                                                  p);

                                if (it == end(source_px_data))
                                {
                                        dest_px_data.push_back(p);
                                }
                        }
                }
        }
}

static void load_images()
{
        TRACE_FUNC_BEGIN;

        // Main menu logo
        SDL_Surface* tmp_srf = IMG_Load(logo_img_path.c_str());

        ASSERT(tmp_srf && "Failed to load main menu logo image");

        main_menu_logo_srf_ =
                SDL_ConvertSurface(tmp_srf, screen_srf_->format, 0);

        SDL_FreeSurface(tmp_srf);

        // Skull
        tmp_srf = IMG_Load(skull_img_path.c_str());

        ASSERT(tmp_srf && "Failed to load skull image");

        skull_srf_ = SDL_ConvertSurface(tmp_srf, screen_srf_->format, 0);

        SDL_FreeSurface(tmp_srf);

        TRACE_FUNC_END;
}

static void load_font()
{
        TRACE_FUNC_BEGIN;

        const std::string font_path = fonts_path + "/" + config::font_name();

        SDL_Surface* const font_srf_tmp = IMG_Load(font_path.c_str());

        if (!font_srf_tmp)
        {
                TRACE << "Failed to load font: " << IMG_GetError() << std::endl;

                PANIC;
        }

        const Uint32 img_color = SDL_MapRGB(
                font_srf_tmp->format,
                255,
                255,
                255);

        const PxPos cell_px_dims(
                config::gui_cell_px_w(),
                config::gui_cell_px_h());

        for (int x = 0; x < (int)font_nr_x_; ++x)
        {
                for (int y = 0; y < (int)font_nr_y_; ++y)
                {
                        auto& px_data = font_px_data_[x][y];

                        px_data.clear();

                        const int sheet_x0 =
                                x * cell_px_dims.value.x;

                        const int sheet_y0 =
                                y * cell_px_dims.value.y;

                        const int sheet_x1 =
                                sheet_x0 + cell_px_dims.value.x - 1;

                        const int sheet_y1 =
                                sheet_y0 + cell_px_dims.value.y - 1;

                        for (int sheet_x = sheet_x0;
                             sheet_x <= sheet_x1;
                             ++sheet_x)
                        {
                                for (int sheet_y = sheet_y0;
                                     sheet_y <= sheet_y1;
                                     ++sheet_y)
                                {
                                        const auto current_px =
                                                px(*font_srf_tmp,
                                                   sheet_x,
                                                   sheet_y);

                                        const bool is_img_px =
                                                current_px == img_color;

                                        if (is_img_px)
                                        {
                                                const int x_relative =
                                                        sheet_x - sheet_x0;

                                                const int y_relative =
                                                        sheet_y - sheet_y0;

                                                px_data.push_back(
                                                        P(x_relative,
                                                          y_relative));
                                        }
                                }
                        }

                        load_contour(
                                px_data,
                                font_contour_px_data_[x][y],
                                cell_px_dims);
                }
        }

        SDL_FreeSurface(font_srf_tmp);

        TRACE_FUNC_END;
}

static void load_tiles()
{
        TRACE_FUNC_BEGIN;

        const PxPos cell_px_dims(
                config::map_cell_px_w(),
                config::map_cell_px_h());

        for (size_t i = 0; i < (size_t)TileId::END; ++i)
        {
                const auto id = (TileId)i;

                const std::string img_name = tile_id_to_str_map.at(id);

                const std::string img_path =
                        tiles_path + "/" + img_name + ".png";

                SDL_Surface* const tile_srf_tmp = IMG_Load(img_path.c_str());

                if (!tile_srf_tmp)
                {
                        TRACE_ERROR_RELEASE
                                << "Could not load tile image, at: "
                                << img_path
                                << std::endl;

                        PANIC;
                }

                // Verify width and height of loaded image
                if ((tile_srf_tmp->w != cell_px_dims.value.x) ||
                    (tile_srf_tmp->h != cell_px_dims.value.y))
                {
                        TRACE_ERROR_RELEASE
                                << "Tile image at \""
                                << img_path
                                << "\" has wrong size: "
                                << tile_srf_tmp->w
                                << "x"
                                << tile_srf_tmp->h
                                << ", expected: "
                                << cell_px_dims.value.x
                                << "x"
                                << cell_px_dims.value.y
                                << std::endl;

                        PANIC;
                }

                const Uint32 img_color =
                        SDL_MapRGB(tile_srf_tmp->format, 255, 255, 255);

                auto& px_data = tile_px_data_[i];

                px_data.clear();

                for (int x = 0; x < cell_px_dims.value.x; ++x)
                {
                        for (int y = 0; y < cell_px_dims.value.y; ++y)
                        {
                                const auto current_px =
                                        px(*tile_srf_tmp, x, y);

                                const bool is_img_px =
                                        current_px == img_color;

                                if (is_img_px)
                                {
                                        px_data.push_back(P(x, y));
                                }
                        }
                }

                load_contour(
                        px_data,
                        tile_contour_px_data_[i],
                        cell_px_dims);

                SDL_FreeSurface(tile_srf_tmp);
        }

        TRACE_FUNC_END;
}

static void put_pixels_on_screen(
        const std::vector<P> px_data,
        const PxPos pos,
        const Color& color)
{
        const auto sdl_color = color.sdl_color();

        const int px_color = SDL_MapRGB(
                screen_srf_->format,
                sdl_color.r,
                sdl_color.g,
                sdl_color.b);

        for (const auto p_relative : px_data)
        {
                const int screen_px_x = pos.value.x + p_relative.x;
                const int screen_px_y = pos.value.y + p_relative.y;

                put_px_ptr_(*screen_srf_,
                            screen_px_x,
                            screen_px_y,
                            px_color);
        }
}

static void put_pixels_on_screen(
        const TileId tile,
        const PxPos pos,
        const Color& color)
{
        const auto& pixel_data = tile_px_data_[(size_t)tile];

        put_pixels_on_screen(pixel_data, pos, color);
}

static void put_pixels_on_screen(
        const char character,
        const PxPos pos,
        const Color& color)
{
        const P sheet_pos(gfx::character_pos(character));

        const auto& pixel_data = font_px_data_[sheet_pos.x][sheet_pos.y];

        put_pixels_on_screen(pixel_data, pos, color);
}

static void draw_character(
        const char character,
        const PxPos pos,
        const Color& color,
        const bool draw_bg = true,
        const Color& bg_color = Color(0, 0, 0))
{
        if (draw_bg)
        {
                const PxPos cell_dims(
                        config::gui_cell_px_w(),
                        config::gui_cell_px_h());

                io::draw_rectangle_solid(
                        pos,
                        cell_dims,
                        bg_color);
        }

        // Draw contour if neither foreground/background is black
        if ((color != colors::black()) &&
            (bg_color != colors::black()))
        {
                const P char_pos(gfx::character_pos(character));

                const auto& contour_px_data =
                        font_contour_px_data_[char_pos.x][char_pos.y];

                put_pixels_on_screen(
                        contour_px_data,
                        pos,
                        sdl_color_black);
        }

        put_pixels_on_screen(
                character,
                pos,
                color);
}

static void cover_area(const PxRect area, const Color& color)
{
        const PxPos px_pos(
                area.value.p0.x,
                area.value.p0.y);

        const PxPos px_dims(
                area.value.w(),
                area.value.h());

        io::draw_rectangle_solid(
                px_pos,
                px_dims,
                color);
}

static PxRect gui_to_px_rect(const R rect)
{
        PxRect px_rect;

        const int gui_cell_px_w = config::gui_cell_px_w();
        const int gui_cell_px_h = config::gui_cell_px_h();

        px_rect.value = rect.scaled_up(
                gui_cell_px_w,
                gui_cell_px_h);

        // We must include ALL pixels in the given gui area
        px_rect.value.p1 =
                px_rect.value.p1.with_offsets(
                        gui_cell_px_w - 1,
                        gui_cell_px_h - 1);

        return px_rect;
}

static int panel_px_w(const Panel panel)
{
        return io::gui_to_px_coords_x(panels::get_w(panel));
}

static int panel_px_h(const Panel panel)
{
        return io::gui_to_px_coords_y(panels::get_h(panel));
}

static PxPos panel_px_dims(const Panel panel)
{
        return io::gui_to_px_coords(panels::get_dims(panel));
}

static void draw_text(
        const std::string& str,
        PxPos pos,
        const Color& color,
        const bool draw_bg,
        const Color& bg_color)
{
        if (!is_inited() || !panels::is_valid())
        {
                return;
        }

        if ((pos.value.y < 0) ||
            (pos.value.y >= panel_px_h(Panel::screen)))
        {
                return;
        }

        const int cell_px_w = config::gui_cell_px_w();
        const size_t msg_w = str.size();
        const int msg_px_w = msg_w * cell_px_w;

        const auto sdl_color = color.sdl_color();

        const auto sdl_bg_color = bg_color.sdl_color();

        const auto sdl_color_gray = colors::gray();

        const int screen_px_w = panel_px_w(Panel::screen);
        const int msg_px_x1 = pos.value.x + msg_px_w - 1;
        const bool msg_w_fit_on_screen = msg_px_x1 < screen_px_w;

        // X position to start drawing dots instead when the message does not
        // fit on the screen horizontally.
        const int px_x_dots = screen_px_w - (cell_px_w * 3);

        for (size_t i = 0; i < msg_w; ++i)
        {
                if (pos.value.x < 0 || pos.value.x >= screen_px_w)
                {
                        return;
                }

                const bool draw_dots =
                        !msg_w_fit_on_screen &&
                        (pos.value.x >= px_x_dots);

                if (draw_dots)
                {
                        draw_character(
                                '.',
                                pos,
                                sdl_color_gray,
                                draw_bg,
                                bg_color);
                }
                else // Whole message fits, or we are not yet near the edge
                {
                        draw_character(
                                str[i],
                                pos,
                                sdl_color,
                                draw_bg,
                                sdl_bg_color);
                }

                pos.value.x += cell_px_w;
        }
}

static PxPos get_window_px_dims()
{
        PxPos px_dims;

        SDL_GetWindowSize(
                sdl_window_,
                &px_dims.value.x,
                &px_dims.value.y);

        return px_dims;
}

static P get_window_gui_dims()
{
        const PxPos px_dims = get_window_px_dims();

        return io::px_to_gui_coords(px_dims);
}

static void try_set_window_gui_cells(P new_gui_dims)
{
        const P min_gui_dims = io::min_screen_gui_dims();

        new_gui_dims.x = std::max(new_gui_dims.x, min_gui_dims.x);
        new_gui_dims.y = std::max(new_gui_dims.y, min_gui_dims.y);

        const PxPos new_px_dims = io::gui_to_px_coords(new_gui_dims);

        SDL_SetWindowSize(
                sdl_window_,
                new_px_dims.value.x,
                new_px_dims.value.y);
}

static void resize_window_to_nearest_gui_cells()
{
        P gui_dims = get_window_gui_dims();

        const P min_gui_dims = io::min_screen_gui_dims();

        gui_dims.x = std::max(gui_dims.x, min_gui_dims.x);
        gui_dims.y = std::max(gui_dims.y, min_gui_dims.y);

        PxPos px_dims;

        px_dims.value = gui_dims.scaled_up(
                config::gui_cell_px_w(),
                config::gui_cell_px_h());

        SDL_SetWindowSize(sdl_window_, px_dims.value.x, px_dims.value.y);
}

static void on_window_resized()
{
        PxPos new_px_dims = get_window_px_dims();

        TRACE << "New window size: "
              << new_px_dims.value.x
              << ", "
              << new_px_dims.value.y
              << std::endl;

        panels::init(io::px_to_gui_coords(new_px_dims));

        init_renderer(new_px_dims);

        init_screen_surface(new_px_dims);

        init_screen_texture(new_px_dims);

        states::on_window_resized();

        states::draw();

        io::update_screen();
}

// -----------------------------------------------------------------------------
// io
// -----------------------------------------------------------------------------
namespace io
{

void init()
{
        TRACE_FUNC_BEGIN;

        cleanup();

        const PxPos resolution = get_resolution();

        PxPos screen_px_dims;

        if (config::is_fullscreen())
        {
                panels::init(io::px_to_gui_coords(resolution));

                screen_px_dims = panel_px_dims(Panel::screen);

                init_window(screen_px_dims);

                if (!sdl_window_)
                {
                        // Fullscreen failed, fall back on windowed mode instead
                        config::set_fullscreen(false);
                }
        }

        // NOTE: Fullscreen may have been disabled while attempting to set up a
        // fullscreen "window" (see above), so we check again here if fullscreen
        // is enabled
        if (!config::is_fullscreen())
        {
                // Put an upper limit on the initial screen size
                // const PxPos init_dims(
                //         std::min(resolution.value.x, 1280),
                //         std::min(resolution.value.y, 720));

                // panels::init(io::px_to_gui_coords(init_dims));

                panels::init(io::min_screen_gui_dims());

                screen_px_dims = panel_px_dims(Panel::screen);

                init_window(screen_px_dims);
        }

        if (!sdl_window_)
        {
                TRACE_ERROR_RELEASE << "Failed to set up window"
                                    << std::endl
                                    << SDL_GetError()
                                    << std::endl;
        }

        init_renderer(screen_px_dims);

        init_screen_surface(screen_px_dims);

        bpp_ = screen_srf_->format->BytesPerPixel;

        TRACE << "bpp: " << bpp_ << std::endl;

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
                TRACE_ERROR_RELEASE << "Invalid bpp: " << bpp_ << std::endl;

                PANIC;
        }

        init_screen_texture(screen_px_dims);

        load_font();

        if (config::is_tiles_mode())
        {
                load_tiles();

                load_images();
        }

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

        if (skull_srf_)
        {
                SDL_FreeSurface(skull_srf_);
                skull_srf_ = nullptr;
        }

        TRACE_FUNC_END;
}

void update_screen()
{
        if (is_inited())
        {
                SDL_UpdateTexture(
                        screen_texture_,
                        nullptr,
                        screen_srf_->pixels,
                        screen_srf_->pitch);

                SDL_RenderCopy(
                        sdl_renderer_,
                        screen_texture_,
                        nullptr,
                        nullptr);

                SDL_RenderPresent(sdl_renderer_);
        }
}

void clear_screen()
{
        if (is_inited())
        {
                SDL_FillRect(
                        screen_srf_,
                        nullptr,
                        SDL_MapRGB(screen_srf_->format, 0, 0, 0));
        }
}

P min_screen_gui_dims()
{
        // Hard minimum required gui cells
        const int hard_min_gui_cells_x = 80;
        const int hard_min_gui_cells_y = 25;

        // Minimum required resolution
        const int min_res_w = 800;
        const int min_res_h = 600;

        const int gui_cell_w = config::gui_cell_px_w();
        const int gui_cell_h = config::gui_cell_px_h();

        // Use the maximum of:
        // * The minimum required resolution, converted to gui cells, rounded up
        // * The hard minimum required number of gui cells

        int min_gui_cells_x = (min_res_w + gui_cell_w - 1) / gui_cell_w;
        int min_gui_cells_y = (min_res_h + gui_cell_h - 1) / gui_cell_h;

        min_gui_cells_x = std::max(min_gui_cells_x, hard_min_gui_cells_x);
        min_gui_cells_y = std::max(min_gui_cells_y, hard_min_gui_cells_y);

        return P(min_gui_cells_x, min_gui_cells_y);
}

void on_fullscreen_toggled()
{
        clear_screen();

        update_screen();

        // TODO: This should not be necessary - but without this call there is
        // some strange behavior with the window losing focus, and becoming
        // hidden behind other windows when toggling fullscreen (until you
        // alt-tab back to the window)
        sdl_base::init();

        init();

        states::draw();

        update_screen();
}

int gui_to_px_coords_x(const int value)
{
        return value * config::gui_cell_px_w();
}

int gui_to_px_coords_y(const int value)
{
        return value * config::gui_cell_px_h();
}

int map_to_px_coords_x(const int value)
{
        return value * config::map_cell_px_w();
}

int map_to_px_coords_y(const int value)
{
        return value * config::map_cell_px_h();
}

PxPos gui_to_px_coords(const P pos)
{
        PxPos px_pos;

        px_pos.value.x = gui_to_px_coords_x(pos.x);
        px_pos.value.y = gui_to_px_coords_y(pos.y);

        return px_pos;
}

PxPos gui_to_px_coords(const int x, const int y)
{
        return gui_to_px_coords(P(x, y));
}

PxPos map_to_px_coords(const P pos)
{
        PxPos px_pos;

        px_pos.value.x = map_to_px_coords_x(pos.x);
        px_pos.value.y = map_to_px_coords_y(pos.y);

        return px_pos;
}

PxPos map_to_px_coords(const int x, const int y)
{
        return map_to_px_coords(P(x, y));
}

P px_to_gui_coords(const PxPos px_pos)
{
        return P(px_pos.value.x / config::gui_cell_px_w(),
                 px_pos.value.y / config::gui_cell_px_h());
}

P px_to_map_coords(const PxPos px_pos)
{
        return P(px_pos.value.x / config::map_cell_px_w(),
                 px_pos.value.y / config::map_cell_px_h());
}

P gui_to_map_coords(const P gui_pos)
{
        const PxPos px_coords = gui_to_px_coords(gui_pos);

        return px_to_map_coords(px_coords);
}

PxPos gui_to_px_coords(const Panel panel, const P offset)
{
        const P pos = panels::get_area(panel).p0 + offset;

        return gui_to_px_coords(pos);
}

PxPos map_to_px_coords(const Panel panel, const P offset)
{
        const PxPos px_p0 = gui_to_px_coords(panels::get_area(panel).p0);

        const PxPos px_offset = map_to_px_coords(offset);

        PxPos pos;

        pos.value = px_p0.value + px_offset.value;

        return pos;
}

void draw_tile(
        const TileId tile,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg,
        const Color& bg_color)
{
        if (!is_inited() || !panels::is_valid())
        {
                return;
        }

        const PxPos px_pos = map_to_px_coords(panel, pos);

        if (draw_bg)
        {
                const PxPos cell_dims(
                        config::map_cell_px_w(),
                        config::map_cell_px_h());

                draw_rectangle_solid(
                        px_pos,
                        cell_dims,
                        bg_color);
        }

        // Draw contour if neither the foreground nor background is black
        if ((color != colors::black()) &&
            (bg_color != colors::black()))
        {
                const auto& contour_px_data =
                        tile_contour_px_data_[(size_t)tile];

                put_pixels_on_screen(
                        contour_px_data,
                        px_pos,
                        sdl_color_black);
        }

        put_pixels_on_screen(
                tile,
                px_pos,
                color.sdl_color());
}

void draw_character(
        const char character,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg,
        const Color& bg_color)
{
        if (!is_inited() || !panels::is_valid())
        {
                return;
        }

        const PxPos px_pos = gui_to_px_coords(panel, pos);

        const auto sdl_color = color.sdl_color();

        const auto sdl_color_bg = bg_color.sdl_color();

        draw_character(
                character,
                px_pos,
                sdl_color,
                draw_bg,
                sdl_color_bg);
}

void draw_text(
        const std::string& str,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg,
        const Color& bg_color)
{
        if (!is_inited() || !panels::is_valid())
        {
                return;
        }

        PxPos px_pos = gui_to_px_coords(panel, pos);

        draw_text(
                str,
                px_pos,
                color,
                draw_bg,
                bg_color);
}

void draw_text_center(
        const std::string& str,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg,
        const Color& bg_color,
        const bool is_pixel_pos_adj_allowed)
{
        if (!is_inited() || !panels::is_valid())
        {
                return;
        }

        const int len = str.size();
        const int len_half = len / 2;
        const int x_pos_left = pos.x - len_half;

        PxPos px_pos = gui_to_px_coords(
                panel,
                P(x_pos_left, pos.y));

        if (is_pixel_pos_adj_allowed)
        {
                const int pixel_x_adj =
                        ((len_half * 2) == len) ?
                        (config::gui_cell_px_w() / 2) :
                        0;

                px_pos.value += P(pixel_x_adj, 0);
        }

        draw_text(
                str,
                px_pos,
                color,
                draw_bg,
                bg_color);
}

void draw_text_right(
        const std::string& str,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg,
        const Color& bg_color)
{
        if (!is_inited() || !panels::is_valid())
        {
                return;
        }

        const int x_pos_left = pos.x - str.size() + 1;

        PxPos px_pos = gui_to_px_coords(
                panel,
                P(x_pos_left, pos.y));

        draw_text(
                str,
                px_pos,
                color,
                draw_bg,
                bg_color);
}

void draw_rectangle_solid(
        const PxPos pos,
        const PxPos dims,
        const Color& color)
{
        if (is_inited() && panels::is_valid())
        {
                SDL_Rect sdl_rect = {
                        (Sint16)pos.value.x,
                        (Sint16)pos.value.y,
                        (Uint16)dims.value.x,
                        (Uint16)dims.value.y
                };

                const SDL_Color& sdl_color = color.sdl_color();

                SDL_FillRect(screen_srf_,
                             &sdl_rect,
                             SDL_MapRGB(screen_srf_->format,
                                        sdl_color.r,
                                        sdl_color.g,
                                        sdl_color.b));
        }
}

void cover_panel(const Panel panel, const Color& color)
{
        if (!is_inited() || !panels::is_valid())
        {
                return;
        }

        const PxRect px_area = gui_to_px_rect(panels::get_area(panel));

        cover_area(px_area, color);
}

void cover_area(
        const Panel panel,
        const R area,
        const Color& color)
{
        const P panel_p0 = panels::get_p0(panel);

        const R screen_area = area.with_offset(panel_p0);

        const PxRect px_area = gui_to_px_rect(screen_area);

        cover_area(px_area, color);
}

void cover_area(
        const Panel panel,
        const P offset,
        const P dims,
        const Color& color)
{
        cover_area(
                panel,
                {offset, offset + dims - 1},
                color);
}

void cover_cell(const Panel panel, const P offset)
{
        cover_area(panel, offset, P(1, 1));
}

void draw_box(const R& border, const Color& color)
{
        const PxRect outer_px_border = gui_to_px_rect(border);

        const int gui_cell_w = config::gui_cell_px_w();
        const int gui_cell_h = config::gui_cell_px_h();

        const int x_offset = gui_cell_w / 2;
        const int y_offset = gui_cell_h / 2;

        const int x0 = outer_px_border.value.p0.x + x_offset;
        const int y0 = outer_px_border.value.p0.y + y_offset;
        const int x1 = outer_px_border.value.p1.x - x_offset;
        const int y1 = outer_px_border.value.p1.y - y_offset;

        draw_rectangle_solid(
                PxPos(x0, y0),
                {x1 - x0 + 1, 2},
                color);

        draw_rectangle_solid(
                PxPos(x0, y1),
                {x1 - x0 + 1, 2},
                color);

        draw_rectangle_solid(
                PxPos(x0, y0),
                {2, y1 - y0 + 1},
                color);

        draw_rectangle_solid(
                PxPos(x1, y0),
                {2, y1 - y0 + 1},
                color);

        // Vertical bars
        // const int y0_vert = border.p0.y + 1;
        // const int y1_vert = border.p1.y - 1;

        // for (int y = y0_vert; y <= y1_vert; ++y)
        // {
        //         draw_symbol(
        //                 TileId::popup_ver,
        //                 '|',
        //                 panel,
        //                 P(border.p0.x, y),
        //                 color);

        //         draw_symbol(
        //                 TileId::popup_ver,
        //                 '|',
        //                 panel,
        //                 P(border.p1.x, y),
        //                 color);
        // }

        // Horizontal bars
        // const int x0_vert = border.p0.x + 1;
        // const int x1_vert = border.p1.x - 1;

        // for (int x = x0_vert; x <= x1_vert; ++x)
        // {
        //         draw_symbol(
        //                 TileId::popup_hor,
        //                 '-',
        //                 panel,
        //                 P(x, border.p0.y),
        //                 color);

        //         draw_symbol(
        //                 TileId::popup_hor,
        //                 '-',
        //                 panel,
        //                 P(x, border.p1.y),
        //                 color);
        // }

        // const std::vector<P> corners
        // {
        //         {border.p0.x, border.p0.y}, //Top left
        //         {border.p1.x, border.p0.y}, //Top right
        //         {border.p0.x, border.p1.y}, //Btm left
        //         {border.p1.x, border.p1.y}  //Brm right
        // };

        // Corners
        // draw_symbol(
        //         TileId::popup_top_l,
        //         '+',
        //         panel,
        //         corners[0],
        //         color);

        // draw_symbol(
        //         TileId::popup_top_r,
        //         '+',
        //         panel,
        //         corners[1],
        //         color);

        // draw_symbol(
        //         TileId::popup_btm_l,
        //         '+',
        //         panel,
        //         corners[2],
        //         color);

        // draw_symbol(
        //         TileId::popup_btm_r,
        //         '+',
        //         panel,
        //         corners[3],
        //         color);
}

void draw_descr_box(const std::vector<ColoredString>& lines)
{
        cover_panel(Panel::item_descr);

        P pos(0, 0);

        for (const auto& line : lines)
        {
                const auto formatted =
                        text_format::split(
                                line.str,
                                panels::get_w(Panel::item_descr));

                for (const auto& formatted_line : formatted)
                {
                        draw_text(
                                formatted_line,
                                Panel::item_descr,
                                pos,
                                line.color);

                        ++pos.y;
                }

                ++pos.y;
        }
}

void draw_main_menu_logo()
{
        if (is_inited() && panels::is_valid())
        {
                const int screen_px_w = panel_px_w(Panel::screen);

                const int logo_px_h = main_menu_logo_srf_->w;

                const PxPos pos((screen_px_w - logo_px_h) / 2, 0);

                blit_surface(*main_menu_logo_srf_, pos);
        }
}

void draw_skull(const P pos)
{
        if (is_inited() && panels::is_valid())
        {
                const PxPos px_pos(
                        pos.x * config::map_cell_px_w(),
                        pos.y * config::map_cell_px_h());

                blit_surface(*skull_srf_, px_pos);
        }
}

void draw_blast_at_field(
        const P center_pos,
        const int radius,
        bool forbidden_cells[map_w][map_h],
        const Color& color_inner,
        const Color& color_outer)
{
        TRACE_FUNC_BEGIN;

        if (!is_inited() || !panels::is_valid())
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
                        if (forbidden_cells[pos.x][pos.y] ||
                            !viewport::is_in_ivew(pos))
                        {
                                continue;
                        }

                        const bool is_outer =
                                pos.x == center_pos.x - radius ||
                                pos.x == center_pos.x + radius ||
                                pos.y == center_pos.y - radius ||
                                pos.y == center_pos.y + radius;

                        const Color color =
                                is_outer ?
                                color_outer :
                                color_inner;

                        draw_symbol(
                                TileId::blast1,
                                '*',
                                Panel::map,
                                viewport::to_view_pos(pos),
                                color);

                        is_any_blast_rendered = true;
                }
        }

        update_screen();

        if (is_any_blast_rendered)
        {
                sdl_base::sleep(config::delay_explosion() / 2);
        }

        for (pos.y = std::max(1, center_pos.y - radius);
             pos.y <= std::min(map_h - 2, center_pos.y + radius);
             pos.y++)
        {
                for (pos.x = std::max(1, center_pos.x - radius);
                     pos.x <= std::min(map_w - 2, center_pos.x + radius);
                     pos.x++)
                {
                        if (forbidden_cells[pos.x][pos.y] ||
                            !viewport::is_in_ivew(pos))
                        {
                                continue;
                        }

                        const bool is_outer =
                                pos.x == center_pos.x - radius ||
                                pos.x == center_pos.x + radius ||
                                pos.y == center_pos.y - radius ||
                                pos.y == center_pos.y + radius;

                        const Color color =
                                is_outer ?
                                color_outer :
                                color_inner;

                        draw_symbol(
                                TileId::blast2,
                                '*',
                                Panel::map,
                                viewport::to_view_pos(pos),
                                color);
                }
        }

        update_screen();

        if (is_any_blast_rendered)
        {
                sdl_base::sleep(config::delay_explosion() / 2);
        }

        TRACE_FUNC_END;
}

void draw_blast_at_cells(const std::vector<P>& positions, const Color& color)
{
        TRACE_FUNC_BEGIN;

        if (!is_inited() || !panels::is_valid())
        {
                TRACE_FUNC_END;

                return;
        }

        states::draw();

        for (const P pos : positions)
        {
                if (!viewport::is_in_ivew(pos))
                {
                        continue;
                }

                draw_symbol(
                        TileId::blast1,
                        '*',
                        Panel::map,
                        viewport::to_view_pos(pos),
                        color);
        }

        update_screen();

        sdl_base::sleep(config::delay_explosion() / 2);

        for (const P pos : positions)
        {
                if (!viewport::is_in_ivew(pos))
                {
                        continue;
                }

                draw_symbol(
                        TileId::blast2,
                        '*',
                        Panel::map,
                        viewport::to_view_pos(pos),
                        color);
        }

        update_screen();

        sdl_base::sleep(config::delay_explosion() / 2);

        TRACE_FUNC_END;
}

void draw_blast_at_seen_cells(const std::vector<P>& positions,
                              const Color& color)
{
        if (is_inited() && panels::is_valid())
        {
                std::vector<P> positions_with_vision;

                for (const P p : positions)
                {
                        if (map::cells[p.x][p.y].is_seen_by_player)
                        {
                                positions_with_vision.push_back(p);
                        }
                }

                if (!positions_with_vision.empty())
                {
                        io::draw_blast_at_cells(positions_with_vision, color);
                }
        }
}

void draw_blast_at_seen_actors(const std::vector<Actor*>& actors,
                               const Color& color)
{
        if (is_inited() && panels::is_valid())
        {
                std::vector<P> positions;

                for (Actor* const actor : actors)
                {
                        positions.push_back(actor->pos);
                }

                draw_blast_at_seen_cells(positions, color);
        }
}

void draw_symbol(
        const TileId tile,
        const char character,
        const Panel panel,
        const P pos,
        const Color& color,
        const bool draw_bg,
        const Color& color_bg)
{
        if (config::is_tiles_mode())
        {
                draw_tile(
                        tile,
                        panel,
                        pos,
                        color,
                        draw_bg,
                        color_bg);
        }
        else
        {
                draw_character(
                        character,
                        panel,
                        pos,
                        color,
                        draw_bg,
                        color_bg);
        }
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

        bool is_window_resized = false;

        while (!is_done)
        {
                sdl_base::sleep(1);

                const bool did_poll_event = SDL_PollEvent(&sdl_event_);

                if (!did_poll_event)
                {
                        continue;
                }

                const SDL_Keymod mod = SDL_GetModState();

                const bool is_shift_held = mod & KMOD_SHIFT;
                const bool is_ctrl_held = mod & KMOD_CTRL;
                const bool is_alt_held = mod & KMOD_ALT;

                switch (sdl_event_.type)
                {
                case SDL_WINDOWEVENT:
                {
                        switch (sdl_event_.window.event)
                        {
                        // case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                                TRACE << "Window resized" << std::endl;

                                if (!config::is_fullscreen())
                                {
                                        on_window_resized();

                                        is_window_resized = true;
                                }
                        }
                        break;

                        case SDL_WINDOWEVENT_RESTORED:
                        {
                                TRACE << "Window restored" << std::endl;
                        }
                        // Fallthrough
                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                        {
                                TRACE << "Window gained focus" << std::endl;

                                clear_events();

                                sdl_base::sleep(100);
                        }
                        // Fallthrough
                        case SDL_WINDOWEVENT_EXPOSED:
                        {
                                TRACE << "Window exposed" << std::endl;

                                states::draw();

                                io::update_screen();
                        }
                        break;

                        default:
                        {
                        }
                        break;
                        } // window event switch
                }
                break; // case SDL_WINDOWEVENT

                case SDL_QUIT:
                {
                        ret = InputData(SDLK_ESCAPE);

                        is_done = true;
                }
                break;

                case SDL_KEYDOWN:
                {
                        const int key = sdl_event_.key.keysym.sym;

                        // Do not return shift/control/alt as separate events
                        if (key == SDLK_LSHIFT ||
                            key == SDLK_RSHIFT ||
                            key == SDLK_LCTRL ||
                            key == SDLK_RCTRL ||
                            key == SDLK_LALT ||
                            key == SDLK_RALT)
                        {
                                continue;
                        }

                        ret = InputData(
                                key,
                                is_shift_held,
                                is_ctrl_held);

                        if ((key >= SDLK_F1) && (key <= SDLK_F9))
                        {
                                // F keys
                                is_done = true;

                                continue;
                        }

                        // Not an F key

                        switch (key)
                        {
                        case SDLK_RETURN:
                        case SDLK_RETURN2:
                        case SDLK_KP_ENTER:
                        {
                                if (is_alt_held)
                                {
                                        TRACE << "Alt-Enter pressed"
                                              << std::endl;

                                        config::set_fullscreen(
                                                !config::is_fullscreen());

                                        on_fullscreen_toggled();

                                        // SDL_SetWindowSize(sdl_window_,

                                        // TODO: For some reason, the alt key
                                        // gets "stuck" after toggling
                                        // fullscreen, and must be cleared here
                                        // manually. Don't know if this is an
                                        // issue in the IA code, or an SDL bug.
                                        SDL_SetModState(KMOD_NONE);

                                        clear_events();

                                        flush_input();

                                        continue;
                                }
                                else // Alt is not held
                                {
                                        ret.key = SDLK_RETURN;

                                        is_done = true;
                                }
                        }
                        break; // case SDLK_KP_ENTER

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
                break; // case SDL_KEYDOWN

                case SDL_TEXTINPUT:
                {
                        const char c = sdl_event_.text.text[0];

                        if (c == '+' || c == '-')
                        {
                                if (config::is_fullscreen() ||
                                    is_window_maximized())
                                {
                                        continue;
                                }

                                P gui_dims = get_window_gui_dims();

                                if (c == '+')
                                {
                                        if (is_ctrl_held)
                                        {
                                                ++gui_dims.y;
                                        }
                                        else
                                        {
                                                ++gui_dims.x;
                                        }
                                }
                                else if (c == '-')
                                {
                                        if (is_ctrl_held)
                                        {
                                                --gui_dims.y;
                                        }
                                        else
                                        {
                                                --gui_dims.x;
                                        }
                                }

                                try_set_window_gui_cells(gui_dims);

                                is_window_resized = true;

                                continue;
                        }

                        if ((c == 'o' || c == 'O') && is_o_return)
                        {
                                const bool is_shift_held = c == 'O';

                                ret = InputData(
                                        SDLK_RETURN,
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
                break; // case SDL_TEXT_INPUT

                default:
                {
                }
                break;

                } // event switch

        } // while

        SDL_StopTextInput();

        // Adjust window size to nearest gui cells?
        if (!config::is_fullscreen() && is_window_resized)
        {
                resize_window_to_nearest_gui_cells();

                on_window_resized();

                clear_events();
        }

        return ret;
}

} // io
