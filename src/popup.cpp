#include "popup.hpp"

#include "config.hpp"
#include "render.hpp"
#include "text_format.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "cmn_types.hpp"
#include "menu_input.hpp"
#include "audio.hpp"

using namespace std;

namespace popup
{

namespace
{

const int TEXT_W_STD    = 39;
const int TEXT_X0_STD   = MAP_W_HALF - ((TEXT_W_STD) / 2);

int print_box_and_get_title_y_pos(const int TEXT_H_TOT, const int TEXT_W)
{
    const int BOX_W       = TEXT_W + 2;
    const int BOX_H       = TEXT_H_TOT + 2;

    const int X0          = MAP_W_HALF - ((TEXT_W) / 2) - 1;

    const int Y0          = MAP_H_HALF - (BOX_H / 2) - 1;
    const int X1          = X0 + BOX_W - 1;
    const int Y1          = Y0 + BOX_H - 1;

    render::cover_area(Panel::map, P(X0, Y0), P(BOX_W, BOX_H));
    render::draw_box(Rect(X0, Y0, X1, Y1), Panel::map);

    return Y0 + 1;
}

void menu_msg_drawing_helper(const vector<string>& lines,
                             const vector<string>& choices,
                             const bool            DRAW_MAP_AND_INTERFACE,
                             const size_t          cur_choice,
                             const int             TEXT_X0,
                             const int             TEXT_H_TOT,
                             const string&         title)
{
    if (DRAW_MAP_AND_INTERFACE)
    {
        render::draw_map_and_interface(false);
    }

    int text_width = TEXT_W_STD;

    //If no message lines, set width to widest menu option or title with
    if (lines.empty())
    {
        text_width = title.size();

        for (const string& s : choices)
        {
            text_width = max(text_width, int(s.size()));
        }

        text_width += 2;
    }

    int y = print_box_and_get_title_y_pos(TEXT_H_TOT, text_width);

    if (!title.empty())
    {
        render::draw_text_centered(title, Panel::map, P(MAP_W_HALF, y), clr_popup_title,
                                   clr_black, true);
    }

    const bool SHOW_MSG_CENTERED = lines.size() == 1;

    for (const string& line : lines)
    {
        y++;

        if (SHOW_MSG_CENTERED)
        {
            render::draw_text_centered(line, Panel::map, P(MAP_W_HALF, y), clr_white,
                                       clr_black, true);
        }
        else //Draw the message with left alignment
        {
            render::draw_text(line, Panel::map, P(TEXT_X0, y), clr_white);
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

        render::draw_text_centered(choices[i], Panel::map, P(MAP_W_HALF, y),
                                   clr, clr_black, true);
        y++;
    }

    render::update_screen();
}

} //namespace

void show_msg(const std::string& msg,
              const bool         DRAW_MAP_AND_INTERFACE,
              const std::string& title,
              const Sfx_id        sfx,
              const int          W_CHANGE)
{
    if (DRAW_MAP_AND_INTERFACE)
    {
        render::draw_map_and_interface(false);
    }

    const int TEXT_W = TEXT_W_STD + W_CHANGE;

    vector<string> lines;
    text_format::split(msg, TEXT_W, lines);

    const int TEXT_H_TOT =  int(lines.size()) + 3;

    int y = print_box_and_get_title_y_pos(TEXT_H_TOT, TEXT_W);

    if (sfx != Sfx_id::END)
    {
        audio::play(sfx);
    }

    if (!title.empty())
    {
        render::draw_text_centered(title, Panel::map, P(MAP_W_HALF, y), clr_popup_title,
                                   clr_black, true);
    }

    const bool SHOW_MSG_CENTERED = lines.size() == 1;

    for (string& line : lines)
    {
        y++;

        if (SHOW_MSG_CENTERED)
        {
            render::draw_text_centered(line, Panel::map, P(MAP_W_HALF, y),
                                       clr_white, clr_black, true);
        }
        else
        {
            const int TEXT_X0 = TEXT_X0_STD - ((W_CHANGE + 1) / 2);

            render::draw_text(line, Panel::map, P(TEXT_X0, y), clr_white);
        }

        msg_log::add_line_to_history(line);
    }

    y += 2;

    render::draw_text_centered("[space/esc/enter] to close", Panel::map, P(MAP_W_HALF, y),
                               clr_menu_medium);

    render::update_screen();

    query::wait_for_confirm();

    if (DRAW_MAP_AND_INTERFACE) {render::draw_map_and_interface();}
}

int show_menu_msg(const string&           msg,
                  const bool              DRAW_MAP_AND_INTERFACE,
                  const vector<string>&   choices,
                  const string&           title,
                  const Sfx_id             sfx)
{
    if (config::is_bot_playing())
    {
        return 0;
    }

    vector<string> lines;
    text_format::split(msg, TEXT_W_STD, lines);

    const int TITLE_H         = title.empty() ? 0 : 1;
    const int NR_MSG_LINES    = int(lines.size());
    const int NR_BLANK_LINES  = (NR_MSG_LINES == 0 && TITLE_H == 0) ? 0 : 1;
    const int NR_CHOICES      = int(choices.size());

    const int TEXT_H_TOT = TITLE_H + NR_MSG_LINES + NR_BLANK_LINES + NR_CHOICES;

    Menu_browser browser(NR_CHOICES);

    if (sfx != Sfx_id::END) {audio::play(sfx);}

    menu_msg_drawing_helper(lines, choices, DRAW_MAP_AND_INTERFACE, browser.y(),
                            TEXT_X0_STD, TEXT_H_TOT, title);

    while (true)
    {
        const Menu_action action = menu_input::action(browser, Menu_input_mode::scroll);

        switch (action)
        {
        case Menu_action::moved:
            menu_msg_drawing_helper(lines, choices, DRAW_MAP_AND_INTERFACE,
                                    browser.y(), TEXT_X0_STD, TEXT_H_TOT, title);
            break;

        case Menu_action::esc:
        case Menu_action::space:
            if (DRAW_MAP_AND_INTERFACE)
            {
                render::draw_map_and_interface();
            }

            return NR_CHOICES - 1;

        case Menu_action::selected:
        case Menu_action::selected_shift:
            if (DRAW_MAP_AND_INTERFACE)
            {
                render::draw_map_and_interface();
            }

            return browser.y();
        }
    }
}

} //Popup
