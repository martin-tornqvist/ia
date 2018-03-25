#ifndef MANUAL_HPP
#define MANUAL_HPP

#include <vector>
#include <string>

#include "state.hpp"
#include "info_screen_state.hpp"
#include "global.hpp"
#include "browser.hpp"

struct ManualPage
{
        std::string title = "";

        std::vector<std::string> lines = {};
};

class BrowseManual: public State
{
public:
        BrowseManual() :
                State(),
                browser_(),
                pages_(),
                top_idx_(0) {}

        void on_start() override;

        void draw() override;

        void on_window_resized() override;

        void update() override;

        StateId id() override;

private:
        MenuBrowser browser_;

        std::vector<std::string> raw_lines_;

        std::vector<ManualPage> pages_;

        int top_idx_;
};

class BrowseManualPage: public InfoScreenState
{
public:
        BrowseManualPage(const ManualPage& page) :
                InfoScreenState(),
                page_(page),
                top_idx_(0) {}

        void draw() override;

        void update() override;

        StateId id() override;

private:
        std::string title() const override;

        InfoScreenType type() const override
        {
                return InfoScreenType::scrolling;
        }

        const ManualPage& page_;

        int top_idx_;
};

#endif // MANUAL_HPP
