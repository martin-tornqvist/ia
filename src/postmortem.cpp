#include "postmortem.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "init.hpp"
#include "render.hpp"
#include "input.hpp"
#include "actor_player.hpp"
#include "dungeon_master.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "menu_input_handling.hpp"
#include "highscore.hpp"
#include "player_bon.hpp"
#include "text_format.hpp"
#include "menu_browser.hpp"
#include "feature_rigid.hpp"
#include "utils.hpp"

using namespace std;

namespace postmortem
{

namespace
{

struct Str_and_clr
{
    Str_and_clr(const string& str_, const Clr clr_) :
        str(str_),
        clr(clr_) {}

    Str_and_clr() :
        str(""),
        clr(clr_white) {}

    std::string str;
    Clr         clr;
};

void mk_info_lines(vector<Str_and_clr>& out)
{
    TRACE_FUNC_BEGIN;

    const Clr clr_heading  = clr_white_high;
    const Clr clr_info     = clr_white;

    TRACE << "Finding number of killed monsters" << endl;
    vector<string> unique_killed_names;
    int nr_kills_tot_all_mon = 0;

    for (const auto& d : actor_data::data)
    {
        if (d.id != Actor_id::player && d.nr_kills > 0)
        {
            nr_kills_tot_all_mon += d.nr_kills;

            if (d.is_unique)
            {
                unique_killed_names.push_back(d.name_a);
            }
        }
    }

    out.push_back(Str_and_clr(" " + map::player->get_name_a(), clr_heading));

    out.push_back(Str_and_clr("   * Explored to the depth of dungeon level "
                              + to_str(map::dlvl), clr_info));
    out.push_back(Str_and_clr("   * Was " + to_str(min(100, map::player->get_insanity())) +
                              "% insane", clr_info));
    out.push_back(Str_and_clr("   * Killed " + to_str(nr_kills_tot_all_mon) + " monsters ",
                              clr_info));

    //TODO: This is ugly as hell
    if (map::player->phobias[int(Phobia::dog)])
        out.push_back(Str_and_clr("   * Had a phobia of dogs", clr_info));

    if (map::player->phobias[int(Phobia::rat)])
        out.push_back(Str_and_clr("   * Had a phobia of rats", clr_info));

    if (map::player->phobias[int(Phobia::spider)])
        out.push_back(Str_and_clr("   * Had a phobia of spiders", clr_info));

    if (map::player->phobias[int(Phobia::undead)])
        out.push_back(Str_and_clr("   * Had a phobia of the dead", clr_info));

    if (map::player->phobias[int(Phobia::cramped_place)])
        out.push_back(Str_and_clr("   * Had a phobia of cramped spaces", clr_info));

    if (map::player->phobias[int(Phobia::open_place)])
        out.push_back(Str_and_clr("   * Had a phobia of open places", clr_info));

    if (map::player->phobias[int(Phobia::deep_places)])
        out.push_back(Str_and_clr("   * Had a phobia of deep places", clr_info));

    if (map::player->phobias[int(Phobia::dark)])
        out.push_back(Str_and_clr("   * Had a phobia of darkness", clr_info));

    if (map::player->obsessions[int(Obsession::masochism)])
        out.push_back(Str_and_clr("   * Had a masochistic obsession", clr_info));

    if (map::player->obsessions[int(Obsession::sadism)])
        out.push_back(Str_and_clr("   * Had a sadistic obsession", clr_info));

    out.push_back(Str_and_clr(" ", clr_info));

    TRACE << "Finding traits gained" << endl;
    out.push_back(Str_and_clr(" Traits gained:", clr_heading));
    string traits_line;
    player_bon::get_all_picked_traits_titles_line(traits_line);

    if (traits_line.empty())
    {
        out.push_back(Str_and_clr("   * None", clr_info));
    }
    else
    {
        vector<string> abilities_lines;
        text_format::line_to_lines(traits_line, 60, abilities_lines);

        for (string& str : abilities_lines)
        {
            out.push_back(Str_and_clr("   " + str, clr_info));
        }
    }

    out.push_back(Str_and_clr(" ", clr_info));

    out.push_back(Str_and_clr(" Unique monsters killed:", clr_heading));

    if (unique_killed_names.empty())
    {
        out.push_back(Str_and_clr("   * None", clr_info));
    }
    else
    {
        for (string& monster_name : unique_killed_names)
        {
            out.push_back(Str_and_clr("   * " + monster_name, clr_info));
        }
    }

    out.push_back(Str_and_clr(" ", clr_info));

    out.push_back(Str_and_clr(" Last messages:", clr_heading));
    const vector< vector<Msg> >& history = msg_log::get_history();
    int history_element = max(0, int(history.size()) - 20);

    for (size_t i = history_element; i < history.size(); ++i)
    {
        string row = "";

        for (size_t ii = 0; ii < history[i].size(); ii++)
        {
            string msg_str = "";
            history[i][ii].get_str_with_repeats(msg_str);
            row += msg_str + " ";
        }

        out.push_back(Str_and_clr("   " + row, clr_info));
    }

    out.push_back(Str_and_clr(" ", clr_info));

    TRACE << "Drawing the final map" << endl;
    out.push_back(Str_and_clr(" The final moment:", clr_heading));

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    if (utils::is_pos_inside_map(Pos(x + dx, y + dy)))
                    {
                        const auto* const f = map::cells[x + dx][y + dy].rigid;

                        if (f->is_los_passable())
                        {
                            map::cells[x][y].is_seen_by_player = true;
                        }
                    }
                }
            }
        }
    }

    render::draw_map(); //To set the glyph array

    for (int y = 0; y < MAP_H; ++y)
    {
        string cur_row = "";

        for (int x = 0; x < MAP_W; ++x)
        {
            if (Pos(x, y) == map::player->pos)
            {
                cur_row.push_back('@');
            }
            else
            {
                if (
                    render::render_array[x][y].glyph == ' ' &&
                    (y == 0 || x == 0 || y == MAP_H - 1 || x == MAP_W - 1))
                {
                    cur_row.push_back('*');
                }
                else
                {
                    const auto& wall_d       = feature_data::get_data(Feature_id::wall);
                    const auto& rubble_high_d = feature_data::get_data(Feature_id::rubble_high);
                    const auto& statue_d     = feature_data::get_data(Feature_id::statue);

                    if (
                        render::render_array[x][y].glyph == wall_d.glyph ||
                        render::render_array[x][y].glyph == rubble_high_d.glyph)
                    {
                        cur_row.push_back('#');
                    }
                    else if (render::render_array[x][y].glyph == statue_d.glyph)
                    {
                        cur_row.push_back('M');
                    }
                    else
                    {
                        cur_row.push_back(render::render_array[x][y].glyph);
                    }
                }
            }
        }

        out.push_back(Str_and_clr(cur_row, clr_info));
        cur_row.clear();
    }

    TRACE_FUNC_END;
    TRACE_FUNC_END;
}

