#ifndef FOV_HPP
#define FOV_HPP

#include "rl_utils.hpp"
#include "cmn.hpp"

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

R get_fov_rect(const P& p);

bool is_in_fov_range(const P& p0, const P& p1);

Los_result check_cell(const P& p0,
                      const P& p1,
                      const bool hard_blocked[MAP_W][MAP_H]);

void run(const P& p0,
         const bool hard_blocked[MAP_W][MAP_H],
         Los_result out[MAP_W][MAP_H]);

} //fov

#endif
