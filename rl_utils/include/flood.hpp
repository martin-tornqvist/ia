#ifndef RL_UTILS_FLOOD_HPP
#define RL_UTILS_FLOOD_HPP

void floodfill(const P& p0,
               const bool blocked[map_w][map_h],
               int out[map_w][map_h],
               int travel_lmt = -1,
               const P& p1 = P(-1, -1),
               const bool allow_diagonal = true);

#endif // RL_UTILS_FLOOD_HPP
