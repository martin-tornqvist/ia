#include "create_character.hpp"

#include "render.hpp"
#include "actor_player.hpp"
#include "input.hpp"
#include "menu_input.hpp"
#include "text_format.hpp"
#include "map.hpp"
#include "dungeon_master.hpp"

namespace create_character
{

const int top_more_y    = 1;
const int btm_more_y    = screen_h - 1;
const int OPT_X0        = 0;
const int OPT_Y0        = top_more_y + 1;
const int OPT_X1        = 22;
const int OPT_Y1        = btm_more_y - 1;
const int opt_h         = OPT_Y1 - OPT_Y0 + 1;

namespace
{

const int descr_x0  = OPT_X1 + 2;
const int DESCR_Y0  = OPT_Y0;
const int DESCR_X1  = screen_w - 1;
const int DESCR_Y1  = OPT_Y1;

const int descr_w   = DESCR_X1 - DESCR_Y1 + 1;

namespace enter_name
{

void draw(const std::string& current_string)
{
    render::clear_screen();

    render::draw_text_center("What is your name?",
                             Panel::screen,
                             P(map_w_half, 0),
                             clr_title);
    const int y_name = 3;

    const std::string name_str = current_string.size() < player_name_max_len ?
                                 current_string + "_" : current_string;

    const size_t NAME_X0 = map_w_half - (player_name_max_len / 2);
    const size_t NAME_X1 = NAME_X0 + player_name_max_len - 1;

    render::draw_text(name_str,
                      Panel::screen,
                      P(NAME_X0, y_name),
                      clr_brown_gray);

    R box_rect(P(NAME_X0 - 1, y_name - 1),
               P(NAME_X1 + 1, y_name + 1));

    render::draw_box(box_rect);

    render::update_screen();
}

void read_keys(std::string& current_string, bool& done)
{
    const KeyData& d = input::input(false);

    if (d.sdl_key == SDLK_RETURN)
    {
        done = true;
        current_string = current_string.empty() ? "Player" : current_string;
        return;
    }

    if (current_string.size() < player_name_max_len)
    {
        if (
            d.sdl_key == SDLK_SPACE ||
            (d.key >= int('a') && d.key <= int('z')) ||
            (d.key >= int('A') && d.key <= int('Z')) ||
            (d.key >= int('0') && d.key <= int('9')))
        {
            if (d.sdl_key == SDLK_SPACE)
            {
                current_string.push_back(' ');
            }
            else
            {
                current_string.push_back(char(d.key));
            }

            draw(current_string);
            return;
        }
    }

    if (current_string.size() > 0)
    {
        if (d.sdl_key == SDLK_BACKSPACE)
        {
            current_string.erase(current_string.end() - 1);
            draw(current_string);
        }
    }
}

void run()
{
    std::string name = "";

    draw(name);

    bool done = false;

    while (!done)
    {
        if (config::is_bot_playing())
        {
            name = "Bot";
            done = true;
        }
        else
        {
            read_keys(name, done);
        }
    }

    ActorDataT& def = map::player->data();
    def.name_a = def.name_the = name;
}

} //enter_name

void draw_pick_bg(const std::vector<Bg>& bgs, const MenuBrowser& browser)
{
    render::clear_screen();

    render::draw_text_center("What is your background?",
                             Panel::screen,
                             P(map_w_half, 0),
                             clr_title,
                             clr_black,
                             true);

    int y = OPT_Y0;

    const Bg bg_marked = bgs[browser.y()];

    //------------------------------------------------------------- BACKGROUNDS
    std::string key_str = "a) ";

    for (const Bg bg : bgs)
    {
        const std::string   bg_name     = player_bon::bg_title(bg);
        const bool          is_marked   = bg == bg_marked;
        const Clr&          drw_clr     = is_marked ? clr_menu_highlight : clr_menu_drk;

        render::draw_text(key_str + bg_name,
                          Panel::screen,
                          P(OPT_X0, y),
                          drw_clr);

        ++y;
        ++key_str[0];
    }

    //------------------------------------------------------------- DESCRIPTION
    y = DESCR_Y0;

    const std::vector<std::string> descr =
        player_bon::bg_descr(bg_marked);

    ASSERT(!descr.empty());

    for (const std::string& descr_entry : descr)
    {
        if (descr_entry.empty())
        {
            ++y;
            continue;
        }

        std::vector<std::string> formatted_lines;

        text_format::split(descr_entry,
                           descr_w,
                           formatted_lines);

        for (const std::string& line : formatted_lines)
        {
            render::draw_text(line,
                              Panel::screen,
                              P(descr_x0, y),
                              clr_text);
            ++y;
        }
    }

    render::update_screen();
}

void pick_bg()
{
    if (config::is_bot_playing())
    {
        player_bon::pick_bg(Bg(rnd::range(0, (int)Bg::END - 1)));
    }
    else //Human playing
    {
        std::vector<Bg> bgs;
        player_bon::pickable_bgs(bgs);

        MenuBrowser browser(bgs.size());

        //Let the browser start at Rogue, to recommend it as the default choice
        browser.set_y((int)Bg::rogue);

        draw_pick_bg(bgs, browser);

        while (true)
        {
            const MenuAction action = menu_input::action(browser);

            switch (action)
            {
            case MenuAction::moved:
                draw_pick_bg(bgs, browser);
                break;

            case MenuAction::selected:
            case MenuAction::selected_shift:
                player_bon::pick_bg(bgs[browser.y()]);
                return;

            case MenuAction::esc:
            case MenuAction::space:
                break;
            }
        }
    }
}

void draw_pick_trait(const std::vector<Trait>& traits, const MenuBrowser& browser)
{
    render::clear_screen();

    const int clvl = dungeon_master::clvl();

    std::string title = clvl == 1 ?
                        "Which additional trait do you start with?" :
                        "You have reached a new level! Which trait do you gain?";

    render::draw_text_center(title,
                             Panel::screen,
                             P(map_w_half, 0),
                             clr_title,
                             clr_black,
                             true);

    const int   browser_y       = browser.y();
    const Trait trait_marked    = traits[browser_y];
    const Bg    player_bg       = player_bon::bg();

    //------------------------------------------------------------- TRAITS
    int y = OPT_Y0;

    const Range idx_range_shown = browser.range_shown();

    std::string key_str = "a) ";

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        const Trait trait           = traits[i];
        std::string trait_name      = player_bon::trait_title(trait);
        const bool  is_idx_marked   = browser_y == i;
        const bool  is_picked       = player_bon::traits[(size_t)trait];
        const bool  is_prereqs_ok   = player_bon::is_prereqs_ok(trait);

        Clr clr = clr_magenta_lgt;

        if (is_prereqs_ok)
        {
            if (is_picked)
            {
                if (is_idx_marked)
                {
                    clr = clr_green_lgt;
                }
                else //Not marked
                {
                    clr = clr_green;
                }
            }
            else //Not picked
            {
                if (is_idx_marked)
                {
                    clr = clr_menu_highlight;
                }
                else //Not marked
                {
                    clr = clr_menu_drk;
                }
            }
        }
        else //Prerequisites not met
        {
            if (is_idx_marked)
            {
                clr = clr_red_lgt;
            }
            else //Not marked
            {
                clr = clr_red;
            }
        }

        render::draw_text(key_str + trait_name,
                          Panel::screen,
                          P(OPT_X0, y),
                          clr);

        ++y;
        ++key_str[0];
    }

