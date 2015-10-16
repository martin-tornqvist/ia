#include "room.hpp"

#include <algorithm>

#include "init.hpp"
#include "utils.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "feature_rigid.hpp"
#include "map_gen.hpp"
#include "gods.hpp"

#ifdef DEMO_MODE
#include "render.hpp"
#include "sdl_wrapper.hpp"
#endif // DEMO_MODE

namespace
{

std::vector<Room_type> room_bucket_;

void add_to_room_bucket(const Room_type type, const size_t NR)
{
    if (NR > 0)
    {
        room_bucket_.reserve(room_bucket_.size() + NR);

        for (size_t i = 0; i < NR; ++i)
        {
            room_bucket_.push_back(type);
        }
    }
}

} //namespace

//------------------------------------------------------------------- ROOM FACTORY
namespace room_factory
{

void init_room_bucket()
{
    TRACE_FUNC_BEGIN;

    room_bucket_.clear();

    const int DLVL = map::dlvl;

    if (DLVL <= DLVL_LAST_EARLY_GAME)
    {
        add_to_room_bucket(Room_type::human,    rnd::range(0, 4));
        add_to_room_bucket(Room_type::ritual,   rnd::range(0, 1));
        add_to_room_bucket(Room_type::spider,   rnd::range(0, 2));
        add_to_room_bucket(Room_type::crypt,    rnd::range(0, 4));
        add_to_room_bucket(Room_type::monster,  rnd::range(0, 2));
        add_to_room_bucket(Room_type::flooded,  rnd::range(0, 1));
        add_to_room_bucket(Room_type::muddy,    rnd::range(0, 1));

        //Add one plain room for every non-plain room
        const size_t NR_PLAIN_ROOMS = room_bucket_.size();

        add_to_room_bucket(Room_type::plain, NR_PLAIN_ROOMS);
    }
    else if (DLVL <= DLVL_LAST_MID_GAME)
    {
        add_to_room_bucket(Room_type::human,    rnd::range(0, 2));
        add_to_room_bucket(Room_type::ritual,   rnd::range(1, 1));
        add_to_room_bucket(Room_type::spider,   rnd::range(0, 3));
        add_to_room_bucket(Room_type::crypt,    rnd::range(0, 4));
        add_to_room_bucket(Room_type::monster,  rnd::range(0, 3));
        add_to_room_bucket(Room_type::flooded,  rnd::range(0, 2));
        add_to_room_bucket(Room_type::muddy,    rnd::range(0, 2));
        add_to_room_bucket(Room_type::cave,     rnd::range(1, 4));
        add_to_room_bucket(Room_type::chasm,    rnd::range(0, 2));
        add_to_room_bucket(Room_type::forest,   rnd::range(1, 4));

        //Add one plain room for every non-plain room
        const size_t NR_PLAIN_ROOMS = room_bucket_.size();

        add_to_room_bucket(Room_type::plain, NR_PLAIN_ROOMS);
    }
    else
    {
        add_to_room_bucket(Room_type::monster,  rnd::range(0, 3));
        add_to_room_bucket(Room_type::spider,   rnd::range(0, 3));
        add_to_room_bucket(Room_type::flooded,  rnd::range(1, 2));
        add_to_room_bucket(Room_type::muddy,    rnd::range(1, 2));
        add_to_room_bucket(Room_type::cave,     rnd::range(3, 4));
        add_to_room_bucket(Room_type::chasm,    rnd::range(0, 3));
        add_to_room_bucket(Room_type::forest,   rnd::range(1, 3));
    }

    std::random_shuffle(begin(room_bucket_), end(room_bucket_));

    TRACE_FUNC_END;
}

Room* mk(const Room_type type, const Rect& r)
{
    switch (type)
    {
    case Room_type::cave:
        return new Cave_room(r);

    case Room_type::chasm:
        return new Chasm_room(r);

    case Room_type::crypt:
        return new Crypt_room(r);

    case Room_type::flooded:
        return new Flooded_room(r);

    case Room_type::human:
        return new Human_room(r);

    case Room_type::monster:
        return new Monster_room(r);

    case Room_type::muddy:
        return new Muddy_room(r);

    case Room_type::plain:
        return new Plain_room(r);

    case Room_type::ritual:
        return new Ritual_room(r);

    case Room_type::spider:
        return new Spider_room(r);

    case Room_type::forest:
        return new Forest_room(r);

    case Room_type::END_OF_STD_ROOMS:
        TRACE << "Illegal room type id: " << int (type) << std::endl;
        assert(false);
        return nullptr;

    case Room_type::corr_link:
        return new Corr_link_room(r);

    case Room_type::crumble_room:
        return new Crumble_room(r);

    case Room_type::river:
        return new River_room(r);
    }

    TRACE << "Unhandled room type id: " << int (type) << std::endl;
    assert(false);
    return nullptr;
}

Room* mk_random_allowed_std_room(const Rect& r, const bool IS_SUBROOM)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    auto room_bucket_it = begin(room_bucket_);

