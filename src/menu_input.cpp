#include "menu_input.hpp"

#include "input.hpp"
#include "config.hpp"

//---------------------------------------------------------- MENU BROWSER
Menu_browser::Menu_browser(const int nr_items, const int list_h) :
    nr_items_       (nr_items),
    y_              (0),
    list_h_         (list_h),
    range_shown_    (Range(-1, -1))
{
    set_y_nearest_valid();

    update_range_shown();
}

void Menu_browser::move(const Ver_dir dir)
{
    const int last_idx = nr_items_ - 1;

    if (dir == Ver_dir::up)
    {
        y_ = y_ == 0 ? last_idx : (y_ - 1);
    }
    else //Down
    {
        y_ = y_ == last_idx ? 0 : (y_ + 1);
    }

    update_range_shown();
}

void Menu_browser::move_page(const Ver_dir dir)
{
    if (dir == Ver_dir::up)
    {
        if (list_h_ >= 0)
        {
            y_ -= list_h_;
        }
        else //List height undefined (i.e. showing all)
        {
            y_ = 0;
        }
    }
    else //Down
    {
        if (list_h_ >= 0)
        {
            y_ += list_h_;
        }
        else //List height undefined (i.e. showing all)
        {
            y_ = nr_items_ - 1;
        }
    }

    set_y_nearest_valid();

    update_range_shown();
}

void Menu_browser::set_y(const int y)
{
    y_ = y;

    set_y_nearest_valid();

    update_range_shown();
}

Range Menu_browser::range_shown() const
{
    //Shown ranged defined?
    if (list_h_ >= 0)
    {
        return range_shown_;
    }
    else //List height undefined (i.e. showing all)
    {
        //Just return a range of the total number of items
        return Range(0, nr_items_ - 1);
    }
}

void Menu_browser::update_range_shown()
{
    //Shown ranged defined?
    if (list_h_ >= 0)
    {
        const int top_idx_shown = (y_ / list_h_) * list_h_;
        const int btm_idx_shown = std::min(top_idx_shown + list_h_, nr_items_) - 1;

        range_shown_.set(top_idx_shown, btm_idx_shown);
    }
}

void Menu_browser::set_y_nearest_valid()
{
    set_constr_in_range(0, y_, nr_items_ - 1);
}

int Menu_browser::nr_items_shown() const
{
    //Shown ranged defined?
    if (list_h_ >= 0)
    {
        return range_shown_.len();
    }
    else //List height undefined (i.e. showing all)
    {
        //Just return total number of items
        return nr_items_;
    }
}

int Menu_browser::top_idx_shown() const
{
    //Shown ranged defined?
    if (list_h_ >= 0)
    {
        return range_shown_.min; //List height undefined (i.e. showing all)
    }
    else //Not showing all items
    {
        return 0;
    }
}

int Menu_browser::btm_idx_shown() const
{
    //Shown ranged defined?
    if (list_h_ >= 0)
    {
        return range_shown_.max;
    }
    else //List height undefined (i.e. showing all)
    {
        return nr_items_ - 1;
    }
}

bool Menu_browser::is_on_top_page() const
{
    //Shown ranged defined?
    if (list_h_ >= 0)
    {
        return range_shown_.min == 0;
    }
    else //List height undefined (i.e. showing all)
    {
        return true;
    }
}

bool Menu_browser::is_on_btm_page() const
{
    //Shown ranged defined?
    if (list_h_ >= 0)
    {
        return range_shown_.max == nr_items_ - 1;
    }
    else //List height undefined (i.e. showing all)
    {
        return true;
    }
}

//---------------------------------------------------------- MENU INPUT
namespace menu_input
{

Menu_action action(Menu_browser& browser, Menu_input_mode mode)
{
    while (true)
    {
        Key_data d = input::input();

        if (
            d.sdl_key == SDLK_UP    ||
            d.key == '8'            ||
            (mode == Menu_input_mode::scroll && d.key == 'k'))
        {
            browser.move(Ver_dir::up);
            return Menu_action::moved;
        }

        if (
            d.sdl_key == SDLK_DOWN  ||
            d.key == '2'            ||
            (mode == Menu_input_mode::scroll && d.key == 'j'))
        {
            browser.move(Ver_dir::down);
            return Menu_action::moved;
        }

        if (d.sdl_key == SDLK_PAGEUP)
        {
            browser.move_page(Ver_dir::up);
            return Menu_action::moved;
        }

        if (d.sdl_key == SDLK_PAGEDOWN)
        {
            browser.move_page(Ver_dir::down);
            return Menu_action::moved;
        }

        if (mode == Menu_input_mode::scroll_and_letters)
        {
            bool is_shift_held  = false;
            char c              = d.key;

            const bool is_lower_case_letter = c >= 'a' && c <= 'z';
            const bool is_upper_case_letter = c >= 'A' && c <= 'Z';

            //First, if this is an upper case letter, convert to lower case
            if (is_upper_case_letter)
            {
                is_shift_held   = true;
                c               = c - 'A' + 'a';
            }

            //Is a letter between a and z pressed?
            const char last_letter = 'a' + browser.nr_items_shown() - 1;

            if (c >= 'a' && c <= last_letter)
            {
                const int top_idx_shown = browser.top_idx_shown();
                const int idx           = top_idx_shown + c - 'a';

                browser.set_y(idx);

                return is_shift_held ?
                       Menu_action::selected_shift : Menu_action::selected;
            }

            //If this is a letter (lower or upper case), we don't want to handle input in any
            //other way for this keypress (if it was outside the range of indexes, then nothing
            //should happen)
            if (is_lower_case_letter || is_upper_case_letter)
            {
                continue;
            }
        }

        if (d.sdl_key == SDLK_RETURN)
        {
            return d.is_shift_held ?
                   Menu_action::selected_shift : Menu_action::selected;
        }

        if (d.sdl_key == SDLK_SPACE)
        {
            return Menu_action::space;
        }

        if (d.sdl_key == SDLK_ESCAPE)
        {
            return Menu_action::esc;
        }
    }

    return Menu_action::esc;
}

} //menu_input