    //Draw "more" labels
    if (!browser.is_on_top_page())
    {
        render::draw_text("(More - Page Up)",
                          Panel::screen,
                          P(OPT_X0, top_more_y),
                          clr_white_high);
    }

    if (!browser.is_on_btm_page())
    {
        render::draw_text("(More - Page Down)",
                          Panel::screen,
                          P(OPT_X0, btm_more_y),
                          clr_white_high);
    }

    //------------------------------------------------------------- DESCRIPTION
    y = DESCR_Y0;

    std::string descr = player_bon::trait_descr(trait_marked);

    std::vector<std::string> formatted_descr;

    text_format::split(descr,
                       descr_w,
                       formatted_descr);

    for (const std::string& str : formatted_descr)
    {
        render::draw_text(str,
                          Panel::screen,
                          P(descr_x0, y),
                          clr_white);
        ++y;
    }

    //------------------------------------------------------------- PREREQUISITES
    std::vector<Trait> trait_marked_prereqs;

    Bg trait_marked_bg_prereq = Bg::END;

    player_bon::trait_prereqs(trait_marked,
                              player_bg,
                              trait_marked_prereqs,
                              trait_marked_bg_prereq);

    const int Y0_PREREQS = 10;

    y = Y0_PREREQS;

    if (!trait_marked_prereqs.empty() || trait_marked_bg_prereq != Bg::END)
    {
        const std::string label = "Prerequisite(s):";

        render::draw_text(label,
                          Panel::screen,
                          P(descr_x0, y),
                          clr_white);

        std::vector<StrAndClr> prereq_titles;

        std::string prereq_str = "";

        const Clr& clr_prereq_ok        = clr_green;
        const Clr& clr_prereq_not_ok    = clr_red;

        if (trait_marked_bg_prereq != Bg::END)
        {
            const Clr& clr =
                (player_bon::bg() == trait_marked_bg_prereq) ?
                clr_prereq_ok :
                clr_prereq_not_ok;

            const std::string bg_title = player_bon::bg_title(trait_marked_bg_prereq);

            prereq_titles.push_back(StrAndClr(bg_title, clr));
        }

        for (Trait prereq_trait : trait_marked_prereqs)
        {
            const bool is_picked = player_bon::traits[(size_t)prereq_trait];

            const Clr& clr =
                is_picked ?
                clr_prereq_ok :
                clr_prereq_not_ok;

            const std::string trait_title = player_bon::trait_title(prereq_trait);

            prereq_titles.push_back(StrAndClr(trait_title, clr));
        }

        const int prereq_list_x = descr_x0 + label.size() + 1;

        for (const StrAndClr& title : prereq_titles)
        {
            render::draw_text(title.str,
                              Panel::screen,
                              P(prereq_list_x, y),
                              title.clr);

            ++y;
        }
    }

