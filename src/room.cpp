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

using namespace std;

namespace
{

vector<Room_type> room_bucket_;

void add_to_room_bucket(const Room_type type, const size_t NR)
{
    if (NR > 0)
    {
        room_bucket_.reserve(room_bucket_.size() + NR);

        for (size_t i = 0; i < NR; ++i) {room_bucket_.push_back(type);}
    }
}

} //namespace

//------------------------------------------------------------------- ROOM FACTORY
namespace Room_factory
{

void init_room_bucket()
{
    TRACE_FUNC_BEGIN;

    room_bucket_.clear();

    const int DLVL = Map::dlvl;

    if (DLVL <= DLVL_LAST_EARLY_GAME)
    {
        add_to_room_bucket(Room_type::human,    Rnd::range(2, 4));
        add_to_room_bucket(Room_type::ritual,   Rnd::range(1, 1));
        add_to_room_bucket(Room_type::spider,   Rnd::range(0, 3));
        add_to_room_bucket(Room_type::crypt,    Rnd::range(1, 4));
        add_to_room_bucket(Room_type::monster,  Rnd::range(0, 3));
        add_to_room_bucket(Room_type::flooded,  Rnd::range(0, 1));
        add_to_room_bucket(Room_type::muddy,    Rnd::range(0, 1));

        const size_t NR_PLAIN_ROOM_PER_THEMED = 1;

        add_to_room_bucket(Room_type::plain, room_bucket_.size() * NR_PLAIN_ROOM_PER_THEMED);
    }
    else if (DLVL <= DLVL_LAST_MID_GAME)
    {
        add_to_room_bucket(Room_type::human,    Rnd::range(1, 3));
        add_to_room_bucket(Room_type::ritual,   Rnd::range(1, 1));
        add_to_room_bucket(Room_type::spider,   Rnd::range(0, 3));
        add_to_room_bucket(Room_type::crypt,    Rnd::range(0, 4));
        add_to_room_bucket(Room_type::monster,  Rnd::range(0, 3));
        add_to_room_bucket(Room_type::flooded,  Rnd::range(0, 2));
        add_to_room_bucket(Room_type::muddy,    Rnd::range(0, 2));
        add_to_room_bucket(Room_type::cave,     Rnd::range(1, 4));
        add_to_room_bucket(Room_type::chasm,    Rnd::range(0, 2));
        add_to_room_bucket(Room_type::forest,   Rnd::range(1, 4));

        const size_t NR_PLAIN_ROOM_PER_THEMED = 1;

        add_to_room_bucket(Room_type::plain, room_bucket_.size() * NR_PLAIN_ROOM_PER_THEMED);
    }
    else
    {
        add_to_room_bucket(Room_type::monster,  Rnd::range(0, 3));
        add_to_room_bucket(Room_type::spider,   Rnd::range(0, 3));
        add_to_room_bucket(Room_type::flooded,  Rnd::range(0, 2));
        add_to_room_bucket(Room_type::muddy,    Rnd::range(0, 2));
        add_to_room_bucket(Room_type::cave,     Rnd::range(3, 4));
        add_to_room_bucket(Room_type::chasm,    Rnd::range(0, 2));
        add_to_room_bucket(Room_type::forest,   Rnd::range(1, 4));
    }

    std::random_shuffle(begin(room_bucket_), end(room_bucket_));

    TRACE_FUNC_END;
}

Room* mk(const Room_type type, const Rect& r)
{
    switch (type)
    {
    case Room_type::cave:          return new Cave_room(r);
    case Room_type::chasm:         return new Chasm_room(r);
    case Room_type::crypt:         return new Crypt_room(r);
    case Room_type::flooded:       return new Flooded_room(r);
    case Room_type::human:         return new Human_room(r);
    case Room_type::monster:       return new Monster_room(r);
    case Room_type::muddy:         return new Muddy_room(r);
    case Room_type::plain:         return new Plain_room(r);
    case Room_type::ritual:        return new Ritual_room(r);
    case Room_type::spider:        return new Spider_room(r);
    case Room_type::forest:        return new Forest_room(r);
    case Room_type::END_OF_STD_ROOMS:
        TRACE << "Illegal room type id: " << int (type) << endl;
        assert(false);
        return nullptr;
    case Room_type::corr_link:      return new Corr_link_room(r);
    case Room_type::crumble_room:   return new Crumble_room(r);
    case Room_type::river:         return new River_room(r);
    }
    TRACE << "Unhandled room type id: " << int (type) << endl;
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
            const Room_type    room_type  = *room_bucket_it;
            room                        = mk(room_type, r);
            room->is_sub_room_            = IS_SUBROOM;
            Std_room* const    std_room   = static_cast<Std_room*>(room);

            if (std_room->is_allowed())
            {
                room_bucket_.erase(room_bucket_it);

                TRACE_FUNC_END_VERBOSE << "Made room type: " << int (room_type) << endl;
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
    r_          (r),
    type_       (type),
    is_sub_room_  (false) {}

void Room::mk_drk() const
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (Map::room_map[x][y] == this) {Map::cells[x][y].is_dark = true;}
        }
    }

