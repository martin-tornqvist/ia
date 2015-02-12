#ifndef MAPTEMPLATES_H
#define MAPTEMPLATES_H

#include <vector>
#include <string>

#include "FeatureData.h"
#include "ItemData.h"
#include "ActorData.h"

enum class MapTemplId
{
    church,
    egypt,
    leng,
    ratsInTheWalls,
    bossLevel,
    trapezohedronLevel,
    END
};

struct MapTemplCell
{
    MapTemplCell(FeatureId featureId_ = FeatureId::END,
                 ActorId actorId_     = ActorId::END,
                 ItemId itemId_       = ItemId::END,
                 int val_             = 0) :
        featureId (featureId_),
        actorId   (actorId_),
        itemId    (itemId_),
        val       (val_) {}

    FeatureId featureId;
    ActorId   actorId;
    ItemId    itemId;
    int       val; //Can be used for different things depending on context
};

struct MapTempl
{
public:
    MapTempl() {cells_.clear();}

    const MapTemplCell& getCell(const int X, const int Y) const {return cells_[Y][X];}

    void addRow(std::vector<MapTemplCell>& row) {cells_.push_back(row);}

    Pos getDims() const {return Pos(cells_.back().size(), cells_.size());}

private:
    std::vector< std::vector<MapTemplCell> > cells_;
};

namespace MapTemplHandling
{

void init();

const MapTempl& getTempl(const MapTemplId id);

} //MapTemplHandling

#endif
