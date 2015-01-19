#ifndef FEATURE_EVENT_H
#define FEATURE_EVENT_H

#include <vector>

#include "FeatureMob.h"

class Event: public Mob
{
public:
    virtual ~Event() {}

    virtual void onNewTurn() override = 0;

    std::string getName(const Article article) const override final
    {
        (void)article;
        return "";
    }

    Clr getClr() const override final {return clrBlack;}

protected:
    Event(const Pos&);
};

class EventWallCrumble: public Event
{
public:
    EventWallCrumble(const Pos&, std::vector<Pos>& walls, std::vector<Pos>& inner);

    //Spawn-by-id compliant ctor (do not use for normal cases):
    EventWallCrumble(const Pos& pos) : Event(pos) {}

    FeatureId getId() const override {return FeatureId::eventWallCrumble;}

    void onNewTurn() override;

private:
    std::vector<Pos> wallCells_;
    std::vector<Pos> innerCells_;
};

class EventRatsInTheWallsDiscovery: public Event
{
public:
    EventRatsInTheWallsDiscovery(const Pos& pos);

    FeatureId getId() const override {return FeatureId::eventRatsInTheWallsDiscovery;}

    void onNewTurn() override;
};

#endif
