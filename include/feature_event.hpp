#ifndef FEATURE_EVENT_HPP
#define FEATURE_EVENT_HPP

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

class EventWallCrumble: public Event
{
public:
    EventWallCrumble(const P& p, std::vector<P>& walls, std::vector<P>& inner);

    //Spawn-by-id compliant ctor (do not use for normal cases):
    EventWallCrumble(const P& p) :
        Event(p) {}

    ~EventWallCrumble() {}

    FeatureId id() const override
    {
        return FeatureId::event_wall_crumble;
    }

    void on_new_turn() override;

private:
    std::vector<P> wall_cells_;
    std::vector<P> inner_cells_;
};

class EventSnakeEmerge: public Event
{
public:
    EventSnakeEmerge();

    //Spawn-by-id compliant ctor (do not use for normal cases):
    EventSnakeEmerge(const P& p) :
        Event(p) {}

    ~EventSnakeEmerge() {}

    FeatureId id() const override
    {
        return FeatureId::event_snake_emerge;
    }

    bool try_find_p();

    void on_new_turn() override;

private:
    R allowed_emerge_rect(const P& p) const;

    bool is_ok_feature_at(const P& p) const;

    void blocked_cells(const R& r, bool out[map_w][map_h]) const;

    void emerge_p_bucket(const P& p, bool blocked[map_w][map_h], std::vector<P>& out) const;

    const Range allowed_emerge_dist_range = Range(2, fov_std_radi_int - 1);

    const size_t min_nr_snakes_ = 3;
};

class EventRatsInTheWallsDiscovery: public Event
{
public:
    EventRatsInTheWallsDiscovery(const P& feature_pos);

    FeatureId id() const override
    {
        return FeatureId::event_rat_cave_discovery;
    }

    void on_new_turn() override;
};

#endif
