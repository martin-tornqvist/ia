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
#include "menu_input.hpp"
#include "highscore.hpp"
#include "player_bon.hpp"
#include "text_format.hpp"
#include "feature_rigid.hpp"
#include "utils.hpp"
#include "save_handling.hpp"

namespace postmortem
{

namespace
{

struct History_Event
{
    History_Event(const std::string msg, const int TURN_NR) :
        msg     (msg),
        TURN_NR (TURN_NR) {}

    const std::string msg;
    const int TURN_NR;
};

std::vector<History_Event> events_;

struct Str_and_clr
{
    Str_and_clr(const std::string& str_, const Clr clr_) :
        str(str_),
        clr(clr_) {}

    Str_and_clr() :
        str(""),
        clr(clr_white) {}

    std::string str;
    Clr clr;
};

void mk_info_lines(std::vector<Str_and_clr>& out)
{
    TRACE_FUNC_BEGIN;

    const Clr clr_heading  = clr_white_high;
    const Clr clr_info     = clr_white;

    TRACE << "Finding number of killed monsters" << std::endl;
    std::vector<std::string> unique_killed_names;
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

    out.push_back(Str_and_clr(map::player->name_a(), clr_heading));

    out.push_back(Str_and_clr("   * Explored to the depth of dungeon level "
                              + to_str(map::dlvl), clr_info));

    out.push_back(Str_and_clr("   * Was " + to_str(std::min(100, map::player->ins())) +
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

    TRACE << "Finding traits gained" << std::endl;
    out.push_back(Str_and_clr("Traits gained:", clr_heading));
    std::string traits_line = player_bon::all_picked_traits_titles_line();

    if (traits_line.empty())
    {
        out.push_back(Str_and_clr("   * None", clr_info));
    }
    else
    {
        std::vector<std::string> abilities_lines;
        text_format::line_to_lines(traits_line, 60, abilities_lines);

        for (std::string& str : abilities_lines)
        {
            out.push_back(Str_and_clr("   " + str, clr_info));
        }
    }

    out.push_back(Str_and_clr(" ", clr_info));

    out.push_back(Str_and_clr("Unique monsters killed:", clr_heading));

    if (unique_killed_names.empty())
    {
        out.push_back(Str_and_clr("   * None", clr_info));
    }
    else
    {
        for (std::string& monster_name : unique_killed_names)
        {
            out.push_back(Str_and_clr("   * " + monster_name, clr_info));
        }
    }

    out.push_back(Str_and_clr(" ", clr_info));

    out.push_back(Str_and_clr("Last messages:", clr_heading));
    const std::vector< std::vector<Msg> >& history = msg_log::history();
    int history_element = std::max(0, int(history.size()) - 20);

    for (size_t i = history_element; i < history.size(); ++i)
    {
        std::string row = "";

        for (size_t ii = 0; ii < history[i].size(); ii++)
        {
            std::string msg_str = "";
            history[i][ii].str_with_repeats(msg_str);
            row += msg_str + " ";
        }

        out.push_back(Str_and_clr("   " + row, clr_info));
    }

    out.push_back(Str_and_clr(" ", clr_info));

    TRACE << "Drawing the final map" << std::endl;
    out.push_back(Str_and_clr("The final moment:", clr_heading));

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
        std::string cur_row = "";

        for (int x = 0; x < MAP_W; ++x)
        {
            if (Pos(x, y) == map::player->pos)
            {
                cur_row.push_back('@');
            }
            else //Not player pos
            {
                const auto& wall_d          = feature_data::data(Feature_id::wall);
                const auto& rubble_high_d   = feature_data::data(Feature_id::rubble_high);
                const auto& statue_d        = feature_data::data(Feature_id::statue);

                auto& cur_render_data = render::render_array[x][y];

                if (
                    cur_render_data.glyph == wall_d.glyph ||
                    cur_render_data.glyph == rubble_high_d.glyph)
                {
                    cur_row.push_back('#');
                }
                else if (cur_render_data.glyph == statue_d.glyph)
                {
                    cur_row.push_back('M');
                }
                else //Not wall, rubble or statue
                {
                    cur_row.push_back(cur_render_data.glyph);
                }
            }
        }

        out.push_back(Str_and_clr(cur_row, clr_info));
        cur_row.clear();
    }

    TRACE_FUNC_END;
    TRACE_FUNC_END;
}

void render(const std::vector<Str_and_clr>& lines, const int TOP_ELEMENT)
{
    render::clear_screen();

    const std::string decoration_line(MAP_W, '-');
    render::draw_text(decoration_line, Panel::screen, Pos(0, 0), clr_gray);

    const int X_LABEL = 3;

    render::draw_text("Displaying game summary", Panel::screen,
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

void run_info(const std::vector<Str_and_clr>& lines)
{
    const int LINE_JUMP           = 3;
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;
    const int NR_LINES_TOT        = lines.size();

    int top_nr = 0;

    while (true)
    {
        render(lines, top_nr);

        const Key_data& d = input::input();

        if (d.sdl_key == SDLK_DOWN || d.key == '2' || d.key == 'j')
        {
            top_nr += LINE_JUMP;

            if (NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
            {
                top_nr = 0;
            }
            else
            {
                top_nr = std::min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, top_nr);
            }
        }
        else if (d.sdl_key == SDLK_UP || d.key == '8' || d.key == 'k')
        {
            top_nr = std::max(0, top_nr - LINE_JUMP);
        }
        else if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }
    }
}

void mk_memorial_file(const std::vector<Str_and_clr>& lines)
{
    const std::string time_stamp =
        dungeon_master::start_time().time_str(Time_type::second, false);

    const std::string file_name = map::player->name_a() + "_" + time_stamp + ".txt";

    const std::string file_path = "data/" + file_name;

    //Write memorial file
    std::ofstream file;
    file.open(file_path.data(), std::ios::trunc);

    for (const Str_and_clr& line : lines)
    {
        file << line.str << std::endl;
    }

    file.close();

    render::draw_text("Wrote file: " + file_path, Panel::screen, Pos(1, 1), clr_white_high);
    render::update_screen();
}

void render_menu(const Menu_browser& browser)
{
    render::cover_panel(Panel::screen);

    Pos pos(SCREEN_W / 2, 10);

    //Draw options
    render::draw_text_centered("Show game summary", Panel::screen, pos,
                               browser.is_at_idx(0) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_centered("Write memorial file", Panel::screen, pos,
                               browser.is_at_idx(1) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_centered("View High Scores", Panel::screen, pos,
                               browser.is_at_idx(2) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_centered("View message log", Panel::screen, pos,
                               browser.is_at_idx(3) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_centered("Return to main menu", Panel::screen, pos,
                               browser.is_at_idx(4) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_centered("Quit the game", Panel::screen, pos,
                               browser.is_at_idx(5) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    if (config::is_tiles_mode())
    {
        render::draw_skull({10, 2});
    }

    render::draw_box({0, 0, SCREEN_W - 1, SCREEN_H - 1});

    render::update_screen();
}

} //namespace

void init()
{
    events_.clear();
}

void save()
{
    save_handling::put_int(events_.size());

    for (const History_Event& event : events_)
    {
        save_handling::put_str(event.msg);
        save_handling::put_int(event.TURN_NR);
    }
}

void load()
{
    const int NR_EVENTS = save_handling::get_int();

    for (int i = 0; i < NR_EVENTS; ++i)
    {
        const std::string   msg     = save_handling::get_str();
        const int           TURN_NR = save_handling::get_int();

        events_.push_back({msg, TURN_NR});
    }
}

void add_history_event(const std::string msg)
{
    const int TURN_NR = game_time::turn();

    events_.push_back({msg, TURN_NR});
}

void run(bool* const quit_game)
{
    std::vector<Str_and_clr> lines;

    mk_info_lines(lines);

    Menu_browser browser(6);

    render_menu(browser);

    while (true)
    {
        const Menu_action action = menu_input::action(browser);

        switch (action)
        {
        case Menu_action::esc:
        case Menu_action::space:
            break;

        case Menu_action::moved:
            render_menu(browser);
            break;

        case Menu_action::selected:
        case Menu_action::selected_shift:
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
            break;
        }
    }
}

} //Postmortem
