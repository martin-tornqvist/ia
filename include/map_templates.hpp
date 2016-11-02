#ifndef MAP_TEMPLATES_HPP
#define MAP_TEMPLATES_HPP

#include <string>

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

namespace map_templates
{

void init();

const Array2<char>& level_templ(LevelTemplId id);

const Array2<char>* random_room_templ(const P& max_dim);

} // map_templates

#endif // MAP_TEMPLATES_HPP
