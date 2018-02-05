#ifndef RL_UTILS_MISC_HPP
#define RL_UTILS_MISC_HPP

void set_constr_in_range(const int min,
                         int& val,
                         const int max);

void set_constr_in_range(const double min,
                         double& val,
                         const double max);

int constr_in_range(const int min,
                    const int val,
                    const int max);

int constr_in_range(const double min,
                    const double val,
                    const double max);

// Takes a boolean map array, and populates a vector with positions inside the
// given area matching the specified value to store (true/false).
std::vector<P> to_vec(const bool a[map_w][map_h],
                      const bool value_to_store,
                      const R& area_to_parse = R(0, 0, map_w - 1, map_h - 1));

bool is_pos_inside(const P& pos, const R& area);

bool is_area_inside(const R& inner,
                    const R& outer,
                    const bool count_equal_as_inside);

bool is_pos_adj(const P& pos1,
                const P& pos2,
                const bool count_same_cell_as_adj);

P closest_pos(const P& p, const std::vector<P>& positions);

// Distance as the king moves in chess
// The distance between (x0, y0) and (x1, y1) is defined as:
// max(|x1 - x0|, |y1 - y0|).
// This is typically the model used for movement in roguelikes.
int king_dist(const int x0,
              const int y0,
              const int x1,
              const int y1);

int king_dist(const P& p0, const P& p1);

// Taxicab distance, or "rectilinear distance", "Manhattan distance", etc.
// The distance between (x0, y0) and (x1, y1) is defined as:
// |x1 - x0| + |y1 - y0|.
int taxi_dist(const P& p0, const P& p1);

bool is_val_in_range(const int v, const Range range);

int to_int(const std::string& in);

#endif // RL_UTILS_MISC_HPP
