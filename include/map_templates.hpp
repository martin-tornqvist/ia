#ifndef MAPTEMPLATES_HPP
#define MAPTEMPLATES_HPP

#include <vector>
#include <string>

#include "feature_data.hpp"
#include "item_data.hpp"
#include "actor_data.hpp"

enum class Map_templ_id
{
    intro_forest,
    egypt,
    leng,
    rats_in_the_walls,
    boss_level,
    trapez_level,
    END
};

struct Map_templ_cell
{
    Map_templ_cell(char ch,
                   Feature_id feature_id = Feature_id::END,
                   Actor_id actor_id = Actor_id::END,
                   Item_id item_id = Item_id::END) :
        ch          (ch),
        feature_id  (feature_id),
        actor_id    (actor_id),
        item_id     (item_id) {}

    char ch;
    Feature_id feature_id;
    Actor_id actor_id;
    Item_id item_id;
};

struct Map_templ
{
public:
    Map_templ()
    {
        cells_.clear();
    }

    const Map_templ_cell& cell(const P& p) const
    {
        return cells_[p.y][p.x];
    }

    void add_row(std::vector<Map_templ_cell>& row)
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
    std::vector< std::vector<Map_templ_cell> > cells_;
};

namespace map_templ_handling
{

void init();

const Map_templ& templ(const Map_templ_id id);

} //map_templ_handling

#endif
