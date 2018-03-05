#include "postmortem.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "audio.hpp"
#include "init.hpp"
#include "io.hpp"
#include "actor_player.hpp"
#include "game.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "browser.hpp"
#include "highscore.hpp"
#include "player_bon.hpp"
#include "text_format.hpp"
#include "feature_rigid.hpp"
#include "saving.hpp"
#include "query.hpp"
#include "create_character.hpp"
#include "draw_map.hpp"

namespace
{

std::vector<ColoredString> info_lines_;

} // namespace

// -----------------------------------------------------------------------------
// Postmortem menu
// -----------------------------------------------------------------------------
PostmortemMenu::PostmortemMenu(const IsWin is_win) :
    State       (),
    browser_    (),
    is_win_     (is_win)
{
    browser_.reset(6);
}

StateId PostmortemMenu::id()
{
    return StateId::postmortem_menu;
}

void PostmortemMenu::on_start()
{
    // Game summary file path
    const std::string game_summary_time_stamp =
        game::start_time().time_str(TimeType::second, false);

    const std::string game_summary_filename =
        map::player->name_a() +
        "_" +
        game_summary_time_stamp +
        ".txt";

    const std::string game_summary_file_path =
        "res/data/" +
        game_summary_filename;

    // Highscore entry
    const auto highscore_entry =
        highscore::make_entry_from_current_game_data(
            game_summary_file_path,
            is_win_);

    highscore::append_entry_to_highscores_file(highscore_entry);

    info_lines_.clear();

    const Color color_heading = colors::light_white();

    const Color color_info = colors::white();

    const std::string offset = "   ";

    TRACE << "Finding number of killed monsters" << std::endl;

    std::vector<std::string> unique_killed_names;

    int nr_kills_tot_all_mon = 0;

    for (const auto& d : actor_data::data)
    {
        if (d.id != ActorId::player && d.nr_kills > 0)
        {
            nr_kills_tot_all_mon += d.nr_kills;

            if (d.is_unique)
            {
                unique_killed_names.push_back(d.name_a);
            }
        }
    }

    const std::string name = highscore_entry.name();

    const std::string bg = player_bon::bg_title(highscore_entry.bg());

    info_lines_.push_back({name + " (" + bg + ")", color_heading});

    const int dlvl = highscore_entry.dlvl();

    if (dlvl == 0)
    {
        info_lines_.push_back(
        {
            offset + "Died before entering the dungeon",
            color_info
        });
    }
    else // DLVL is at least 1
    {
        info_lines_.push_back(
        {
            offset + "Explored to dungeon level " + std::to_string(dlvl),
            color_info
        });

    }

    const int turn_count = highscore_entry.turn_count();

    info_lines_.push_back(
    {
        offset + "Spent " + std::to_string(turn_count) + " turns",
        color_info
    });

    const int ins = highscore_entry.ins();

    info_lines_.push_back(
    {
        offset + "Was " + std::to_string(ins) + "% insane",
        color_info
    });

    info_lines_.push_back(
    {
        offset + "Killed " + std::to_string(nr_kills_tot_all_mon) +" monsters",
        color_info
    });

    const int xp = highscore_entry.xp();

    info_lines_.push_back(
    {
        offset + "Gained " + std::to_string(xp) + " experience points",
        color_info
    });

    const int score = highscore_entry.score();

    info_lines_.push_back(
    {
        offset + "Gained a score of " + std::to_string(score),
        color_info
    });

    const std::vector<const InsSympt*> sympts =
        insanity::active_sympts();

    if (!sympts.empty())
    {
        for (const InsSympt* const sympt : sympts)
        {
            const std::string sympt_descr = sympt->postmortem_msg();

            if (!sympt_descr.empty())
            {
                info_lines_.push_back(
                {
                    offset + sympt_descr, color_info
                });
            }
        }
    }

    info_lines_.push_back({"", color_info});

    info_lines_.push_back({"Traits gained:", color_heading});

    std::string traits_line =
        player_bon::all_picked_traits_titles_line();

    if (traits_line.empty())
    {
        info_lines_.push_back({offset + "None", color_info});
    }
    else
    {
        const auto abilities_lines = text_format::split(traits_line, 60);

        for (const std::string& str : abilities_lines)
        {
            info_lines_.push_back({offset + str, color_info});
        }
    }

    info_lines_.push_back({"", color_info});

    info_lines_.push_back(
    {
        "Unique monsters killed:",
        color_heading
    });

    if (unique_killed_names.empty())
    {
        info_lines_.push_back(
        {
            offset + "None",
            color_info
        });
    }
    else
    {
        for (std::string& monster_name : unique_killed_names)
        {
            info_lines_.push_back(
            {
                offset + "" + monster_name,
                color_info
            });
        }
    }

    info_lines_.push_back({"", color_info});

    info_lines_.push_back(
    {
        "History of " + map::player->name_the(),
        color_heading
    });

    const std::vector<HistoryEvent>& events =
        game::history();

    int longest_turn_w = 0;

    for (const auto& event : events)
    {
        const int turn_w = std::to_string(event.turn).size();

        longest_turn_w = std::max(turn_w, longest_turn_w);
    }

    for (const auto& event : events)
    {
        std::string ev_str = std::to_string(event.turn);

        const int turn_w = ev_str.size();

        ev_str.append(longest_turn_w - turn_w, ' ');

        ev_str += " " + event.msg;

        info_lines_.push_back(
        {
            offset + ev_str,
            color_info
        });
    }

    info_lines_.push_back({"", color_info});

    info_lines_.push_back(
    {
        "Last messages:",
        color_heading
    });

    const std::vector< std::vector<Msg> >& history =
        msg_log::history();

    int history_element =
        std::max(0, (int)history.size() - 20);

    for (size_t i = history_element; i < history.size(); ++i)
    {
        std::string row = "";

        for (size_t ii = 0; ii < history[i].size(); ii++)
        {
            std::string msg_str = "";

            history[i][ii].str_with_repeats(msg_str);

            row += msg_str + " ";
        }

        info_lines_.push_back(
        {
            offset + row,
            color_info
        });
    }

    info_lines_.push_back({"", color_info});

    TRACE << "Drawing the final map" << std::endl;

    info_lines_.push_back(
    {
        "The final moment:",
        color_heading
    });

    // Also dump the lines to a memorial file
    make_memorial_file(game_summary_file_path);

    // If running text mode, load the graveyard ascii art
    if (!config::is_tiles_mode())
    {
        ascii_graveyard_lines_.clear();

        std::string current_line;

        std::ifstream file("res/ascii_graveyard");

        if (file.is_open())
        {
            while (getline(file, current_line))
            {
                // if (current_line.size() > 0)
                // {
                ascii_graveyard_lines_.push_back(current_line);
                // }
            }
        }
        else
        {
            TRACE << "Failed to open ascii graveyard file" << std::endl;
            ASSERT(false);
        }

        file.close();
    }
}

