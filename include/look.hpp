#ifndef LOOK_HPP
#define LOOK_HPP

#include <string>

#include "rl_utils.hpp"
#include "state.hpp"
#include "global.hpp"

class Actor;

struct ActorDescrText
{
    ActorDescrText() :
        str (),
        clr (clr_black),
        p   () {}

    std::string str;

    Clr clr;

    P p;
};

class ViewActorDescr: public State
{
public:
    ViewActorDescr(Actor& actor) :
        State       (),
        lines_      (),
        top_idx_    (0),
        actor_      (actor) {}

    void on_start() override;

    void draw() override;

    void update() override;

private:
    void put_text(const std::string str,
                  const P& p,
                  const Clr& clr);

    std::string auto_description_str() const;

    std::string mon_speed_str() const;

    std::string mon_dwell_lvl_str() const;

    void mon_shock_str(std::string& shock_str_out,
                       std::string& punct_str_out) const;

    //
    // NOTE: The y position of the Text struct is not used at all here, instead
    //       the indexes of the lines_ vector represents the y position.
    //       This is more convenient when iterating over the lines to draw text.
    //
    std::vector< std::vector<ActorDescrText> > lines_;

    int top_idx_;

    Actor& actor_;
};

namespace look
{

void print_location_info_msgs(const P& pos);

} // look

#endif // LOOK_HPP
