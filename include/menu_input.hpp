#ifndef MENU_INPUT_HPP
#define MENU_INPUT_HPP

#include "rl_utils.hpp"
#include "global.hpp"

class Menu_browser
{
public:
    Menu_browser(const int NR_ITEMS, const int LIST_H = - 1);

    Menu_browser() = delete;

    Menu_browser& operator=(const Menu_browser&) = default;

    void move(const Ver_dir dir);

    void move_page(const Ver_dir dir);

    int y() const
    {
        return y_;
    }

    void set_y(const int Y);

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

    bool is_at_idx(const int IDX) const
    {
        return y_ == IDX;
    }

private:
    void set_y_nearest_valid();

    void update_range_shown();

    int nr_items_;
    int y_;
    int  list_h_;
    Range range_shown_;
};

enum class Menu_action
{
    moved,
    selected,
    selected_shift,
    space,
    esc
};

enum class Menu_input_mode
{
    scroll_and_letters,
    scroll
};

namespace menu_input
{

Menu_action action(Menu_browser& browser,
                   Menu_input_mode mode = Menu_input_mode::scroll_and_letters);

} //menu_input

#endif
