#include "browser.hpp"

#include "io.hpp"
#include "config.hpp"

MenuBrowser::MenuBrowser(const int nr_items, const int list_h) :
    nr_items_       (0),
    y_              (0),
    list_h_         (-1),
    range_shown_    (Range(-1, -1))
{
    reset(nr_items, list_h);
}

MenuBrowser::MenuBrowser() :
    nr_items_       (0),
    y_              (0),
    list_h_         (-1),
    range_shown_    (Range(-1, -1))
{

}

MenuAction MenuBrowser::read(const InputData& input,
                             MenuInputMode mode)
{
    if (input.key == SDLK_UP    ||
        input.key == '8'        ||
        (mode == MenuInputMode::scrolling && input.key == 'k'))
    {
        move(VerDir::up);
        return MenuAction::moved;
    }

    if (input.key == SDLK_DOWN  ||
        input.key == '2'        ||
        (mode == MenuInputMode::scrolling && input.key == 'j'))
    {
        move(VerDir::down);
        return MenuAction::moved;
    }

    if (input.key == SDLK_PAGEUP)
    {
        move_page(VerDir::up);
        return MenuAction::moved;
    }

    if (input.key == SDLK_PAGEDOWN)
    {
        move_page(VerDir::down);
        return MenuAction::moved;
    }

    if (mode == MenuInputMode::scrolling_and_letters)
    {
        bool is_shift_held  = false;
        char c              = input.key;

        const bool is_lower_case_letter = c >= 'a' && c <= 'z';
        const bool is_upper_case_letter = c >= 'A' && c <= 'Z';

        //First, if this is an upper case letter, convert to lower case
        if (is_upper_case_letter)
        {
            is_shift_held   = true;
            c               = c - 'A' + 'a';
        }

        //Is a letter between a and z pressed?
        const char last_letter = 'a' + nr_items_shown() - 1;

        if (c >= 'a' && c <= last_letter)
        {
            const int top = top_idx_shown();
            const int idx = top + c - 'a';

            set_y(idx);

            return is_shift_held ?
                MenuAction::selected_shift :
                MenuAction::selected;
        }

        // If this is a letter (lower or upper case), we don't want to handle
        // input in any other way for this keypress (if it was outside the
        // range of indexes, then nothing should happen)
        if (is_lower_case_letter || is_upper_case_letter)
        {
            return MenuAction::none;
        }
    }

    if (input.key == SDLK_RETURN)
    {
        return input.is_shift_held ?
            MenuAction::selected_shift :
            MenuAction::selected;
    }

    if (input.key == SDLK_SPACE)
    {
        return MenuAction::space;
    }

    if (input.key == SDLK_ESCAPE)
    {
        return MenuAction::esc;
    }

    return MenuAction::none;
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
    // Shown ranged defined?
    if (list_h_ >= 0)
    {
        return range_shown_;
    }
    else // List height undefined (i.e. showing all)
    {
        // Just return a range of the total number of items
        return Range(0, nr_items_ - 1);
    }
}

void MenuBrowser::update_range_shown()
{
    // Shown ranged defined?
    if (list_h_ >= 0)
    {
        const int top = (y_ / list_h_) * list_h_;
        const int btm = std::min(top + list_h_, nr_items_) - 1;

        range_shown_.set(top, btm);
    }
}

void MenuBrowser::set_y_nearest_valid()
{
    set_constr_in_range(0, y_, nr_items_ - 1);
}

int MenuBrowser::nr_items_shown() const
{
    // Shown ranged defined?
    if (list_h_ >= 0)
    {
        return range_shown_.len();
    }
    else // List height undefined (i.e. showing all)
    {
        // Just return total number of items
        return nr_items_;
    }
}

int MenuBrowser::top_idx_shown() const
{
    // Shown ranged defined?
    if (list_h_ >= 0)
    {
        return range_shown_.min; // List height undefined (i.e. showing all)
    }
    else // Not showing all items
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

void MenuBrowser::reset(const int nr_items, const int list_h)
{
    nr_items_   = nr_items;
    list_h_     = list_h;

    set_y_nearest_valid();

    update_range_shown();
}
