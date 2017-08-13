#include "populate_monsters.hpp"

#include <algorithm>

#include "init.hpp"
#include "room.hpp"
#include "feature_trap.hpp"
#include "map.hpp"
#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "map_parsing.hpp"
#include "game_time.hpp"
#include "flood.hpp"

namespace populate_mon
{

namespace
{

int random_out_of_depth()
{
    int nr_levels = 0;

    if ((map::dlvl > 0) &&
        rnd::one_in(14))
    {
        nr_levels = 3;
    }

    return nr_levels;
}

std::vector<ActorId> valid_auto_spawn_monsters(const int nr_lvls_out_of_depth)
{
    std::vector<ActorId> ret;

    const int effective_dlvl =
        constr_in_range(1,
                        map::dlvl + nr_lvls_out_of_depth,
                        dlvl_last);

    // Get list of actors currently on the level (to help avoid spawning
    // multiple uniques, note that this could otherwise happen for example with
    // Zuul - he is allowed to spawn freely after he appears from a possessed
    // Cultist priest)
    bool spawned_ids[(size_t)ActorId::END] = {};

    for (const auto* const actor : game_time::actors)
    {
        spawned_ids[(size_t)actor->id()] = true;
    }

    for (const auto& d : actor_data::data)
    {
        if (d.id != ActorId::player &&
            d.is_auto_spawn_allowed &&
            d.nr_left_allowed_to_spawn != 0 &&
            effective_dlvl >= d.spawn_min_dlvl &&
            effective_dlvl <= d.spawn_max_dlvl &&
            !(d.is_unique && spawned_ids[(size_t)d.id]))
        {
            ret.push_back(d.id);
        }
    }

    return ret;
}

bool mk_random_group_for_room(const RoomType room_type,
                              const std::vector<P>& sorted_free_cells,
                              bool blocked_out[map_w][map_h],
                              const bool is_roaming_allowed)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    const int nr_lvls_out_of_depth_allowed = random_out_of_depth();

    auto id_bucket = valid_auto_spawn_monsters(nr_lvls_out_of_depth_allowed);

    // Remove monsters which do not belong in this room
    for (size_t i = 0; i < id_bucket.size(); ++i)
    {
        // Ocassionally allow any monster type, to mix things up a bit
        const int allow_any_one_in_n = 11;

        if (!rnd::one_in(allow_any_one_in_n))
        {
            bool is_native = false;

            const auto id = id_bucket[i];

            const ActorDataT& d = actor_data::data[(size_t)id];

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
        TRACE_VERBOSE << "Found no valid monsters to spawn at room type ("
                      << std::to_string(int(room_type)) + ")" << std::endl;
        TRACE_FUNC_END_VERBOSE;
        return false;
    }
    else // Found valid monster IDs
    {
        const ActorId id = id_bucket[rnd::range(0, id_bucket.size() - 1)];

        mk_group_at(id,
                    sorted_free_cells,
                    blocked_out,
                    is_roaming_allowed);

        TRACE_FUNC_END_VERBOSE;
        return true;
    }
}

void mk_group_of_random_at(const std::vector<P>& sorted_free_cells,
                           bool blocked_out[map_w][map_h],
                           const int nr_lvls_out_of_depth_allowed,
                           const bool is_roaming_allowed)
{
    const auto id_bucket =
        valid_auto_spawn_monsters(nr_lvls_out_of_depth_allowed);

    if (!id_bucket.empty())
    {
        const ActorId id = id_bucket[rnd::range(0, id_bucket.size() - 1)];

        mk_group_at(id,
                    sorted_free_cells,
                    blocked_out,
                    is_roaming_allowed);
    }
}

} // namespace

std::vector<P> mk_sorted_free_cells(const P& origin,
                                    const bool blocked[map_w][map_h])
{
    std::vector<P> out;

    const int radi = 10;

    const int x0 = constr_in_range(1, origin.x - radi, map_w - 2);
    const int y0 = constr_in_range(1, origin.y - radi, map_h - 2);
    const int x1 = constr_in_range(1, origin.x + radi, map_w - 2);
    const int y1 = constr_in_range(1, origin.y + radi, map_h - 2);

    for (int x = x0; x <= x1; ++x)
    {
        for (int y = y0; y <= y1; ++y)
        {
            if (!blocked[x][y])
            {
                out.push_back(P(x, y));
            }
        }
    }

    IsCloserToPos sorter(origin);

    std::sort(begin(out), end(out), sorter);

    return out;
}

void mk_group_at(const ActorId id,
                 const std::vector<P>& sorted_free_cells,
                 bool blocked_out[map_w][map_h],
                 const bool is_roaming_allowed)
{
    const ActorDataT& d = actor_data::data[(size_t)id];

    int max_nr_in_group = 1;

    // First, determine the type of group by a weighted choice
    std::vector<int> weights;

    for (const auto& rule : d.group_sizes)
    {
        weights.push_back(rule.weight);
    }

    const int rnd_choice = rnd::weighted_choice(weights);

    const MonGroupSize group_size = d.group_sizes[rnd_choice].group_size;

    // Determine the actual amount of monsters to spawn based on the group type
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
            ASSERT(!blocked_out[p.x][p.y]);
        }

