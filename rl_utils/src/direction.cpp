#include "rl_utils.hpp"

namespace dir_utils
{

const std::vector<P> cardinal_list
{
    P(-1,  0),
    P( 1,  0),
    P( 0, -1),
    P( 0,  1)
};

const std::vector<P> cardinal_list_w_center
{
    P( 0,  0),
    P(-1,  0),
    P( 1,  0),
    P( 0, -1),
    P( 0,  1)
};

const std::vector<P> dir_list
{
    P(-1,  0),
    P( 1,  0),
    P( 0, -1),
    P( 0,  1),
    P(-1, -1),
    P(-1,  1),
    P( 1, -1),
    P( 1,  1)
};

const std::vector<P> dir_list_w_center
{
    P( 0,  0),
    P(-1,  0),
    P( 1,  0),
    P( 0, -1),
    P( 0,  1),
    P(-1, -1),
    P(-1,  1),
    P( 1, -1),
    P( 1,  1)
};

namespace
{

const std::string compass_dir_names[3][3] =
{
    {"NW", "N", "NE"},
    {"W",  "",  "E",},
    {"SW", "S", "SE"}
};

const double pi_db            = 3.14159265;
const double angle_45_db      = 2 * pi_db / 8;
const double angle_45_half_db = angle_45_db / 2.0;

const double edge[4] =
{
    angle_45_half_db + (angle_45_db * 0),
    angle_45_half_db + (angle_45_db * 1),
    angle_45_half_db + (angle_45_db * 2),
    angle_45_half_db + (angle_45_db * 3)
};

} // namespace

Dir dir(const P& offset)
{
    ASSERT(offset.x >= -1 &&
           offset.y >= -1 &&
           offset.x <=  1 &&
           offset.y <=  1);

    if (offset.y == -1)
    {
        return offset.x == -1 ? Dir::up_left :
               offset.x ==  0 ? Dir::up :
               offset.x ==  1 ? Dir::up_right :
               Dir::END;
    }

    if (offset.y == 0)
    {
        return offset.x == -1 ? Dir::left :
               offset.x ==  0 ? Dir::center :
               offset.x ==  1 ? Dir::right :
               Dir::END;
    }

    if (offset.y == 1)
    {
        return offset.x == -1 ? Dir::down_left :
               offset.x ==  0 ? Dir::down :
               offset.x ==  1 ? Dir::down_right :
               Dir::END;
    }

    return Dir::END;
}

P offset(const Dir dir)
{
    ASSERT(dir != Dir::END);

    switch (dir)
    {
    case Dir::down_left:
        return P(-1, 1);

    case Dir::down:
        return P(0, 1);

    case Dir::down_right:
        return P(1, 1);

    case Dir::left:
        return P(-1, 0);

    case Dir::center:
        return P(0, 0);

    case Dir::right:
        return P(1, 0);

    case Dir::up_left:
        return P(-1, -1);

    case Dir::up:
        return P(0, -1);

    case Dir::up_right:
        return P(1, -1);

    case Dir::END:
        return P(0, 0);
    }

    return P(0, 0);
}

P rnd_adj_pos(const P& origin, const bool is_center_allowed)
{
    const std::vector<P>* vec = nullptr;

    if (is_center_allowed)
    {
        vec = &dir_list_w_center;
    }
    else // Center not allowed
    {
        vec = &dir_list;
    }

    const size_t idx = rnd::range(0, vec->size() - 1);

    return origin + vec->at(idx);
}

void compass_dir_name(const P& from_pos,
                      const P& to_pos,
                      std::string& dst)
{
    dst = "";

    const P offset(to_pos - from_pos);

    const double angle_db = atan2(-offset.y, offset.x);

    if (angle_db < -edge[2] && angle_db > -edge[3])
    {
        dst = "SW";
    }
    else if (angle_db <= -edge[1] && angle_db >= -edge[2])
    {
        dst = "S";
    }
    else if (angle_db < -edge[0] && angle_db > -edge[1])
    {
        dst = "SE";
    }
    else if (angle_db >= -edge[0] && angle_db <= edge[0])
    {
        dst = "E";
    }
    else if (angle_db > edge[0] && angle_db < edge[1])
    {
        dst = "NE";
    }
    else if (angle_db >= edge[1] && angle_db <= edge[2])
    {
        dst = "N";
    }
    else if (angle_db > edge[2] && angle_db < edge[3])
    {
        dst = "NW";
    }
    else
    {
        dst = "W";
    }
}

void compass_dir_name(const Dir dir, std::string& dst)
{

    const P& o = offset(dir);

    dst = compass_dir_names[o.x + 1][o.y + 1];
}

void compass_dir_name(const P& offs, std::string& dst)
{
    dst = compass_dir_names[offs.x + 1][offs.y + 1];
}

} // dir_utils
