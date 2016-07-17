#ifndef MAPTEMPLATES_HPP
#define MAPTEMPLATES_HPP

#include <vector>
#include <string>

#include "feature_data.hpp"
#include "item_data.hpp"
#include "actor_data.hpp"

enum class MapTemplId
{
    intro_forest,
    egypt,
    leng,
    rats_in_the_walls,
    boss_level,
    trapez_level,
    END
};

struct MapTemplCell
{
    MapTemplCell(char ch,
                   FeatureId feature_id = FeatureId::END,
                   ActorId actor_id = ActorId::END,
                   ItemId item_id = ItemId::END) :
        ch          (ch),
        feature_id  (feature_id),
        actor_id    (actor_id),
        item_id     (item_id) {}

    char ch;
    FeatureId feature_id;
    ActorId actor_id;
    ItemId item_id;
};

struct MapTempl
{
public:
    MapTempl()
    {
        cells_.clear();
    }

    const MapTemplCell& cell(const P& p) const
    {
        return cells_[p.y][p.x];
    }

    void add_row(std::vector<MapTemplCell>& row)
    {
        cells_.push_back(row);
    }

    P dims() const
    {
        return P(cells_.back().size(), cells_.size());
    }

private:
    //NOTE: The cells are stored as a "list of rows" - i.e. the inner vector represents the x range,
    //and the outer vector represents the y range.
    std::vector< std::vector<MapTemplCell> > cells_;
};

namespace map_templ_handling
{

void init();

const MapTempl& templ(const MapTemplId id);

} //map_templ_handling

#endif