        Actor* const actor = actor_factory::mk(id, p);

        Mon* const mon = static_cast<Mon*>(actor);

        mon->is_roaming_allowed_ = is_roaming_allowed;

        if (i == 0)
        {
            origin_actor = actor;
        }
        else // Not origin actor
        {
            // The monster may have been assigned a leader when placed
            // (e.g. Ghouls allied to a player Ghoul, or other special cases).
            // If not, we assign the origin monster as leader of this group.
            if (!mon->leader_)
            {
                mon->leader_ = origin_actor;
            }
        }

        if (blocked_out != nullptr)
        {
            blocked_out[p.x][p.y] = true;
        }
    }
}

void try_spawn_due_to_time_passed()
{
    TRACE_FUNC_BEGIN;

    if (game_time::actors.size() >= max_nr_actors_on_map)
    {
        return;
    }

    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCommon(ParseActors::yes)
        .run(blocked);

    const int min_dist_to_player = fov_std_radi_int + 1;

    const P& player_pos = map::player->pos;

    const int x0 = std::max(0, player_pos.x - min_dist_to_player);
    const int y0 = std::max(0, player_pos.y - min_dist_to_player);
    const int x1 = std::min(map_w - 1, player_pos.x + min_dist_to_player);
    const int y1 = std::min(map_h - 1, player_pos.y + min_dist_to_player);

    for (int x = x0; x <= x1; ++x)
    {
        for (int y = y0; y <= y1; ++y)
        {
            blocked[x][y] = true;
        }
    }

    std::vector<P> free_cells_vector;

    for (int x = 1; x < map_w - 2; ++x)
    {
        for (int y = 1; y < map_h - 2; ++y)
        {
            if (!blocked[x][y])
            {
                free_cells_vector.push_back(P(x, y));
            }
        }
    }

    if (!free_cells_vector.empty())
    {
        const P origin = rnd::element(free_cells_vector);

        free_cells_vector = mk_sorted_free_cells(origin, blocked);

        if (!free_cells_vector.empty())
        {
            if (map::cells[origin.x][origin.y].is_explored)
            {
                const int nr_ood = random_out_of_depth();

                mk_group_of_random_at(free_cells_vector,
                                      blocked,
                                      nr_ood,
                                      true);
            }
        }
    }

    TRACE_FUNC_END;
}

void populate_intro_lvl()
{
    const int nr_groups_to_spawn = rnd::range(1, 3);

    bool blocked[map_w][map_h];

    const int min_dist_from_player = fov_std_radi_int + 3;

    map_parsers::BlocksMoveCommon(ParseActors::yes)
        .run(blocked);

    const P& player_pos = map::player->pos;

    const int x0 = std::max(0, player_pos.x - min_dist_from_player);
    const int y0 = std::max(0, player_pos.y - min_dist_from_player);
    const int x1 = std::min(map_w - 1, player_pos.x + min_dist_from_player) - 1;
    const int y1 = std::min(map_h - 1, player_pos.y + min_dist_from_player) - 1;

    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            blocked[x][y] = true;
        }
    }

    // Find possible monsters that can spawn on intro level (dlvl 0)
    std::vector<ActorId> ids_can_spawn_intro_lvl;

    for (size_t i = 0; i < (size_t)ActorId::END; ++i)
    {
        if (actor_data::data[i].spawn_min_dlvl == 0)
        {
            ids_can_spawn_intro_lvl.push_back(ActorId(i));
        }
    }

    ASSERT(!ids_can_spawn_intro_lvl.empty());

    for (int i = 0; i < nr_groups_to_spawn; ++i)
    {
        std::vector<P> origin_bucket;

        for (int y = 1; y < map_h - 1; ++y)
        {
            for (int x = 1; x < map_w - 1; ++x)
            {
                if (!blocked[x][y])
                {
                    origin_bucket.push_back(P(x, y));
                }
            }
        }

        const int origin_element = rnd::range(0, origin_bucket.size() - 1);

        const P origin = origin_bucket[origin_element];

        const auto sorted_free_cells = mk_sorted_free_cells(origin, blocked);

        if (!sorted_free_cells.empty())
        {
            const int id_element =
                rnd::range(0, ids_can_spawn_intro_lvl.size() - 1);

            const ActorId id = ids_can_spawn_intro_lvl[id_element];

            mk_group_at(id,
                        sorted_free_cells,
                        blocked,
                        true);
        }
    }
}

