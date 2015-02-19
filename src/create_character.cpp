#include "create_character.hpp"

#include "render.hpp"
#include "actor_player.hpp"
#include "input.hpp"
#include "menu_input_handling.hpp"
#include "text_format.hpp"
#include "utils.hpp"
#include "map.hpp"

using namespace std;

namespace Create_character
{

namespace
{

namespace Enter_name
{

void draw(const string& cur_string)
{
    Render::clear_screen();
    Render::draw_popup_box(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

    Render::draw_text_centered("What is your name?", Panel::screen,
                             Pos(MAP_W_HALF, 0), clr_white);
    const int Y_NAME = 2;
    const string NAME_STR =
        cur_string.size() < PLAYER_NAME_MAX_LEN ? cur_string + "_" :
        cur_string;
    const size_t NAME_X0 = MAP_W_HALF - (PLAYER_NAME_MAX_LEN / 2);
    const size_t NAME_X1 = NAME_X0 + PLAYER_NAME_MAX_LEN - 1;
    Render::draw_text(NAME_STR, Panel::screen, Pos(NAME_X0, Y_NAME),
                     clr_menu_highlight);
    Rect box_rect(Pos(NAME_X0 - 1, Y_NAME - 1), Pos(NAME_X1 + 1, Y_NAME + 1));
    Render::draw_popup_box(box_rect);
    Render::update_screen();
}

void read_keys(string& cur_string, bool& is_done)
{
    const Key_data& d = Input::get_input(false);

    if (d.sdl_key == SDLK_RETURN)
    {
        is_done = true;
        cur_string = cur_string.empty() ? "Player" : cur_string;
        return;
    }

    if (cur_string.size() < PLAYER_NAME_MAX_LEN)
    {
        if (
            d.sdl_key == SDLK_SPACE ||
            (d.key >= int('a') && d.key <= int('z')) ||
            (d.key >= int('A') && d.key <= int('Z')) ||
            (d.key >= int('0') && d.key <= int('9')))
        {
            if (d.sdl_key == SDLK_SPACE)
            {
                cur_string.push_back(' ');
            }
            else
            {
                cur_string.push_back(char(d.key));
            }
            draw(cur_string);
            return;
        }
    }

    if (cur_string.size() > 0)
    {
        if (d.sdl_key == SDLK_BACKSPACE)
        {
            cur_string.erase(cur_string.end() - 1);
            draw(cur_string);
        }
    }
}

void run()
{
    string name = "";
    draw(name);
    bool is_done = false;
    while (!is_done)
    {
        if (Config::is_bot_playing())
        {
            name = "AZATHOTH";
            is_done = true;
        }
        else
        {
            read_keys(name, is_done);
        }
    }
    Actor_data_t& def = Map::player->get_data();
    def.name_a      = def.name_the = name;
}

} //Enter_name

void draw_pick_bg(const vector<Bg>& bgs, const Menu_browser& browser)
{
    Render::clear_screen();
    Render::draw_popup_box(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

    Render::draw_text_centered("Choose your background", Panel::screen,
                             Pos(MAP_W_HALF, 0), clr_white, clr_black, true);

    const Pos& browser_pos = browser.get_pos();

    const int Y0_BGS = 2;

    int y = Y0_BGS;

    const Bg marked_bg = bgs[browser_pos.y];

    const int NR_BGS = bgs.size();

    //------------------------------------------------------------- BACKGROUNDS
    for (int i = 0; i < NR_BGS; ++i)
    {
        const Bg bg = bgs[i];
        string name = "";
        Player_bon::get_bg_title(bg, name);
        const bool IS_MARKED  = bg == marked_bg;
        const Clr& drw_clr     = IS_MARKED ? clr_menu_highlight : clr_menu_drk;
        Render::draw_text_centered(name, Panel::screen, Pos(MAP_W_HALF, y), drw_clr);
        y++;
    }
    y++;

    const int BGS_BOX_W_HALF = 7;
    Rect box_rect(Pos(MAP_W_HALF - BGS_BOX_W_HALF, Y0_BGS - 1),
                 Pos(MAP_W_HALF + BGS_BOX_W_HALF, Y0_BGS + NR_BGS));
    Render::draw_popup_box(box_rect);

    //------------------------------------------------------------- DESCRIPTION
    const int MARGIN_W_DESCR  = 12;
    const int X0_DESCR        = MARGIN_W_DESCR;
    const int MAX_W_DESCR     = MAP_W - (MARGIN_W_DESCR * 2);

    vector<string> raw_descr_lines;
    Player_bon::get_bg_descr(marked_bg, raw_descr_lines);
    for (string& raw_line : raw_descr_lines)
    {
        vector<string> formatted_lines;
        Text_format::line_to_lines(raw_line, MAX_W_DESCR, formatted_lines);
        for (string& line : formatted_lines)
        {
            Render::draw_text(line, Panel::screen, Pos(X0_DESCR, y), clr_white);
            y++;
        }
    }
    Render::update_screen();
}

void pick_bg()
{
    if (Config::is_bot_playing())
    {
        Player_bon::pick_bg(Bg(Rnd::range(0, int(Bg::END) - 1)));
    }
    else
    {
        vector<Bg> bgs;
        Player_bon::get_pickable_bgs(bgs);

        Menu_browser browser(bgs.size(), 0);
        draw_pick_bg(bgs, browser);

        while (true)
        {
            const Menu_action action = Menu_input_handling::get_action(browser);
            switch (action)
            {
            case Menu_action::browsed: {draw_pick_bg(bgs, browser);} break;

            case Menu_action::esc:
            case Menu_action::space: {} break;

            case Menu_action::selected:
            {
                Player_bon::pick_bg(bgs[browser.get_pos().y]);
                return;
            } break;

            case Menu_action::selected_shift: {} break;
            }
        }
    }
}

void draw_pick_trait(
    const vector<Trait>& traits1, const vector<Trait>& traits2,
    const Menu_browser& browser, const bool IS_CHARACTER_CREATION)
{

    Render::clear_screen();
    Render::draw_popup_box(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

    const int NR_TRAITS_1 = traits1.size();
    const int NR_TRAITS_2 = traits2.size();

    int len_of_longest_in_col2 = -1;
    for (const Trait& id : traits2)
    {
        string title = "";
        Player_bon::get_trait_title(id, title);
        const int CUR_LEN = title.length();
        if (CUR_LEN > len_of_longest_in_col2) {len_of_longest_in_col2 = CUR_LEN;}
    }

    const int MARGIN_W        = 19;
    const int X_COL_ONE       = MARGIN_W;
    const int X_COL_TWO_RIGHT = MAP_W - MARGIN_W - 1;
    const int X_COL_TWO       = X_COL_TWO_RIGHT - len_of_longest_in_col2 + 1;

    string title = IS_CHARACTER_CREATION ?
                   "Which additional trait do you start with?" :
                   "You have reached a new level! Which trait do you gain?";

    Render::draw_text_centered(title, Panel::screen, Pos(MAP_W_HALF, 0),
                             clr_white, clr_black, true);

    const Pos& browser_pos = browser.get_pos();

    //------------------------------------------------------------- TRAITS
    const int Y0_TRAITS = 2;
    int y = Y0_TRAITS;
    for (int i = 0; i < NR_TRAITS_1; ++i)
    {
        const Trait trait = traits1[i];
        string name = "";
        Player_bon::get_trait_title(trait, name);
        const bool IS_MARKED  = browser_pos.x == 0 && browser_pos.y == int(i);
        const Clr& drw_clr     = IS_MARKED ? clr_menu_highlight : clr_menu_drk;
        Render::draw_text(name, Panel::screen, Pos(X_COL_ONE, y), drw_clr);
        y++;
    }
    y = Y0_TRAITS;
    for (int i = 0; i < NR_TRAITS_2; ++i)
    {
        const Trait trait = traits2[i];
        string name = "";
        Player_bon::get_trait_title(trait, name);
        const bool IS_MARKED  = browser_pos.x == 1 && browser_pos.y == int(i);
        const Clr& drw_clr     = IS_MARKED ? clr_menu_highlight : clr_menu_drk;
        Render::draw_text(name, Panel::screen, Pos(X_COL_TWO, y), drw_clr);
        y++;
    }

    //Draw frame around traits
    Rect box_rect(
        Pos(MARGIN_W - 2, Y0_TRAITS - 1),
        Pos(X_COL_TWO_RIGHT + 2, Y0_TRAITS + traits1.size()));
    Render::draw_popup_box(box_rect);

    //------------------------------------------------------------- DESCRIPTION
    const int Y0_DESCR = Y0_TRAITS + NR_TRAITS_1 + 1;
    const int X0_DESCR = X_COL_ONE;
    y = Y0_DESCR;
    const Trait marked_trait =
        browser_pos.x == 0 ? traits1[browser_pos.y] : traits2[browser_pos.y];
    string descr = "";
    Player_bon::get_trait_descr(marked_trait, descr);
    const int MAX_W_DESCR = X_COL_TWO_RIGHT - X_COL_ONE + 1;
    vector<string> descr_lines;
    Text_format::line_to_lines(
        "Effect(s): " + descr, MAX_W_DESCR, descr_lines);
    for (const string& str : descr_lines)
    {
        Render::draw_text(str, Panel::screen, Pos(X0_DESCR, y), clr_white);
        y++;
    }

    //------------------------------------------------------------- PREREQUISITES
    const int Y0_PREREQS = 17;
    y = Y0_PREREQS;
    vector<Trait> trait_prereqs;
    Bg bg_prereq = Bg::END;
    Player_bon::get_trait_prereqs(marked_trait, trait_prereqs, bg_prereq);
    if (!trait_prereqs.empty() || bg_prereq != Bg::END)
    {
        Render::draw_text("This trait had the following prerequisite(s):",
                         Panel::screen, Pos(X0_DESCR, y), clr_white);
        y++;

        string prereq_str = "";

        if (bg_prereq != Bg::END)
        {
            Player_bon::get_bg_title(bg_prereq, prereq_str);
        }

        for (Trait prereq_trait : trait_prereqs)
        {
            string prereq_title = "";
            Player_bon::get_trait_title(prereq_trait, prereq_title);
            prereq_str += (prereq_str.empty() ? "" : ", ") + prereq_title;
        }

        vector<string> prereq_lines;
        Text_format::line_to_lines(prereq_str, MAX_W_DESCR, prereq_lines);
        for (const string& str : prereq_lines)
        {
            Render::draw_text(str, Panel::screen, Pos(X0_DESCR, y), clr_white);
            y++;
        }
    }

    //------------------------------------------------------------- PREVIOUS
    y = Y0_PREREQS + 4;
    const int MAX_W_PREV_PICKS  = SCREEN_W - 2;
    string picked_str = "";
    Player_bon::get_all_picked_traits_titles_line(picked_str);
    if (picked_str != "")
    {
        picked_str = "Trait(s) gained: " + picked_str;
        vector<string> picked_lines;
        Text_format::line_to_lines(picked_str, MAX_W_PREV_PICKS, picked_lines);
        for (const string& str : picked_lines)
        {
            Render::draw_text(
                str, Panel::screen, Pos(1, y), clr_white);
            y++;
        }
    }

    Render::update_screen();
}

} //namespace

void create_character()
{
    pick_bg();
    pick_new_trait(true);
    Enter_name::run();
}

void pick_new_trait(const bool IS_CHARACTER_CREATION)
{
    if (!Config::is_bot_playing())
    {
        vector<Trait> pickable_traits;
        Player_bon::get_pickable_traits(pickable_traits);

        if (!pickable_traits.empty())
        {

            const int NR_TRAITS_TOT = int(pickable_traits.size());
            const int NR_TRAITS_2   = NR_TRAITS_TOT / 2;
            const int NR_TRAITS_1   = NR_TRAITS_TOT - NR_TRAITS_2;

            vector<Trait> traits1; traits1.clear();
            vector<Trait> traits2; traits2.clear();

            for (int i = 0; i < NR_TRAITS_TOT; ++i)
            {
                const Trait trait = pickable_traits[i];
                if (i < NR_TRAITS_1)
                {
                    traits1.push_back(trait);
                }
                else
                {
                    traits2.push_back(trait);
                }
            }

            Menu_browser browser(traits1.size(), traits2.size());
            draw_pick_trait(traits1, traits2, browser, IS_CHARACTER_CREATION);

            while (true)
            {
                const Menu_action action = Menu_input_handling::get_action(browser);
                switch (action)
                {
                case Menu_action::browsed:
                {
                    draw_pick_trait(traits1, traits2, browser, IS_CHARACTER_CREATION);
                } break;

                case Menu_action::esc:
                case Menu_action::space: {} break;

                case Menu_action::selected:
                {
                    const Pos pos = browser.get_pos();
                    Player_bon::pick_trait(pos.x == 0 ? traits1[pos.y] : traits2[pos.y]);
                    if (!IS_CHARACTER_CREATION) {Render::draw_map_and_interface();}
                    return;
                } break;

                case Menu_action::selected_shift: {} break;
                }
            }
        }
    }
}

} //Create_character
