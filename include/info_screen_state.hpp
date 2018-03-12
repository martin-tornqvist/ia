#ifndef INFO_SCREEN_STATE_HPP
#define INFO_SCREEN_STATE_HPP

#include "state.hpp"

enum class InfoScreenType
{
        scrolling,
        single_screen
};

class InfoScreenState: public State
{
public:
        InfoScreenState() :
                State() {}

protected:
        int max_nr_lines_on_screen() const;

        void draw_interface() const;

        virtual std::string title() const = 0;

        virtual InfoScreenType type() const = 0;
};

#endif // INFO_SCREEN_STATE_HPP
