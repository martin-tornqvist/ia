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
    Event(const Pos&);
};

class Event_wall_crumble: public Event
{
public:
    Event_wall_crumble(const Pos&, std::vector<Pos>& walls, std::vector<Pos>& inner);

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Event_wall_crumble(const Pos& feature_pos) : Event(feature_pos) {}

    Feature_id id() const override
    {
        return Feature_id::event_wall_crumble;
    }

    void on_new_turn() override;

private:
    std::vector<Pos> wall_cells_;
    std::vector<Pos> inner_cells_;
};

class Event_rats_in_the_walls_discovery: public Event
{
public:
    Event_rats_in_the_walls_discovery(const Pos& feature_pos);

    Feature_id id() const override
    {
        return Feature_id::event_rats_in_the_walls_discovery;
    }

    void on_new_turn() override;
};

#endif
