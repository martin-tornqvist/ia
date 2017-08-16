#include "mapgen.hpp"

#include "room.hpp"
#include "feature_rigid.hpp"
#include "feature_door.hpp"

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

            const P p(p0 + templ_p);

            const char c = templ(templ_p);

            bool is_room_cell = true;

            switch (c)
            {
            case '.':
            {
                map::put(new Floor(p));
            }
            break;

            case '#':
            {
                map::put(new Wall(p));

                is_room_cell = false;
            }
            break;

            case '-':
            {
                map::put(new Altar(p));
            }
            break;

            case '~':
            {
                auto* liquid = new LiquidShallow(p);

                liquid->type_ = LiquidType::water;

                map::put(liquid);
            }
            break;

            case '0':
            {
                map::put(new Brazier(p));
            }
            break;

            case 'P':
            {
                map::put(new Statue(p));
            }
            break;

            case '+':
            {
                Wall* mimic = new Wall(p);

                map::put(new Door(p, mimic, DoorType::wood));
            }
            break;

            case 'x':
            {
                map::put(new Door(p, nullptr, DoorType::gate));
            }
            break;

            case '=':
            {
                map::put(new Grating(p));
            }
            break;

            case '"':
            {
                map::put(new Vines(p));
            }
            break;

            case '*':
            {
                map::put(new Chains(p));
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
                map::room_map[p.x][p.y] = nullptr;
            }
        } // y loop
    } // x loop
}

Room* mk_template_room(const RoomTempl& templ, Region& region)
{
    const P dims(templ.symbols.dims());

    // Random position inside the region
    const P p0(region.r.p0.x + rnd::range(0, region.r.w() - dims.x),
               region.r.p0.y + rnd::range(0, region.r.h() - dims.y));

    const P p1(p0.x + dims.x - 1,
               p0.y + dims.y - 1);

    const R r(p0, p1);

    auto* room = new TemplateRoom(r, templ.type);

    register_room(*room);

    // Place features on the map based on the template

    //
    // NOTE: This must be done AFTER "register_room", since it may remove some
    //       of its cells from the global room map (e.g. untouched cells)
    //
    put_templ_features(templ.symbols, p0);

    region.main_room = room;
    region.is_free = false;

    return room;

} // mk_template_room

} // namespace

Room* mk_room(Region& region)
{
    ASSERT(!region.main_room);

    ASSERT(region.is_free);

    const int templ_room_one_in_n = 17;

    // Make a templated room?
    if (map::dlvl <= dlvl_last_mid_game &&
        rnd::one_in(templ_room_one_in_n))
    {
        const P max_dims(region.r.dims());

        const auto* templ =
            map_templates::random_room_templ(max_dims);

        if (templ)
        {
            auto& symbols = templ->symbols;

            if ((symbols.dims().x > max_dims.x) ||
                (symbols.dims().y > max_dims.y))
            {
                ASSERT(false);
            }
            else
            {
                Room* const room = mk_template_room(*templ, region);

                return room;
            }
        }

        // Failed to make a templated room - fine, make a normal procedural room
    }

    // Make a procedural room

    const R room_rect = region.rnd_room_rect();

    auto* room = room_factory::mk_random_room(room_rect, IsSubRoom::no);

    register_room(*room);

    mk_floor(*room);

    region.main_room = room;
    region.is_free = false;

    return room;
}

Room* mk_room(const R& r, const IsSubRoom is_sub_room)
{
    auto* room = room_factory::mk_random_room(r, is_sub_room);

    register_room(*room);

    mk_floor(*room);

    return room;
}

} // mapgen