void PostmortemMenu::on_popped()
{
    // The postmortem info is the last state which needs to access session data
    init::cleanup_session();
}

void PostmortemMenu::draw()
{
    P pos;

    if (config::is_tiles_mode())
    {
        pos.set(50, 10);
    }
    else // Text mode
    {
        pos.set(56, 13);
    }

    std::vector<std::string> labels =
    {
        "New journey",
        "Show game summary",
        "View high scores",
        "View message log",
        "Return to main menu",
        "Quit the game"
    };

    for (size_t i = 0; i < labels.size(); ++i)
    {
        const std::string& label = labels[i];

        const Color& color =
            browser_.is_at_idx(i) ?
            colors::menu_highlight() :
            colors::menu_dark();

        io::draw_text(label,
                      Panel::screen,
                      pos,
                      color);

        ++pos.y;
    }

    if (config::is_tiles_mode())
    {
        io::draw_skull(P(27, 2));
    }
    else // Text mode
    {
        int y = 0;

        for (const auto& line : ascii_graveyard_lines_)
        {
            io::draw_text(line,
                          Panel::screen,
                          P(0, y),
                          colors::gray());

            ++y;
        }

        const std::string name = map::player->name_the();

        const int name_x = 44 - ((name.length() - 1) / 2);

        io::draw_text(name,
                      Panel::screen,
                      P(name_x, 20),
                      colors::gray());
    }

    io::draw_box(R(0, 0, screen_w - 1, screen_h - 1));
}

