#include "mapgen.hpp"

#include "room.hpp"

namespace mapgen
{

Room* mk_room(Region& region)
{
    ASSERT(!region.main_room);

    ASSERT(region.is_free);

    const R room_rect = region.rnd_room_rect();

    auto* room = room_factory::mk_random_room(room_rect,
                                              IsSubRoom::no);

    register_room(*room);

    mk_floor(*room);

    region.main_room = room;
    region.is_free = false;

    return room;
}

Room* mk_room(const R& r, const IsSubRoom is_sub_room)
{
    auto* room = room_factory::mk_random_room(r,
                                              is_sub_room);

    register_room(*room);

    mk_floor(*room);

    return room;
}

} // mapgen
