#include "populate_monsters.hpp"

#include <algorithm>

#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "feature_trap.hpp"
#include "flood.hpp"
#include "game_time.hpp"
#include "init.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "room.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
enum class AllowSpawnUniqueMon
{
        no,
        yes
};

static const int min_dist_to_player = fov_radi_int + 4;

static int random_out_of_depth()
{
        int nr_levels = 0;

        if ((map::dlvl > 0) && rnd::one_in(14))
        {
                nr_levels = 3;
        }

        return nr_levels;
}

static std::vector<ActorId> valid_auto_spawn_monsters(
        const int nr_lvls_out_of_depth,
        const AllowSpawnUniqueMon allow_spawn_unique)
{
        std::vector<ActorId> ret;

        const int effective_dlvl =
                constr_in_range(
                        1,
                        map::dlvl + nr_lvls_out_of_depth,
                        dlvl_last);

        // Get list of actors currently on the level to help avoid spawning
        // multiple unique monsters of the same id
        bool spawned_ids[(size_t)ActorId::END] = {};

        for (const auto* const actor : game_time::actors)
        {
                spawned_ids[(size_t)actor->id()] = true;
        }

        for (const auto& d : actor_data::data)
        {
                if (d.id == ActorId::player)
                {
                        continue;
                }

                if (!d.is_auto_spawn_allowed)
                {
                        continue;
                }

                if (d.nr_left_allowed_to_spawn == 0)
                {
                        continue;
                }

                if (effective_dlvl < d.spawn_min_dlvl)
                {
                        continue;
                }

                if ((d.spawn_max_dlvl != -1) &&
                    (effective_dlvl > d.spawn_max_dlvl))
                {
                        continue;
                }

                if (d.is_unique && spawned_ids[(size_t)d.id])
                {
                        continue;
                }

                if (d.is_unique &&
                    (allow_spawn_unique == AllowSpawnUniqueMon::no))
                {
                        continue;
                }

                ret.push_back(d.id);
        }

        return ret;
}

static bool make_random_group_for_room(
        const RoomType room_type,
        const std::vector<P>& sorted_free_cells,
        Array2<bool>& blocked_out)
{
        TRACE_FUNC_BEGIN_VERBOSE;

        const int nr_lvls_out_of_depth_allowed = random_out_of_depth();

        auto id_bucket =
                valid_auto_spawn_monsters(nr_lvls_out_of_depth_allowed,
                                          AllowSpawnUniqueMon::yes);

        // Remove monsters which do not belong in this room
        for (size_t i = 0; i < id_bucket.size(); ++i)
        {
                // Ocassionally allow any monster type, to mix things up a bit
                const int allow_any_one_in_n = 20;

                if (!rnd::one_in(allow_any_one_in_n))
                {
                        bool is_native = false;

                        const auto id = id_bucket[i];

                        const ActorData& d = actor_data::data[(size_t)id];

                        for (const auto native_room_type : d.native_rooms)
                        {
                                if (native_room_type == room_type)
                                {
                                        is_native = true;

                                        break;
                                }
                        }

                        if (!is_native)
                        {
                                id_bucket.erase(id_bucket.begin() + i);

                                --i;
                        }
                }
        }

        if (id_bucket.empty())
        {
                TRACE_VERBOSE
                        << "Found no valid monsters to spawn at room type ("
                        << std::to_string(int(room_type)) + ")" << std::endl;

                TRACE_FUNC_END_VERBOSE;

                return false;
        }
        else // Found valid monster IDs
        {
                const auto id = rnd::element(id_bucket);

                populate_mon::make_group_at(
                        id,
                        sorted_free_cells,
                        &blocked_out,
                        MonRoamingAllowed::yes);

                TRACE_FUNC_END_VERBOSE;
                return true;
        }
}

static void make_random_group_at(
        const std::vector<P>& sorted_free_cells,
        Array2<bool>& blocked_out,
        const int nr_lvls_out_of_depth_allowed,
        const MonRoamingAllowed is_roaming_allowed,
        const AllowSpawnUniqueMon allow_spawn_unique)
{
        const auto id_bucket =
                valid_auto_spawn_monsters(nr_lvls_out_of_depth_allowed,
                                          allow_spawn_unique);

        if (!id_bucket.empty())
        {
                const auto id = rnd::element(id_bucket);

                populate_mon::make_group_at(
                        id,
                        sorted_free_cells,
                        &blocked_out,
                        is_roaming_allowed);
        }
}

