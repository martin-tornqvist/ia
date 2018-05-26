#ifndef RL_UTILS_PATHFIND_HPP
#define RL_UTILS_PATHFIND_HPP

//------------------------------------------------------------------------------
// The path goes from target to origin, not including the origin.
//
// "randomize_steps", when true, for each step if there are multiple valid
// (nearer) choices, pick one at random. Otherwise iterate over a predefined
// list of offsets until a valid step is found. The second way is more optimized
// and is the default behavior (best for e.g. AI), while the randomized method
// can produces nicer results in some cases (e.g. corridors).
//------------------------------------------------------------------------------
std::vector<P> pathfind(
    const P& p0,                                // Origin
    const P& p1,                                // Target
    const Array2<bool>& blocked,                // Blocked cells
    const bool allow_diagonal = true,           // Cardinals only?
    const bool randomize_steps = false);        // See above

std::vector<P> pathfind_with_flood(
    const P& p0,                                // Origin
    const P& p1,                                // Target
    const Array2<int>& flood,                   // Floodfill
    const bool allow_diagonal = true,           // Cardinals only?
    const bool randomize_steps = false);        // See above

#endif // RL_UTILS_PATHFIND_HPP
