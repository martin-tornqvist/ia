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

void mk_info_lines(std::vector<Str_and_clr>& out)
{
    TRACE_FUNC_BEGIN;

    const Clr clr_heading  = clr_white_high;
    const Clr clr_info     = clr_white;

    const std::string offset            = "   ";
    const std::string bullet_point_str  = offset + "* ";

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

    const Highscore_entry* const score = highscore::final_score();

    assert(score);

    out.push_back({map::player->name_a(), clr_heading});

    const int DLVL = score->dlvl();

    if (DLVL == 0)
    {
        out.push_back({bullet_point_str + "Died before entering the dungeon" , clr_info});
    }
    else //DLVL is at least 1
    {
        out.push_back({bullet_point_str + "Explored to the depth of dungeon level " +
                       to_str(DLVL), clr_info
                      });

    }

    out.push_back({bullet_point_str + "Was " + to_str(score->ins()) + "% insane", clr_info
                  });

    out.push_back({bullet_point_str + "Killed " + to_str(nr_kills_tot_all_mon) + " monsters",
                   clr_info
                  });

    out.push_back({bullet_point_str + "Gained " + to_str(score->xp()) + " experience points",
                   clr_info
                  });

    out.push_back({bullet_point_str + "Gained a score of " + to_str(score->score()), clr_info
                  });

    const std::vector<const Ins_sympt*> sympts = insanity::active_sympts();

    if (!sympts.empty())
    {
        for (const Ins_sympt* const sympt : sympts)
        {
            const std::string sympt_descr = sympt->char_descr_msg();

            if (!sympt_descr.empty())
            {
                out.push_back({bullet_point_str + sympt_descr, clr_info});
            }
        }
    }

    out.push_back({"", clr_info});
    out.push_back({"Traits gained:", clr_heading});
    std::string traits_line = player_bon::all_picked_traits_titles_line();

    if (traits_line.empty())
    {
        out.push_back({bullet_point_str + "None", clr_info});
    }
    else
    {
        std::vector<std::string> abilities_lines;
        text_format::split(traits_line, 60, abilities_lines);

        for (std::string& str : abilities_lines)
        {
            out.push_back({offset + str, clr_info});
        }
    }

    out.push_back({"", clr_info});
    out.push_back({"Unique monsters killed:", clr_heading});

    if (unique_killed_names.empty())
    {
        out.push_back({bullet_point_str + "None", clr_info});
    }
    else
    {
        for (std::string& monster_name : unique_killed_names)
        {
            out.push_back({bullet_point_str + "" + monster_name, clr_info});
        }
    }

    out.push_back({"", clr_info});
    out.push_back({"History of " + map::player->name_the(), clr_heading});

    const std::vector<History_event>& events = dungeon_master::history();

    for (const auto& event : events)
    {
        std::string ev_str = to_str(event.TURN);

        const int TURN_STR_MAX_W = 10;

        text_format::pad_before_to(ev_str, TURN_STR_MAX_W);

        ev_str += ": " + event.msg;

        out.push_back({offset + ev_str, clr_info});
    }

    out.push_back({"", clr_info});
    out.push_back({"Last messages:", clr_heading});
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

        out.push_back({offset + row, clr_info});
    }

    out.push_back({"", clr_info});

    TRACE << "Drawing the final map" << std::endl;
    out.push_back({"The final moment:", clr_heading});

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    if (utils::is_pos_inside_map(P(x + dx, y + dy)))
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
            if (P(x, y) == map::player->pos)
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

        out.push_back({cur_row, clr_info});
        cur_row.clear();
    }

    TRACE_FUNC_END;
    TRACE_FUNC_END;
}

void render(const std::vector<Str_and_clr>& lines, const int TOP_ELEMENT)
{
    render::clear_screen();

    render::draw_info_scr_interface("Displaying game summary");

    const int NR_LINES_TOT = int(lines.size());

    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

    int y_pos = 1;

    for (
        int i = TOP_ELEMENT;
        i < NR_LINES_TOT && ((i - TOP_ELEMENT) < MAX_NR_LINES_ON_SCR);
        ++i)
    {
        render::draw_text(lines[i].str,
                          Panel::screen,
                          P(0, y_pos++),
                          lines[i].clr);
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

    render::draw_text("Wrote file: " + file_path, Panel::screen, P(1, 1), clr_white_high);
    render::update_screen();
}

void render_menu(const Menu_browser& browser)
{
    render::cover_panel(Panel::screen);

    P pos(SCREEN_W / 2, 10);

    //Draw options
    render::draw_text_center("Show game summary", Panel::screen, pos,
                             browser.is_at_idx(0) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("Write memorial file", Panel::screen, pos,
                             browser.is_at_idx(1) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("View High Scores", Panel::screen, pos,
                             browser.is_at_idx(2) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("View message log", Panel::screen, pos,
                             browser.is_at_idx(3) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("Return to main menu", Panel::screen, pos,
                             browser.is_at_idx(4) ? clr_menu_highlight : clr_menu_drk);
    ++pos.y;

    render::draw_text_center("Quit the game", Panel::screen, pos,
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
                highscore::run_highscore_screen();
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