// -----------------------------------------------------------------------------
// populate_mon
// -----------------------------------------------------------------------------
namespace populate_mon
{

void make_group_at(
        const ActorId id,
        const std::vector<P>& sorted_free_cells,
        Array2<bool>* const blocked_out,
        const MonRoamingAllowed is_roaming_allowed)
{
        const ActorData& d = actor_data::data[(size_t)id];

        int max_nr_in_group = 1;

        // First, determine the type of group by a weighted choice
        std::vector<int> weights;

        for (const auto& rule : d.group_sizes)
        {
                weights.push_back(rule.weight);
        }

        const int rnd_choice = rnd::weighted_choice(weights);

        const MonGroupSize group_size = d.group_sizes[rnd_choice].group_size;

        // Determine the number of monsters to spawn based on the group type
        switch (group_size)
        {
        case MonGroupSize::few:
                max_nr_in_group = rnd::range(2, 3);
                break;

        case MonGroupSize::pack:
                max_nr_in_group = rnd::range(4, 5);
                break;

        case MonGroupSize::swarm:
                max_nr_in_group = rnd::range(7, 9);
                break;

        default:
                break;
        }

        Actor* origin_actor = nullptr;

        const int nr_free_cells = sorted_free_cells.size();

        const int nr_can_be_spawned = std::min(nr_free_cells, max_nr_in_group);

        for (int i = 0; i < nr_can_be_spawned; ++i)
        {
                const P& p = sorted_free_cells[i];

                if (blocked_out != nullptr)
                {
                        ASSERT(!blocked_out->at(p));
                }

                Actor* const actor = actor_factory::make(id, p);

                Mon* const mon = static_cast<Mon*>(actor);

                mon->is_roaming_allowed_ = is_roaming_allowed;

                if (i == 0)
                {
                        origin_actor = actor;
                }
                else // Not origin actor
                {
                        // The monster may have been assigned a leader when
                        // placed (e.g. Ghouls allied to a player Ghoul, or
                        // other special cases). If not, we assign the origin
                        // monster as leader of this group.
                        if (!mon->leader_)
                        {
                                mon->leader_ = origin_actor;
                        }
                }

                if (blocked_out != nullptr)
                {
                        blocked_out->at(p) = true;
                }
        }
} // make_group_at

std::vector<P> make_sorted_free_cells(
        const P& origin,
        const Array2<bool>& blocked)
{
        std::vector<P> out;

        const int radi = 10;

        const P dims = blocked.dims();

        const int x0 = constr_in_range(1, origin.x - radi, dims.x - 2);
        const int y0 = constr_in_range(1, origin.y - radi, dims.y - 2);
        const int x1 = constr_in_range(1, origin.x + radi, dims.x - 2);
        const int y1 = constr_in_range(1, origin.y + radi, dims.y - 2);

        for (int x = x0; x <= x1; ++x)
        {
                for (int y = y0; y <= y1; ++y)
                {
                        if (!blocked.at(x, y))
                        {
                                out.push_back(P(x, y));
                        }
                }
        }

        IsCloserToPos sorter(origin);

        std::sort(begin(out), end(out), sorter);

        return out;
}

void make_random_group()
{
        TRACE_FUNC_BEGIN;

        if (game_time::actors.size() >= max_nr_actors_on_map)
        {
                return;
        }

        Array2<bool> blocked(map::dims());

        map_parsers::BlocksMoveCommon(ParseActors::yes)
                .run(blocked, blocked.rect());

        const P& player_pos = map::player->pos;

        const int x0 = std::max(
                0,
                player_pos.x - min_dist_to_player);

        const int y0 = std::max(
                0,
                player_pos.y - min_dist_to_player);

        const int x1 = std::min(
                map::w() - 1,
                player_pos.x + min_dist_to_player);

        const int y1 = std::min(
                map::h() - 1,
                player_pos.y + min_dist_to_player);

        for (int x = x0; x <= x1; ++x)
        {
                for (int y = y0; y <= y1; ++y)
                {
                        blocked.at(x, y) = true;
                }
        }

        std::vector<P> free_cells_vector;

        for (int x = 1; x < map::w() - 2; ++x)
        {
                for (int y = 1; y < map::h() - 2; ++y)
                {
                        if (!blocked.at(x, y))
                        {
                                free_cells_vector.push_back(P(x, y));
                        }
                }
        }

        if (!free_cells_vector.empty())
        {
                const P origin = rnd::element(free_cells_vector);

                free_cells_vector = make_sorted_free_cells(origin, blocked);

                if (!free_cells_vector.empty())
                {
                        if (map::cells.at(origin).is_explored)
                        {
                                const int nr_ood = random_out_of_depth();

                                make_random_group_at(
                                        free_cells_vector,
                                        blocked,
                                        nr_ood,
                                        MonRoamingAllowed::yes,
                                        AllowSpawnUniqueMon::no);
                        }
                }
        }

        TRACE_FUNC_END;
} // make_random_group

void populate_std_lvl()
{
        TRACE_FUNC_BEGIN;

        const int nr_groups_to_spawn = rnd::range(4, 6);

        int nr_groups_spawned = 0;

        Array2<bool> blocked(map::dims());

        map_parsers::BlocksMoveCommon(ParseActors::yes)
                .run(blocked, blocked.rect());

        const P& player_p = map::player->pos;

        const auto flood = floodfill(player_p, blocked);

        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                const int v = flood.at(i);

                if ((v > 0) &&
                    (v < min_dist_to_player))
                {
                        blocked.at(i) = true;
                }
        }

