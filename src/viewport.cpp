#include "viewport.hpp"

#include "rl_utils.hpp"
#include "panel.hpp"
#include "io.hpp"
#include "global.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static P p0_;

static P get_map_view_dims()
{
        const P gui_coord_dims = panels::get_dims(Panel::map);

        P map_coord_dims = io::gui_to_map_coords(gui_coord_dims);

        map_coord_dims.x = std::min(map_coord_dims.x, map_w);
        map_coord_dims.y = std::min(map_coord_dims.y, map_h);

        return map_coord_dims;
}

// -----------------------------------------------------------------------------
// viewport
// -----------------------------------------------------------------------------
namespace viewport
{

R get_map_view_area()
{
        const P map_view_dims = get_map_view_dims();

        return R(p0_, p0_ + map_view_dims - 1);
}

void focus_on(const P map_pos)
{
        const P map_view_dims = get_map_view_dims();

        p0_.x = map_pos.x - (map_view_dims.x / 2);
        p0_.y = map_pos.y - (map_view_dims.y / 2);

        set_constr_in_range(
                0,
                p0_.x,
                map_w - map_view_dims.x);

        set_constr_in_range(
                0,
                p0_.y,
                map_h - map_view_dims.y);
}

bool is_in_ivew(const P map_pos)
{
        return get_map_view_area().is_pos_inside(map_pos);
}

P to_view_pos(const P map_pos)
{
        return map_pos - p0_;
}

P to_map_pos(const P view_pos)
{
        return view_pos + p0_;
}

} // viewport
