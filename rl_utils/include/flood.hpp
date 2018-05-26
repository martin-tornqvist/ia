#ifndef RL_UTILS_FLOOD_HPP
#define RL_UTILS_FLOOD_HPP

Array2<int> floodfill(
        const P& p0,
        const Array2<bool>& blocked,
        int travel_lmt = -1,
        const P& p1 = P(-1, -1),
        const bool allow_diagonal = true);

#endif // RL_UTILS_FLOOD_HPP