void render(const vector<Str_and_clr>& lines, const int TOP_ELEMENT)
{
    render::clear_screen();

    const string decoration_line(MAP_W, '-');
    render::draw_text(decoration_line, Panel::screen, Pos(0, 0), clr_gray);

    const int X_LABEL = 3;

    render::draw_text(" Displaying postmortem information ", Panel::screen,
                      Pos(X_LABEL, 0), clr_gray);

    render::draw_text(decoration_line, Panel::screen, Pos(0, SCREEN_H - 1),
                      clr_gray);

    render::draw_text(info_scr_cmd_info, Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clr_gray);

    const int NR_LINES_TOT = int(lines.size());
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;
    int y_pos = 1;

    for (
        int i = TOP_ELEMENT;
        i < NR_LINES_TOT && ((i - TOP_ELEMENT) < MAX_NR_LINES_ON_SCR);
        ++i)
    {
        render::draw_text(lines[i].str, Panel::screen, Pos(0, y_pos++), lines[i].clr);
    }

    render::update_screen();
}

void run_info(const vector<Str_and_clr>& lines)
{
    const int LINE_JUMP           = 3;
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;
    const int NR_LINES_TOT        = lines.size();

    int top_nr = 0;

    while (true)
    {
        render(lines, top_nr);

        const Key_data& d = input::get_input();

        if (d.sdl_key == SDLK_DOWN || d.key == '2' || d.key == 'j')
        {
            top_nr += LINE_JUMP;

            if (NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
            {
                top_nr = 0;
            }
            else
            {
                top_nr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, top_nr);
            }
        }
        else if (d.sdl_key == SDLK_UP || d.key == '8' || d.key == 'k')
        {
            top_nr = max(0, top_nr - LINE_JUMP);
        }
        else if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }
    }
}

