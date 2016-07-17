#include "query.hpp"

#include <iostream>

#include "config.hpp"
#include "input.hpp"
#include "render.hpp"

namespace query
{

namespace
{

bool is_inited_ = false;

} //namespace

void init()
{
    is_inited_ = true;
}

void wait_for_key_press()
{
    if (is_inited_ && !config::is_bot_playing())
    {
        input::input();
    }
}

YesNoAnswer yes_or_no(char key_for_special_event)
{
    if (!is_inited_ || config::is_bot_playing())
    {
        return YesNoAnswer::yes;
    }

    KeyData d = input::input();

    while (
        d.key    != 'y'             &&
        d.key    != 'n'             &&
        d.sdl_key != SDLK_ESCAPE    &&
        d.sdl_key != SDLK_SPACE     &&
        (d.key != key_for_special_event || key_for_special_event == -1))
    {
        d = input::input();
    }

    if (d.key == key_for_special_event && key_for_special_event != -1)
    {
        return YesNoAnswer::special;
    }

    if (d.key == 'y')
    {
        return YesNoAnswer::yes;
    }

    return YesNoAnswer::no;
}

KeyData letter(const bool accept_enter)
{
    if (!is_inited_ || config::is_bot_playing())
    {
        return 'a';
    }

    while (true)
    {
        KeyData d = input::input();

        if (
            (accept_enter && d.sdl_key == SDLK_RETURN) ||
            d.sdl_key == SDLK_ESCAPE ||
            d.sdl_key == SDLK_SPACE  ||
            (d.key >= 'a' && d.key <= 'z') ||
            (d.key >= 'A' && d.key <= 'Z'))
        {
            return d;
        }
    }

    return KeyData();
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

    render::cover_area(Panel::screen,
                       pos,
                       P(max_nr_digits + 1, 1));

    const std::string str = (ret_num == 0 ? "" : to_str(ret_num)) + "_";

    render::draw_text(str, Panel::screen, pos, clr);

    render::update_screen();

    while (true)
    {
        KeyData d;

        while (
            (d.key < '0' || d.key > '9')    &&
            d.sdl_key != SDLK_RETURN        &&
            d.sdl_key != SDLK_SPACE         &&
            d.sdl_key != SDLK_ESCAPE        &&
            d.sdl_key != SDLK_BACKSPACE)
        {
            d = input::input();
        }

        if (d.sdl_key == SDLK_RETURN)
        {
            return std::max(min, ret_num);
        }

        if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            return cancel_returns_default ? default_value : -1;
        }

        const std::string ret_num_str = to_str(ret_num);

        const int cur_num_digits = ret_num_str.size();

        if (d.sdl_key == SDLK_BACKSPACE)
        {
            ret_num = ret_num / 10;

            render::cover_area(Panel::screen,
                               pos,
                               P(max_nr_digits + 1, 1));

            render::draw_text((ret_num == 0 ? "" : to_str(ret_num)) + "_",
                              Panel::screen,
                              pos,
                              clr);

            render::update_screen();
            continue;
        }

        if (cur_num_digits < max_nr_digits)
        {
            int cur_digit = d.key - '0';

            ret_num = std::max(min, ret_num * 10 + cur_digit);

            render::cover_area(Panel::screen,
                               pos,
                               P(max_nr_digits + 1, 1));

            render::draw_text((ret_num == 0 ? "" : to_str(ret_num)) + "_",
                              Panel::screen,
                              pos,
                              clr);

            render::update_screen();
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

    //Determine criteria for confirming more prompt (decided by config)
    if (config::is_any_key_confirm_more())
    {
        wait_for_key_press();
    }
    else //Only some keys confirm more prompts
    {
        while (true)
        {
            const KeyData d = input::input();

            if (
                d.sdl_key == SDLK_SPACE     ||
                d.sdl_key == SDLK_ESCAPE    ||
                d.sdl_key == SDLK_RETURN    ||
                d.sdl_key == SDLK_TAB)
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

    while (true)
    {
        const KeyData d = input::input();

        if (
            d.sdl_key == SDLK_SPACE     ||
            d.sdl_key == SDLK_ESCAPE    ||
            d.sdl_key == SDLK_RETURN)
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

    while (true)
    {
        const KeyData d = input::input();

        if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            return Dir::END;
        }

        if (d.sdl_key == SDLK_RIGHT || d.key == '6' || d.key == 'l')
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

        if (d.sdl_key == SDLK_PAGEUP || d.key == '9' || d.key == 'u')
        {
            return Dir::up_right;
        }

        if (d.sdl_key == SDLK_UP || d.key == '8' || d.key == 'k')
        {
            return Dir::up;
        }

        if (d.sdl_key == SDLK_END || d.key == '7' || d.key == 'y')
        {
            return Dir::up_left;
        }

        if (d.sdl_key == SDLK_LEFT || d.key == '4' || d.key == 'h')
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

        if (d.sdl_key == SDLK_END || d.key == '1' || d.key == 'b')
        {
            return Dir::down_left;
        }

        if (d.sdl_key == SDLK_DOWN || d.key == '2' || d.key == 'j')
        {
            return Dir::down;
        }

        if (d.sdl_key == SDLK_PAGEDOWN || d.key == '3' || d.key == 'n')
        {
            return Dir::down_right;
        }

        if (allow_center == AllowCenter::yes && (d.key == '5' || d.key == '.'))
        {
            return Dir::center;
        }
    }

    //Unreachable
    return Dir::END;
}

} //query
