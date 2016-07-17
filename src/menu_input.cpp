#include "menu_input.hpp"

#include "input.hpp"
#include "config.hpp"

//---------------------------------------------------------- MENU BROWSER
MenuBrowser::MenuBrowser(const int nr_items, const int list_h) :
    nr_items_       (nr_items),
    y_              (0),
    list_h_         (list_h),
    range_shown_    (Range(-1, -1))
{
    set_y_nearest_valid();

    update_range_shown();
}

void MenuBrowser::move(const VerDir dir)
{
    const int last_idx = nr_items_ - 1;

    if (dir == VerDir::up)
    {
        y_ = y_ == 0 ? last_idx : (y_ - 1);
    }
    else //Down
    {
        y_ = y_ == last_idx ? 0 : (y_ + 1);
    }

    update_range_shown();
}

void MenuBrowser::move_page(const VerDir dir)
{
    if (dir == VerDir::up)
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

void MenuBrowser::set_y(const int y)
{
    y_ = y;

    set_y_nearest_valid();

    update_range_shown();
}

Range MenuBrowser::range_shown() const
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

void MenuBrowser::update_range_shown()
{
    //Shown ranged defined?
    if (list_h_ >= 0)
    {
        const int top_idx_shown = (y_ / list_h_) * list_h_;
        const int btm_idx_shown = std::min(top_idx_shown + list_h_, nr_items_) - 1;

        range_shown_.set(top_idx_shown, btm_idx_shown);
    }
}

void MenuBrowser::set_y_nearest_valid()
{
    set_constr_in_range(0, y_, nr_items_ - 1);
}

int MenuBrowser::nr_items_shown() const
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

int MenuBrowser::top_idx_shown() const
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

int MenuBrowser::btm_idx_shown() const
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

bool MenuBrowser::is_on_top_page() const
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

bool MenuBrowser::is_on_btm_page() const
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

MenuAction action(MenuBrowser& browser, MenuInputMode mode)
{
    while (true)
    {
        KeyData d = input::input();

        if (
            d.sdl_key == SDLK_UP    ||
            d.key == '8'            ||
            (mode == MenuInputMode::scroll && d.key == 'k'))
        {
            browser.move(VerDir::up);
            return MenuAction::moved;
        }

        if (
            d.sdl_key == SDLK_DOWN  ||
            d.key == '2'            ||
            (mode == MenuInputMode::scroll && d.key == 'j'))
        {
            browser.move(VerDir::down);
            return MenuAction::moved;
        }

        if (d.sdl_key == SDLK_PAGEUP)
        {
            browser.move_page(VerDir::up);
            return MenuAction::moved;
        }

        if (d.sdl_key == SDLK_PAGEDOWN)
        {
            browser.move_page(VerDir::down);
            return MenuAction::moved;
        }

        if (mode == MenuInputMode::scroll_and_letters)
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
                       MenuAction::selected_shift : MenuAction::selected;
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
                   MenuAction::selected_shift : MenuAction::selected;
        }

        if (d.sdl_key == SDLK_SPACE)
        {
            return MenuAction::space;
        }

        if (d.sdl_key == SDLK_ESCAPE)
        {
            return MenuAction::esc;
        }
    }

    return MenuAction::esc;
}

} //menu_input