void mk_memorial_file(const vector<Str_and_clr>& lines)
{
    const string time_stamp =
        dungeon_master::get_start_time().get_time_str(Time_type::second, false);
    const string memorial_file_name = map::player->get_name_a() + "_" + time_stamp + ".txt";
    const string memorial_file_path = "data/" + memorial_file_name;

    //Add memorial file
    ofstream file;
    file.open(memorial_file_path.data(), ios::trunc);

    for (const Str_and_clr& line : lines) {file << line.str << endl;}

    file.close();

    render::draw_text("Wrote file: data/" + memorial_file_name, Panel::screen, Pos(0, 0),
                      clr_white_high);
    render::update_screen();
}

void render_menu(const Menu_browser& browser)
{
    vector<string> ascii_graveyard;

    string cur_line;
    ifstream file("ascii_graveyard");

    if (file.is_open())
    {
        while (getline(file, cur_line))
        {
            if (cur_line.size() > 0)
            {
                ascii_graveyard.push_back(cur_line);
            }
        }
    }
    else
    {
        TRACE << "Failed to open ascii graveyard file" << endl;
        assert(false);
    }

    file.close();

    render::cover_panel(Panel::screen);

    Pos pos(1, SCREEN_H - ascii_graveyard.size());

    for (const string& line : ascii_graveyard)
    {
        const Uint8 K = Uint8(16 + (180 * ((pos.y * 100) / SCREEN_H) / 100));
        const Clr clr = {K, K, K, 0};
        render::draw_text(line, Panel::screen, pos, clr);
        pos.y++;
    }

    pos.set(45, 20);
    const string NAME_STR = map::player->get_data().name_a;
    render::draw_text_centered(NAME_STR, Panel::screen, pos, clr_white);

    //Draw command labels
    pos.set(55, 13);
    render::draw_text("Information", Panel::screen, pos,
                      browser.is_at_idx(0) ? clr_menu_highlight : clr_menu_drk);
    pos.y++;

    render::draw_text("Write memorial file", Panel::screen, pos,
                      browser.is_at_idx(1) ? clr_menu_highlight : clr_menu_drk);
    pos.y++;

    render::draw_text("View High Scores", Panel::screen, pos,
                      browser.is_at_idx(2) ? clr_menu_highlight : clr_menu_drk);
    pos.y++;

    render::draw_text("View message log", Panel::screen, pos,
                      browser.is_at_idx(3) ? clr_menu_highlight : clr_menu_drk);
    pos.y++;

    render::draw_text("Return to main menu", Panel::screen, pos,
                      browser.is_at_idx(4) ? clr_menu_highlight : clr_menu_drk);
    pos.y++;

    render::draw_text("Quit the game", Panel::screen, pos,
                      browser.is_at_idx(5) ? clr_menu_highlight : clr_menu_drk);
    pos.y++;

    render::update_screen();
}

} //namespace

void run(bool* const quit_game)
{
    vector<Str_and_clr> lines;

    mk_info_lines(lines);

    Menu_browser browser(6, 0);

    render_menu(browser);

    while (true)
    {
        const Menu_action action = menu_input_handling::get_action(browser);

        switch (action)
        {
        case Menu_action::esc:
        case Menu_action::space:
        case Menu_action::selected_shift: {} break;

        case Menu_action::browsed:
        {
            render_menu(browser);
        } break;

        case Menu_action::selected:
        {
            if (browser.is_at_idx(0))
            {
                run_info(lines);
                render_menu(browser);
            }
            else if (browser.is_at_idx(1))
            {
                mk_memorial_file(lines);
            }
            else if (browser.is_at_idx(2))
            {
                high_score::run_high_score_screen();
                render_menu(browser);
            }
            else if (browser.is_at_idx(3))
            {
                msg_log::display_history();
                render_menu(browser);
            }
            else if (browser.is_at_idx(4))
            {
                return;
            }
            else if (browser.is_at_idx(5))
            {
                *quit_game = true;
                return;
            }
        } break;
        }
    }
}

} //Postmortem
