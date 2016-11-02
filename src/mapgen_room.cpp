#include "mapgen.hpp"

#include "room.hpp"
#include "feature_rigid.hpp"

namespace mapgen
{

namespace
{

void put_templ_features(const Array2<char>& templ,
                        const P& p0)
{
    const P dims(templ.dims());

    for (int templ_x = 0; templ_x < dims.x; ++templ_x)
    {
        for (int templ_y = 0; templ_y < dims.y; ++templ_y)
        {
            const P templ_p(templ_x, templ_y);

            const P map_p(p0 + templ_p);

            const char c = templ(templ_p);

            bool is_room_cell = true;

            switch (c)
            {
            case '.':
            {
                map::put(new Floor(map_p));
            }
            break;

            case '#':
            {
                map::put(new Wall(map_p));

                is_room_cell = false;
            }
            break;

            case '=':
            {
                map::put(new Grating(map_p));
            }
            break;

            case '-':
            {
                map::put(new Altar(map_p));
            }
            break;

            case '~':
            {
                auto* liquid = new LiquidShallow(map_p);

                liquid->type_ = LiquidType::water;

                map::put(liquid);
            }
            break;

            case '0':
            {
                map::put(new Brazier(map_p));
            }
            break;

            case 'P':
            {
                map::put(new Statue(map_p));
            }
            break;

            // (Space)
            case ' ':
            {
                // Do nothing

                is_room_cell = false;
            }
            break;

            default:
            {
                TRACE << "Illegal template character \"" << c << "\""
                      << " (at template pos "
                      << templ_x << ", " << templ_y << ")"
                      << std::endl;

                // Release mode robustness: invalidate the map
                is_map_valid = false;

                ASSERT(false);

                return;
            }
            break;

            } // switch

            if (!is_room_cell)
            {
                map::room_map[map_p.x][map_p.y] = nullptr;
            }
        } // y loop
    } // x loop
}

Room* mk_template_room(const Array2<char>& templ, Region& region)
{
    const P dims(templ.dims());

    // Random position inside the region
    const P p0(region.r.p0.x + rnd::range(0, region.r.w() - dims.x),
               region.r.p0.y + rnd::range(0, region.r.h() - dims.y));

    const P p1(p0.x + dims.x - 1,
               p0.y + dims.y - 1);

    auto* room = room_factory::mk(RoomType::template_room,
                                  R(p0, p1));

    register_room(*room);

    // Place features on the map based on the template
    //
    // NOTE: This must be done AFTER "register_room", since it may remove some
    //       of its cells from the global room map (e.g. untouched cells)
    //
    put_templ_features(templ, p0);

    region.main_room = room;
    region.is_free = false;

    return room;

} // mk_template_room

} // namespace

Room* mk_room(Region& region)
{
    ASSERT(!region.main_room);

    ASSERT(region.is_free);

    // Make a templated room?
    if (rnd::one_in(11))
    {
        const P max_dims(region.r.dims());

        const auto* templ =
            map_templates::random_room_templ(max_dims);

        if (templ)
        {
            if ((templ->dims().x > max_dims.x) ||
                (templ->dims().y > max_dims.y))
            {
                ASSERT(false);
            }
            else
            {
                Room* const room = mk_template_room(*templ, region);

                return room;
            }
        }

        // Fine, make a normal procedural room
    }

    // Make a procedural room

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
