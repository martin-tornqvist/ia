#ifndef LOOK_HPP
#define LOOK_HPP

#include <string>

#include "rl_utils.hpp"
#include "info_screen_state.hpp"
#include "global.hpp"

class Actor;

struct ActorDescrText
{
        std::string str = {};

        Color color = colors::black();

        int x_pos = 0;
};

class ViewActorDescr: public InfoScreenState
{
public:
        ViewActorDescr(Actor& actor) :
                InfoScreenState(),
                lines_(),
                top_idx_(0),
                actor_(actor) {}

        void on_start() override;

        void draw() override;

        void update() override;

        StateId id() override;

private:
        std::string title() const override
        {
                return "Monster info";
        }

        InfoScreenType type() const override
        {
                return InfoScreenType::scrolling;
        }

        void put_text(const std::string str,
                      const P& p,
                      const Color& color);

        std::string auto_description_str() const;

        std::string mon_speed_str() const;

        std::string mon_dwell_lvl_str() const;

        void mon_shock_str(std::string& shock_str_out,
                           std::string& punct_str_out) const;

        std::vector< std::vector<ActorDescrText> > lines_;

        int top_idx_;

        Actor& actor_;
};

namespace look
{

void print_location_info_msgs(const P& pos);

} // look

#endif // LOOK_HPP
