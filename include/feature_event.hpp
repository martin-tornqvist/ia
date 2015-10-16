#ifndef FEATURE_EVENT_H
#define FEATURE_EVENT_H

#include <vector>

#include "feature_mob.hpp"

class Event: public Mob
{
public:
    virtual ~Event() {}

    virtual void on_new_turn() override = 0;

    std::string name(const Article article) const override final
    {
        (void)article;
        return "";
    }

    Clr clr() const override final
    {
        return clr_black;
    }

protected:
    Event(const P&);
};

class Event_wall_crumble: public Event
{
public:
    Event_wall_crumble(const P&, std::vector<P>& walls, std::vector<P>& inner);

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Event_wall_crumble(const P& feature_pos) : Event(feature_pos) {}

    Feature_id id() const override
    {
        return Feature_id::event_wall_crumble;
    }

    void on_new_turn() override;

private:
    std::vector<P> wall_cells_;
    std::vector<P> inner_cells_;
};

class Event_rats_in_the_walls_discovery: public Event
{
public:
    Event_rats_in_the_walls_discovery(const P& feature_pos);

    Feature_id id() const override
    {
        return Feature_id::event_rats_in_the_walls_discovery;
    }

    void on_new_turn() override;
};

#endif
