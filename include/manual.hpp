#ifndef MANUAL_HPP
#define MANUAL_HPP

#include <vector>
#include <string>

#include "state.hpp"
#include "global.hpp"
#include "browser.hpp"

struct ManualPage
{
    ManualPage() :
        title   (""),
        lines   () {}

    std::string title;

    std::vector<std::string> lines;
};

class BrowseManual: public State
{
public:
    BrowseManual() :
        State       (),
        browser_    (),
        pages_      (),
        top_idx_    (0) {}

    void on_start() override;

    void draw() override;

    void update() override;

    StateId id() override;

private:
    void read_file();

    MenuBrowser browser_;

    std::vector<ManualPage> pages_;

    int top_idx_;
};

class BrowseManualPage: public State
{
public:
    BrowseManualPage(const ManualPage& page) :
        State       (),
        page_       (page),
        top_idx_    (0) {}

    void draw() override;

    void update() override;

    StateId id() override;

private:
    const ManualPage& page_;

    int top_idx_;
};

#endif // MANUAL_HPP