    Room* room = nullptr;

    while (true)
    {
        if (room_bucket_it == end(room_bucket_))
        {
            //No more rooms to pick from, generate a new room bucket
            init_room_bucket();
            room_bucket_it = begin(room_bucket_);
        }
        else
        {
            const Room_type     room_type   = *room_bucket_it;
            room                            = mk(room_type, r);
            room->is_sub_room_              = IS_SUBROOM;
            Std_room* const     std_room    = static_cast<Std_room*>(room);

            if (std_room->is_allowed())
            {
                room_bucket_.erase(room_bucket_it);

                TRACE_FUNC_END_VERBOSE << "Made room type: " << int (room_type) << std::endl;
                break;
            }
            else //Room not allowed (e.g. wrong dimensions)
            {
                delete room;
                ++room_bucket_it; //Try next room type in the bucket
            }
        }
    }

    TRACE_FUNC_END_VERBOSE;
    return room;
}

} //Room_factory

//------------------------------------------------------------------- ROOM
Room::Room(Rect r, Room_type type) :
    r_              (r),
    type_           (type),
    is_sub_room_    (false) {}

void Room::mk_drk() const
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (map::room_map[x][y] == this)
            {
                map::cells[x][y].is_dark = true;
            }
        }
    }

    //Also make sub rooms dark
    for (Room* const sub_room : sub_rooms_)
    {
        sub_room->mk_drk();
    }
}

//------------------------------------------------------------------- STANDARD ROOM
void Std_room::on_pre_connect(bool door_proposals[MAP_W][MAP_H])
{
    on_pre_connect_hook(door_proposals);
}

void Std_room::on_post_connect(bool door_proposals[MAP_W][MAP_H])
{
    place_auto_features();

    on_post_connect_hook(door_proposals);

    //Make dark?
    int pct_chance_dark = base_pct_chance_drk() - 15;

    pct_chance_dark += map::dlvl; //Increase with higher dungeon level

    utils::set_constr_in_range(0, pct_chance_dark, 100);

    if (rnd::percent() < pct_chance_dark)
    {
        mk_drk();
    }
}

size_t Std_room::try_auto_feature_placement(
    const std::vector<P>& adj_to_walls, const std::vector<P>& away_from_walls,
    const std::vector<const Feature_data_t*>& feature_data_bucket, P& pos_ref) const
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (feature_data_bucket.empty())
    {
        pos_ref = P(-1, -1);
        return 0;
    }

    const bool IS_ADJ_TO_WALLS_AVAIL    = !adj_to_walls.empty();
    const bool IS_AWAY_FROM_WALLS_AVAIL = !away_from_walls.empty();

    if (!IS_ADJ_TO_WALLS_AVAIL && !IS_AWAY_FROM_WALLS_AVAIL)
    {
        TRACE_FUNC_END_VERBOSE << "No eligible cells found" << std::endl;
        pos_ref = P(-1, -1);
        return 0;
    }

    //TODO: Use bucket instead

    const int NR_ATTEMPTS_TO_FIND_POS = 100;

    for (int i = 0; i < NR_ATTEMPTS_TO_FIND_POS; ++i)
    {
        const size_t      NR_DATA = feature_data_bucket.size();
        const size_t      ELEMENT = rnd::range(0, NR_DATA - 1);
        const auto* const data    = feature_data_bucket[ELEMENT];

        if (
            IS_ADJ_TO_WALLS_AVAIL &&
            data->room_spawn_rules.placement_rule() == Placement_rule::adj_to_walls)
        {
            pos_ref = adj_to_walls[rnd::range(0, adj_to_walls.size() - 1)];
            TRACE_FUNC_END_VERBOSE;
            return ELEMENT;
        }

        if (
            IS_AWAY_FROM_WALLS_AVAIL &&
            data->room_spawn_rules.placement_rule() == Placement_rule::away_from_walls)
        {
            pos_ref = away_from_walls[rnd::range(0, away_from_walls.size() - 1)];
            TRACE_FUNC_END_VERBOSE;
            return ELEMENT;
        }

        if (data->room_spawn_rules.placement_rule() == Placement_rule::either)
        {
            if (rnd::coin_toss())
            {
                if (IS_ADJ_TO_WALLS_AVAIL)
                {
                    pos_ref = adj_to_walls[rnd::range(0, adj_to_walls.size() - 1)];
                    TRACE_FUNC_END_VERBOSE;
                    return ELEMENT;

                }
            }
            else
            {
                if (IS_AWAY_FROM_WALLS_AVAIL)
                {
                    pos_ref = away_from_walls[rnd::range(0, away_from_walls.size() - 1)];
                    TRACE_FUNC_END_VERBOSE;
                    return ELEMENT;
                }
            }
        }
    }

    TRACE_FUNC_END_VERBOSE;
    return 0;
}

