#include "popup.hpp"

#include "config.hpp"
#include "io.hpp"
#include "text_format.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "browser.hpp"
#include "audio.hpp"

namespace popup
{

namespace
{

const int text_w_std = 39;
const int text_x0_std = map_w_half - ((text_w_std) / 2);

int print_box_and_get_title_y_pos(const int text_h_tot,
                                  const int text_w)
{
    const int box_w = text_w + 2;
    const int box_h = text_h_tot + 2;

    const int x0 = map_w_half - ((text_w) / 2) - 1;

    const int y0 = map_h_half - (box_h / 2) - 1;
    const int x1 = x0 + box_w - 1;
    const int y1 = y0 + box_h - 1;

    io::cover_area(Panel::map,
                       P(x0, y0),
                       P(box_w, box_h));

    io::draw_box(R(x0, y0, x1, y1),
                     Panel::map);

    return y0 + 1;
}

void menu_msg_drawing_helper(const std::vector<std::string>& lines,
                             const std::vector<std::string>& choices,
                             const size_t current_choice,
                             const int text_x0,
                             const int text_h_tot,
                             const std::string& title)
{
    int text_width = text_w_std;

    // If no message lines, set width to widest menu option or title with
    if (lines.empty())
    {
        text_width = title.size();

        for (const std::string& s : choices)
        {
            text_width = std::max(text_width, int(s.size()));
        }

        text_width += 2;
    }

    int y = print_box_and_get_title_y_pos(text_h_tot, text_width);

    if (!title.empty())
    {
        io::draw_text_center(title,
                                   Panel::map,
                                   P(map_w_half, y),
                                   clr_title,
                                   clr_black,
                                   true);
    }

    const bool show_msg_centered = lines.size() == 1;

    for (const std::string& line : lines)
    {
        y++;

        if (show_msg_centered)
        {
            io::draw_text_center(line,
                                     Panel::map,
                                     P(map_w_half, y),
                                     clr_white,
                                     clr_black,
                                     true);
        }
        else // Draw the message with left alignment
        {
            io::draw_text(line,
                              Panel::map,
                              P(text_x0, y),
                              clr_white);
        }

        msg_log::add_line_to_history(line);
    }

    if (!lines.empty() || !title.empty())
    {
        y += 2;
    }

    for (size_t i = 0; i < choices.size(); ++i)
    {
        Clr clr = i == current_choice ? clr_menu_highlight : clr_menu_drk;

        io::draw_text_center(choices[i],
                                   Panel::map,
                                   P(map_w_half, y),
                                   clr,
                                   clr_black,
                                   true);
        ++y;
    }

    io::update_screen();
}

} // namespace

void show_msg(const std::string& msg,
              const std::string& title,
              const SfxId sfx,
              const int w_change)
{
    const int text_w = text_w_std + w_change;

    std::vector<std::string> lines;
    text_format::split(msg, text_w, lines);

    const int text_h_tot =  int(lines.size()) + 3;

    int y = print_box_and_get_title_y_pos(text_h_tot, text_w);

    if (sfx != SfxId::END)
    {
        audio::play(sfx);
    }

    if (!title.empty())
    {
        io::draw_text_center(title, Panel::map,
                                   P(map_w_half, y),
                                   clr_title,
                                   clr_black,
                                   true);
    }

    const bool show_msg_centered = lines.size() == 1;

    for (std::string& line : lines)
    {
        y++;

        if (show_msg_centered)
        {
            io::draw_text_center(line,
                                       Panel::map,
                                       P(map_w_half, y),
                                       clr_white,
                                       clr_black,
                                       true);
        }
        else
        {
            const int text_x0 = text_x0_std - ((w_change + 1) / 2);

            io::draw_text(line,
                              Panel::map,
                              P(text_x0, y),
                              clr_white);
        }

        msg_log::add_line_to_history(line);
    }

    y += 2;

    io::draw_text_center(confirm_info_str_no_space,
                         Panel::map,
                         P(map_w_half, y),
                         clr_popup_label);

    io::update_screen();

    query::wait_for_confirm();
}

int show_menu_msg(const std::string& msg,
                  const std::vector<std::string>& choices,
                  const std::string& title,
                  const SfxId sfx)
{
    if (config::is_bot_playing())
    {
        return 0;
    }

    std::vector<std::string> lines;

    text_format::split(msg, text_w_std, lines);

    const int title_h = title.empty() ? 0 : 1;
    const int nr_msg_lines = int(lines.size());
    const int nr_blank_lines = (nr_msg_lines == 0 && title_h == 0) ? 0 : 1;
    const int nr_choices = int(choices.size());

    const int text_h_tot =
        title_h +
        nr_msg_lines +
        nr_blank_lines +
        nr_choices;

    MenuBrowser browser(nr_choices);

    if (sfx != SfxId::END)
    {
        audio::play(sfx);
    }

    menu_msg_drawing_helper(lines,
                            choices,
                            browser.y(),
                            text_x0_std,
                            text_h_tot,
                            title);

    while (true)
    {
        const auto input = io::get(true);

        const MenuAction action = browser.read(input,
                                               MenuInputMode::scrolling);

        switch (action)
        {
        case MenuAction::moved:
            menu_msg_drawing_helper(lines,
                                    choices,
                                    browser.y(),
                                    text_x0_std,
                                    text_h_tot,
                                    title);
            break;

        case MenuAction::esc:
        case MenuAction::space:
            return nr_choices - 1;

        case MenuAction::selected:
        case MenuAction::selected_shift:
            return browser.y();

        case MenuAction::none:
            break;
        }
    }
}

} // popup