void PostmortemMenu::make_memorial_file(const std::string path) const
{
    // Write memorial file
    std::ofstream file;

    file.open(path.c_str(), std::ios::trunc);

    // Add info lines to file
    for (const ColoredString& line : info_lines_)
    {
        file << line.str << std::endl;
    }

    // Add text map to file
    for (int y = 0; y < map_h; ++y)
    {
        std::string map_line;

        for (int x = 0; x < map_w; ++x)
        {
            char c = draw_map::get_drawn_cell(x, y).character;

            // Printable ASCII character?
            if (c < 32 || c > 126)
            {
                // Nope - well, this is pretty difficult to handle in a good way
                // as things are now. The current strategy is to show the symbol
                // as a wall then (it's probably a wall, rubble, or a statue).

                // TODO: Perhaps text mode should strictly use only printable
                //       basic ASCII symbols?
                c = '#';
            }

            map_line += c;
        }

        file << map_line << std::endl;
    }

    file.close();
}

void PostmortemMenu::update()
{
    const auto input = io::get(true);

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling);

    switch (action)
    {
    case MenuAction::selected:
    case MenuAction::selected_shift:

        // Display postmortem info
        switch (browser_.y())
        {
        case 0:
        {
            // Exit screen
            states::pop();

            init::init_session();

            std::unique_ptr<State> new_game_state(new NewGameState);

            states::push(std::move(new_game_state));

            audio::fade_out_music();

            return;
        }
        break;

        case 1:
        {
            std::unique_ptr<State> postmortem_info(new PostmortemInfo());

            states::push(std::move(postmortem_info));
        }
        break;

        // Show highscores
        case 2:
        {
            std::unique_ptr<State> browse_highscore_state(new BrowseHighscore);

            states::push(std::move(browse_highscore_state));
        }
        break;

        // Display message history
        case 3:
        {
            std::unique_ptr<State> msg_history_state(new MsgHistoryState);

            states::push(std::move(msg_history_state));
        }
        break;

        // Return to main menu
        case 4:
        {
            // Exit screen
            states::pop();

            return;
        }
        break;

        // Quit game
        case 5:
        {
            // Bye!
            states::pop_all();

            return;
        }
        break;
        }
        break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// Postmortem info
// -----------------------------------------------------------------------------
StateId PostmortemInfo::id()
{
    return StateId::postmortem_info;
}

void PostmortemInfo::draw()
{
    io::clear_screen();

    io::draw_info_scr_interface("Game summary",
                                InfScreenType::scrolling);

    const int nr_info_lines = (int)info_lines_.size();

    const int nr_lines_tot = nr_info_lines + map_h;

    int screen_y = 1;

    const bool is_tiles_mode = config::is_tiles_mode();

    for (int i = top_idx_;
         (i < nr_lines_tot) && ((i - top_idx_) < max_nr_lines_on_scr_);
         ++i)
    {
        const bool is_info_lines = i < nr_info_lines;

        if (is_info_lines)
        {
            io::draw_text(info_lines_[i].str,
                          Panel::screen,
                          P(0, screen_y),
                          info_lines_[i].color);
        }
        else // Map lines
        {
            const int map_y = i - nr_info_lines;

            ASSERT(map_y >= 0);
            ASSERT(map_y < map_h);

            for (int x = 0; x < map_w; ++x)
            {
                const CellRenderData& d = draw_map::get_drawn_cell(x, map_y);

                if (is_tiles_mode)
                {
                    io::draw_tile(d.tile,
                                  Panel::screen,
                                  P(x, screen_y),
                                  d.color,
                                  d.color_bg);
                }
                else // Text mode
                {
                    io::draw_character(d.character,
                                       Panel::screen,
                                       P(x, screen_y),
                                       d.color,
                                       true,
                                       d.color_bg);
                }
            } // map x loop
        }

        ++screen_y;
    }

    io::update_screen();
}

void PostmortemInfo::update()
{
    const int line_jump = 3;

    const int nr_lines_tot = info_lines_.size() + map_h;

    const auto input = io::get(false);

    switch (input.key)
    {
    case SDLK_DOWN:
    case '2':
    case 'j':
        top_idx_ += line_jump;

        if (nr_lines_tot <= max_nr_lines_on_scr_)
        {
            top_idx_ = 0;
        }
        else
        {
            top_idx_ = std::min(nr_lines_tot - max_nr_lines_on_scr_, top_idx_);
        }
        break;

    case SDLK_UP:
    case '8':
    case 'k':
        top_idx_ = std::max(0, top_idx_ - line_jump);
        break;

    case SDLK_SPACE:
    case SDLK_ESCAPE:
        // Exit screen
        states::pop();
        break;
    }
}