int Std_room::place_auto_features()
{
    TRACE_FUNC_BEGIN;

    std::vector<const Feature_data_t*> feature_bucket;

    for (int i = 0; i < int (Feature_id::END); ++i)
    {
        const auto& d           = feature_data::data((Feature_id)(i));
        const auto& spawn_rules = d.room_spawn_rules;

        if (
            spawn_rules.is_belonging_to_room_type(type_) &&
            spawn_rules.dlvls_allowed().is_in_range(map::dlvl))
        {
            feature_bucket.push_back(&d);
        }
    }

    std::vector<P> adj_to_walls;
    std::vector<P> away_from_walls;

    map_patterns::cells_in_room(*this, adj_to_walls, away_from_walls);

    std::vector<int> spawn_count(feature_bucket.size(), 0);

    int nr_features_left_to_place   = rnd::range(nr_auto_features_allowed());
    int nr_features_placed          = 0;

    while (true)
    {
        if (nr_features_left_to_place == 0)
        {
            TRACE_FUNC_END << "Placed enough features" << std::endl;
            return nr_features_placed;
        }

        P pos(-1, -1);

        const size_t FEATURE_IDX =
            try_auto_feature_placement(adj_to_walls, away_from_walls, feature_bucket, pos);

        if (pos.x >= 0)
        {
            assert(FEATURE_IDX < feature_bucket.size());

            const Feature_data_t* d = feature_bucket[FEATURE_IDX];

            TRACE << "Placing feature" << std::endl;
            map::put(static_cast<Rigid*>(d->mk_obj(pos)));
            ++spawn_count[FEATURE_IDX];

            nr_features_left_to_place--;
            nr_features_placed++;

            //Check if more of this feature can be spawned. If not, erase it.
            if (spawn_count[FEATURE_IDX] >= d->room_spawn_rules.max_nr_in_room())
            {
                spawn_count   .erase(spawn_count   .begin() + FEATURE_IDX);
                feature_bucket.erase(feature_bucket.begin() + FEATURE_IDX);

                if (feature_bucket.empty())
                {
                    TRACE_FUNC_END << "No more features to place" << std::endl;
                    return nr_features_placed;
                }
            }

            //Erase all adjacent positions
            auto is_adj = [&](const P & p)
            {
                return utils::is_pos_adj(p, pos, true);
            };
            adj_to_walls   .erase(remove_if(begin(adj_to_walls),    end(adj_to_walls),    is_adj),
                                  end(adj_to_walls));
            away_from_walls.erase(remove_if(begin(away_from_walls), end(away_from_walls), is_adj),
                                  end(away_from_walls));
        }
        else //No good feature placement found
        {
            TRACE_FUNC_END << "No remaining positions to place feature" << std::endl;
            return nr_features_placed;
        }
    }
}

//------------------------------------------------------------------- PLAIN ROOM
Range Plain_room::nr_auto_features_allowed() const
{
    if (rnd::one_in(4))
    {
        return {1, 2};
    }
    else
    {
        return {0, 0};
    }
}

int Plain_room::base_pct_chance_drk() const
{
    return 5;
}

void Plain_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    map_gen_utils::cut_room_corners(*this);

    if (rnd::fraction(1, 3))
    {
        map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Plain_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;
}

//------------------------------------------------------------------- HUMAN ROOM
Range Human_room::nr_auto_features_allowed() const
{
    return {3, 6};
}

int Human_room::base_pct_chance_drk() const
{
    return 10;
}

bool Human_room::is_allowed() const
{
    return r_.min_dim() >= 4 && r_.max_dim() <= 8;
}

