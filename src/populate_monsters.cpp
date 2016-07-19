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

namespace populate_mon
{

namespace
{

int random_out_of_depth()
{
    if (map::dlvl == 0)
    {
        return 0;
    }

    if (rnd::one_in(40) && map::dlvl > 1)
    {
        return 5;
    }

    if (rnd::one_in(5))
    {
        return 2;
    }

    return 0;
}

void mk_list_of_mon_can_auto_spawn(const int nr_lvls_out_of_depth,
                                   std::vector<ActorId>& list_ref)
{
    list_ref.clear();

    const int effective_dlvl =
        constr_in_range(1, map::dlvl + nr_lvls_out_of_depth, dlvl_last);

    //Get list of actors currently on the level (to help avoid spawning multiple uniques,
    //note that this could otherwise happen for example with Zuul - he is allowed to
    //spawn freely after he appears from a possessed Cultist priest)
    bool spawned_ids[size_t(ActorId::END)];

    for (bool& v : spawned_ids) {v = false;}

    for (const auto* const actor : game_time::actors)
    {
        spawned_ids[size_t(actor->id())] = true;
    }

    for (const auto& d : actor_data::data)
    {
        if (
            d.id != ActorId::player            &&
            d.is_auto_spawn_allowed             &&
            d.nr_left_allowed_to_spawn != 0     &&
            effective_dlvl >= d.spawn_min_dlvl  &&
            effective_dlvl <= d.spawn_max_dlvl  &&
            !(d.is_unique && spawned_ids[size_t(d.id)]))
        {
            list_ref.push_back(d.id);
        }
    }
}

void mk_group_at(const ActorId id,
                 const std::vector<P>& sorted_free_cells,
                 bool blocked_out[map_w][map_h],
                 const bool is_roaming_allowed)
{
    const ActorDataT& d = actor_data::data[int(id)];

    int max_nr_in_group = 1;

    //First, determine the type of group by a weighted choice
    std::vector<int> weights;

    for (const auto& rule : d.group_sizes)
    {
        weights.push_back(rule.weight);
    }

    const int rnd_choice = rnd::weighted_choice(weights);

    const MonGroupSize group_size = d.group_sizes[rnd_choice].group_size;

    //Determine the actual amount of monsters to spawn based on the group type
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

    const int nr_free_cells     = sorted_free_cells.size();
    const int nr_can_be_spawned = std::min(nr_free_cells, max_nr_in_group);

    for (int i = 0; i < nr_can_be_spawned; ++i)
    {
        const P& p = sorted_free_cells[i];

        ASSERT(!blocked_out[p.x][p.y]);

        Actor* const    actor       = actor_factory::mk(id, p);
        Mon* const      mon         = static_cast<Mon*>(actor);
        mon->is_roaming_allowed_    = is_roaming_allowed;

        if (i == 0)
        {
            origin_actor = actor;
        }
        else //Not origin actor
        {
            //The monster may have been assigned a leader when placed
            //(e.g. Ghouls allied to a player Ghoul, or other special cases).
            //If not, we assign the origin monster as leader of this group.
            if (!mon->leader_)
            {
                mon->leader_ = origin_actor;
            }
        }

        blocked_out[p.x][p.y] = true;
    }
}

bool mk_random_group_for_room(const RoomType room_type,
                              const std::vector<P>& sorted_free_cells,
                              bool blocked_out[map_w][map_h],
                              const bool is_roaming_allowed)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    const int nr_lvls_out_of_depth_allowed = random_out_of_depth();
    std::vector<ActorId> id_bucket;
    mk_list_of_mon_can_auto_spawn(nr_lvls_out_of_depth_allowed, id_bucket);

    for (size_t i = 0; i < id_bucket.size(); ++i)
    {
        const ActorDataT& d = actor_data::data[size_t(id_bucket[i])];

        bool is_mon_native_to_room = false;

        for (const auto native_room_type : d.native_rooms)
        {
            if (native_room_type == room_type)
            {
                is_mon_native_to_room = true;
                break;
            }
        }

        if (!is_mon_native_to_room)
        {
            id_bucket.erase(id_bucket.begin() + i);
            --i;
        }
    }

