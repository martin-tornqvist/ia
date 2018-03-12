#include "info_screen_state.hpp"

#include "io.hpp"
#include "panel.hpp"

int InfoScreenState::max_nr_lines_on_screen() const
{
        return panels::get_h(Panel::screen) - 2;
}

void InfoScreenState::draw_interface() const
{
        const int screen_w = panels::get_w(Panel::screen);
        const int screen_h = panels::get_h(Panel::screen);

        if (config::is_tiles_mode())
        {
                for (int x = 0; x < screen_w; ++x)
                {
                        io::draw_tile(
                                TileId::popup_hor,
                                Panel::screen,
                                P(x, 0),
                                colors::title());

                        io::draw_tile(
                                TileId::popup_hor,
                                Panel::screen,
                                P(x, screen_h - 1),
                                colors::title());
                }
        }
        else // Text mode
        {
                const std::string decoration_line(map_w, '-');

                io::draw_text(
                        decoration_line,
                        Panel::screen,
                        P(0, 0),
                        colors::title());

                io::draw_text(
                        decoration_line,
                        Panel::screen,
                        P(0, screen_h - 1),
                        colors::title());
        }

        const int x_label = 3;

        io::draw_text(
                " " + title() + " ",
                Panel::screen,
                P(x_label, 0),
                colors::title());

        const std::string cmd_info =
                (type() == InfoScreenType::scrolling) ?
                info_screen_tip_scrollable :
                info_screen_tip;

        io::draw_text(
                " " + cmd_info + " ",
                Panel::screen,
                P(x_label, screen_h - 1),
                colors::title());
}
