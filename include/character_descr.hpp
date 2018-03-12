#ifndef CHARACTER_DESCR_HPP
#define CHARACTER_DESCR_HPP

#include "info_screen_state.hpp"
#include "global.hpp"

class CharacterDescr: public InfoScreenState
{
public:
        CharacterDescr() :
                InfoScreenState(),
                top_idx_(0) {}

        void on_start() override;

        void draw() override;

        void update() override;

        StateId id() override;

private:
        std::string title() const override
        {
                return "Character description";
        }

        InfoScreenType type() const override
        {
                return InfoScreenType::scrolling;
        }

        std::vector<ColoredString> lines_;

        int top_idx_;
};

#endif // CHARACTER_DESCR_HPP
