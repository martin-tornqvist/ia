#include "create_character.hpp"

#include "render.hpp"
#include "actor_player.hpp"
#include "input.hpp"
#include "menu_input_handling.hpp"
#include "text_format.hpp"
#include "utils.hpp"
#include "map.hpp"

using namespace std;

namespace create_character
{

namespace
{

namespace enter_name
{

void draw(const string& cur_string)
{
    render::clear_screen();
    render::draw_popup_box(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

    render::draw_text_centered("What is your name?", Panel::screen,
                               Pos(MAP_W_HALF, 0), clr_white);
    const int Y_NAME = 2;
    const string NAME_STR =
        cur_string.size() < PLAYER_NAME_MAX_LEN ? cur_string + "_" :
        cur_string;
    const size_t NAME_X0 = MAP_W_HALF - (PLAYER_NAME_MAX_LEN / 2);
    const size_t NAME_X1 = NAME_X0 + PLAYER_NAME_MAX_LEN - 1;
    render::draw_text(NAME_STR, Panel::screen, Pos(NAME_X0, Y_NAME),
                      clr_menu_highlight);
    Rect box_rect(Pos(NAME_X0 - 1, Y_NAME - 1), Pos(NAME_X1 + 1, Y_NAME + 1));
    render::draw_popup_box(box_rect);
    render::update_screen();
}

void read_keys(string& cur_string, bool& is_done)
{
    const Key_data& d = input::input(false);

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
        if (config::is_bot_playing())
        {
            name = "AZATHOTH";
            is_done = true;
        }
        else
        {
            read_keys(name, is_done);
        }
    }

    Actor_data_t& def = map::player->data();
    def.name_a      = def.name_the = name;
}

} //Enter_name

void draw_pick_bg(const vector<Bg>& bgs, const Menu_browser& browser)
{
    render::clear_screen();
    render::draw_popup_box(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

    render::draw_text_centered("Choose your background", Panel::screen,
                               Pos(MAP_W_HALF, 0), clr_white, clr_black, true);

    const Pos& browser_pos = browser.pos();

    const int Y0_BGS = 2;

    int y = Y0_BGS;

    const Bg marked_bg = bgs[browser_pos.y];

    const int NR_BGS = bgs.size();

    //------------------------------------------------------------- BACKGROUNDS
    for (int i = 0; i < NR_BGS; ++i)
    {
        const Bg bg = bgs[i];
        string name = "";
        player_bon::bg_title(bg, name);
        const bool IS_MARKED  = bg == marked_bg;
        const Clr& drw_clr     = IS_MARKED ? clr_menu_highlight : clr_menu_drk;
        render::draw_text_centered(name, Panel::screen, Pos(MAP_W_HALF, y), drw_clr);
        y++;
    }

    y++;

    const int BGS_BOX_W_HALF = 7;
    Rect box_rect(Pos(MAP_W_HALF - BGS_BOX_W_HALF, Y0_BGS - 1),
                  Pos(MAP_W_HALF + BGS_BOX_W_HALF, Y0_BGS + NR_BGS));
    render::draw_popup_box(box_rect);

    //------------------------------------------------------------- DESCRIPTION
    const int MARGIN_W_DESCR  = 12;
    const int X0_DESCR        = MARGIN_W_DESCR;
    const int MAX_W_DESCR     = MAP_W - (MARGIN_W_DESCR * 2);

    vector<string> raw_descr_lines;
    player_bon::bg_descr(marked_bg, raw_descr_lines);

    for (string& raw_line : raw_descr_lines)
    {
        vector<string> formatted_lines;
        text_format::line_to_lines(raw_line, MAX_W_DESCR, formatted_lines);

        for (string& line : formatted_lines)
        {
            render::draw_text(line, Panel::screen, Pos(X0_DESCR, y), clr_white);
            y++;
        }
    }

    render::update_screen();
}

void pick_bg()
{
    if (config::is_bot_playing())
    {
        player_bon::pick_bg(Bg(rnd::range(0, int(Bg::END) - 1)));
    }
    else
    {
        vector<Bg> bgs;
        player_bon::pickable_bgs(bgs);

        Menu_browser browser(bgs.size(), 0);
        draw_pick_bg(bgs, browser);

        while (true)
        {
            const Menu_action action = menu_input_handling::action(browser);

            switch (action)
            {
            case Menu_action::browsed: {draw_pick_bg(bgs, browser);} break;

            case Menu_action::esc:
            case Menu_action::space: {} break;

            case Menu_action::selected:
            {
                player_bon::pick_bg(bgs[browser.pos().y]);
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

    render::clear_screen();
    render::draw_popup_box(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

    const int NR_TRAITS_1 = traits1.size();
    const int NR_TRAITS_2 = traits2.size();

    int len_of_longest_in_col2 = -1;

    for (const Trait& id : traits2)
    {
        string title = "";
        player_bon::trait_title(id, title);
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

    render::draw_text_centered(title, Panel::screen, Pos(MAP_W_HALF, 0),
                               clr_white, clr_black, true);

    const Pos& browser_pos = browser.pos();

    //------------------------------------------------------------- TRAITS
    const int Y0_TRAITS = 2;
    int y = Y0_TRAITS;

    for (int i = 0; i < NR_TRAITS_1; ++i)
    {
        const Trait trait = traits1[i];
        string name = "";
        player_bon::trait_title(trait, name);
        const bool IS_MARKED  = browser_pos.x == 0 && browser_pos.y == int(i);
        const Clr& drw_clr     = IS_MARKED ? clr_menu_highlight : clr_menu_drk;
        render::draw_text(name, Panel::screen, Pos(X_COL_ONE, y), drw_clr);
        y++;
    }

    y = Y0_TRAITS;

    for (int i = 0; i < NR_TRAITS_2; ++i)
    {
        const Trait trait = traits2[i];
        string name = "";
        player_bon::trait_title(trait, name);
        const bool IS_MARKED  = browser_pos.x == 1 && browser_pos.y == int(i);
        const Clr& drw_clr     = IS_MARKED ? clr_menu_highlight : clr_menu_drk;
        render::draw_text(name, Panel::screen, Pos(X_COL_TWO, y), drw_clr);
        y++;
    }

    //Draw frame around traits
    Rect box_rect(
        Pos(MARGIN_W - 2, Y0_TRAITS - 1),
        Pos(X_COL_TWO_RIGHT + 2, Y0_TRAITS + traits1.size()));
    render::draw_popup_box(box_rect);

    //------------------------------------------------------------- DESCRIPTION
    const int Y0_DESCR = Y0_TRAITS + NR_TRAITS_1 + 1;
    const int X0_DESCR = X_COL_ONE;
    y = Y0_DESCR;
    const Trait marked_trait =
        browser_pos.x == 0 ? traits1[browser_pos.y] : traits2[browser_pos.y];
    string descr = "";
    player_bon::trait_descr(marked_trait, descr);
    const int MAX_W_DESCR = X_COL_TWO_RIGHT - X_COL_ONE + 1;
    vector<string> descr_lines;
    text_format::line_to_lines(
        "Effect(s): " + descr, MAX_W_DESCR, descr_lines);

    for (const string& str : descr_lines)
    {
        render::draw_text(str, Panel::screen, Pos(X0_DESCR, y), clr_white);
        y++;
    }

    //------------------------------------------------------------- PREREQUISITES
    const int Y0_PREREQS = 17;
    y = Y0_PREREQS;
    vector<Trait> trait_prereqs;
    Bg bg_prereq = Bg::END;
    player_bon::trait_prereqs(marked_trait, trait_prereqs, bg_prereq);

    if (!trait_prereqs.empty() || bg_prereq != Bg::END)
    {
        render::draw_text("This trait had the following prerequisite(s):",
                          Panel::screen, Pos(X0_DESCR, y), clr_white);
        y++;

        string prereq_str = "";

        if (bg_prereq != Bg::END)
        {
            player_bon::bg_title(bg_prereq, prereq_str);
        }

        for (Trait prereq_trait : trait_prereqs)
        {
            string prereq_title = "";
            player_bon::trait_title(prereq_trait, prereq_title);
            prereq_str += (prereq_str.empty() ? "" : ", ") + prereq_title;
        }

        vector<string> prereq_lines;
        text_format::line_to_lines(prereq_str, MAX_W_DESCR, prereq_lines);

        for (const string& str : prereq_lines)
        {
            render::draw_text(str, Panel::screen, Pos(X0_DESCR, y), clr_white);
            y++;
        }
    }

    //------------------------------------------------------------- PREVIOUS
    y = Y0_PREREQS + 4;
    const int MAX_W_PREV_PICKS  = SCREEN_W - 2;
    string picked_str = "";
    player_bon::all_picked_traits_titles_line(picked_str);

    if (picked_str != "")
    {
        picked_str = "Trait(s) gained: " + picked_str;
        vector<string> picked_lines;
        text_format::line_to_lines(picked_str, MAX_W_PREV_PICKS, picked_lines);

        for (const string& str : picked_lines)
        {
            render::draw_text(
                str, Panel::screen, Pos(1, y), clr_white);
            y++;
        }
    }

    render::update_screen();
}

} //namespace

void create_character()
{
    pick_bg();
    pick_new_trait(true);
    enter_name::run();
}

void pick_new_trait(const bool IS_CHARACTER_CREATION)
{
    if (!config::is_bot_playing())
    {
        vector<Trait> pickable_traits;
        player_bon::pickable_traits(pickable_traits);

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
                const Menu_action action = menu_input_handling::action(browser);

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
                    const Pos pos = browser.pos();
                    player_bon::pick_trait(pos.x == 0 ? traits1[pos.y] : traits2[pos.y]);

                    if (!IS_CHARACTER_CREATION) {render::draw_map_and_interface();}

                    return;
                } break;

                case Menu_action::selected_shift: {} break;
                }
            }
        }
    }
}

} //Create_character
