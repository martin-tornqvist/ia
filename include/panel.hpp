#ifndef PANEL_HPP
#define PANEL_HPP

#include "rl_utils.hpp"

enum class Panel
{
        screen,
        map,
        status_lines,
        log,
        create_char_menu,
        create_char_descr,
        item_menu,
        item_descr,
        END
};

namespace panels
{

void init();

R get_area(const Panel panel);

P get_p0(const Panel panel);

P get_p1(const Panel panel);

int get_x0(const Panel panel);

int get_y0(const Panel panel);

int get_x1(const Panel panel);

int get_y1(const Panel panel);

int get_w(const Panel panel);

int get_h(const Panel panel);

int get_center_x(const Panel panel);

int get_center_y(const Panel panel);

} // panels

#endif // PANEL_HPP