    //Also make sub rooms dark
    for (Room* const sub_room : sub_rooms_) {sub_room->mk_drk();}
}

//------------------------------------------------------------------- STANDARD ROOM
void Std_room::on_pre_connect(bool door_proposals[MAP_W][MAP_H])
{
    on_pre_connect_(door_proposals);
}

void Std_room::on_post_connect(bool door_proposals[MAP_W][MAP_H])
{
    place_auto_features();

    on_post_connect_(door_proposals);

    //Make dark?
    int pct_chance_dark = get_base_pct_chance_drk() - 15;
    pct_chance_dark += Map::dlvl; //Increase with higher dungeon level
    constr_in_range(0, pct_chance_dark, 100);

    if (Rnd::percent() < pct_chance_dark)
    {
        mk_drk();
    }
}

size_t Std_room::try_get_auto_feature_placement(
    const vector<Pos>& adj_to_walls, const vector<Pos>& away_from_walls,
    const vector<const Feature_data_t*>& feature_data_bucket, Pos& pos_ref) const
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (feature_data_bucket.empty())
    {
        pos_ref = Pos(-1, -1);
        return 0;
    }

    const bool IS_ADJ_TO_WALLS_AVAIL    = !adj_to_walls.empty();
    const bool IS_AWAY_FROM_WALLS_AVAIL = !away_from_walls.empty();

    if (!IS_ADJ_TO_WALLS_AVAIL && !IS_AWAY_FROM_WALLS_AVAIL)
    {
        TRACE_FUNC_END_VERBOSE << "No eligible cells found" << endl;
        pos_ref = Pos(-1, -1);
        return 0;
    }

    //TODO: Use bucket instead

    const int NR_ATTEMPTS_TO_FIND_POS = 100;
    for (int i = 0; i < NR_ATTEMPTS_TO_FIND_POS; ++i)
    {
        const size_t      NR_DATA = feature_data_bucket.size();
        const size_t      ELEMENT = Rnd::range(0, NR_DATA - 1);
        const auto* const data    = feature_data_bucket[ELEMENT];

        if (
            IS_ADJ_TO_WALLS_AVAIL &&
            data->room_spawn_rules.get_placement_rule() == Placement_rule::adj_to_walls)
        {
            pos_ref = adj_to_walls[Rnd::range(0, adj_to_walls.size() - 1)];
            TRACE_FUNC_END_VERBOSE;
            return ELEMENT;
        }

        if (
            IS_AWAY_FROM_WALLS_AVAIL &&
            data->room_spawn_rules.get_placement_rule() == Placement_rule::away_from_walls)
        {
            pos_ref = away_from_walls[Rnd::range(0, away_from_walls.size() - 1)];
            TRACE_FUNC_END_VERBOSE;
            return ELEMENT;
        }

        if (data->room_spawn_rules.get_placement_rule() == Placement_rule::either)
        {
            if (Rnd::coin_toss())
            {
                if (IS_ADJ_TO_WALLS_AVAIL)
                {
                    pos_ref = adj_to_walls[Rnd::range(0, adj_to_walls.size() - 1)];
                    TRACE_FUNC_END_VERBOSE;
                    return ELEMENT;

                }
            }
            else
            {
                if (IS_AWAY_FROM_WALLS_AVAIL)
                {
                    pos_ref = away_from_walls[Rnd::range(0, away_from_walls.size() - 1)];
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

    vector<const Feature_data_t*> feature_bucket;

    for (int i = 0; i < int (Feature_id::END); ++i)
    {
        const auto& d           = Feature_data::get_data((Feature_id)(i));
        const auto& spawn_rules  = d.room_spawn_rules;

        if (
            spawn_rules.is_belonging_to_room_type(type_) &&
            Utils::is_val_in_range(Map::dlvl, spawn_rules.get_dlvls_allowed()))
        {
            feature_bucket.push_back(&d);
        }
    }

    vector<Pos> adj_to_walls;
    vector<Pos> away_from_walls;

    Map_patterns::get_cells_in_room(*this, adj_to_walls, away_from_walls);

    vector<int> spawn_count(feature_bucket.size(), 0);

    int nr_features_left_to_place = Rnd::range(get_nr_auto_features_allowed());
    int nr_features_placed      = 0;

    while (true)
    {
        if (nr_features_left_to_place == 0)
        {
            TRACE_FUNC_END << "Placed enough features" << endl;
            return nr_features_placed;
        }

        Pos pos(-1, -1);

        const size_t FEATURE_IDX =
            try_get_auto_feature_placement(adj_to_walls, away_from_walls, feature_bucket, pos);

        if (pos.x >= 0)
        {
            assert(FEATURE_IDX < feature_bucket.size());

            const Feature_data_t* d = feature_bucket[FEATURE_IDX];

            TRACE << "Placing feature" << endl;
            Map::put(static_cast<Rigid*>(d->mk_obj(pos)));
            ++spawn_count[FEATURE_IDX];

            nr_features_left_to_place--;
            nr_features_placed++;

            //Check if more of this feature can be spawned. If not, erase it.
            if (spawn_count[FEATURE_IDX] >= d->room_spawn_rules.get_max_nr_in_room())
            {
                spawn_count   .erase(spawn_count   .begin() + FEATURE_IDX);
                feature_bucket.erase(feature_bucket.begin() + FEATURE_IDX);

                if (feature_bucket.empty())
                {
                    TRACE_FUNC_END << "No more features to place" << endl;
                    return nr_features_placed;
                }
            }

            //Erase all adjacent positions
            auto is_adj = [&](const Pos & p) {return Utils::is_pos_adj(p, pos, true);};
            adj_to_walls   .erase(remove_if(begin(adj_to_walls),    end(adj_to_walls),    is_adj),
                                end(adj_to_walls));
            away_from_walls.erase(remove_if(begin(away_from_walls), end(away_from_walls), is_adj),
                                end(away_from_walls));
        }
        else //No good feature placement found
        {
            TRACE_FUNC_END << "No remaining positions to place feature" << endl;
            return nr_features_placed;
        }
    }
}

//------------------------------------------------------------------- PLAIN ROOM
Range Plain_room::get_nr_auto_features_allowed() const
{
    if (Rnd::one_in(4))
    {
        return {1, 2};
    }
    else
    {
        return {0, 0};
    }
}

int Plain_room::get_base_pct_chance_drk() const
{
    return 5;
}

void Plain_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    Map_gen_utils::cut_room_corners (*this);
    if (Rnd::fraction(1, 3)) {Map_gen_utils::mk_pillars_in_room(*this);}
}

void Plain_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

}

//------------------------------------------------------------------- HUMAN ROOM
Range Human_room::get_nr_auto_features_allowed() const
{
    return {3, 6};
}

int Human_room::get_base_pct_chance_drk() const
{
    return 10;
}

bool Human_room::is_allowed() const
{
    return r_.get_min_dim() >= 4 && r_.get_max_dim() <= 8;
}

void Human_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    Map_gen_utils::cut_room_corners (*this);
    if (Rnd::fraction(1, 3)) {Map_gen_utils::mk_pillars_in_room(*this);}
}

