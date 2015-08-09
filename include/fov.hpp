#ifndef FOV_H
#define FOV_H

#include "cmn_data.hpp"
#include "cmn_types.hpp"

struct Los_result
{
    Los_result() :
        is_blocked_hard     (false),
        is_blocked_by_drk   (false) {}

    bool is_blocked_hard;
    bool is_blocked_by_drk;
};

namespace fov
{

Rect get_fov_rect(const Pos& p);

bool is_in_fov_range(const Pos& p0, const Pos& p1);

Los_result check_cell(const Pos& p0,
                      const Pos& p1,
                      const bool hard_blocked[MAP_W][MAP_H]);

void run(const Pos& p0,
         const bool hard_blocked[MAP_W][MAP_H],
         Los_result out[MAP_W][MAP_H]);

} //fov

#endif
