#include "query.hpp"

#include <iostream>

#include "config.hpp"
#include "input.hpp"
#include "render.hpp"
#include "converters.hpp"

using namespace std;

namespace Query
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
    if (is_inited_ && !Config::is_bot_playing())
    {
        Input::get_input();
    }
}

Yes_no_answer yes_or_no(char key_for_special_event)
{
    if (!is_inited_ || Config::is_bot_playing()) {return Yes_no_answer::yes;}

    Key_data d = Input::get_input();
    while (
        d.key    != 'y'          &&
        d.key    != 'n'          &&
        d.sdl_key != SDLK_ESCAPE  &&
        d.sdl_key != SDLK_SPACE   &&
        (d.key != key_for_special_event || key_for_special_event == -1))
    {
        d = Input::get_input();
    }
    if (d.key == key_for_special_event && key_for_special_event != -1)
    {
        return Yes_no_answer::special;
    }
    if (d.key == 'y')
    {
        return Yes_no_answer::yes;
    }

    return Yes_no_answer::no;
}

Key_data letter(const bool ACCEPT_ENTER)
{
    if (!is_inited_ || Config::is_bot_playing()) {return 'a';}

    while (true)
    {
        Key_data d = Input::get_input();

        if (
            (ACCEPT_ENTER && d.sdl_key == SDLK_RETURN) ||
            d.sdl_key == SDLK_ESCAPE ||
            d.sdl_key == SDLK_SPACE  ||
            (d.key >= 'a' && d.key <= 'z') || (d.key >= 'A' && d.key <= 'Z'))
        {
            return d;
        }
    }

    return Key_data();
}

int number(const Pos& pos, const Clr clr, const int MIN, const int MAX_NR_DIGITS,
           const int DEFAULT, const bool CANCEL_RETURNS_DEFAULT)
{
    if (!is_inited_ || Config::is_bot_playing()) {return 0;}

    int ret_num = max(MIN, DEFAULT);
    Render::cover_area(Panel::screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
    const string str = (ret_num == 0 ? "" : to_str(ret_num)) + "_";
    Render::draw_text(str, Panel::screen, pos, clr);
    Render::update_screen();

    while (true)
    {
        Key_data d;
        while ((d.key < '0' || d.key > '9') && d.sdl_key != SDLK_RETURN &&
                d.sdl_key != SDLK_SPACE && d.sdl_key != SDLK_ESCAPE &&
                d.sdl_key != SDLK_BACKSPACE)
        {
            d = Input::get_input();
        }

        if (d.sdl_key == SDLK_RETURN)
        {
            return max(MIN, ret_num);
        }

        if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            return CANCEL_RETURNS_DEFAULT ? DEFAULT : -1;
        }

        const string ret_num_str = to_str(ret_num);
        const int CUR_NUM_DIGITS = ret_num_str.size();

        if (d.sdl_key == SDLK_BACKSPACE)
        {
            ret_num = ret_num / 10;
            Render::cover_area(Panel::screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
            Render::draw_text((ret_num == 0 ? "" : to_str(ret_num)) + "_",
                             Panel::screen, pos, clr);
            Render::update_screen();
            continue;
        }

        if (CUR_NUM_DIGITS < MAX_NR_DIGITS)
        {
            int cur_digit = d.key - '0';
            ret_num = max(MIN, ret_num * 10 + cur_digit);
            Render::cover_area(Panel::screen, pos, Pos(MAX_NR_DIGITS + 1, 1));
            Render::draw_text((ret_num == 0 ? "" : to_str(ret_num)) + "_",
                             Panel::screen, pos, clr);
            Render::update_screen();
        }
    }
    return -1;
}

void wait_for_confirm()
{
    if (is_inited_ && !Config::is_bot_playing())
    {
        Key_data d = Input::get_input();
        while (d.sdl_key != SDLK_SPACE && d.sdl_key != SDLK_ESCAPE && d.sdl_key != SDLK_RETURN)
        {
            d = Input::get_input();
        }
    }
}

Dir dir()
{
    if (!is_inited_ || Config::is_bot_playing())
    {
        return Dir::END;
    }

    Key_data d = Input::get_input();

    while (d.sdl_key != SDLK_RIGHT   && d.sdl_key != SDLK_UP       &&
            d.sdl_key != SDLK_LEFT    && d.sdl_key != SDLK_DOWN     &&
            d.sdl_key != SDLK_ESCAPE  && d.sdl_key != SDLK_SPACE    &&
            d.sdl_key != SDLK_PAGEUP  && d.sdl_key != SDLK_HOME     &&
            d.sdl_key != SDLK_END     && d.sdl_key != SDLK_PAGEDOWN &&
            d.key != 'h' && d.key != 'j' && d.key != 'k' && d.key != 'l' &&
            d.key != 'y' && d.key != 'u' && d.key != 'b' && d.key != 'n' &&
            (d.key < '1' || d.key > '9' || d.key == '5'))
    {
        d = Input::get_input();
    }

    if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
    {
        return Dir::center;
    }
    if (d.sdl_key == SDLK_RIGHT    || d.key == '6' || d.key == 'l')
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
    if (d.sdl_key == SDLK_PAGEUP   || d.key == '9' || d.key == 'u')
    {
        return Dir::up_right;
    }
    if (d.sdl_key == SDLK_UP       || d.key == '8' || d.key == 'k')
    {
        return Dir::up;
    }
    if (d.sdl_key == SDLK_END      || d.key == '7' || d.key == 'y')
    {
        return Dir::up_left;
    }
    if (d.sdl_key == SDLK_LEFT     || d.key == '4' || d.key == 'h')
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
    if (d.sdl_key == SDLK_END      || d.key == '1' || d.key == 'b')
    {
        return Dir::down_left;
    }
    if (d.sdl_key == SDLK_DOWN     || d.key == '2' || d.key == 'j')
    {
        return Dir::down;
    }
    if (d.sdl_key == SDLK_PAGEDOWN || d.key == '3' || d.key == 'n')
    {
        return Dir::down_right;
    }

    return Dir::center;
}

} //Query