void Human_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    if (Rnd::coin_toss())
    {
        bool blocked[MAP_W][MAP_H];
        Map_parse::run(Cell_check::Blocks_move_cmn(false), blocked);

        for (int x = r_.p0.x + 1; x <= r_.p1.x - 1; ++x)
        {
            for (int y = r_.p0.y + 1; y <= r_.p1.y - 1; ++y)
            {
                if (!blocked[x][y] && Map::room_map[x][y] == this)
                {
                    Carpet* const carpet = new Carpet(Pos(x, y));
                    Map::put(carpet);
                }
            }
        }
    }
}

//------------------------------------------------------------------- RITUAL ROOM
Range Ritual_room::get_nr_auto_features_allowed() const
{
    return {1, 5};
}

int Ritual_room::get_base_pct_chance_drk() const
{
    return 15;
}

bool Ritual_room::is_allowed() const
{
    return r_.get_min_dim() >= 4 && r_.get_max_dim() <= 8;
}

void Ritual_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    Map_gen_utils::cut_room_corners (*this);
    if (Rnd::fraction(1, 3)) {Map_gen_utils::mk_pillars_in_room(*this);}
}

void Ritual_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    Gods::set_random_god();

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(false), blocked);

    const int BLOODY_CHAMBER_PCT = 60;
    if (Rnd::percent() < BLOODY_CHAMBER_PCT)
    {
        Pos origin(-1, -1);
        vector<Pos> origin_bucket;
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            for (int x = r_.p0.x; x <= r_.p1.x; ++x)
            {
                if (Map::cells[x][y].rigid->get_id() == Feature_id::altar)
                {
                    origin = Pos(x, y);
                    y = 999;
                    x = 999;
                }
                else
                {
                    if (!blocked[x][y]) {origin_bucket.push_back(Pos(x, y));}
                }
            }
        }
        if (!origin_bucket.empty())
        {
            if (origin.x == -1)
            {
                const int ELEMENT = Rnd::range(0, origin_bucket.size() - 1);
                origin = origin_bucket[ELEMENT];
            }
            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    if ((dx == 0 && dy == 0) || (Rnd::percent() < BLOODY_CHAMBER_PCT / 2))
                    {
                        const Pos pos = origin + Pos(dx, dy);
                        if (!blocked[pos.x][pos.y])
                        {
                            Map::mk_gore(pos);
                            Map::mk_blood(pos);
                        }
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------- SPIDER ROOM
Range Spider_room::get_nr_auto_features_allowed() const
{
    return {0, 3};
}

int Spider_room::get_base_pct_chance_drk() const
{
    return 30;
}

bool Spider_room::is_allowed() const
{
    return r_.get_min_dim() >= 3 && r_.get_max_dim() <= 8;
}

void Spider_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    //Early game : Always reshape by cutting corners
    //Mid    -   : "Flip a coin"
    //Late   -   : Always reshape by cavifying
    const bool IS_EARLY = Map::dlvl <= DLVL_LAST_EARLY_GAME;
    const bool IS_MID   = !IS_EARLY && Map::dlvl <= DLVL_LAST_MID_GAME;

    if (IS_EARLY || (IS_MID && Rnd::coin_toss()))
    {
        Map_gen_utils::cut_room_corners (*this);
    }
    else
    {
        Map_gen_utils::cavify_room(*this);
    }

    if ((IS_EARLY || IS_MID) && Rnd::fraction(1, 3))
    {
        Map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Spider_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;
}

//------------------------------------------------------------------- CRYPT ROOM
Range Crypt_room::get_nr_auto_features_allowed() const
{
    return {3, 6};
}

int Crypt_room::get_base_pct_chance_drk() const
{
    return 60;
}

bool Crypt_room::is_allowed() const
{
    return r_.get_min_dim() >= 3  && r_.get_max_dim() <= 12;
}

void Crypt_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    Map_gen_utils::cut_room_corners (*this);
    if (Rnd::fraction(1, 3)) {Map_gen_utils::mk_pillars_in_room(*this);}
}

void Crypt_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;
}