    if (id_bucket.empty())
    {
        TRACE_VERBOSE << "Found no valid monsters to spawn at room type ("
                      << to_str(int(room_type)) + ")" << std::endl;
        TRACE_FUNC_END_VERBOSE;
        return false;
    }
    else //Found valid monster IDs
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
    std::vector<ActorId> id_bucket;
    mk_list_of_mon_can_auto_spawn(nr_lvls_out_of_depth_allowed, id_bucket);

    if (!id_bucket.empty())
    {
        const ActorId id = id_bucket[rnd::range(0, id_bucket.size() - 1)];

        mk_group_at(id, sorted_free_cells, blocked_out, is_roaming_allowed);
    }
}

void mk_sorted_free_cells(const P& origin,
                          const bool blocked[map_w][map_h],
                          std::vector<P>& vector_ref)
{
    vector_ref.clear();

    const int radi = 10;
    const int X0 = constr_in_range(1, origin.x - radi, map_w - 2);
    const int Y0 = constr_in_range(1, origin.y - radi, map_h - 2);
    const int X1 = constr_in_range(1, origin.x + radi, map_w - 2);
    const int Y1 = constr_in_range(1, origin.y + radi, map_h - 2);

    for (int x = X0; x <= X1; ++x)
    {
        for (int y = Y0; y <= Y1; ++y)
        {
            if (!blocked[x][y])
            {
                vector_ref.push_back(P(x, y));
            }
        }
    }

    IsCloserToPos sorter(origin);
    std::sort(vector_ref.begin(), vector_ref.end(), sorter);
}

} //namespace

