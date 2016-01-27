#include "init.hpp"

#include "map.hpp"

#include "feature.hpp"
#include "actor_factory.hpp"
#include "item_factory.hpp"
#include "game_time.hpp"
#include "render.hpp"
#include "map_gen.hpp"
#include "item.hpp"
#include "utils.hpp"
#include "feature_rigid.hpp"
#include "save_handling.hpp"

#ifdef DEMO_MODE
#include "sdl_wrapper.hpp"
#endif // DEMO_MODE

Cell::Cell() :
    is_explored         (false),
    is_seen_by_player   (false),
    is_lit              (false),
    is_dark             (false),
    player_los          (),
    item                (nullptr),
    rigid               (nullptr),
    player_visual_memory(Cell_render_data()),
    pos(P(-1, -1)) {}

Cell::~Cell()
{
    if (rigid)
    {
        delete rigid;
    }

    if (item)
    {
        delete item;
    }
}

void Cell::reset()
{
    is_explored = is_seen_by_player = is_lit = is_dark = false;

    player_los.is_blocked_hard      = true;
    player_los.is_blocked_by_drk    = false;

    player_visual_memory = Cell_render_data();

    pos.set(-1, -1);

    if (rigid)
    {
        delete rigid;
        rigid = nullptr;
    }

    if (item)
    {
        delete item;
        item = nullptr;
    }
}

namespace map
{

Player*             player  = nullptr;
int                 dlvl    = 0;
Cell                cells[MAP_W][MAP_H];
std::vector<Room*>  room_list;
Room*               room_map[MAP_W][MAP_H];
Clr                 wall_clr;

namespace
{

void reset_cells(const bool MAKE_STONE_WALLS)
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            cells[x][y].reset();
            cells[x][y].pos = P(x, y);

            room_map[x][y] = nullptr;

            render::render_array[x][y]              = Cell_render_data();
            render::render_array_no_actors[x][y]    = Cell_render_data();

            if (MAKE_STONE_WALLS)
            {
                put(new Wall(P(x, y)));
            }
        }
    }
}

} //namespace

void init()
{
    dlvl = 0;

    room_list.clear();

    reset_cells(false);

    const P player_pos(PLAYER_START_X, PLAYER_START_Y);

    player = static_cast<Player*>(actor_factory::mk(Actor_id::player, player_pos));
}

void cleanup()
{
    reset_map();

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            delete cells[x][y].rigid;
            cells[x][y].rigid = nullptr;
        }
    }

    //NOTE: game_time deletes the player object (the actor list is the owner of this memory)
    player = nullptr;
}

void save()
{
    save_handling::put_int(dlvl);
}

void load()
{
    dlvl = save_handling::get_int();
}

void reset_map()
{
    actor_factory::delete_all_mon();

    for (auto* room : room_list)
    {
        delete room;
    }

    room_list.clear();

    reset_cells(true);
    game_time::erase_all_mobs();
    game_time::reset_turn_type_and_actor_counters();

    //Occasionally set wall color to something unusual
    if (rnd::one_in(7))
    {
        std::vector<Clr> wall_clr_bucket =
        {
            clr_nosf_sepia,
            clr_nosf_sepia_drk,
            clr_red,
            clr_brown,
            clr_brown_drk,
            clr_brown_gray,
        };

        const size_t IDX = rnd::range(0, wall_clr_bucket.size() - 1);

        wall_clr = wall_clr_bucket[IDX];
    }
    else //Standard wall color
    {
        wall_clr = clr_gray;

        //Randomize the color slightly (subtle effect)
        wall_clr.r += rnd::range(-6, 6);
        wall_clr.g += rnd::range(-6, 6);
        wall_clr.b += rnd::range(-6, 6);
    }
}

Rigid* put(Rigid* const f)
{
    assert(f);

    const P p     = f->pos();
    Cell&     cell  = cells[p.x][p.y];

    delete cell.rigid;

    cell.rigid = f;

#ifdef DEMO_MODE

    if (f->id() == Feature_id::floor)
    {
        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                map::cells[x][y].is_seen_by_player = map::cells[x][y].is_explored = true;
            }
        }

        render::draw_map();
        render::draw_glyph('X', Panel::map, p, clr_yellow);
        render::update_screen();
        sdl_wrapper::sleep(10); //NOTE: Delay must be > 1 for user input to be read
    }

#endif // DEMO_MODE

    return f;
}

void cpy_render_array_to_visual_memory()
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const Cell_render_data render_data = render::render_array_no_actors[x][y];

            assert(!render_data.is_aware_of_hostile_mon_here);
            assert(!render_data.is_aware_of_allied_mon_here);
            assert(!render_data.is_living_actor_seen_here);

            cells[x][y].player_visual_memory = render_data;
        }
    }
}

void mk_blood(const P& origin)
{
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const P         c   = origin + P(dx, dy);
            Rigid* const    f   = cells[c.x][c.y].rigid;

            if (f->can_have_blood())
            {
                if (rnd::one_in(3))
                {
                    f->mk_bloody();
                }
            }
        }
    }
}

void mk_gore(const P& origin)
{
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const P c = origin + P(dx, dy);

            if (rnd::one_in(3))
            {
                cells[c.x][c.y].rigid->try_put_gore();
            }
        }
    }
}

void delete_and_remove_room_from_list(Room* const room)
{
    for (size_t i = 0; i < room_list.size(); ++i)
    {
        if (room_list[i] == room)
        {
            delete room;
            room_list.erase(room_list.begin() + i);
            return;
        }
    }

    assert(false && "Tried to remove non-existing room");
}

bool is_pos_seen_by_player(const P& p)
{
    assert(utils::is_pos_inside_map(p));
    return cells[p.x][p.y].is_seen_by_player;
}

} //map
