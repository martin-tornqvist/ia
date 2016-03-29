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
    Event_wall_crumble(const P& p, std::vector<P>& walls, std::vector<P>& inner);

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Event_wall_crumble(const P& p) :
        Event(p) {}

    ~Event_wall_crumble() {}

    Feature_id id() const override
    {
        return Feature_id::event_wall_crumble;
    }

    void on_new_turn() override;

private:
    std::vector<P> wall_cells_;
    std::vector<P> inner_cells_;
};

class Event_snake_emerge: public Event
{
public:
    Event_snake_emerge();

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Event_snake_emerge(const P& p) :
        Event(p) {}

    ~Event_snake_emerge() {}

    Feature_id id() const override
    {
        return Feature_id::event_snake_emerge;
    }

    bool try_find_p();

    void on_new_turn() override;

private:
    R allowed_emerge_rect(const P& p) const;

    bool is_ok_feature_at(const P& p) const;

    void blocked_cells(const R& r, bool out[MAP_W][MAP_H]) const;

    void emerge_p_bucket(const P& p, bool blocked[MAP_W][MAP_H], std::vector<P>& out) const;

    const Range allowed_emerge_dist_range = Range(2, FOV_STD_RADI_INT - 1);

    const size_t MIN_NR_SNAKES_ = 3;
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