    render::update_screen();
}

} //namespace

void create_character()
{
    pick_bg();
    pick_new_trait();

    //Some backgrounds and traits may have affected maximum hp and spi (either
    //positively or negatively), so here we need to set the current hp and SPI
    //equal to the maximum values.
    map::player->set_hp_and_spi_to_max();

    enter_name::run();
}

void pick_new_trait()
{
    if (config::is_bot_playing())
    {
        return;
    }

    const Bg player_bg = player_bon::bg();

    std::vector<Trait> traits;

    player_bon::trait_list_for_bg(player_bg, traits);

    const int list_h = OPT_Y1 - OPT_Y0 + 1;

    MenuBrowser browser(traits.size(), list_h);

    while (true)
    {
        draw_pick_trait(traits, browser);

        const MenuAction action = menu_input::action(browser);

        switch (action)
        {
        case MenuAction::selected:
        case MenuAction::selected_shift:
        {
            const Trait     trait       = traits[browser.y()];
            const bool      is_selected = player_bon::traits[(size_t)trait];

            if (!is_selected)
            {
                const bool is_prereqs_ok = player_bon::is_prereqs_ok(trait);

                if (is_prereqs_ok)
                {
                    player_bon::pick_trait(traits[browser.y()]);

                    return;
                }
            }
        }
        break;

        case MenuAction::moved:
        case MenuAction::esc:
        case MenuAction::space:
            break;
        }
    }
}

} //create_character