void Human_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    map_gen_utils::cut_room_corners(*this);

    if (rnd::fraction(1, 3))
    {
        map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Human_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    if (rnd::coin_toss())
    {
        bool blocked[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

        for (int x = r_.p0.x + 1; x <= r_.p1.x - 1; ++x)
        {
            for (int y = r_.p0.y + 1; y <= r_.p1.y - 1; ++y)
            {
                if (!blocked[x][y] && map::room_map[x][y] == this)
                {
                    Carpet* const carpet = new Carpet(P(x, y));
                    map::put(carpet);
                }
            }
        }
    }
}

//------------------------------------------------------------------- RITUAL ROOM
Range Ritual_room::nr_auto_features_allowed() const
{
    return {1, 5};
}

int Ritual_room::base_pct_chance_drk() const
{
    return 15;
}

bool Ritual_room::is_allowed() const
{
    return r_.min_dim() >= 4 && r_.max_dim() <= 8;
}

void Ritual_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    map_gen_utils::cut_room_corners(*this);

    if (rnd::fraction(1, 3))
    {
        map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Ritual_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    gods::set_random_god();

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    const int BLOODY_CHAMBER_PCT = 60;

    if (rnd::percent() < BLOODY_CHAMBER_PCT)
    {
        P origin(-1, -1);
        std::vector<P> origin_bucket;

        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            for (int x = r_.p0.x; x <= r_.p1.x; ++x)
            {
                if (map::cells[x][y].rigid->id() == Feature_id::altar)
                {
                    origin = P(x, y);
                    y = 999;
                    x = 999;
                }
                else
                {
                    if (!blocked[x][y])
                    {
                        origin_bucket.push_back(P(x, y));
                    }
                }
            }
        }

        if (!origin_bucket.empty())
        {
            if (origin.x == -1)
            {
                const int ELEMENT = rnd::range(0, origin_bucket.size() - 1);
                origin = origin_bucket[ELEMENT];
            }

            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    if ((dx == 0 && dy == 0) || (rnd::percent() < BLOODY_CHAMBER_PCT / 2))
                    {
                        const P pos = origin + P(dx, dy);

                        if (!blocked[pos.x][pos.y])
                        {
                            map::mk_gore(pos);
                            map::mk_blood(pos);
                        }
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------- SPIDER ROOM
Range Spider_room::nr_auto_features_allowed() const
{
    return {0, 3};
}

int Spider_room::base_pct_chance_drk() const
{
    return 30;
}

bool Spider_room::is_allowed() const
{
    return r_.min_dim() >= 3 && r_.max_dim() <= 8;
}

void Spider_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    //Early game : Always reshape by cutting corners
    //Mid    -   : "Flip a coin"
    //Late   -   : Always reshape by cavifying
    const bool IS_EARLY = map::dlvl <= DLVL_LAST_EARLY_GAME;
    const bool IS_MID   = !IS_EARLY && map::dlvl <= DLVL_LAST_MID_GAME;

    if (IS_EARLY || (IS_MID && rnd::coin_toss()))
    {
        map_gen_utils::cut_room_corners(*this);
    }
    else
    {
        map_gen_utils::cavify_room(*this);
    }

    if ((IS_EARLY || IS_MID) && rnd::fraction(1, 3))
    {
        map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Spider_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;
}

//------------------------------------------------------------------- CRYPT ROOM
Range Crypt_room::nr_auto_features_allowed() const
{
    return {3, 6};
}

int Crypt_room::base_pct_chance_drk() const
{
    return 60;
}

bool Crypt_room::is_allowed() const
{
    return r_.min_dim() >= 3  && r_.max_dim() <= 12;
}

void Crypt_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    map_gen_utils::cut_room_corners(*this);

    if (rnd::fraction(1, 3))
    {
        map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Crypt_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;
}

//------------------------------------------------------------------- MONSTER ROOM
Range Monster_room::nr_auto_features_allowed() const
{
    return {0, 6};
}

int Monster_room::base_pct_chance_drk() const
{
    return 75;
}

bool Monster_room::is_allowed() const
{
    return r_.min_dim() >= 4 && r_.max_dim() <= 8;
}

void Monster_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    const bool IS_EARLY = map::dlvl <= DLVL_LAST_EARLY_GAME;
    const bool IS_MID   = !IS_EARLY && map::dlvl <= DLVL_LAST_MID_GAME;

    if (IS_EARLY || IS_MID)
    {
        if (rnd::fraction(3, 4))
        {
            map_gen_utils::cut_room_corners(*this);
        }

        if (rnd::fraction(1, 3))
        {
            map_gen_utils::mk_pillars_in_room(*this);
        }
    }
    else //Is late game
    {
        map_gen_utils::cavify_room(*this);
    }
}

void Monster_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    int       nr_blood_put  = 0;
    const int NR_TRIES    = 1000; //TODO: Hacky, needs improving

    for (int i = 0; i < NR_TRIES; ++i)
    {
        for (int x = r_.p0.x; x <= r_.p1.x; ++x)
        {
            for (int y = r_.p0.y; y <= r_.p1.y; ++y)
            {
                if (!blocked[x][y] && map::room_map[x][y] == this && rnd::fraction(2, 5))
                {
                    map::mk_gore(P(x, y));
                    map::mk_blood(P(x, y));
                    nr_blood_put++;
                }
            }
        }

        if (nr_blood_put > 0)
        {
            break;
        }
    }
}

//------------------------------------------------------------------- FLOODED ROOM
Range Flooded_room::nr_auto_features_allowed() const
{
    return {0, 0};
}

int Flooded_room::base_pct_chance_drk() const
{
    return 25;
}

bool Flooded_room::is_allowed() const
{
    return true;
}

void Flooded_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    //Early game : Always reshape by cutting corners
    //Mid    -   : "Flip a coin"
    //Late   -   : Always reshape by cavifying
    const bool IS_EARLY = map::dlvl <= DLVL_LAST_EARLY_GAME;
    const bool IS_MID   = !IS_EARLY && map::dlvl <= DLVL_LAST_MID_GAME;

    if (IS_EARLY || (IS_MID && rnd::coin_toss()))
    {
        map_gen_utils::cut_room_corners(*this);
    }
    else
    {
        map_gen_utils::cavify_room(*this);
    }

    if ((IS_EARLY || IS_MID) && rnd::fraction(1, 3))
    {
        map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Flooded_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

#ifndef NDEBUG

    //Sanity check (look for some features that should not exist in this room)
    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (map::room_map[x][y] == this)
            {
                const auto id = map::cells[x][y].rigid->id();

                if (
                    id == Feature_id::chest    ||
                    id == Feature_id::tomb     ||
                    id == Feature_id::cabinet  ||
                    id == Feature_id::fountain)
                {
                    TRACE << "Illegal feature found in room" << std::endl;
                    assert(false);
                }
            }
        }
    }

#endif // NDEBUG

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    const int LIQUID_ONE_IN_N = rnd::range(2, 4);

    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (
                !blocked[x][y] && map::room_map[x][y] == this &&
                rnd::one_in(LIQUID_ONE_IN_N))
            {
                Liquid_shallow* const liquid = new Liquid_shallow(P(x, y));
                liquid->type_ = Liquid_type::water;
                map::put(liquid);
            }
        }
    }
}