//------------------------------------------------------------------- MONSTER ROOM
Range Monster_room::get_nr_auto_features_allowed() const
{
    return {0, 6};
}

int Monster_room::get_base_pct_chance_drk() const
{
    return 75;
}

bool Monster_room::is_allowed() const
{
    return r_.get_min_dim() >= 4 && r_.get_max_dim() <= 8;
}

void Monster_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    const bool IS_EARLY = Map::dlvl <= DLVL_LAST_EARLY_GAME;
    const bool IS_MID   = !IS_EARLY && Map::dlvl <= DLVL_LAST_MID_GAME;

    if (IS_EARLY || IS_MID)
    {
        if (Rnd::fraction(3, 4))
        {
            Map_gen_utils::cut_room_corners (*this);
        }
        if (Rnd::fraction(1, 3))
        {
            Map_gen_utils::mk_pillars_in_room(*this);
        }
    }
    else //Is late game
    {
        Map_gen_utils::cavify_room(*this);
    }
}

void Monster_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(false), blocked);

    int       nr_blood_put  = 0;
    const int NR_TRIES    = 1000; //TODO: Hacky, needs improving

    for (int i = 0; i < NR_TRIES; ++i)
    {
        for (int x = r_.p0.x; x <= r_.p1.x; ++x)
        {
            for (int y = r_.p0.y; y <= r_.p1.y; ++y)
            {
                if (!blocked[x][y] && Map::room_map[x][y] == this && Rnd::fraction(2, 5))
                {
                    Map::mk_gore(Pos(x, y));
                    Map::mk_blood(Pos(x, y));
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
Range Flooded_room::get_nr_auto_features_allowed() const
{
    return {0, 0};
}

int Flooded_room::get_base_pct_chance_drk() const
{
    return 25;
}

bool Flooded_room::is_allowed() const
{
    return true;
}

void Flooded_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    //Early game : Always reshape by cutting corners
    //Mid    -   : "Flip a coin"
    //Late   -   : Always reshape by cavifying
    const bool IS_EARLY = Map::dlvl <= DLVL_LAST_EARLY_GAME;
    const bool IS_MID   = !IS_EARLY && Map::dlvl <= DLVL_LAST_MID_GAME;

    if (IS_EARLY || (IS_MID && Rnd::coin_toss()))
    {
        Map_gen_utils::cut_room_corners (*this);
    }
    else
    {
        Map_gen_utils::cavify_room(*this);
    }

    if ((IS_EARLY || IS_MID) && Rnd::fraction(1, 3))
    {
        Map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Flooded_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

#ifndef NDEBUG
    //Sanity check (look for some features that should not exist in this room)
    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (Map::room_map[x][y] == this)
            {
                const auto id = Map::cells[x][y].rigid->get_id();
                if (
                    id == Feature_id::chest    ||
                    id == Feature_id::tomb     ||
                    id == Feature_id::cabinet  ||
                    id == Feature_id::fountain)
                {
                    TRACE << "Illegal feature found in room" << endl;
                    assert(false);
                }
            }
        }
    }
#endif // NDEBUG

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(false), blocked);

    const int LIQUID_ONE_IN_N = Rnd::range(2, 4);

    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (
                !blocked[x][y] && Map::room_map[x][y] == this &&
                Rnd::one_in(LIQUID_ONE_IN_N))
            {
                Liquid_shallow* const liquid = new Liquid_shallow(Pos(x, y));
                liquid->type_ = Liquid_type::water;
                Map::put(liquid);
            }
        }
    }
}

