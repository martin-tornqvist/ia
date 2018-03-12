#include "panel.hpp"

#include "io.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static R panels_[(size_t)Panel::END];

void set_panel(const Panel panel,
               const int x0,
               const int y0,
               const int x1,
               const int y1)
{
        panels_[(size_t)panel] = {x0, y0, x1, y1};
}

// -----------------------------------------------------------------------------
// panels
// -----------------------------------------------------------------------------
namespace panels
{

void init()
{
        set_panel(Panel::log,
                  0,
                  0,
                  map_w - 1,
                  1);

        set_panel(Panel::map,
                  0,
                  get_y1(Panel::log) + 1,
                  map_w - 1,
                  get_y1(Panel::log) + map_h);

        set_panel(Panel::status_lines,
                  0,
                  get_y1(Panel::map) + 1,
                  map_w - 1,
                  get_y1(Panel::map) + 3);

        set_panel(Panel::screen,
                  0,
                  0,
                  map_w - 1,
                  get_y1(Panel::status_lines));

        set_panel(Panel::create_char_menu,
                  0,
                  1,
                  23,
                  get_y1(Panel::screen));

        set_panel(Panel::create_char_descr,
                  get_x1(Panel::create_char_menu) + 2,
                  1,
                  get_x1(Panel::screen),
                  get_y1(Panel::screen));

        set_panel(Panel::item_menu,
                  0,
                  1,
                  47,
                  get_y1(Panel::screen) - 1);

        set_panel(Panel::item_descr,
                  get_x1(Panel::item_menu) + 2,
                  1,
                  get_x1(Panel::screen),
                  get_y1(Panel::screen));
}

R get_area(const Panel panel)
{
        return panels_[(size_t)panel];
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