//------------------------------------------------------------------- MUDDY ROOM
Range Muddy_room::nr_auto_features_allowed() const
{
    return {0, 0};
}

int Muddy_room::base_pct_chance_drk() const
{
    return 25;
}

bool Muddy_room::is_allowed() const
{
    return true;
}

void Muddy_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

#ifndef NDEBUG

    //Sanity check (look for some features that should not exist in this room)
    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (map::room_map[x][y] == this)
            {
                const auto id = map::cells[x][y].rigid->id();

                if (
                    id == Feature_id::chest    ||
                    id == Feature_id::tomb     ||
                    id == Feature_id::cabinet  ||
                    id == Feature_id::fountain)
                {
                    TRACE << "Illegal feature found in room" << std::endl;
                    assert(false);
                }
            }
        }
    }

#endif // NDEBUG

    //Early game : Always reshape by cutting corners
    //Mid    -   : "Flip a coin"
    //Late   -   : Always reshape by cavifying
    const bool IS_EARLY = map::dlvl <= DLVL_LAST_EARLY_GAME;
    const bool IS_MID   = !IS_EARLY && map::dlvl <= DLVL_LAST_MID_GAME;

    if (IS_EARLY || (IS_MID && rnd::coin_toss()))
    {
        map_gen_utils::cut_room_corners(*this);
    }
    else
    {
        map_gen_utils::cavify_room(*this);
    }

    if ((IS_EARLY || IS_MID) && rnd::fraction(1, 3))
    {
        map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Muddy_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    const int LIQUID_ONE_IN_N = rnd::range(2, 4);

    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (!blocked[x][y] && map::room_map[x][y] == this && rnd::one_in(LIQUID_ONE_IN_N))
            {
                Liquid_shallow* const liquid = new Liquid_shallow(P(x, y));
                liquid->type_ = Liquid_type::mud;
                map::put(liquid);
            }
        }
    }
}

//------------------------------------------------------------------- CAVE ROOM
Range Cave_room::nr_auto_features_allowed() const
{
    return {2, 6};
}

int Cave_room::base_pct_chance_drk() const
{
    return 30;
}

bool Cave_room::is_allowed() const
{
    return !is_sub_room_;
}

void Cave_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    map_gen_utils::cavify_room(*this);
}

void Cave_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;
}

//------------------------------------------------------------------- FOREST ROOM
Range Forest_room::nr_auto_features_allowed() const
{
    if (rnd::one_in(3))
    {
        return {2, 6};
    }
    else
    {
        return {0, 1};
    }
}

int Forest_room::base_pct_chance_drk() const
{
    return 10;
}

bool Forest_room::is_allowed() const
{
    return !is_sub_room_;
}

void Forest_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    map_gen_utils::cavify_room(*this);
}

void Forest_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    std::vector<P> tree_pos_bucket;

    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (!blocked[x][y] && map::room_map[x][y] == this)
            {
                const P p(x, y);
                tree_pos_bucket.push_back(p);

                if (rnd::one_in(6))
                {
                    map::put(new Bush(p));
                }
                else
                {
                    map::put(new Grass(p));
                }
            }
        }
    }

    random_shuffle(begin(tree_pos_bucket), end(tree_pos_bucket));

    int nr_trees_placed = 0;

    const int TREE_ONE_IN_N = rnd::range(2, 5);

    while (!tree_pos_bucket.empty())
    {
        const P p = tree_pos_bucket.back();
        tree_pos_bucket.pop_back();

        if (rnd::one_in(TREE_ONE_IN_N))
        {
            blocked[p.x][p.y] = true;

            if (map_parse::is_map_connected(blocked))
            {
                map::put(new Tree(p));
                ++nr_trees_placed;
            }
            else
            {
                blocked[p.x][p.y] = false;
            }
        }
    }
}