//------------------------------------------------------------------- MUDDY ROOM
Range Muddy_room::get_nr_auto_features_allowed() const
{
    return {0, 0};
}

int Muddy_room::get_base_pct_chance_drk() const
{
    return 25;
}

bool Muddy_room::is_allowed() const
{
    return true;
}

void Muddy_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

#ifndef NDEBUG
    //Sanity check (look for some features that should not exist in this room)
    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (Map::room_map[x][y] == this)
            {
                const auto id = Map::cells[x][y].rigid->get_id();
                if (
                    id == Feature_id::chest    ||
                    id == Feature_id::tomb     ||
                    id == Feature_id::cabinet  ||
                    id == Feature_id::fountain)
                {
                    TRACE << "Illegal feature found in room" << endl;
                    assert(false);
                }
            }
        }
    }
#endif // NDEBUG

    //Early game : Always reshape by cutting corners
    //Mid    -   : "Flip a coin"
    //Late   -   : Always reshape by cavifying
    const bool IS_EARLY = Map::dlvl <= DLVL_LAST_EARLY_GAME;
    const bool IS_MID   = !IS_EARLY && Map::dlvl <= DLVL_LAST_MID_GAME;

    if (IS_EARLY || (IS_MID && Rnd::coin_toss()))
    {
        Map_gen_utils::cut_room_corners (*this);
    }
    else
    {
        Map_gen_utils::cavify_room(*this);
    }

    if ((IS_EARLY || IS_MID) && Rnd::fraction(1, 3))
    {
        Map_gen_utils::mk_pillars_in_room(*this);
    }
}

