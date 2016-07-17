#include "popup.hpp"

#include "config.hpp"
#include "render.hpp"
#include "text_format.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "menu_input.hpp"
#include "audio.hpp"

namespace popup
{

namespace
{

const int text_w_std    = 39;
const int TEXT_X0_STD   = map_w_half - ((text_w_std) / 2);

int print_box_and_get_title_y_pos(const int text_h_tot, const int text_w)
{
    const int box_w       = text_w + 2;
    const int box_h       = text_h_tot + 2;

    const int X0          = map_w_half - ((text_w) / 2) - 1;

    const int Y0          = map_h_half - (box_h / 2) - 1;
    const int X1          = X0 + box_w - 1;
    const int Y1          = Y0 + box_h - 1;

    render::cover_area(Panel::map, P(X0, Y0), P(box_w, box_h));
    render::draw_box(R(X0, Y0, X1, Y1), Panel::map);

    return Y0 + 1;
}

void menu_msg_drawing_helper(const std::vector<std::string>& lines,
                             const std::vector<std::string>& choices,
                             const bool draw_map_state,
                             const size_t cur_choice,
                             const int TEXT_X0,
                             const int text_h_tot,
                             const std::string& title)
{
    if (draw_map_state)
    {
        render::draw_map_state(Update_screen::no);
    }

    int text_width = text_w_std;

    //If no message lines, set width to widest menu option or title with
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
        render::draw_text_center(title,
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
            render::draw_text_center(line,
                                       Panel::map,
                                       P(map_w_half, y),
                                       clr_white,
                                       clr_black,
                                       true);
        }
        else //Draw the message with left alignment
        {
            render::draw_text(line,
                              Panel::map,
                              P(TEXT_X0, y),
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
        Clr clr = i == cur_choice ? clr_menu_highlight : clr_menu_drk;

        render::draw_text_center(choices[i],
                                   Panel::map,
                                   P(map_w_half, y),
                                   clr,
                                   clr_black,
                                   true);
        ++y;
    }

    render::update_screen();
}

} //namespace

void show_msg(const std::string& msg,
              const bool draw_map_state,
              const std::string& title,
              const Sfx_id sfx,
              const int w_change)
{
    if (draw_map_state)
    {
        render::draw_map_state(Update_screen::no);
    }

    const int text_w = text_w_std + w_change;

    std::vector<std::string> lines;
    text_format::split(msg, text_w, lines);

    const int text_h_tot =  int(lines.size()) + 3;

    int y = print_box_and_get_title_y_pos(text_h_tot, text_w);

    if (sfx != Sfx_id::END)
    {
        audio::play(sfx);
    }

    if (!title.empty())
    {
        render::draw_text_center(title, Panel::map,
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
            render::draw_text_center(line,
                                       Panel::map,
                                       P(map_w_half, y),
                                       clr_white,
                                       clr_black,
                                       true);
        }
        else
        {
            const int TEXT_X0 = TEXT_X0_STD - ((w_change + 1) / 2);

            render::draw_text(line,
                              Panel::map,
                              P(TEXT_X0, y),
                              clr_white);
        }

        msg_log::add_line_to_history(line);
    }

    y += 2;

    render::draw_text_center("[space/esc/enter] to continue",
                               Panel::map,
                               P(map_w_half, y),
                               clr_popup_label);

    render::update_screen();

    query::wait_for_confirm();

    if (draw_map_state)
    {
        render::draw_map_state();
    }
}

int show_menu_msg(const std::string& msg,
                  const bool draw_map_state,
                  const std::vector<std::string>& choices,
                  const std::string& title,
                  const Sfx_id sfx)
{
    if (config::is_bot_playing())
    {
        return 0;
    }

    std::vector<std::string> lines;
    text_format::split(msg, text_w_std, lines);

    const int title_h         = title.empty() ? 0 : 1;
    const int nr_msg_lines    = int(lines.size());
    const int nr_blank_lines  = (nr_msg_lines == 0 && title_h == 0) ? 0 : 1;
    const int nr_choices      = int(choices.size());

    const int text_h_tot = title_h + nr_msg_lines + nr_blank_lines + nr_choices;

    Menu_browser browser(nr_choices);

    if (sfx != Sfx_id::END) {audio::play(sfx);}

    menu_msg_drawing_helper(lines,
                            choices,
                            draw_map_state,
                            browser.y(),
                            TEXT_X0_STD,
                            text_h_tot,
                            title);

    while (true)
    {
        const Menu_action action = menu_input::action(browser, Menu_input_mode::scroll);

        switch (action)
        {
        case Menu_action::moved:
            menu_msg_drawing_helper(lines,
                                    choices,
                                    draw_map_state,
                                    browser.y(),
                                    TEXT_X0_STD,
                                    text_h_tot,
                                    title);
            break;

        case Menu_action::esc:
        case Menu_action::space:
            if (draw_map_state)
            {
                render::draw_map_state();
            }

            return nr_choices - 1;

        case Menu_action::selected:
        case Menu_action::selected_shift:
            if (draw_map_state)
            {
                render::draw_map_state();
            }

            return browser.y();
        }
    }
}

} //Popup
