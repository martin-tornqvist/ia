#ifndef MAP_TEMPLATES_HPP
#define MAP_TEMPLATES_HPP

#include <string>

#include "room.hpp"
#include "rl_utils.hpp"
#include "array2.hpp"

enum class LevelTemplId
{
    intro_forest,
    egypt,
    leng,
    rat_cave,
    boss_level,
    trapez_level,
    END
};

struct RoomTempl
{
    RoomTempl() :
            symbols(P(0, 0)),
            type((RoomType)0),
            base_templ_idx(0) {}

    Array2<char> symbols;

    RoomType type;

    // Eeach template in the data file is automatically rotated/flipped, to
    // create variants - this is the index of the origin template from the file
    // (i.e. there will be many templates with index 0, then index 1, etc)
    size_t base_templ_idx;
};

enum class RoomTemplStatus
{
    unused,
    placed, // Plcaed on the map
    used    // Included in a map which was not discarded
};

namespace map_templates
{

void init();

void save();

void load();

const Array2<char>& level_templ(LevelTemplId id);

RoomTempl* random_room_templ(const P& max_dims);

void clear_base_room_templates_used();

void on_base_room_template_placed(const RoomTempl& templ);

void on_map_discarded();

void on_map_ok();

} // map_templates

#endif // MAP_TEMPLATES_HPP