//------------------------------------------------------------------- CHASM ROOM
Range Chasm_room::nr_auto_features_allowed() const
{
    return {0, 0};
}

int Chasm_room::base_pct_chance_drk() const
{
    return 25;
}

bool Chasm_room::is_allowed() const
{
    return r_.min_dim() >= 5 &&
           r_.max_dim() <= 9;
}

void Chasm_room::on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    map_gen_utils::cavify_room(*this);
}

void Chasm_room::on_post_connect_hook(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (map::room_map[x][y] != this)
            {
                blocked[x][y] = true;
            }
        }
    }

    bool blocked_expanded[MAP_W][MAP_H];
    map_parse::expand(blocked, blocked_expanded);

    P origin;

    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (!blocked_expanded[x][y])
            {
                origin.set(x, y);
            }
        }
    }

    int flood[MAP_W][MAP_H];

    flood_fill::run(origin, blocked_expanded, flood, 10000, { -1,  -1}, false);

    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            const P p(x, y);

            if (p == origin || flood[x][y] != 0)
            {
                map::put(new Chasm(p));
            }
        }
    }
}

//------------------------------------------------------------------- RIVER ROOM
void River_room::on_pre_connect(bool door_proposals[MAP_W][MAP_H])
{
    TRACE_FUNC_BEGIN;

    //Strategy: Expand the the river on both sides until parallel to the closest center
    //cell of another room

    const bool IS_HOR = axis_ == Axis::hor;

    TRACE << "Finding room centers" << std::endl;
    bool centers[MAP_W][MAP_H];
    utils::reset_array(centers, false);

    for (Room* const room : map::room_list)
    {
        if (room != this)
        {
            const P c_pos(room->r_.center_pos());
            centers[c_pos.x][c_pos.y] = true;
        }
    }

    TRACE << "Finding closest room center coordinates on both sides "
          << "(y coordinate if horizontal river, x if vertical)" << std::endl;
    int closest_center0 = -1;
    int closest_center1 = -1;

    //Using nestled scope to avoid declaring x and y at function scope
    {
        int x, y;

        //i_outer and i_inner should be references to x or y.
        auto find_closest_center0 =
            [&](const Range & r_outer, const Range & r_inner, int& i_outer, int& i_inner)
        {
            for (i_outer = r_outer.min; i_outer >= r_outer.max; --i_outer)
            {
                for (i_inner = r_inner.min; i_inner <= r_inner.max; ++i_inner)
                {
                    if (centers[x][y])
                    {
                        closest_center0 = i_outer;
                        break;
                    }
                }

                if (closest_center0 != -1)
                {
                    break;
                }
            }
        };

        auto find_closest_center1 =
            [&](const Range & r_outer, const Range & r_inner, int& i_outer, int& i_inner)
        {
            for (i_outer = r_outer.min; i_outer <= r_outer.max; ++i_outer)
            {
                for (i_inner = r_inner.min; i_inner <= r_inner.max; ++i_inner)
                {
                    if (centers[x][y])
                    {
                        closest_center1 = i_outer;
                        break;
                    }
                }

                if (closest_center1 != -1) {break;}
            }
        };

        if (IS_HOR)
        {
            const int RIVER_Y = r_.p0.y;
            find_closest_center0(Range(RIVER_Y - 1, 1),         Range(1, MAP_W - 2),  y, x);
            find_closest_center1(Range(RIVER_Y + 1, MAP_H - 2), Range(1, MAP_W - 2),  y, x);
        }
        else
        {
            const int RIVER_X = r_.p0.x;
            find_closest_center0(Range(RIVER_X - 1, 1),         Range(1, MAP_H - 2),  x, y);
            find_closest_center1(Range(RIVER_X + 1, MAP_W - 2), Range(1, MAP_H - 2),  x, y);
        }
    }

    TRACE << "Expanding and filling river" << std::endl;

    bool blocked[MAP_W][MAP_H];

    //Within the expansion limits, mark all cells not belonging to another room as free.
    //All other cells are considered as blocking.
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            blocked[x][y] = true;

            if (
                (axis_ == Axis::hor && (y >= closest_center0 && y <= closest_center1)) ||
                (axis_ == Axis::ver && (x >= closest_center0 && x <= closest_center1))
            )
            {
                Room* r       = map::room_map[x][y];
                blocked[x][y] = r && r != this;
            }
        }
    }

    bool blocked_expanded[MAP_W][MAP_H];
    map_parse::expand(blocked, blocked_expanded);

    int flood[MAP_W][MAP_H];
    const P origin(r_.center_pos());
    flood_fill::run(origin, blocked_expanded, flood, INT_MAX, P(-1, -1), true);

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const P p(x, y);

            if (flood[x][y] > 0 || p == origin)
            {
//        Liquid_deep* const liquid  = new Liquid_deep(p);
//        liquid->type_             = Liquid_type::water;
                map::put(new Chasm(p));
                map::room_map[x][y] = this;
                r_.p0.x = std::min(r_.p0.x, x);
                r_.p0.y = std::min(r_.p0.y, y);
                r_.p1.x = std::max(r_.p1.x, x);
                r_.p1.y = std::max(r_.p1.y, y);
            }
        }
    }

    TRACE << "Making bridge(s)" << std::endl;

    //Mark which side each cell belongs to
    enum Side {in_river, side0, side1};
    Side sides[MAP_W][MAP_H];

    //Scoping to avoid declaring x and y at function scope
    {
        int x, y;

        //i_outer and i_inner should be references to x or y.
        auto mark_sides =
            [&](const Range & r_outer, const Range & r_inner, int& i_outer, int& i_inner)
        {
            for (i_outer = r_outer.min; i_outer <= r_outer.max; ++i_outer)
            {
                bool is_on_side0 = true;

                for (i_inner = r_inner.min; i_inner <= r_inner.max; ++i_inner)
                {
                    if (map::room_map[x][y] == this)
                    {
                        is_on_side0 = false;
                        sides[x][y] = in_river;
                    }
                    else
                    {
                        sides[x][y] = is_on_side0 ? side0 : side1;
                    }
                }
            }
        };

        if (axis_ == Axis::hor)
        {
            mark_sides(Range(1, MAP_W - 2), Range(1, MAP_H - 2), x, y);
        }
        else
        {
            mark_sides(Range(1, MAP_H - 2), Range(1, MAP_W - 2), y, x);
        }
    }

    bool valid_room_entries0[MAP_W][MAP_H];
    bool valid_room_entries1[MAP_W][MAP_H];

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            valid_room_entries0[x][y] = valid_room_entries1[x][y] = false;
        }
    }

    const int EDGE_D = 4;

    for (int x = EDGE_D; x < MAP_W - EDGE_D; ++x)
    {
        for (int y = EDGE_D; y < MAP_H - EDGE_D; ++y)
        {
            const Feature_id feature_id = map::cells[x][y].rigid->id();

            if (feature_id == Feature_id::wall && !map::room_map[x][y])
            {
                const P p(x, y);
                int nr_cardinal_floor  = 0;
                int nr_cardinal_river  = 0;

                for (const auto& d : dir_utils::cardinal_list)
                {
                    const auto p_adj(p + d);
                    const auto* const f = map::cells[p_adj.x][p_adj.y].rigid;

                    if (f->id() == Feature_id::floor)
                    {
                        nr_cardinal_floor++;
                    }

                    if (map::room_map[p_adj.x][p_adj.y] == this)
                    {
                        nr_cardinal_river++;
                    }
                }

                if (nr_cardinal_floor == 1 && nr_cardinal_river == 1)
                {
                    switch (sides[x][y])
                    {
                    case side0:
                        valid_room_entries0[x][y] = true;
                        break;

                    case side1:
                        valid_room_entries1[x][y] = true;
                        break;

                    case in_river:
                        break;
                    }
                }
            }
        }
    }