void try_spawn_due_to_time_passed()
{
    TRACE_FUNC_BEGIN;

    if (game_time::actors.size() >= max_nr_actors_on_map)
    {
        return;
    }

    bool blocked[map_w][map_h];
    map_parse::run(cell_check::BlocksMoveCmn(true), blocked);

    const int min_dist_to_player = fov_std_radi_int + 3;

    const P& player_pos = map::player->pos;

    const int X0 = std::max(0,           player_pos.x - min_dist_to_player);
    const int Y0 = std::max(0,           player_pos.y - min_dist_to_player);
    const int X1 = std::min(map_w - 1,   player_pos.x + min_dist_to_player);
    const int Y1 = std::min(map_h - 1,   player_pos.y + min_dist_to_player);

    for (int x = X0; x <= X1; ++x)
    {
        for (int y = Y0; y <= Y1; ++y)
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
        const int   element = rnd::range(0, free_cells_vector.size() - 1);
        const P&    origin  = free_cells_vector[element];

        mk_sorted_free_cells(origin, blocked, free_cells_vector);

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
    const int nr_groups_allowed = rnd::range(1, 3);

    bool blocked[map_w][map_h];

    const int min_dist_from_player = fov_std_radi_int + 3;
    map_parse::run(cell_check::BlocksMoveCmn(true), blocked);

    const P& player_pos = map::player->pos;

    const int X0 = std::max(0, player_pos.x - min_dist_from_player);
    const int Y0 = std::max(0, player_pos.y - min_dist_from_player);
    const int X1 = std::min(map_w - 1, player_pos.x + min_dist_from_player) - 1;
    const int Y1 = std::min(map_h - 1, player_pos.y + min_dist_from_player) - 1;

    for (int y = Y0; y <= Y1; ++y)
    {
        for (int x = X0; x <= X1; ++x)
        {
            blocked[x][y] = true;
        }
    }

    //Find possible monsters that can spawn on intro level (dlvl 0)
    std::vector<ActorId> ids_can_spawn_intro_lvl;

    for (size_t i = 0; i < size_t(ActorId::END); ++i)
    {
        if (actor_data::data[i].spawn_min_dlvl == 0)
        {
            ids_can_spawn_intro_lvl.push_back(ActorId(i));
        }
    }

    ASSERT(!ids_can_spawn_intro_lvl.empty());

    for (int i = 0; i < nr_groups_allowed; ++i)
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
        std::vector<P> sorted_free_cells;
        mk_sorted_free_cells(origin, blocked, sorted_free_cells);


        if (!sorted_free_cells.empty())
        {
            const int id_element = rnd::range(0, ids_can_spawn_intro_lvl.size() - 1);

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

    const int nr_groups_allowed_on_map = rnd::range(5, 9);

    int nr_groups_spawned = 0;

    bool blocked[map_w][map_h];

    const int min_dist_from_player = fov_std_radi_int - 1;

    map_parse::run(cell_check::BlocksMoveCmn(true), blocked);

    const P& player_pos = map::player->pos;

    const int X0 = std::max(0, player_pos.x - min_dist_from_player);
    const int Y0 = std::max(0, player_pos.y - min_dist_from_player);
    const int X1 = std::min(map_w - 1, player_pos.x + min_dist_from_player);
    const int Y1 = std::min(map_h - 1, player_pos.y + min_dist_from_player);

    for (int x = X0; x <= X1; ++x)
    {
        for (int y = Y0; y <= Y1; ++y)
        {
            blocked[x][y] = true;
        }
    }

    //First, attempt to populate all non-plain standard rooms
    for (Room* const room : map::room_list)
    {
        if (
            room->type_ != RoomType::plain &&
            (int)room->type_ < (int)RoomType::END_OF_STD_ROOMS)
        {
            //TODO: This is not a good method to calculate the number of room cells
            //(the room may be irregularly shaped), parse the room map instead
            const int room_w = room->r_.p1.x - room->r_.p0.x + 1;
            const int room_h = room->r_.p1.y - room->r_.p0.y + 1;
            const int nr_cells_in_room = room_w * room_h;

            const int max_nr_groups_in_room = room->max_nr_mon_groups_spawned();

            for (int i = 0; i < max_nr_groups_in_room; ++i)
            {
                //Randomly pick a free position inside the room
                std::vector<P> origin_bucket;

                for (int y = room->r_.p0.y; y <= room->r_.p1.y; ++y)
                {
                    for (int x = room->r_.p0.x; x <= room->r_.p1.x; ++x)
                    {
                        if (map::room_map[x][y] == room && !blocked[x][y])
                        {
                            origin_bucket.push_back(P(x, y));
                        }
                    }
                }

                //If room is too full (due to spawned monsters and features),
                //stop spawning in this room
                const int nr_origin_candidates = origin_bucket.size();

                if (nr_origin_candidates < (nr_cells_in_room / 3))
                {
                    break;
                }

                //Spawn monsters in room
                if (nr_origin_candidates > 0)
                {
                    const int   element = rnd::range(0, nr_origin_candidates - 1);
                    const P&    origin  = origin_bucket[element];

                    std::vector<P> sorted_free_cells;
                    mk_sorted_free_cells(origin, blocked, sorted_free_cells);

                    const bool did_make_group =
                        mk_random_group_for_room(room->type_,
                                                 sorted_free_cells,
                                                 blocked,
                                                 true);

                    if (did_make_group)
                    {
                        ++nr_groups_spawned;

                        if (nr_groups_spawned >= nr_groups_allowed_on_map)
                        {
                            TRACE_FUNC_END;
                            return;
                        }
                    }
                }
            }

            //After attempting to populate a non-plain themed room,
            //mark that area as forbidden
            for (int y = room->r_.p0.y; y <= room->r_.p1.y; ++y)
            {
                for (int x = room->r_.p0.x; x <= room->r_.p1.x; ++x)
                {
                    blocked[x][y] = true;
                }
            }
        }
    }

    //Second, place groups randomly in plain-themed areas until no more groups to place
    std::vector<P> origin_bucket;

    for (int y = 1; y < map_h - 1; ++y)
    {
        for (int x = 1; x < map_w - 1; ++x)
        {
            if (map::room_map[x][y])
            {
                if (
                    !blocked[x][y] &&
                    map::room_map[x][y]->type_ == RoomType::plain)
                {
                    origin_bucket.push_back(P(x, y));
                }
            }
        }
    }

    if (!origin_bucket.empty())
    {
        while (nr_groups_spawned < nr_groups_allowed_on_map)
        {
            const int   element = rnd::range(0, origin_bucket.size() - 1);
            const P     origin  = origin_bucket[element];

            std::vector<P> sorted_free_cells;

            mk_sorted_free_cells(origin,
                                 blocked,
                                 sorted_free_cells);

            const bool did_make_group =
                mk_random_group_for_room(RoomType::plain,
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

} //populate_mon
