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

namespace
{

std::vector<StrAndClr> info_lines_;

} // namespace

// -----------------------------------------------------------------------------
// Postmortem menu
// -----------------------------------------------------------------------------
PostmortemMenu::PostmortemMenu() :
    State       (),
    browser_    ()
{
    browser_.reset(7);
}

void PostmortemMenu::on_start()
{
    info_lines_.clear();

    const Clr clr_heading = clr_white_lgt;
    const Clr clr_info = clr_white;

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

    const HighscoreEntry* const score = highscore::final_score();

    ASSERT(score);

    info_lines_.push_back({map::player->name_a(), clr_heading});

    const int dlvl = score->dlvl();

    if (dlvl == 0)
    {
        info_lines_.push_back(
        {
            offset + "Died before entering the dungeon",
            clr_info
        });
    }
    else // DLVL is at least 1
    {
        info_lines_.push_back(
        {
            offset + "Explored to dungeon level " + std::to_string(dlvl),
            clr_info
        });

    }

    info_lines_.push_back(
    {
        offset + "Was " + std::to_string(score->ins()) + "% insane",
        clr_info
    });

    info_lines_.push_back(
    {
        offset + "Killed " + std::to_string(nr_kills_tot_all_mon) +" monsters",
        clr_info
    });

    info_lines_.push_back(
    {
        offset + "Gained " + std::to_string(score->xp()) + " experience points",
        clr_info
    });

    info_lines_.push_back(
    {
        offset + "Gained a score of " + std::to_string(score->score()),
        clr_info
    });

    const std::vector<const InsSympt*> sympts =
        insanity::active_sympts();

    if (!sympts.empty())
    {
        for (const InsSympt* const sympt : sympts)
        {
            const std::string sympt_descr = sympt->char_descr_msg();

            if (!sympt_descr.empty())
            {
                info_lines_.push_back(
                {
                    offset + sympt_descr, clr_info
                });
            }
        }
    }

    info_lines_.push_back({"", clr_info});

    info_lines_.push_back({"Traits gained:", clr_heading});

    std::string traits_line =
        player_bon::all_picked_traits_titles_line();

    if (traits_line.empty())
    {
        info_lines_.push_back({offset + "None", clr_info});
    }
    else
    {
        std::vector<std::string> abilities_lines;

        text_format::split(traits_line, 60, abilities_lines);

        for (std::string& str : abilities_lines)
        {
            info_lines_.push_back({offset + str, clr_info});
        }
    }

    info_lines_.push_back({"", clr_info});

    info_lines_.push_back(
    {
        "Unique monsters killed:",
        clr_heading
    });

    if (unique_killed_names.empty())
    {
        info_lines_.push_back(
        {
            offset + "None",
            clr_info
        });
    }
    else
    {
        for (std::string& monster_name : unique_killed_names)
        {
            info_lines_.push_back(
            {
                offset + "" + monster_name,
                clr_info
            });
        }
    }

    info_lines_.push_back({"", clr_info});

    info_lines_.push_back(
    {
        "History of " + map::player->name_the(),
        clr_heading
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
            clr_info
        });
    }

    info_lines_.push_back({"", clr_info});

    info_lines_.push_back(
    {
        "Last messages:",
        clr_heading
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
            clr_info
        });
    }

    info_lines_.push_back({"", clr_info});

    TRACE << "Drawing the final map" << std::endl;

    info_lines_.push_back(
    {
        "The final moment:",
        clr_heading
    });
}

void PostmortemMenu::on_popped()
{
    // The postmortem info is the last state which needs to access session data
    init::cleanup_session();
}

void PostmortemMenu::draw()
{
    P pos(48, 10);

    std::vector<std::string> labels =
    {
        "New journey",
        "Show game summary",
        "Write memorial file",
        "View high scores",
        "View message log",
        "Return to main menu",
        "Quit the game"
    };

    for (size_t i = 0; i < labels.size(); ++i)
    {
        const std::string& label = labels[i];

        const Clr& clr =
            browser_.is_at_idx(i) ?
            clr_menu_highlight :
            clr_menu_drk;

        io::draw_text(label,
                      Panel::screen,
                      pos,
                      clr);

        ++pos.y;
    }

    if (config::is_tiles_mode())
    {
        io::draw_skull(P(24, 1));
    }

    io::draw_box(R(0, 0, screen_w - 1, screen_h - 1));
}

void PostmortemMenu::mk_memorial_file() const
{
    const std::string time_stamp =
        game::start_time().time_str(TimeType::second, false);

    const std::string file_name =
        map::player->name_a() + "_" + time_stamp + ".txt";

    const std::string file_path = "res/data/" + file_name;

    // Write memorial file
    std::ofstream file;
    file.open(file_path.data(), std::ios::trunc);

    // Add info lines to file
    for (const StrAndClr& line : info_lines_)
    {
        file << line.str << std::endl;
    }

    // Add text map to file
    for (int y = 0; y < map_h; ++y)
    {
        std::string map_line;

        for (int x = 0; x < map_w; ++x)
        {
            char c = game::render_array[x][y].glyph;

            // Printable ASCII character?
            if (c < 32 || c > 126)
            {
                // Nope - well, this is pretty difficult to handle in a good way
                // as things are now. The current strategy is to show the symbol
                // as a wall then (it's probably a wall, rubble, or a statue).
                //
                // TODO: Perhaps text mode should strictly use only printable
                //       basic ASCII symbols?
                //
                c = '#';
            }

            map_line += c;
        }

        file << map_line << std::endl;
    }

    file.close();

    io::draw_text("Wrote: " + file_path + any_key_info_str,
                  Panel::screen,
                  P(1, 1),
                  clr_white_lgt);

    io::update_screen();

    query::wait_for_key_press();
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

        //
        // Display postmortem info
        //
        switch (browser_.y())
        {
        case 0:
        {
            //
            // Exit screen
            //
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

        //
        // Store memorial file
        //
        case 2:
        {
            mk_memorial_file();
        }
        break;

        //
        // Show highscores
        //
        case 3:
        {
            std::unique_ptr<State> browse_highscore_state(new BrowseHighscore);

            states::push(std::move(browse_highscore_state));
        }
        break;

        //
        // Display message history
        //
        case 4:
        {
            std::unique_ptr<State> msg_history_state(new MsgHistoryState);

            states::push(std::move(msg_history_state));
        }
        break;

        //
        // Return to main menu
        //
        case 5:
        {
            //
            // Exit screen
            //
            states::pop();

            return;
        }
        break;

        //
        // Quit game
        //
        case 6:
        {
            //
            // Bye!
            //
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
                          info_lines_[i].clr);
        }
        else // Map lines
        {
            const int map_y = i - nr_info_lines;

            ASSERT(map_y >= 0);
            ASSERT(map_y < map_h);

            if (is_tiles_mode)
            {
                for (int x = 0; x < map_w; ++x)
                {
                    const CellRenderData& d = game::render_array[x][map_y];

                    io::draw_tile(d.tile,
                                  Panel::screen,
                                  P(x, screen_y),
                                  d.clr,
                                  d.clr_bg);
                }
            }
            else // Text mode
            {
                for (int x = 0; x < map_w; ++x)
                {
                    const CellRenderData& d = game::render_array[x][map_y];

                    io::draw_glyph(d.glyph,
                                   Panel::screen,
                                   P(x, screen_y),
                                   d.clr,
                                   true,
                                   d.clr_bg);
                }
            }
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
        //
        // Exit screen
        //
        states::pop();
        break;
    }
}
