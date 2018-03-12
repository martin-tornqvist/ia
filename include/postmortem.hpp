#ifndef POSTMORTEM_HPP
#define POSTMORTEM_HPP

#include "state.hpp"
#include "info_screen_state.hpp"
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
        void make_memorial_file(const std::string path) const;

        MenuBrowser browser_;

        IsWin is_win_;

        std::vector<std::string> ascii_graveyard_lines_;
};

class PostmortemInfo: public InfoScreenState
{
public:
        PostmortemInfo() :
                InfoScreenState(),
                top_idx_(0) {}

        void draw() override;

        void update() override;

        StateId id() override;

private:
        std::string title() const override
        {
                return "Game summary";
        }

        InfoScreenType type() const override
        {
                return InfoScreenType::scrolling;
        }

        int top_idx_;
};

#endif // POSTMORTEM_HPP
