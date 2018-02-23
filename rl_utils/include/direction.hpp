#ifndef RL_UTILS_DIRECTION_HPP
#define RL_UTILS_DIRECTION_HPP

#include <vector>
#include <string>

class P;

enum class Dir
{
    down_left   = 1,
    down        = 2,
    down_right  = 3,
    left        = 4,
    center      = 5,
    right       = 6,
    up_left     = 7,
    up          = 8,
    up_right    = 9,
    END
};

namespace dir_utils
{

extern const std::vector<P> cardinal_list;
extern const std::vector<P> cardinal_list_w_center;
extern const std::vector<P> dir_list;
extern const std::vector<P> dir_list_w_center;

Dir dir(const P& offset_values);

P offset(const Dir dir);

P rnd_adj_pos(const P& origin, const bool is_center_allowed);

std::string compass_dir_name(const P& from_pos, const P& to_pos);

std::string compass_dir_name(const Dir dir);

std::string compass_dir_name(const P& offs);

} // dir_utils

#endif // RL_UTILS_DIRECTION_HPP
