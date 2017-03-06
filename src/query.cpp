#include "query.hpp"

#include <iostream>

#include "config.hpp"
#include "io.hpp"

namespace query
{

namespace
{

bool is_inited_ = false;

} // namespace

void init()
{
    is_inited_ = true;
}

void wait_for_key_press()
{
    if (is_inited_ && !config::is_bot_playing())
    {
        io::update_screen();

        io::get(true);
    }
}

BinaryAnswer yes_or_no(char key_for_special_event)
{
    if (!is_inited_ || config::is_bot_playing())
    {
        return BinaryAnswer::yes;
    }

    io::update_screen();

    auto d = io::get(false);

    while (d.key != 'y' &&
           d.key != 'n' &&
           d.key != SDLK_ESCAPE &&
           d.key != SDLK_SPACE &&
           (d.key != key_for_special_event || key_for_special_event == -1))
    {
        d = io::get(false);
    }

    if (d.key == key_for_special_event &&
        key_for_special_event != -1)
    {
        return BinaryAnswer::special;
    }

    if (d.key == 'y')
    {
        return BinaryAnswer::yes;
    }

    return BinaryAnswer::no;
}

InputData letter(const bool accept_enter)
{
    if (!is_inited_ || config::is_bot_playing())
    {
        return 'a';
    }

    io::update_screen();

    while (true)
    {
        const auto d = io::get(false);

        if ((accept_enter && d.key == SDLK_RETURN) ||
            d.key == SDLK_ESCAPE ||
            d.key == SDLK_SPACE ||
            (d.key >= 'a' && d.key <= 'z') ||
            (d.key >= 'A' && d.key <= 'Z'))
        {
            return d;
        }
    }

    return InputData();
}

int number(const P& pos,
           const Clr clr,
           const int min,
           const int max_nr_digits,
           const int default_value,
           const bool cancel_returns_default)
{
    if (!is_inited_ || config::is_bot_playing())
    {
        return 0;
    }

    int ret_num = std::max(min, default_value);

    io::cover_area(Panel::screen,
                       pos,
                       P(max_nr_digits + 1, 1));

    const std::string str = (ret_num == 0 ? "" : std::to_string(ret_num)) + "_";

    io::draw_text(str, Panel::screen, pos, clr);

    io::update_screen();

    while (true)
    {
        InputData d;

        while ((d.key < '0' || d.key > '9') &&
               d.key != SDLK_RETURN &&
               d.key != SDLK_SPACE &&
               d.key != SDLK_ESCAPE &&
               d.key != SDLK_BACKSPACE)
        {
            d = io::get(true);
        }

        if (d.key == SDLK_RETURN)
        {
            return std::max(min, ret_num);
        }

        if (d.key == SDLK_SPACE || d.key == SDLK_ESCAPE)
        {
            return cancel_returns_default ? default_value : -1;
        }

        const std::string ret_num_str = std::to_string(ret_num);

        const int current_num_digits = ret_num_str.size();

        if (d.key == SDLK_BACKSPACE)
        {
            ret_num = ret_num / 10;

            io::cover_area(Panel::screen,
                               pos,
                               P(max_nr_digits + 1, 1));

            io::draw_text((ret_num == 0 ? "" : std::to_string(ret_num)) + "_",
                              Panel::screen,
                              pos,
                              clr);

            io::update_screen();
            continue;
        }

        if (current_num_digits < max_nr_digits)
        {
            int current_digit = d.key - '0';

            ret_num = std::max(min, ret_num * 10 + current_digit);

            io::cover_area(Panel::screen,
                               pos,
                               P(max_nr_digits + 1, 1));

            io::draw_text((ret_num == 0 ? "" : std::to_string(ret_num)) + "_",
                              Panel::screen,
                              pos,
                              clr);

            io::update_screen();
        }
    }

    return -1;
}

void wait_for_msg_more()
{
    if (!is_inited_ || config::is_bot_playing())
    {
        return;
    }

    io::update_screen();

    // Determine criteria for confirming more prompt (decided by config)
    if (config::is_any_key_confirm_more())
    {
        wait_for_key_press();
    }
    else // Only some keys confirm more prompts
    {
        while (true)
        {
            const auto d = io::get(true);

            if (d.key == SDLK_SPACE ||
                d.key == SDLK_ESCAPE ||
                d.key == SDLK_RETURN ||
                d.key == SDLK_TAB)
            {
                break;
            }
        }
    }
}

void wait_for_confirm()
{
    if (!is_inited_ || config::is_bot_playing())
    {
        return;
    }

    io::update_screen();

    while (true)
    {
        const auto d = io::get(true);

        if (d.key == SDLK_SPACE ||
            d.key == SDLK_ESCAPE ||
            d.key == SDLK_RETURN)
        {
            break;
        }
    }
}

Dir dir(const AllowCenter allow_center)
{
    if (!is_inited_ || config::is_bot_playing())
    {
        return Dir::END;
    }

    io::update_screen();

    while (true)
    {
        const auto d = io::get(false);

        if (d.key == SDLK_SPACE || d.key == SDLK_ESCAPE)
        {
            return Dir::END;
        }

        if (d.key == SDLK_RIGHT || d.key == '6' || d.key == 'l')
        {
            if (d.is_shift_held)
            {
                return Dir::up_right;
            }
            else if (d.is_ctrl_held)
            {
                return Dir::down_right;
            }
            else
            {
                return Dir::right;
            }
        }

        if (d.key == SDLK_PAGEUP || d.key == '9' || d.key == 'u')
        {
            return Dir::up_right;
        }

        if (d.key == SDLK_UP || d.key == '8' || d.key == 'k')
        {
            return Dir::up;
        }

        if (d.key == SDLK_END || d.key == '7' || d.key == 'y')
        {
            return Dir::up_left;
        }

        if (d.key == SDLK_LEFT || d.key == '4' || d.key == 'h')
        {
            if (d.is_shift_held)
            {
                return Dir::up_left;
            }
            else if (d.is_ctrl_held)
            {
                return Dir::down_left;
            }
            else
            {
                return Dir::left;
            }
        }

        if (d.key == SDLK_END || d.key == '1' || d.key == 'b')
        {
            return Dir::down_left;
        }

        if (d.key == SDLK_DOWN || d.key == '2' || d.key == 'j')
        {
            return Dir::down;
        }

        if (d.key == SDLK_PAGEDOWN || d.key == '3' || d.key == 'n')
        {
            return Dir::down_right;
        }

        if (allow_center == AllowCenter::yes && (d.key == '5' || d.key == '.'))
        {
            return Dir::center;
        }
    }

    // Unreachable
    return Dir::END;
}

} // query
