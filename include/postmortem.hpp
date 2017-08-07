#ifndef POSTMORTEM_HPP
#define POSTMORTEM_HPP

#include "state.hpp"
#include "browser.hpp"
#include "global.hpp"

class PostmortemMenu: public State
{
public:
    PostmortemMenu(const IsWin is_win);

    void on_start() override;

    void on_popped() override;

    void draw() override;

    void update() override;

    StateId id() override;

private:
    void mk_memorial_file(const std::string path) const;

    MenuBrowser browser_;

    IsWin is_win_;
};

class PostmortemInfo: public State
{
public:
    PostmortemInfo() :
        State                   (),
        max_nr_lines_on_scr_    (screen_h - 2),
        top_idx_                (0) {}

    void draw() override;

    void update() override;

    StateId id() override;

private:
    const int max_nr_lines_on_scr_;

    int top_idx_;
};

#endif // POSTMORTEM_HPP
