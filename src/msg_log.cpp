#include "msg_log.hpp"

#include <vector>
#include <string>

#include "init.hpp"
#include "io.hpp"
#include "query.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "text_format.hpp"

namespace msg_log
{

namespace
{

std::vector<Msg> lines_[2];

const size_t history_capacity_ = 200;

size_t history_size_ = 0;

size_t history_count_ = 0;

std::vector<Msg> history_[history_capacity_];

const std::string more_str = "-More-";

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

// Used by normal log and history viewer
void draw_line(const std::vector<Msg>& line, const int y_pos)
{
    for (const Msg& msg : line)
    {
        std::string str = "";

        msg.str_with_repeats(str);

        io::draw_text(str,
                      Panel::log,
                      P(msg.x_pos_, y_pos),
                      msg.clr_);
    }
}

} // namespace

void init()
{
    for (std::vector<Msg>& line : lines_)
    {
        line.clear();
    }

    history_size_   = 0;
    history_count_  = 0;
}

void draw()
{
    const int nr_lines_with_content =
        lines_[0].empty() ? 0 :
        lines_[1].empty() ? 1 : 2;

    if (nr_lines_with_content > 0)
    {
        io::cover_area(Panel::log,
                       P(0, 0),
                       P(map_w, nr_lines_with_content));

        for (int i = 0; i < nr_lines_with_content; ++i)
        {
            draw_line(lines_[i], i);
        }
    }
}

void clear()
{
    for (std::vector<Msg>& line : lines_)
    {
        if (!line.empty())
        {
            // Add cleared line to history
            history_[history_count_ % history_capacity_] = line;

            ++history_count_;

            if (history_size_ < history_capacity_)
            {
                ++history_size_;
            }

            line.clear();
        }
    }
}

void add(const std::string& str,
         const Clr& clr,
         const bool interrupt_all_player_actions,
         const MorePromptOnMsg add_more_prompt_on_msg)
{
    ASSERT(!str.empty());

#ifndef NDEBUG
    if (str[0] == ' ')
    {
        TRACE << "Message starts with space: \"" << str << "\"" << std::endl;
        ASSERT(false);
    }
#endif

    int current_line_nr = lines_[1].empty() ? 0 : 1;

    Msg* prev_msg = nullptr;

    if (!lines_[current_line_nr].empty())
    {
        prev_msg = &lines_[current_line_nr].back();
    }

    bool is_repeated = false;

    // Check if message is identical to previous
    if (add_more_prompt_on_msg == MorePromptOnMsg::no &&
        prev_msg)
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
        const int repeat_str_len = 4;

        const int padding_len =
            repeat_str_len +
            ((current_line_nr == 0) ? 0 :
             (more_str.size() + 1));

        int x_pos = x_after_msg(prev_msg);

        const bool is_msg_fit =
            (x_pos + (int)str.size() + padding_len - 1) < map_w;

        if (!is_msg_fit)
        {
            if (current_line_nr == 0)
            {
                current_line_nr = 1;
            }
            else // Current line number is not zero
            {
                more_prompt();

                current_line_nr = 0;
            }

            x_pos = 0;
        }

        lines_[current_line_nr].push_back(Msg(str, clr, x_pos));
    }

    io::clear_screen();

    states::draw();

    if (add_more_prompt_on_msg == MorePromptOnMsg::yes)
    {
        more_prompt();
    }

    // Messages may stop long actions like first aid and quick walk
    if (interrupt_all_player_actions)
    {
        map::player->interrupt_actions();
    }

    // Some actions are always interrupted by messages, regardless of the
    // "interrupt_all_player_actions" parameter
    map::player->on_log_msg_printed();
}

void more_prompt()
{
    // If the current log is empty, do nothing
    if (lines_[0].empty())
    {
        return;
    }

    states::draw();

    draw();

    int x_pos = 0;

    int line_nr =
        lines_[1].empty() ?
        0 : 1;

    if (!lines_[line_nr].empty())
    {
        Msg* const last_msg = &lines_[line_nr].back();

        x_pos = x_after_msg(last_msg);

        if (line_nr == 0)
        {
            if ((x_pos + (int)more_str.size() - 1) >= map_w)
            {
                x_pos = 0;
                line_nr  = 1;
            }
        }
    }

    io::draw_text(more_str,
                  Panel::log,
                  P(x_pos, line_nr),
                  clr_black,
                  clr_gray);

    query::wait_for_msg_more();

    clear();
}

void add_line_to_history(const std::string& line_to_add)
{
    std::vector<Msg> history_line;

    const Msg msg(line_to_add,
                  clr_white,
                  0);

    history_[history_count_ % history_capacity_] = {msg};

    ++history_count_;

    if (history_size_ < history_capacity_)
    {
        ++history_size_;
    }
}

const std::vector< std::vector<Msg> > history()
{
    std::vector< std::vector<Msg> > ret;

    ret.reserve(history_size_);

    size_t start = 0;

    if (history_count_ >= history_capacity_)
    {
        start = history_count_ - history_capacity_;
    }

    for (size_t i = start; i < history_count_; ++i)
    {
        const auto& line = history_[i % history_capacity_];

        ret.push_back(line);
    }

    return ret;
}

} // msg_log

// -----------------------------------------------------------------------------
// Message history state
// -----------------------------------------------------------------------------
states::StateId MsgHistoryState::id()
{
    return states::StateId::MESSAGE_STATE;
}

void MsgHistoryState::on_start()
{
    history_ = msg_log::history();

    const int history_size = history_.size();

    top_line_nr_ = history_size - max_nr_lines_on_scr_;
    top_line_nr_ = std::max(0, top_line_nr_);

    btm_line_nr_ = top_line_nr_ + max_nr_lines_on_scr_ - 1;
    btm_line_nr_ = std::min(history_size - 1, btm_line_nr_);
}

void MsgHistoryState::draw()
{
    std::string title = "";

    if (history_.empty())
    {
        title = "No message history";
    }
    else // History has content
    {
        const std::string msg_nr_str_first  = std::to_string(top_line_nr_ + 1);
        const std::string msg_nr_str_last   = std::to_string(btm_line_nr_ + 1);

        title =
            "Messages " +
            msg_nr_str_first + "-" +
            msg_nr_str_last +
            " of " + std::to_string(history_.size());
    }

    io::draw_info_scr_interface(title,
                                InfScreenType::scrolling);

    int y = 1;

    for (int i = top_line_nr_; i <= btm_line_nr_; ++i)
    {
        const auto& line = history_[i];

        msg_log::draw_line(line, y);

        ++y;
    }
}

void MsgHistoryState::update()
{
    const int line_jump = 3;

    const int max_nr_lines_on_scr = screen_h - 2;

    const int history_size = history_.size();

    const auto input = io::get(false);

    switch (input.key)
    {
    case SDLK_DOWN:
    case '2':
    case 'j':
    {
        top_line_nr_ += line_jump;

        const int top_nr_max =
            std::max(0, history_size - max_nr_lines_on_scr);

        top_line_nr_ =
            std::min(top_nr_max, top_line_nr_);
    }
    break;

    case SDLK_UP:
    case '8':
    case 'k':
    {
        top_line_nr_ =
            std::max(0, top_line_nr_ - line_jump);
    }
    break;

    case SDLK_SPACE:
    case SDLK_ESCAPE:
    {
        states::pop();

        return;
    }
    break;

    default:
        break;
    }

    btm_line_nr_ = std::min(top_line_nr_ + max_nr_lines_on_scr - 1,
                            history_size - 1);
}