        blocked.at(player_p) = true;

        // First, attempt to populate all non-plain standard rooms
        for (Room* const room : map::room_list)
        {
                if ((room->type_ == RoomType::plain) ||
                    (room->type_ >= RoomType::END_OF_STD_ROOMS))
                {
                        continue;
                }

                // TODO: This is not a good method to calculate the
                // number of room cells (the room may be irregularly
                // shaped), parse the room map instead
                const int room_w = room->r_.p1.x - room->r_.p0.x + 1;
                const int room_h = room->r_.p1.y - room->r_.p0.y + 1;

                const int nr_cells_in_room = room_w * room_h;

                const int max_nr_groups_in_room =
                        room->max_nr_mon_groups_spawned();

                const int nr_groups_to_try =
                        rnd::range(1, max_nr_groups_in_room);

                for (int i = 0; i < nr_groups_to_try; ++i)
                {
                        // Randomly pick a free position inside the room
                        std::vector<P> origin_bucket;

                        for (int y = room->r_.p0.y;
                             y <= room->r_.p1.y;
                             ++y)
                        {
                                for (int x = room->r_.p0.x;
                                     x <= room->r_.p1.x;
                                     ++x)
                                {
                                        const bool is_current_room =
                                                map::room_map.at(x, y) == room;

                                        if (is_current_room &&
                                            !blocked.at(x, y))
                                        {
                                                origin_bucket.push_back(
                                                        P(x, y));
                                        }
                                }
                        }

                        // If room is too full (due to spawned monsters
                        // and features), stop spawning in this room
                        const int nr_origin_candidates = origin_bucket.size();

                        if (nr_origin_candidates < (nr_cells_in_room / 3))
                        {
                                break;
                        }

                        // Spawn monsters in room
                        if (nr_origin_candidates > 0)
                        {
                                const P origin = rnd::element(origin_bucket);

                                const auto sorted_free_cells =
                                        make_sorted_free_cells(origin, blocked);

                                const bool did_make_group =
                                        make_random_group_for_room(
                                                room->type_,
                                                sorted_free_cells,
                                                blocked);

                                if (did_make_group)
                                {
                                        ++nr_groups_spawned;

                                        if (nr_groups_spawned >=
                                            nr_groups_to_spawn)
                                        {
                                                TRACE_FUNC_END;
                                                return;
                                        }
                                }
                        }
                }

                // After attempting to populate a non-plain themed room,
                // mark that area as forbidden
                for (int y = room->r_.p0.y;
                     y <= room->r_.p1.y;
                     ++y)
                {
                        for (int x = room->r_.p0.x;
                             x <= room->r_.p1.x;
                             ++x)
                        {
                                if (map::room_map.at(x, y) == room)
                                {
                                        blocked.at(x, y) = true;
                                }
                        }
                }

        }

        // Second, place groups randomly in plain-themed areas until no more
        // groups to place
        std::vector<P> origin_bucket;

        for (int y = 1; y < map::h() - 1; ++y)
        {
                for (int x = 1; x < map::w() - 1; ++x)
                {
                        Room* const room = map::room_map.at(x, y);

                        if (!blocked.at(x, y) &&
                            room &&
                            (room->type_ == RoomType::plain))
                        {
                                origin_bucket.push_back(P(x, y));
                        }
                }
        }

        if (!origin_bucket.empty())
        {
                while (nr_groups_spawned < nr_groups_to_spawn)
                {
                        const P origin = rnd::element(origin_bucket);

                        const auto sorted_free_cells =
                                make_sorted_free_cells(origin, blocked);

                        const bool did_make_group =
                                make_random_group_for_room(
                                        RoomType::plain,
                                        sorted_free_cells,
                                        blocked);

                        if (did_make_group)
                        {
                                ++nr_groups_spawned;
                        }
                }
        }

        TRACE_FUNC_END;
} // populate_std_lvl

} // populate_mon
