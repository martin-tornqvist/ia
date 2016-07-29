#include "mapgen.hpp"
#include "map.hpp"
#include "feature_rigid.hpp"

namespace mapgen
{

void reserve_river(Region regions[3][3])
{
    TRACE_FUNC_BEGIN;

    R         room_rect;
    Region*   river_region      = nullptr;
    const int reserved_padding  = 2;

    auto init_room_rect = [&](int& len0, int& len1, int& breadth0, int& breadth1,
                              const P & reg0, const P & reg2)
    {
         const R regions_tot_rect(regions[reg0.x][reg0.y].r.p0,
                                  regions[reg2.x][reg2.y].r.p1);

        room_rect       = regions_tot_rect;
        river_region    = &regions[reg0.x][reg0.y];
        const int c     = (breadth1 + breadth0) / 2;
        breadth0        = c - reserved_padding;
        breadth1        = c + reserved_padding;

        ASSERT(is_area_inside(room_rect, regions_tot_rect, true));

        len0--; //Extend room rectangle to map edge
        len1++;
    };

    const Axis axis = rnd::coin_toss() ? Axis::hor : Axis::ver;

    if (axis == Axis::hor)
    {
        init_room_rect(room_rect.p0.x,
                       room_rect.p1.x,
                       room_rect.p0.y,
                       room_rect.p1.y,
                       P(0, 1),
                       P(2, 1));
    }
    else //Vertical
    {
        init_room_rect(room_rect.p0.y,
                       room_rect.p1.y,
                       room_rect.p0.x,
                       room_rect.p1.x,
                       P(1, 0),
                       P(1, 2));
    }

    Room* const room            = room_factory::mk(RoomType::river, room_rect);
    RiverRoom* const river_room = static_cast<RiverRoom*>(room);

    river_room->axis_       = axis;
    river_region->main_room = room;
    river_region->is_free   = false;

    if (axis == Axis::hor)
    {
        regions[1][1] = regions[2][1] = *river_region;
    }
    else //Vertical
    {
        regions[1][1] = regions[1][2] = *river_region;
    }

    map::room_list.push_back(room);

    auto mk = [&](const int X0, const int X1, const int Y0, const int Y1)
    {
        TRACE_VERBOSE << "Reserving river space with floor cells "
                      << "X0: " << X0 << " X1: " << X1 << " Y0: " << Y0 << " Y1: " << Y1
                      << std:: endl;

        for (int x = X0; x <= X1; ++x)
        {
            for (int y = Y0; y <= Y1; ++y)
            {
                //Just put floor for now, river feature will be placed later
                map::put(new Floor(P(x, y)));
                map::room_map[x][y] = room;
            }
        }
    };

    if (axis == Axis::hor)
    {
        mk(room_rect.p0.x + 1,
           room_rect.p1.x - 1,
           room_rect.p0.y,
           room_rect.p1.y);
    }
    else //Vertical axis
    {
        mk(room_rect.p0.x,
           room_rect.p1.x,
           room_rect.p0.y + 1,
           room_rect.p1.y - 1);
    }

    TRACE_FUNC_END;
}

} //mapgen
