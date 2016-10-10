#ifndef CHARACTER_DESCR_HPP
#define CHARACTER_DESCR_HPP

#include "state.hpp"
#include "global.hpp"

class CharacterDescr: public State
{
public:
    CharacterDescr() :
        State       (),
        top_idx_    (0) {}

    void on_start() override;

    void draw() override;

    void update() override;

private:
    std::vector<StrAndClr> lines_;

    int top_idx_;
};

#endif // CHARACTER_DESCR_HPP
