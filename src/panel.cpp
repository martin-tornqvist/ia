#include "panel.hpp"

#include "io.hpp"
#include "rl_utils.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static R panels_[(size_t)Panel::END];

static bool is_valid_;

static void set_panel_area(
        const Panel panel,
        const int x0,
        const int y0,
        const int x1,
        const int y1)
{
        panels_[(size_t)panel] = {x0, y0, x1, y1};
}

static void set_x0(const Panel panel, int x)
{
        R& r =  panels_[(size_t)panel];

        const int w = r.w();

        r.p0.x = x;
        r.p1.x = x + w - 1;
}

static void set_y0(const Panel panel, int y)
{
        R& r =  panels_[(size_t)panel];

        const int h = r.h();

        r.p0.y = y;
        r.p1.y = y + h - 1;
}

static void set_w(const Panel panel, int w)
{
        R& r =  panels_[(size_t)panel];

        r.p1.x = r.p0.x + w - 1;
}

static void finalize_screen_dims()
{
        R& screen = panels_[(size_t)Panel::screen];

        for (const R& panel : panels_)
        {
                screen.p1.x = std::max(screen.p1.x, panel.p1.x);
                screen.p1.y = std::max(screen.p1.y, panel.p1.y);
        }

        TRACE << "Screen GUI size was set to: "
              << panels::get_w(Panel::screen)
              << ", "
              << panels::get_h(Panel::screen)
              << std::endl;
}

static void validate_panels(const P max_gui_dims)
{
        TRACE_FUNC_BEGIN;

        is_valid_ = true;

        const R& screen = panels_[(size_t)Panel::screen];

        for (const R& panel : panels_)
        {
                if ((panel.p1.x >= max_gui_dims.x) ||
                    (panel.p1.y >= max_gui_dims.y) ||
                    (panel.p1.x > screen.p1.x) ||
                    (panel.p1.y > screen.p1.y) ||
                    (panel.p0.x > panel.p1.x) ||
                    (panel.p0.y > panel.p1.y))
                {
                        TRACE << "Window too small for panel requiring size: "
                              << panel.p1.x
                              << ", "
                              << panel.p1.y
                              << std::endl
                              << "With position: "
                              << panel.p0.x
                              << ", "
                              << panel.p0.y
                              << std::endl;

                        is_valid_ = false;

                        break;
                }
        }

        // In addition to requirements from individual panels, we also put some
        // requirements on the screen (window) size itself - smaller screen than
        // this is not reasonable to go on with...
        const P min_gui_dims = io::min_screen_gui_dims();

        if ((screen.p1.x + 1) < min_gui_dims.x ||
            (screen.p1.y + 1) < min_gui_dims.y)
        {
                TRACE << "Window too small for static minimum screen limit: "
                      << min_gui_dims.x
                      << ", "
                      << min_gui_dims.y
                      << std::endl;

                is_valid_ = false;
        }

#ifndef NDEBUG
        if (is_valid_)
        {
                TRACE << "Panels OK" << std::endl;
        }
        else
        {
                TRACE << "Panels NOT OK" << std::endl;
        }
#endif // NDDEBUG

        TRACE_FUNC_END;
}

// -----------------------------------------------------------------------------
// panels
// -----------------------------------------------------------------------------
namespace panels
{

void init(const P max_gui_dims)
{
        TRACE_FUNC_BEGIN;

        TRACE << "Maximum allowed GUI size: "
              << max_gui_dims.x
              << ", "
              << max_gui_dims.y
              << std::endl;

        for (auto& panel : panels_)
        {
                panel = R(0, 0, 0, 0);
        }

        // (Finalized later)
        set_panel_area(Panel::player_stats, 0, 0, 16, max_gui_dims.y - 1);

        // (Finalized later)
        set_panel_area(Panel::log_border, 0, 0, 0, 3);

        const int map_panel_w = max_gui_dims.x - get_w(Panel::player_stats);

        const int map_panel_h = max_gui_dims.y - get_h(Panel::log_border);

        set_panel_area(
                Panel::map,
                0,
                0,
                map_panel_w - 1,
                map_panel_h - 1);

        set_x0(Panel::player_stats, get_x1(Panel::map) + 1);

        set_w(Panel::log_border, get_w(Panel::map));

        set_y0(Panel::log_border, get_y1(Panel::map) + 1);

        set_panel_area(
                Panel::log,
                get_x0(Panel::log_border) + 1,
                get_y0(Panel::log_border) + 1,
                get_x1(Panel::log_border) - 1,
                get_y1(Panel::log_border) - 1);

        finalize_screen_dims();

        set_panel_area(
                Panel::create_char_menu,
                1,
                2,
                25,
                get_y1(Panel::screen));

        set_panel_area(
                Panel::create_char_descr,
                get_x1(Panel::create_char_menu) + 2,
                2,
                get_x1(Panel::screen) - 1,
                get_y1(Panel::screen));

        set_panel_area(
                Panel::item_menu,
                1,
                1,
                44,
                get_y1(Panel::screen) - 1);

        set_panel_area(
                Panel::item_descr,
                get_x1(Panel::item_menu) + 2,
                1,
                get_x1(Panel::screen),
                get_y1(Panel::screen) - 1);

        validate_panels(max_gui_dims);

        TRACE_FUNC_END;
}

bool is_valid()
{
        return is_valid_;
}

R get_area(const Panel panel)
{
        return panels_[(size_t)panel];
}

P get_dims(const Panel panel)
{
        return get_area(panel).dims();
}

P get_p0(const Panel panel)
{
        return get_area(panel).p0;
}

P get_p1(const Panel panel)
{
        return get_area(panel).p1;
}

int get_x0(const Panel panel)
{
        return get_area(panel).p0.x;
}

int get_y0(const Panel panel)
{
        return get_area(panel).p0.y;
}

int get_x1(const Panel panel)
{
        return get_area(panel).p1.x;
}

int get_y1(const Panel panel)
{
        return get_area(panel).p1.y;
}

int get_w(const Panel panel)
{
        return get_area(panel).w();
}

int get_h(const Panel panel)
{
        return get_area(panel).h();
}

P get_center(const Panel panel)
{
        const P center(
                get_center_x(panel),
                get_center_y(panel));

        return center;
}

int get_center_x(const Panel panel)
{
        const int x0 = get_x0(panel);
        const int x1 = get_x1(panel);

        return (x1 - x0) / 2;
}

int get_center_y(const Panel panel)
{
        const int y0 = get_y0(panel);
        const int y1 = get_y1(panel);

        return (y1 - y0) / 2;
}

} // panels
