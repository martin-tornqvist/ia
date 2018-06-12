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
#include "actor_player.hpp"

#ifndef NDEBUG
#include "sdl_base.hpp"
#include "viewport.hpp"
#endif // NDEBUG

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static P dims_(0, 0);

// -----------------------------------------------------------------------------
// Cell
// -----------------------------------------------------------------------------
Cell::Cell() :
        is_explored(false),
        is_seen_by_player(false),
        player_los(),
        item(nullptr),
        rigid(nullptr) {}

Cell::~Cell()
{
        delete rigid;
        delete item;
}

void Cell::reset()
{
        is_explored = false;

        is_seen_by_player = false;

        player_los.is_blocked_hard = true;

        player_los.is_blocked_by_drk = false;

        delete rigid;
        rigid = nullptr;

        delete item;
        item = nullptr;
}

// -----------------------------------------------------------------------------
// map
// -----------------------------------------------------------------------------
namespace map
{

Player* player = nullptr;

int dlvl = 0;

Color wall_color;

Array2<Cell> cells(0, 0);

Array2<bool> light(0, 0);
Array2<bool> dark(0, 0);

std::vector<Room*> room_list;

Array2<Room*> room_map(0, 0);

std::vector<ChokePointData> choke_point_data;

void init()
{
        dlvl = 0;

        room_list.clear();

        Actor* actor = actor_factory::make(ActorId::player, {0, 0});

        player = static_cast<Player*>(actor);
}

void cleanup()
{
        reset(P(0, 0));

        // NOTE: The player object is deleted elsewhere
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

void reset(const P& dims)
{
        actor_factory::delete_all_mon();

        game_time::erase_all_mobs();

        game_time::reset_turn_type_and_actor_counters();

        for (auto* room : room_list)
        {
                delete room;
        }

        room_list.clear();

        choke_point_data.clear();

        for (auto& cell : cells)
        {
                cell.reset();
        }

        dims_ = dims;

        cells.resize(dims_);
        light.resize(dims_);
        dark.resize(dims_);
        room_map.resize(dims_);

        for (int x = 0; x < w(); ++x)
        {
                for (int y = 0; y < h(); ++y)
                {
                        put(new Wall(P(x, y)));
                }
        }

        // Occasionally set wall color to something unusual
        if (rnd::one_in(3))
        {
                std::vector<Color> wall_color_bucket = {
                        colors::red(),
                        colors::sepia(),
                        colors::dark_sepia(),
                        colors::dark_brown(),
                        colors::gray_brown(),
                };

                wall_color = rnd::element(wall_color_bucket);
        }
        else // Standard wall color
        {
                wall_color = colors::gray();
        }
}

int w()
{
        return dims_.x;
}

int h()
{
        return dims_.y;
}

P dims()
{
        return dims_;
}

R rect()
{
        return R({0, 0}, dims_ - 1);
}

size_t nr_cells()
{
        return cells.length();
}

Rigid* put(Rigid* const f)
{
        ASSERT(f);

        const P p = f->pos();

        Cell& cell = cells.at(p);

        delete cell.rigid;

        cell.rigid = f;

#ifndef NDEBUG
        if (init::is_demo_mapgen)
        {
                if (f->id() == FeatureId::floor)
                {
                        if (!viewport::is_in_view(p))
                        {
                                viewport::focus_on(p);
                        }

                        for (auto& cell : cells)
                        {
                                cell.is_seen_by_player =
                                        cell.is_explored = true;
                        }

                        states::draw();

                        io::draw_symbol(
                                TileId::aim_marker_line,
                                'X',
                                Panel::map,
                                viewport::to_view_pos(p),
                                colors::yellow());

                        io::update_screen();

                        // NOTE: Delay must be > 1 for user input to be read
                        sdl_base::sleep(3);
                }
        }
#endif // NDEBUG

        return f;
}

void update_vision()
{
        game_time::update_light_map();

        map::player->update_fov();

        map::player->update_mon_awareness();

        states::draw();
}

void make_blood(const P& origin)
{
        for (int dx = -1; dx <= 1; ++dx)
        {
                for (int dy = -1; dy <= 1; ++dy)
                {
                        const P c = origin + P(dx, dy);

                        Rigid* const f = cells.at(c).rigid;

                        if (f->can_have_blood())
                        {
                                if (rnd::one_in(3))
                                {
                                        f->make_bloody();
                                }
                        }
                }
        }
}

void make_gore(const P& origin)
{
        for (int dx = -1; dx <= 1; ++dx)
        {
                for (int dy = -1; dy <= 1; ++dy)
                {
                        const P c = origin + P(dx, dy);

                        if (rnd::one_in(3))
                        {
                                cells.at(c).rigid->try_put_gore();
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
        ASSERT(is_pos_inside_map(p));

        return cells.at(p).is_seen_by_player;
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

Array2<Actor*> get_actor_array()
{
        Array2<Actor*> a(dims());

        for (Actor* actor : game_time::actors)
        {
                const P& p = actor->pos;

                a.at(p) = actor;
        }

        return a;
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

        // Store all actors with distance equal to the nearest distance
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

bool is_pos_inside_map(const P& pos)
{
        return
                (pos.x >= 0) &&
                (pos.y >= 0) &&
                (pos.x < w()) &&
                (pos.y < h());
}

bool is_pos_inside_outer_walls(const P& pos)
{
        return
                (pos.x > 0) &&
                (pos.y > 0) &&
                (pos.x < (w() - 1)) &&
                (pos.y < (h() - 1));
}

bool is_area_inside_map(const R& area)
{
        return
                is_pos_inside_map(area.p0) &&
                is_pos_inside_map(area.p1);
}

} // map
