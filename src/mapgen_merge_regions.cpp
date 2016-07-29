#include "mapgen.hpp"

namespace mapgen
{

void mk_merged_regions_and_rooms(Region regions[3][3])
{
    TRACE_FUNC_BEGIN;

    const int nr_attempts = rnd::range(0, 2);

    for (int attempt_cnt = 0; attempt_cnt < nr_attempts; ++attempt_cnt)
    {
        P reg_idx_1, reg_idx_2;

        //Find two non-occupied regions
        int nr_tries_to_find_regions = 100;
        bool is_good_regions_found = false;

        while (!is_good_regions_found)
        {
            --nr_tries_to_find_regions;

            if (nr_tries_to_find_regions <= 0)
            {
                return;
            }

            reg_idx_1 = P(rnd::range(0, 2), rnd::range(0, 1));
            reg_idx_2 = P(reg_idx_1 + P(0, 1));

            is_good_regions_found = regions[reg_idx_1.x][reg_idx_1.y].is_free &&
                                    regions[reg_idx_2.x][reg_idx_2.y].is_free;
        }

        //Expand region 1 over both areas
        auto& reg1      = regions[reg_idx_1.x][reg_idx_1.y];
        auto& reg2      = regions[reg_idx_2.x][reg_idx_2.y];

        reg1.r          = R(reg1.r.p0, reg2.r.p1);

        reg2.r          = R(-1, -1, -1, -1);

        reg1.is_free    = reg2.is_free = false;

        //Make a room for region 1
        auto rnd_padding = []()
        {
            return rnd::range(0, 4);
        };

         const R padding(rnd_padding(),
                         rnd_padding(),
                         rnd_padding(),
                         rnd_padding());

         const R room_rect(reg1.r.p0 + padding.p0,
                           reg1.r.p1 - padding.p1);

        Room* const room =
            room_factory::mk_random_allowed_std_room(room_rect, false);

        reg1.main_room = room;
        register_room(*room);
        mk_floor_in_room(*room);
    }

    TRACE_FUNC_END;
}

} //mapgen