void populate_std_lvl()
{
    TRACE_FUNC_BEGIN;

    const int nr_groups_to_spawn = rnd::range(4, 7);

    int nr_groups_spawned = 0;

    bool blocked[map_w][map_h];

    const int min_dist_from_player = fov_std_radi_int + 2;

    map_parsers::BlocksMoveCommon(ParseActors::yes)
        .run(blocked);

    int flood[map_w][map_h];

    const P& player_p = map::player->pos;

    floodfill(player_p,
              blocked,
              flood);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const int v = flood[x][y];

            if ((v > 0) &&
                (v < min_dist_from_player))
            {
                blocked[x][y] = true;
            }
        }
    }

    blocked[player_p.x][player_p.y] = true;

    // First, attempt to populate all non-plain standard rooms
    for (Room* const room : map::room_list)
    {
        if ((room->type_ != RoomType::plain) &&
            ((int)room->type_ < (int)RoomType::END_OF_STD_ROOMS))
        {
            //
            // TODO: This is not a good method to calculate the number of room
            //       cells (the room may be irregularly shaped), parse the room
            //       map instead
            //
            const int room_w = room->r_.p1.x - room->r_.p0.x + 1;
            const int room_h = room->r_.p1.y - room->r_.p0.y + 1;

            const int nr_cells_in_room = room_w * room_h;

            const int max_nr_groups_in_room = room->max_nr_mon_groups_spawned();

            const int nr_groups_to_try = rnd::range(1, max_nr_groups_in_room);

            for (int i = 0; i < nr_groups_to_try; ++i)
            {
                // Randomly pick a free position inside the room
                std::vector<P> origin_bucket;

                for (int y = room->r_.p0.y; y <= room->r_.p1.y; ++y)
                {
                    for (int x = room->r_.p0.x; x <= room->r_.p1.x; ++x)
                    {
                        if ((map::room_map[x][y] == room) &&
                            !blocked[x][y])
                        {
                            origin_bucket.push_back(P(x, y));
                        }
                    }
                }

                // If room is too full (due to spawned monsters and features),
                // stop spawning in this room
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
                        mk_sorted_free_cells(origin, blocked);

                    const bool did_make_group =
                        mk_random_group_for_room(
                            room->type_,
                            sorted_free_cells,
                            blocked,
                            true);

                    if (did_make_group)
                    {
                        ++nr_groups_spawned;

                        if (nr_groups_spawned >= nr_groups_to_spawn)
                        {
                            TRACE_FUNC_END;
                            return;
                        }
                    }
                }
            }

            // After attempting to populate a non-plain themed room,
            // mark that area as forbidden
            for (int y = room->r_.p0.y; y <= room->r_.p1.y; ++y)
            {
                for (int x = room->r_.p0.x; x <= room->r_.p1.x; ++x)
                {
                    if (map::room_map[x][y] == room)
                    {
                        blocked[x][y] = true;
                    }
                }
            }
        }
    }

    // Second, place groups randomly in plain-themed areas until no more groups
    // to place
    std::vector<P> origin_bucket;

    for (int y = 1; y < map_h - 1; ++y)
    {
        for (int x = 1; x < map_w - 1; ++x)
        {
            Room* const room = map::room_map[x][y];

            if (!blocked[x][y] &&
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
                mk_sorted_free_cells(origin, blocked);

            const bool did_make_group =
                mk_random_group_for_room(
                    RoomType::plain,
                    sorted_free_cells,
                    blocked,
                    true);

            if (did_make_group)
            {
                ++nr_groups_spawned;
            }
        }
    }

    TRACE_FUNC_END;
}

} // populate_mon
