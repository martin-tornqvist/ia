#include "msg_log.hpp"

#include <vector>
#include <string>

#include "init.hpp"
#include "input.hpp"
#include "render.hpp"
#include "query.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "text_format.hpp"

namespace msg_log
{

namespace
{

std::vector<Msg>                lines_[2];
std::vector< std::vector<Msg> > history_;
const std::string               more_str = "-More-";

int x_after_msg(const Msg* const msg)
{
    if (!msg)
    {
        return 0;
    }

    std::string str = "";
    msg->str_with_repeats(str);
    return msg->x_pos_ + str.size() + 1;
}

//Used by normal log and history viewer
void draw_line(const std::vector<Msg>& line_to_draw, const int Y_POS)
{
    for (const Msg& msg : line_to_draw)
    {
        std::string str = "";

        msg.str_with_repeats(str);

        render::draw_text(str,
                          Panel::log,
                          P(msg.x_pos_, Y_POS),
                          msg.clr_);
    }
}

} //namespace

void init()
{
    for (std::vector<Msg>& line : lines_)
    {
        line.clear();
    }

    history_.clear();
}

void clear()
{
    for (std::vector<Msg>& line : lines_)
    {
        if (!line.empty())
        {
            history_.push_back(line);

            while (history_.size() > 300)
            {
                history_.erase(history_.begin());
            }

            line.clear();
        }
    }
}

void draw(const Update_screen update)
{
    const int NR_LINES_WITH_CONTENT = lines_[0].empty() ? 0 :
                                      lines_[1].empty() ? 1 : 2;

    if (NR_LINES_WITH_CONTENT > 0)
    {
        render::cover_area(Panel::log,
                           P(0, 0),
                           P(MAP_W, NR_LINES_WITH_CONTENT));

        for (int i = 0; i < NR_LINES_WITH_CONTENT; ++i)
        {
            draw_line(lines_[i], i);
        }
    }

    if (update == Update_screen::yes)
    {
        render::update_screen();
    }
}

void add(const std::string& str,
         const Clr& clr,
         const bool INTERRUPT_PLAYER_ACTIONS,
         const More_prompt_on_msg add_more_prompt_on_msg)
{
    ASSERT(!str.empty());

#ifndef NDEBUG

    if (str[0] == ' ')
    {
        TRACE << "Message starts with space: \"" << str << "\"" << std::endl;
        ASSERT(false);
    }

#endif

    //If frenzied, change message
    if (map::player->has_prop(Prop_id::frenzied))
    {
        std::string frenzied_str = str;

        bool has_lower_case = false;

        for (auto c : frenzied_str)
        {
            if (c >= 'a' && c <= 'z')
            {
                has_lower_case = true;
                break;
            }
        }

        const char LAST           = frenzied_str.back();
        bool is_ended_by_punctuation = LAST == '.' || LAST == '!';

        if (has_lower_case && is_ended_by_punctuation)
        {
            //Convert to upper case
            text_format::all_to_upper(frenzied_str);

            //Do not put "!" if string contains "..."
            if (frenzied_str.find("...") == std::string::npos)
            {
                //Change "." to "!" at the end
                if (frenzied_str.back() == '.')
                {
                    frenzied_str.back() = '!';
                }

                //Add some "!"
                frenzied_str += "!!";
            }

            add(frenzied_str,
                clr,
                INTERRUPT_PLAYER_ACTIONS,
                add_more_prompt_on_msg);

            return;
        }
    }

    int cur_line_nr = lines_[1].empty() ? 0 : 1;

    Msg* prev_msg = nullptr;

    if (!lines_[cur_line_nr].empty())
    {
        prev_msg = &lines_[cur_line_nr].back();
    }

    bool is_repeated = false;

    //Check if message is identical to previous
    if (add_more_prompt_on_msg == More_prompt_on_msg::no && prev_msg)
    {
        std::string prev_str = "";
        prev_msg->str_raw(prev_str);

        if (prev_str.compare(str) == 0)
        {
            prev_msg->incr_repeat();
            is_repeated = true;
        }
    }

    if (!is_repeated)
    {
        const int REPEAT_STR_LEN = 4;

        const int PADDING_LEN = REPEAT_STR_LEN +
                                (cur_line_nr == 0 ? 0 : (more_str.size() + 1));

        int x_pos = x_after_msg(prev_msg);

        const bool IS_MSG_FIT = x_pos + int(str.size()) + PADDING_LEN - 1 < MAP_W;

        if (!IS_MSG_FIT)
        {
            if (cur_line_nr == 0)
            {
                cur_line_nr = 1;
            }
            else //Current line number is not zero
            {
                more_prompt();
                cur_line_nr = 0;
            }

            x_pos = 0;
        }

        lines_[cur_line_nr].push_back(Msg(str, clr, x_pos));
    }

    if (add_more_prompt_on_msg == More_prompt_on_msg::yes)
    {
        more_prompt();
    }

    //Messages may stop long actions like first aid and quick walk
    if (INTERRUPT_PLAYER_ACTIONS)
    {
        map::player->interrupt_actions();
    }

    map::player->on_log_msg_printed();
}

void more_prompt()
{
    //If the current log is empty, do nothing
    if (lines_[0].empty())
    {
        return;
    }

    render::draw_map_state(Update_screen::no);

    draw(Update_screen::no);

    int x_pos    = 0;
    int line_nr = lines_[1].empty() ? 0 : 1;

    if (!lines_[line_nr].empty())
    {
        Msg* const last_msg = &lines_[line_nr].back();
        x_pos = x_after_msg(last_msg);

        if (line_nr == 0)
        {
            if (x_pos + int(more_str.size()) - 1 >= MAP_W)
            {
                x_pos    = 0;
                line_nr  = 1;
            }
        }
    }

    render::draw_text(more_str,
                      Panel::log,
                      P(x_pos, line_nr),
                      clr_black,
                      clr_gray);

    render::update_screen();
    query::wait_for_msg_more();
    clear();
}

void display_history()
{
    clear();

    const int LINE_JUMP           = 3;
    const int NR_LINES_TOT        = history_.size();
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

    int top_nr = std::max(0, NR_LINES_TOT - MAX_NR_LINES_ON_SCR);
    int btm_nr = std::min(top_nr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

    while (true)
    {
        render::clear_screen();

        std::string title = "";

        if (history_.empty())
        {
            title = "No message history";
        }
        else //History has content
        {
            const std::string msg_nr_first  = to_str(top_nr + 1);
            const std::string msg_nr_last   = to_str(btm_nr + 1);

            title = "Messages " +
                    msg_nr_first + "-" + msg_nr_last +
                    " of " + to_str(history_.size());
        }

        render::draw_info_scr_interface(title, Inf_screen_type::scrolling);

        int y_pos = 1;

        for (int i = top_nr; i <= btm_nr; ++i)
        {
            draw_line(history_[i], y_pos++);
        }

        render::update_screen();

        const Key_data& d = input::input();

        if (d.key == '2' || d.sdl_key == SDLK_DOWN || d.key == 'j')
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
        else if (d.key == '8' || d.sdl_key == SDLK_UP || d.key == 'k')
        {
            top_nr = std::max(0, top_nr - LINE_JUMP);
        }
        else if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }

        btm_nr = std::min(top_nr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);
    }

    render::draw_map_state();
}

void add_line_to_history(const std::string& line_to_add)
{
    std::vector<Msg> history_line;
    history_line.push_back(Msg(line_to_add, clr_white, 0));
    history_.push_back(history_line);
}

const std::vector< std::vector<Msg> >& history()
{
    return history_;
}

} //msg_log
