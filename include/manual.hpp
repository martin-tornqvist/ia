#ifndef MANUAL_HPP
#define MANUAL_HPP

#include <vector>
#include <string>

#include "state.hpp"
#include "global.hpp"

class BrowseManual: public State
{
public:
    BrowseManual() :
        State       (),
        lines_      (),
        top_idx_    (0) {}

    void on_start() override;

    void draw() override;

    void update() override;

private:
    void read_file();

    std::vector<std::string> lines_;

    int top_idx_;
};

#endif // MANUAL_HPP
