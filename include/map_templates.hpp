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
        symbols (),
        type    ((RoomType)0) {}

    Array2<char> symbols;

    RoomType type;
};

namespace map_templates
{

void init();

const Array2<char>& level_templ(LevelTemplId id);

RoomTempl* random_room_templ(const P& max_dims);

} // map_templates

#endif // MAP_TEMPLATES_HPP
