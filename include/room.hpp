//NOTE: This file must not define "ROOM_H" for the include guard. That name should be
//reserved for constants representing room height.
#ifndef MAPGEN_ROOM_H
#define MAPGEN_ROOM_H

#include <vector>
#ifdef MACOSX
#include <numeric>
#endif

#include "rl_utils.hpp"
#include "cmn.hpp"

//---------------------------------------------------------------------------------------
// Room theming occurs both before and after rooms are connected (pre/post-connect).
//   > In pre-connect reshaping is done. The reshaping is called from map_gen_utils
//     (e.g. plus-shape, cavern-shape, pillars, etc)
//
//     When pre-connect starts, it is assumed that all (standard) rooms are rectangular
//     with unbroken walls.
//
//   > In post-connect, auto-features such as chests and altars are placed (this is
//     configured in the feature data), as well as room-specific stuff like trees, altars,
//     etc. It can then be verified for each feature that the map is still connected.
//
// As a rule of thumb, place walkable features in the pre-connect step, and blocking
// features in the post-connect step.
//---------------------------------------------------------------------------------------

struct  Feature_data_t;
class   Room;

enum class Room_type
{
    //Standard rooms (standardized feature spawning and reshaping)
    plain, //NOTE: "plain" must be the first type
    human,
    ritual,
    spider,
    snake_pit,
    crypt,
    monster,
    flooded,
    muddy,
    cave,
    chasm,
    forest,
    END_OF_STD_ROOMS,

    //Exceptional room types
    corr_link,
    crumble_room,
    river
};

namespace room_factory
{

void init_room_bucket();

//NOTE: These functions do not make rooms on the map, they merely create Room objects.
Room* mk(const Room_type type, const R& r);

Room* mk_random_allowed_std_room(const R& r, const bool IS_SUBROOM);

} //Room_factory

class Room
{
public:
    Room(R r, Room_type type);

    Room() = delete;

    virtual ~Room() {}

    virtual void on_pre_connect(bool door_proposals[MAP_W][MAP_H]) = 0;
    virtual void on_post_connect(bool door_proposals[MAP_W][MAP_H]) = 0;

    R                r_;
    const Room_type     type_;
    bool                is_sub_room_;
    std::vector<Room*>  rooms_con_to_;
    std::vector<Room*>  sub_rooms_;

protected:
    void mk_drk() const;
};

class Std_room : public Room
{
public:
    Std_room(R r, Room_type type) : Room(r, type) {}

    virtual ~Std_room() {}

    void on_pre_connect(bool door_proposals[MAP_W][MAP_H]) override final;
    void on_post_connect(bool door_proposals[MAP_W][MAP_H]) override final;

    virtual bool is_allowed() const
    {
        return true;
    }

protected:
    virtual Range nr_auto_features_allowed() const = 0;

    virtual int base_pct_chance_drk() const = 0;

    size_t try_auto_feature_placement(
        const std::vector<P>& adj_to_walls,
        const std::vector<P>& away_from_walls,
        const std::vector<const Feature_data_t*>& feature_data_bucket,
        P& pos_ref) const;

    int place_auto_features();

    virtual void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) = 0;
    virtual void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) = 0;
};

class Plain_room: public Std_room
{
public:
    Plain_room(R r) : Std_room(r, Room_type::plain) {}

    ~Plain_room() {}

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Human_room: public Std_room
{
public:
    Human_room(R r) :
        Std_room(r, Room_type::human) {}

    ~Human_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Ritual_room: public Std_room
{
public:
    Ritual_room(R r) :
        Std_room(r, Room_type::ritual) {}

    ~Ritual_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Spider_room: public Std_room
{
public:
    Spider_room(R r) :
        Std_room(r, Room_type::spider) {}

    ~Spider_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Snake_pit_room: public Std_room
{
public:
    Snake_pit_room(R r) :
        Std_room(r, Room_type::monster) {}

    ~Snake_pit_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override
    {
        return Range(0, 0);
    }

    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Crypt_room: public Std_room
{
public:
    Crypt_room(R r) :
        Std_room(r, Room_type::crypt) {}

    ~Crypt_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Monster_room: public Std_room
{
public:
    Monster_room(R r) :
        Std_room(r, Room_type::monster) {}

    ~Monster_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Flooded_room: public Std_room
{
public:
    Flooded_room(R r) :
        Std_room(r, Room_type::flooded) {}

    ~Flooded_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Muddy_room: public Std_room
{
public:
    Muddy_room(R r) :
        Std_room(r, Room_type::muddy) {}

    ~Muddy_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Cave_room: public Std_room
{
public:
    Cave_room(R r) :
        Std_room(r, Room_type::cave) {}

    ~Cave_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Chasm_room: public Std_room
{
public:
    Chasm_room(R r) :
        Std_room(r, Room_type::chasm) {}

    ~Chasm_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Forest_room: public Std_room
{
public:
    Forest_room(R r) :
        Std_room(r, Room_type::forest) {}

    ~Forest_room() {}

    bool is_allowed() const override;

protected:
    Range nr_auto_features_allowed() const override;
    int base_pct_chance_drk() const override;
    void on_pre_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
    void on_post_connect_hook(bool door_proposals[MAP_W][MAP_H]) override;
};

class Corr_link_room: public Room
{
public:
    Corr_link_room(const R& r) :
        Room(r, Room_type::corr_link) {}

    ~Corr_link_room() {}

    void on_pre_connect(bool door_proposals[MAP_W][MAP_H]) override
    {
        (void)door_proposals;
    }

    void on_post_connect(bool door_proposals[MAP_W][MAP_H]) override
    {
        (void)door_proposals;
    }
};

class Crumble_room: public Room
{
public:
    Crumble_room(const R& r) :
        Room(r, Room_type::crumble_room) {}

    ~Crumble_room() {}

    void on_pre_connect(bool door_proposals[MAP_W][MAP_H]) override
    {
        (void)door_proposals;
    }

    void on_post_connect(bool door_proposals[MAP_W][MAP_H]) override
    {
        (void)door_proposals;
    }
};

class River_room: public Room
{
public:
    River_room(const R& r) :
        Room    (r, Room_type::river),
        axis_   (Axis::hor) {}

    ~River_room() {}

    void on_pre_connect(bool door_proposals[MAP_W][MAP_H]) override;

    void on_post_connect(bool door_proposals[MAP_W][MAP_H]) override
    {
        (void)door_proposals;
    }

    Axis axis_;
};

#endif
