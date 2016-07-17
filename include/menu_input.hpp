#ifndef MENU_INPUT_HPP
#define MENU_INPUT_HPP

#include "rl_utils.hpp"
#include "global.hpp"

class MenuBrowser
{
public:
    MenuBrowser(const int nr_items, const int list_h = - 1);

    MenuBrowser() = delete;

    MenuBrowser& operator=(const MenuBrowser&) = default;

    void move(const VerDir dir);

    void move_page(const VerDir dir);

    int y() const
    {
        return y_;
    }

    void set_y(const int y);

    Range range_shown() const;

    int nr_items_shown() const;

    int top_idx_shown() const;

    int btm_idx_shown() const;

    bool is_on_top_page() const;

    bool is_on_btm_page() const;

    int nr_items_tot() const
    {
        return nr_items_;
    }

    bool is_at_idx(const int idx) const
    {
        return y_ == idx;
    }

private:
    void set_y_nearest_valid();

    void update_range_shown();

    int nr_items_;
    int y_;
    int  list_h_;
    Range range_shown_;
};

enum class MenuAction
{
    moved,
    selected,
    selected_shift,
    space,
    esc
};

enum class MenuInputMode
{
    scroll_and_letters,
    scroll
};

namespace menu_input
{

MenuAction action(MenuBrowser& browser,
                   MenuInputMode mode = MenuInputMode::scroll_and_letters);

} //menu_input

#endif