#ifdef DEMO_MODE
    render::draw_map();

    for (int y = 1; y < MAP_H - 1; ++y)
    {
        for (int x = 1; x < MAP_W - 1; ++x)
        {
            P p(x, y);

            if (valid_room_entries0[x][y])
            {
                render::draw_glyph('0', Panel::map, p, clr_red_lgt);
            }

            if (valid_room_entries1[x][y])
            {
                render::draw_glyph('1', Panel::map, p, clr_red_lgt);
            }

            if (valid_room_entries0[x][y] || valid_room_entries1[x][y])
            {
                render::update_screen();
                sdl_wrapper::sleep(100);
            }
        }
    }

#endif // DEMO_MODE

    std::vector<int> coordinates(IS_HOR ? MAP_W : MAP_H);
    iota(begin(coordinates), end(coordinates), 0);
    random_shuffle(coordinates.begin(), coordinates.end());

    std::vector<int> c_built;

    const int MIN_EDGE_DIST   = 6;
    const int MAX_NR_BRIDGES  = rnd::range(1, 3);

    for (const int BRIDGE_C : coordinates)
    {
        if (
            BRIDGE_C < MIN_EDGE_DIST ||
            (IS_HOR  && BRIDGE_C > MAP_W - 1 - MIN_EDGE_DIST) ||
            (!IS_HOR && BRIDGE_C > MAP_H - 1 - MIN_EDGE_DIST))
        {
            continue;
        }

        bool is_too_close_to_other_bridge = false;
        const int MIN_D = 2;

        for (int c_other : c_built)
        {
            if (utils::is_val_in_range(BRIDGE_C, Range(c_other - MIN_D, c_other + MIN_D)))
            {
                is_too_close_to_other_bridge = true;
                break;
            }
        }

        if (is_too_close_to_other_bridge) {continue;}

        //Check if current bridge coord would connect matching room connections.
        //If so both room_con0 and room_con1 will be set.
        P room_con0(-1, -1);
        P room_con1(-1, -1);
        const int C0_0 = IS_HOR ? r_.p1.y : r_.p1.x;
        const int C1_0 = IS_HOR ? r_.p0.y : r_.p0.x;

        for (int c = C0_0; c != C1_0; --c)
        {
            if (
                (IS_HOR  && sides[BRIDGE_C][c] == side0) ||
                (!IS_HOR && sides[c][BRIDGE_C] == side0))
            {
                break;
            }

            const P p_nxt = IS_HOR ? P(BRIDGE_C, c - 1) : P(c - 1, BRIDGE_C);

            if (valid_room_entries0[p_nxt.x][p_nxt.y])
            {
                room_con0 = p_nxt;
                break;
            }
        }

        const int C0_1 = IS_HOR ? r_.p0.y : r_.p0.x;
        const int C1_1 = IS_HOR ? r_.p1.y : r_.p1.x;

        for (int c = C0_1; c != C1_1; ++c)
        {
            if (
                (IS_HOR  && sides[BRIDGE_C][c] == side1) ||
                (!IS_HOR && sides[c][BRIDGE_C] == side1))
            {
                break;
            }

            const P p_nxt = IS_HOR ? P(BRIDGE_C, c + 1) : P(c + 1, BRIDGE_C);

            if (valid_room_entries1[p_nxt.x][p_nxt.y])
            {
                room_con1 = p_nxt;
                break;
            }
        }

        //Make the bridge if valid connection pairs found
        if (room_con0.x != -1 && room_con1.x != -1)
        {
#ifdef DEMO_MODE
            render::draw_map();
            render::draw_glyph('0', Panel::map, room_con0, clr_green_lgt);
            render::draw_glyph('1', Panel::map, room_con1, clr_yellow);
            render::update_screen();
            sdl_wrapper::sleep(2000);
#endif // DEMO_MODE

            TRACE << "Found valid connection pair at: "
                  << room_con0.x << "," << room_con0.y << " / "
                  << room_con1.x << "," << room_con1.y << std::endl
                  << "Making bridge at pos: " << BRIDGE_C << std::endl;

            if (IS_HOR)
            {
                for (int y = room_con0.y; y <= room_con1.y; ++y)
                {
                    if (map::room_map[BRIDGE_C][y] == this)
                    {
//            auto* const bridge = new Bridge(P(BRIDGE_C, y));
//            bridge->set_dir(ver);
                        auto* const floor = new Floor({BRIDGE_C, y});
                        floor->type_ = Floor_type::cmn;
                        map::put(floor);
                    }
                }
            }
            else //Vertical
            {
                for (int x = room_con0.x; x <= room_con1.x; ++x)
                {
                    if (map::room_map[x][BRIDGE_C] == this)
                    {
//            auto* const bridge = new Bridge(P(x, BRIDGE_C));
//            bridge->set_dir(hor);
                        auto* const floor = new Floor({x, BRIDGE_C});
                        floor->type_ = Floor_type::cmn;
                        map::put(floor);
                    }
                }
            }

            map::put(new Floor(room_con0));
            map::put(new Floor(room_con1));
            door_proposals[room_con0.x][room_con0.y] = true;
            door_proposals[room_con1.x][room_con1.y] = true;
            c_built.push_back(BRIDGE_C);
        }

        if (int (c_built.size()) >= MAX_NR_BRIDGES)
        {
            TRACE << "Enough bridges built" << std::endl;
            break;
        }
    }

    TRACE << "Bridges built/attempted: " << c_built.size() << "/" << MAX_NR_BRIDGES << std::endl;

    if (c_built.empty())
    {
        map_gen::is_map_valid = false;
    }
    else //map is valid (at least one bridge was built)
    {
        bool valid_room_entries[MAP_W][MAP_H];

        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                valid_room_entries[x][y] =
                    valid_room_entries0[x][y] || valid_room_entries1[x][y];

                //Convert some remaining valid room entries to floor
                if (
                    valid_room_entries[x][y] &&
                    find(begin(c_built), end(c_built), x) == end(c_built))
                {
//          if (rnd::one_in(3))
//          {
                    map::put(new Floor(P(x, y)));
                    map::room_map[x][y] = this;
//          }
                }
            }
        }

        //Convert wall cells adjacent to river cells to river
        bool valid_room_entries_expanded[MAP_W][MAP_H];
        map_parse::expand(valid_room_entries, valid_room_entries_expanded, 2);

        for (int x = 2; x < MAP_W - 2; ++x)
        {
            for (int y = 2; y < MAP_H - 2; ++y)
            {
                if (valid_room_entries_expanded[x][y] && map::room_map[x][y] == this)
                {
                    auto* const floor = new Floor(P(x, y));
                    floor->type_      = Floor_type::cmn;
                    map::put(floor);
                    map::room_map[x][y] = nullptr;
                }
            }
        }
    }

    TRACE_FUNC_END;
}
