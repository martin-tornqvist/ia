#ifndef BROWSER_HPP
#define BROWSER_HPP

#include "rl_utils.hpp"
#include "global.hpp"

struct InputData;

enum class MenuAction
{
    none,
    moved,
    selected,
    space,
    esc
};

enum class MenuInputMode
{
    scrolling_and_letters,
    scrolling
};

// TODO: There's probably some public methods here that could be private

class MenuBrowser
{
public:
    MenuBrowser(const int nr_items, const int list_h = - 1);

    MenuBrowser();

    MenuBrowser& operator=(const MenuBrowser&) = default;

    MenuAction read(const InputData& input, MenuInputMode mode);

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

    void reset(const int nr_items, const int list_h = -1);

private:
    void set_y_nearest_valid();

    void update_range_shown();

    int nr_items_;
    int y_;
    int  list_h_;
    Range range_shown_;
};

#endif // BROWSER