void Muddy_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(false), blocked);

    const int LIQUID_ONE_IN_N = Rnd::range(2, 4);

    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (!blocked[x][y] && Map::room_map[x][y] == this && Rnd::one_in(LIQUID_ONE_IN_N))
            {
                Liquid_shallow* const liquid = new Liquid_shallow(Pos(x, y));
                liquid->type_ = Liquid_type::mud;
                Map::put(liquid);
            }
        }
    }
}

//------------------------------------------------------------------- CAVE ROOM
Range Cave_room::get_nr_auto_features_allowed() const
{
    return {2, 6};
}

int Cave_room::get_base_pct_chance_drk() const
{
    return 30;
}

bool Cave_room::is_allowed() const
{
    return !is_sub_room_;
}

void Cave_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    Map_gen_utils::cavify_room(*this);
}

void Cave_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;
}

//------------------------------------------------------------------- FOREST ROOM
Range Forest_room::get_nr_auto_features_allowed() const
{
    if (Rnd::one_in(3))
    {
        return {2, 6};
    }
    else
    {
        return {0, 1};
    }
}

int Forest_room::get_base_pct_chance_drk() const
{
    return 10;
}

bool Forest_room::is_allowed() const
{
    return !is_sub_room_;
}

void Forest_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    Map_gen_utils::cavify_room(*this);
}

