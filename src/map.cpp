#include "map.hpp"

#include <climits>

#include "init.hpp"
#include "rl_utils.hpp"
#include "feature.hpp"
#include "feature_mob.hpp"
#include "actor_factory.hpp"
#include "item_factory.hpp"
#include "game_time.hpp"
#include "io.hpp"
#include "mapgen.hpp"
#include "item.hpp"
#include "feature_rigid.hpp"
#include "saving.hpp"

#ifdef DEMO_MODE
#include "sdl_base.hpp"
#endif // DEMO_MODE

Cell::Cell() :
    is_explored         (false),
    is_seen_by_player   (false),
    is_lit              (false),
    is_dark             (false),
    player_los          (),
    item                (nullptr),
    rigid               (nullptr),
    player_visual_memory(CellRenderData()),
    pos(P(-1, -1)) {}

Cell::~Cell()
{
    delete rigid;

    delete item;
}

void Cell::reset()
{
    is_explored = is_seen_by_player = is_lit = is_dark = false;

    player_los.is_blocked_hard      = true;
    player_los.is_blocked_by_drk    = false;

    player_visual_memory = CellRenderData();

    pos.set(-1, -1);

    delete rigid;
    rigid = nullptr;

    delete item;
    item = nullptr;
}

namespace map
{

Player* player = nullptr;

int dlvl = 0;

Clr wall_clr;

Cell cells[map_w][map_h];

std::vector<Room*> room_list;

Room* room_map[map_w][map_h];

std::vector<ChokePointData> choke_point_data;

namespace
{

void reset_cells(const bool make_stone_walls)
{
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            cells[x][y].reset();
            cells[x][y].pos = P(x, y);

            room_map[x][y] = nullptr;

            if (make_stone_walls)
            {
                put(new Wall(P(x, y)));
            }
        }
    }
}

} // namespace

void init()
{
    dlvl = 0;

    room_list.clear();

    reset_cells(false);

    const P player_pos(player_start_x, player_start_y);

    Actor* actor = actor_factory::mk(ActorId::player, player_pos);

    player = static_cast<Player*>(actor);
}

void cleanup()
{
    reset_map();

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            auto& cell = cells[x][y];

            delete cell.rigid;

            cell.rigid = nullptr;
        }
    }

    // NOTE: game_time deletes the player object (the actor list is the owner of
    //       this memory)
    player = nullptr;
}

void save()
{
    saving::put_int(dlvl);
}

void load()
{
    dlvl = saving::get_int();
}

void reset_map()
{
    actor_factory::delete_all_mon();

    for (auto* room : room_list)
    {
        delete room;
    }

    room_list.clear();

    choke_point_data.clear();

    reset_cells(true);

    game_time::erase_all_mobs();
    game_time::reset_turn_type_and_actor_counters();

    // Occasionally set wall color to something unusual
    if (rnd::one_in(3))
    {
        std::vector<Clr> wall_clr_bucket =
        {
            clr_red,
            clr_nosf_sepia,
            clr_nosf_sepia_drk,
            clr_brown_drk,
            clr_brown_gray,
        };

        wall_clr = rnd::element(wall_clr_bucket);
    }
    else // Standard wall color
    {
        wall_clr = clr_gray;
    }
}

Rigid* put(Rigid* const f)
{
    ASSERT(f);

    const P p = f->pos();

    Cell& cell = cells[p.x][p.y];

    delete cell.rigid;

    cell.rigid = f;

#ifdef DEMO_MODE
    if (f->id() == FeatureId::floor)
    {
        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                map::cells[x][y].is_seen_by_player =
                    map::cells[x][y].is_explored = true;
            }
        }

        states::draw();
        io::draw_glyph('X', Panel::map, p, clr_yellow);
        io::update_screen();

        // NOTE: Delay must be > 1 for user input to be read
        sdl_base::sleep(10);
    }
#endif // DEMO_MODE

    return f;
}

void update_vision()
{
    game_time::update_light_map();

    map::player->update_fov();

    states::draw();
}

void mk_blood(const P& origin)
{
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const P c = origin + P(dx, dy);

            Rigid* const f = cells[c.x][c.y].rigid;

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

    ASSERT(false && "Tried to remove non-existing room");
}

bool is_pos_seen_by_player(const P& p)
{
    ASSERT(map::is_pos_inside_map(p));

    return cells[p.x][p.y].is_seen_by_player;
}

Actor* actor_at_pos(const P& pos, ActorState state)
{
    for (auto* const actor : game_time::actors)
    {
        if (actor->pos == pos &&
            actor->state() == state)
        {
            return actor;
        }
    }

    return nullptr;
}

Mob* first_mob_at_pos(const P& pos)
{
    for (auto* const mob : game_time::mobs)
    {
        if (mob->pos() == pos)
        {
            return mob;
        }
    }

    return nullptr;
}

void actor_cells(const std::vector<Actor*>& actors, std::vector<P>& out)
{
    out.clear();

    for (const auto* const a : actors)
    {
        out.push_back(a->pos);
    }
}

void mk_actor_array(Actor* a[map_w][map_h])
{
    std::fill_n(*a, nr_map_cells, nullptr);

    for (Actor* actor : game_time::actors)
    {
        const P& p = actor->pos;
        a[p.x][p.y] = actor;
    }
}

Actor* random_closest_actor(const P& c, const std::vector<Actor*>& actors)
{
    if (actors.empty())
    {
        return nullptr;
    }

    if (actors.size() == 1)
    {
        return actors[0];
    }

    // Find distance to nearest actor(s)
    int dist_to_nearest = INT_MAX;

    for (Actor* actor : actors)
    {
        const int current_dist = king_dist(c, actor->pos);

        if (current_dist < dist_to_nearest)
        {
            dist_to_nearest = current_dist;
        }
    }

    ASSERT(dist_to_nearest != INT_MAX);

    //Store all actors with distance equal to the nearest distance
    std::vector<Actor*> closest_actors;

    for (Actor* actor : actors)
    {
        if (king_dist(c, actor->pos) == dist_to_nearest)
        {
            closest_actors.push_back(actor);
        }
    }

    ASSERT(!closest_actors.empty());

    const int element = rnd::range(0, closest_actors.size() - 1);

    return closest_actors[element];
}

bool is_pos_inside_map(const P& pos, const bool count_edge_as_inside)
{
    if (count_edge_as_inside)
    {
        return pos.x >= 0 &&
               pos.y >= 0 &&
               pos.x < map_w &&
               pos.y < map_h;
    }
    else //Edge counts as outside the map
    {
        return pos.x > 0 &&
               pos.y > 0 &&
               pos.x < map_w - 1 &&
               pos.y < map_h - 1;
    }
}

bool is_area_inside_map(const R& area)
{
    return is_pos_inside_map(area.p0) &&
           is_pos_inside_map(area.p1);
}

} //map
