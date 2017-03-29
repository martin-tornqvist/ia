#include "mapgen.hpp"

#include "feature_door.hpp"
#include "map_parsing.hpp"

namespace mapgen
{

namespace
{

void try_mk_door(const P& p)
{
    // Check that no other doors are within a certain distance
    const int r = 2;

    for (int dx = -r; dx <= r; ++dx)
    {
        for (int dy = -r; dy <= r; ++dy)
        {
            const P check_pos = p + P(dx, dy);

            if ((dx != 0 || dy != 0) && map::is_pos_inside_map(check_pos))
            {
                const Cell& cell = map::cells[check_pos.x][check_pos.y];

                if (cell.rigid->id() == FeatureId::door)
                {
                    return;
                }
            }
        }
    }

    bool is_good_ver = true;
    bool is_good_hor = true;

    for (int d = -1; d <= 1; d++)
    {
        if (map::cells[p.x + d][p.y].rigid->id() == FeatureId::wall)
        {
            is_good_hor = false;
        }

        if (map::cells[p.x][p.y + d].rigid->id() == FeatureId::wall)
        {
            is_good_ver = false;
        }

        if (d != 0)
        {
            if (map::cells[p.x][p.y + d].rigid->id() != FeatureId::wall)
            {
                is_good_hor = false;
            }

            if (map::cells[p.x + d][p.y].rigid->id() != FeatureId::wall)
            {
                is_good_ver = false;
            }
        }
    }

    if (is_good_hor || is_good_ver)
    {
        //
        // Make most doors "common" wooden doors, and occasionally make gates
        //
        Door* door = nullptr;

        if (rnd::fraction(4, 5))
        {
            const Wall* const mimic = new Wall(p);

            door = new Door(p,
                            mimic,
                            DoorType::wood);
        }
        else // Barred gate
        {
            door = new Door(p,
                            nullptr,
                            DoorType::gate);
        }

        map::put(door);
    }
}

} // namespace

void mk_doors()
{
    TRACE << "Placing doors" << std:: endl;

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (door_proposals[x][y] &&
                rnd::fraction(4, 5))
            {
                try_mk_door(P(x, y));
            }
        }
    }
}

void mk_metal_doors_and_levers()
{
    //
    // Only make metal on some maps, and never late game (theme)
    //
    const Fraction chance_to_mk_metal_door(1, 2);

    if ((map::dlvl >= dlvl_first_late_game) ||
        !chance_to_mk_metal_door.roll())
    {
        return;
    }

    //
    // Find all chokepoints with a door
    //
    std::vector<const ChokePointData*> chokepoint_bucket;

    for (const auto& chokepoint : map::choke_point_data)
    {
        if (chokepoint.sides[0].empty() ||
            chokepoint.sides[1].empty())
        {
            continue;
        }

        const P& p = chokepoint.p;

        auto id = map::cells[p.x][p.y].rigid->id();

        if (id == FeatureId::door)
        {
            chokepoint_bucket.push_back(&chokepoint);
        }
    }

    if (chokepoint_bucket.empty())
    {
        return;
    }

    //
    // Shuffle the chokepoint list
    //
    rnd::shuffle(chokepoint_bucket);

    // Only allow levers in cells completely surrounded by floor
    bool blocked_common_tmp[map_w][map_h];

    map_parsers::IsNotFeature(FeatureId::floor)
        .run(blocked_common_tmp);

    bool blocked_common[map_w][map_h];

    map_parsers::expand(blocked_common_tmp, blocked_common);

    // Block cells with actors
    for (const auto* const actor : game_time::actors)
    {
        // valid[actor->pos.x][actor->pos.y] = false;

        blocked_common[actor->pos.x][actor->pos.y] = true;
    }

    // Do not allow levers in cells too near the player
    bool blocked_move[map_w][map_h];

    map_parsers::BlocksMoveCommon(ParseActors::no)
        .run(blocked_move);

    // Do not consider doors blocking for the sake of floodfilling
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (map::cells[x][y].rigid->id() == FeatureId::door)
            {
                blocked_move[x][y] = false;
            }
        }
    }

    int player_flood[map_w][map_h];

    floodfill(map::player->pos,
              blocked_move,
              player_flood);

    const int min_dist = 20;

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (player_flood[x][y] < min_dist)
            {
                blocked_common[x][y] = true;
            }
        }
    }

    // Cells blocked on sides 1 and 2
    bool blocked_1[map_w][map_h];
    bool blocked_2[map_w][map_h];

    for (const auto* const chokepoint : chokepoint_bucket)
    {
        //
        // Find a lever position both sides of the chokepoint
        //
        memcpy(blocked_1, blocked_common, nr_map_cells);
        memcpy(blocked_2, blocked_common, nr_map_cells);

        // Block all cells closer to the player than the door - it's probably
        // (always?) more interesting to find the door first
        const P& door_p = chokepoint->p;

        const int door_flood_value = player_flood[door_p.x][door_p.y];

        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                if (player_flood[x][y] <= door_flood_value)
                {
                    blocked_1[x][y] = true;
                    blocked_2[x][y] = true;
                }
            }
        }

        const auto& side_1 = chokepoint->sides[0];
        const auto& side_2 = chokepoint->sides[1];

        ASSERT(!side_1.empty());
        ASSERT(!side_2.empty());

        // Block side 2 positions for the side 1 lever
        for (const auto& p : side_2)
        {
            blocked_1[p.x][p.y] = true;
        }

        // Block side 1 positions for the side 2 lever
        for (const auto& p : side_1)
        {
            blocked_2[p.x][p.y] = true;
        }

        std::vector<P> spawn_weight_positions_1;
        std::vector<P> spawn_weight_positions_2;

        std::vector<int> spawn_weights_1;
        std::vector<int> spawn_weights_2;

        mapgen::mk_explore_spawn_weights(
            blocked_1,
            spawn_weight_positions_1,
            spawn_weights_1);

        mapgen::mk_explore_spawn_weights(
            blocked_2,
            spawn_weight_positions_2,
            spawn_weights_2);

        if (spawn_weights_1.empty() ||
            spawn_weights_2.empty())
        {
            // Failed to find valid lever positions, try next door position
            continue;
        }

        const size_t lever_1_p_idx = rnd::weighted_choice(spawn_weights_1);

        const size_t lever_2_p_idx = rnd::weighted_choice(spawn_weights_2);

        const P& lever_1_p = spawn_weight_positions_1[lever_1_p_idx];

        const P& lever_2_p = spawn_weight_positions_2[lever_2_p_idx];

        //
        // OK, we have found valid positions for the door and for both levers,
        // now we can place them on the map
        //
        auto* door = new Door(door_p,
                              nullptr, // No mimic needed
                              DoorType::metal,
                              DoorSpawnState::closed);

        map::put(door);

        auto* const lever_1 = new Lever(lever_1_p);
        auto* const lever_2 = new Lever(lever_2_p);

        lever_1->set_linked_feature(door);
        lever_2->set_linked_feature(door);

        lever_1->add_sibbling(lever_2);
        lever_2->add_sibbling(lever_1);

        map::put(lever_1);
        map::put(lever_2);

        // We are done
        return;
    }
}

} // mapgen