void Forest_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(false), blocked);

    vector<Pos> tree_pos_bucket;

    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            if (!blocked[x][y] && Map::room_map[x][y] == this)
            {
                const Pos p(x, y);
                tree_pos_bucket.push_back(p);

                if (Rnd::one_in(6))
                {
                    Map::put(new Bush(p));
                }
                else
                {
                    Map::put(new Grass(p));
                }
            }
        }
    }

    random_shuffle(begin(tree_pos_bucket), end(tree_pos_bucket));

    int nr_trees_placed = 0;

    const int TREE_ONE_IN_N = Rnd::range(2, 5);

    while (!tree_pos_bucket.empty())
    {
        const Pos p = tree_pos_bucket.back();
        tree_pos_bucket.pop_back();

        if (Rnd::one_in(TREE_ONE_IN_N))
        {
            blocked[p.x][p.y] = true;

            if (Map_parse::is_map_connected(blocked))
            {
                Map::put(new Tree(p));
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
Range Chasm_room::get_nr_auto_features_allowed() const
{
    return {0, 0};
}

int Chasm_room::get_base_pct_chance_drk() const
{
    return 25;
}

bool Chasm_room::is_allowed() const
{
    return r_.get_min_dim() >= 5 &&
           r_.get_max_dim() <= 9;
}

void Chasm_room::on_pre_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    Map_gen_utils::cavify_room(*this);
}

void Chasm_room::on_post_connect_(bool door_proposals[MAP_W][MAP_H])
{
    (void)door_proposals;

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(false), blocked);

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (Map::room_map[x][y] != this)
            {
                blocked[x][y] = true;
            }
        }
    }

    bool blocked_expanded[MAP_W][MAP_H];
    Map_parse::expand(blocked, blocked_expanded);

    Pos origin;

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

    Flood_fill::run(origin, blocked_expanded, flood, 10000, { -1,  -1}, false);

    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
        for (int y = r_.p0.y; y <= r_.p1.y; ++y)
        {
            const Pos p(x, y);

            if (p == origin || flood[x][y] != 0)
            {
                Map::put(new Chasm(p));
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

    const bool IS_HOR = dir_ == hor;

    TRACE << "Finding room centers" << endl;
    bool centers[MAP_W][MAP_H];
    Utils::reset_array(centers, false);

    for (Room* const room : Map::room_list)
    {
        if (room != this)
        {
            const Pos c_pos(room->r_.get_center_pos());
            centers[c_pos.x][c_pos.y] = true;
        }
    }

    TRACE << "Finding closest room center coordinates on both sides "
          << "(y coordinate if horizontal river, x if vertical)" << endl;
    int closest_center0 = -1;
    int closest_center1 = -1;

    //Using nestled scope to avoid declaring x and y at function scope
    {
        int x, y;

        //i_outer and i_inner should be references to x or y.
        auto find_closest_center0 =
            [&](const Range & r_outer, const Range & r_inner, int& i_outer, int& i_inner)
        {
            for (i_outer = r_outer.lower; i_outer >= r_outer.upper; --i_outer)
            {
                for (i_inner = r_inner.lower; i_inner <= r_inner.upper; ++i_inner)
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
            for (i_outer = r_outer.lower; i_outer <= r_outer.upper; ++i_outer)
            {
                for (i_inner = r_inner.lower; i_inner <= r_inner.upper; ++i_inner)
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

    TRACE << "Expanding and filling river" << endl;

    bool blocked[MAP_W][MAP_H];

    //Within the expansion limits, mark all cells not belonging to another room as free.
    //All other cells are considered as blocking.
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            blocked[x][y] = true;
            if (
                (dir_ == hor && (y >= closest_center0 && y <= closest_center1)) ||
                (dir_ == ver && (x >= closest_center0 && x <= closest_center1))
            )
            {
                Room* r       = Map::room_map[x][y];
                blocked[x][y] = r && r != this;
            }
        }
    }
    bool blocked_expanded[MAP_W][MAP_H];
    Map_parse::expand(blocked, blocked_expanded);

    int flood[MAP_W][MAP_H];
    const Pos origin(r_.get_center_pos());
    Flood_fill::run(origin, blocked_expanded, flood, INT_MAX, Pos(-1, -1), true);

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            const Pos p(x, y);
            if (flood[x][y] > 0 || p == origin)
            {
//        Liquid_deep* const liquid  = new Liquid_deep(p);
//        liquid->type_             = Liquid_type::water;
                Map::put(new Chasm(p));
                Map::room_map[x][y] = this;
                r_.p0.x = min(r_.p0.x, x);
                r_.p0.y = min(r_.p0.y, y);
                r_.p1.x = max(r_.p1.x, x);
                r_.p1.y = max(r_.p1.y, y);
            }
        }
    }

    TRACE << "Making bridge(s)" << endl;

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
            for (i_outer = r_outer.lower; i_outer <= r_outer.upper; ++i_outer)
            {
                bool is_on_side0 = true;
                for (i_inner = r_inner.lower; i_inner <= r_inner.upper; ++i_inner)
                {
                    if (Map::room_map[x][y] == this)
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

        if (dir_ == hor)
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
            const Feature_id feature_id = Map::cells[x][y].rigid->get_id();
            if (feature_id == Feature_id::wall && !Map::room_map[x][y])
            {
                const Pos p(x, y);
                int nr_cardinal_floor  = 0;
                int nr_cardinal_river  = 0;
                for (const auto& d : Dir_utils::cardinal_list)
                {
                    const auto p_adj(p + d);
                    const auto* const f = Map::cells[p_adj.x][p_adj.y].rigid;

                    if (f->get_id() == Feature_id::floor)        {nr_cardinal_floor++;}
                    if (Map::room_map[p_adj.x][p_adj.y] == this)  {nr_cardinal_river++;}
                }
                if (nr_cardinal_floor == 1 && nr_cardinal_river == 1)
                {
                    switch (sides[x][y])
                    {
                    case side0:   valid_room_entries0[x][y] = true; break;
                    case side1:   valid_room_entries1[x][y] = true; break;
                    case in_river: {} break;
                    }
                }
            }
        }
    }

#ifdef DEMO_MODE
    Render::draw_map();
    for (int y = 1; y < MAP_H - 1; ++y)
    {
        for (int x = 1; x < MAP_W - 1; ++x)
        {
            Pos p(x, y);
            if (valid_room_entries0[x][y])
            {
                Render::draw_glyph('0', Panel::map, p, clr_red_lgt);
            }
            if (valid_room_entries1[x][y])
            {
                Render::draw_glyph('1', Panel::map, p, clr_red_lgt);
            }
            if (valid_room_entries0[x][y] || valid_room_entries1[x][y])
            {
                Render::update_screen();
                Sdl_wrapper::sleep(100);
            }
        }
    }
#endif // DEMO_MODE

    vector<int> coordinates(IS_HOR ? MAP_W : MAP_H);
    iota(begin(coordinates), end(coordinates), 0);
    random_shuffle(coordinates.begin(), coordinates.end());

    vector<int> c_built;

    const int MIN_EDGE_DIST   = 6;
    const int MAX_NR_BRIDGES  = Rnd::range(1, 3);

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
            if (Utils::is_val_in_range(BRIDGE_C, Range(c_other - MIN_D, c_other + MIN_D)))
            {
                is_too_close_to_other_bridge = true;
                break;
            }
        }
        if (is_too_close_to_other_bridge) {continue;}

        //Check if current bridge coord would connect matching room connections.
        //If so both room_con0 and room_con1 will be set.
        Pos room_con0(-1, -1);
        Pos room_con1(-1, -1);
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
            const Pos p_nxt = IS_HOR ? Pos(BRIDGE_C, c - 1) : Pos(c - 1, BRIDGE_C);
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
            const Pos p_nxt = IS_HOR ? Pos(BRIDGE_C, c + 1) : Pos(c + 1, BRIDGE_C);

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
            Render::draw_map();
            Render::draw_glyph('0', Panel::map, room_con0, clr_green_lgt);
            Render::draw_glyph('1', Panel::map, room_con1, clr_yellow);
            Render::update_screen();
            Sdl_wrapper::sleep(2000);
#endif // DEMO_MODE

            TRACE << "Found valid connection pair at: "
                  << room_con0.x << "," << room_con0.y << " / "
                  << room_con1.x << "," << room_con1.y << endl
                  << "Making bridge at pos: " << BRIDGE_C << endl;
            if (IS_HOR)
            {
                for (int y = room_con0.y; y <= room_con1.y; ++y)
                {
                    if (Map::room_map[BRIDGE_C][y] == this)
                    {
//            auto* const bridge = new Bridge(Pos(BRIDGE_C, y));
//            bridge->set_dir(ver);
                        auto* const floor = new Floor({BRIDGE_C, y});
                        floor->type_ = Floor_type::cmn;
                        Map::put(floor);
                    }
                }
            }
            else //Vertical
            {
                for (int x = room_con0.x; x <= room_con1.x; ++x)
                {
                    if (Map::room_map[x][BRIDGE_C] == this)
                    {
//            auto* const bridge = new Bridge(Pos(x, BRIDGE_C));
//            bridge->set_dir(hor);
                        auto* const floor = new Floor({x, BRIDGE_C});
                        floor->type_ = Floor_type::cmn;
                        Map::put(floor);
                    }
                }
            }
            Map::put(new Floor(room_con0));
            Map::put(new Floor(room_con1));
            door_proposals[room_con0.x][room_con0.y] = true;
            door_proposals[room_con1.x][room_con1.y] = true;
            c_built.push_back(BRIDGE_C);
        }
        if (int (c_built.size()) >= MAX_NR_BRIDGES)
        {
            TRACE << "Enough bridges built" << endl;
            break;
        }
    }
    TRACE << "Bridges built/attempted: " << c_built.size() << "/" << MAX_NR_BRIDGES << endl;

    if (c_built.empty())
    {
        Map_gen::is_map_valid = false;
    }
    else //Map is valid (at least one bridge was built)
    {
        bool valid_room_entries[MAP_W][MAP_H];

        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                valid_room_entries[x][y] = valid_room_entries0[x][y] || valid_room_entries1[x][y];

                //Convert some remaining valid room entries to floor
                if (valid_room_entries[x][y] && find(begin(c_built), end(c_built), x) == end(c_built))
                {
//          if (Rnd::one_in(3))
//          {
                    Map::put(new Floor(Pos(x, y)));
                    Map::room_map[x][y] = this;
//          }
                }
            }
        }

        //Convert wall cells adjacent to river cells to river
        bool valid_room_entries_expanded[MAP_W][MAP_H];
        Map_parse::expand(valid_room_entries, valid_room_entries_expanded, 2);

        for (int x = 2; x < MAP_W - 2; ++x)
        {
            for (int y = 2; y < MAP_H - 2; ++y)
            {
                if (valid_room_entries_expanded[x][y] && Map::room_map[x][y] == this)
                {
                    auto* const floor = new Floor(Pos(x, y));
                    floor->type_      = Floor_type::cmn;
                    Map::put(floor);
                    Map::room_map[x][y] = nullptr;
                }
            }
        }
    }
    TRACE_FUNC_END;
}
