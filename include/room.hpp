#ifndef ROOM_HPP
#define ROOM_HPP

#include <vector>

#include "rl_utils.hpp"
#include "global.hpp"
#include "feature_data.hpp"

//------------------------------------------------------------------------------
//Room theming occurs both pre- and post-connect.
//  >   In pre-connect, reshaping is performed, e.g. plus-shape, cavern-shape,
//      pillars, etc)
//
//      When pre-connect starts, it is assumed that all (standard) rooms are
//      rectangular with unbroken walls.
//
//  >   In post-connect, auto-features such as chests and altars are placed,
//      as well as room-specific stuff like trees, altars, etc. It can then
//      be verified for each feature that the map is still connected.
//
//As a rule of thumb, place walkable features in the pre-connect step, and
//blocking features in the post-connect step.
//------------------------------------------------------------------------------

struct  FeatureDataT;
class   Room;

enum class RoomType
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

struct RoomAutoFeatureRule
{
    RoomAutoFeatureRule() :
        feature_id  (FeatureId::END),
        nr_allowed  (0) {}

    RoomAutoFeatureRule(const FeatureId id, const int nr_allowed) :
        feature_id  (id),
        nr_allowed  (nr_allowed) {}

    FeatureId feature_id;
    int nr_allowed;
};

namespace room_factory
{

void init_room_bucket();

//NOTE: These functions do not make rooms on the map, just Room objects
Room* mk(const RoomType type, const R& r);

Room* mk_random_allowed_std_room(const R& r, const bool is_subroom);

} //room_factory

class Room
{
public:
    Room(R r, RoomType type);

    Room() = delete;

    virtual ~Room() {}

    virtual void on_pre_connect(bool door_proposals[map_w][map_h]) = 0;
    virtual void on_post_connect(bool door_proposals[map_w][map_h]) = 0;

    virtual int max_nr_mon_groups_spawned() const
    {
        return 2;
    }

    R                   r_;
    const RoomType      type_;
    bool                is_sub_room_;
    std::vector<Room*>  rooms_con_to_;
    std::vector<Room*>  sub_rooms_;

protected:
    void mk_drk() const;
};

class StdRoom : public Room
{
public:
    StdRoom(R r, RoomType type) : Room(r, type) {}

    virtual ~StdRoom() {}

    void on_pre_connect(bool door_proposals[map_w][map_h]) override final;
    void on_post_connect(bool door_proposals[map_w][map_h]) override final;

    virtual bool is_allowed() const
    {
        return true;
    }

protected:
    virtual std::vector<RoomAutoFeatureRule> auto_features_allowed() const = 0;

    virtual int base_pct_chance_drk() const = 0;

    P find_auto_feature_placement(const std::vector<P>& adj_to_walls,
                                  const std::vector<P>& away_from_walls,
                                  const FeatureId id) const;

    void place_auto_features();

    virtual void on_pre_connect_hook(bool door_proposals[map_w][map_h]) = 0;
    virtual void on_post_connect_hook(bool door_proposals[map_w][map_h]) = 0;
};

class PlainRoom: public StdRoom
{
public:
    PlainRoom(R r) : StdRoom(r, RoomType::plain) {}

    ~PlainRoom() {}

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class HumanRoom: public StdRoom
{
public:
    HumanRoom(R r) :
        StdRoom(r, RoomType::human) {}

    ~HumanRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class RitualRoom: public StdRoom
{
public:
    RitualRoom(R r) :
        StdRoom(r, RoomType::ritual) {}

    ~RitualRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class SpiderRoom: public StdRoom
{
public:
    SpiderRoom(R r) :
        StdRoom(r, RoomType::spider) {}

    ~SpiderRoom() {}

    bool is_allowed() const override;

    int max_nr_mon_groups_spawned() const override
    {
        return 1;
    }

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class SnakePitRoom: public StdRoom
{
public:
    SnakePitRoom(R r) :
        StdRoom(r, RoomType::monster) {}

    ~SnakePitRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class CryptRoom: public StdRoom
{
public:
    CryptRoom(R r) :
        StdRoom(r, RoomType::crypt) {}

    ~CryptRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class MonsterRoom: public StdRoom
{
public:
    MonsterRoom(R r) :
        StdRoom(r, RoomType::monster) {}

    ~MonsterRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class FloodedRoom: public StdRoom
{
public:
    FloodedRoom(R r) :
        StdRoom(r, RoomType::flooded) {}

    ~FloodedRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class MuddyRoom: public StdRoom
{
public:
    MuddyRoom(R r) :
        StdRoom(r, RoomType::muddy) {}

    ~MuddyRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class CaveRoom: public StdRoom
{
public:
    CaveRoom(R r) :
        StdRoom(r, RoomType::cave) {}

    ~CaveRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class ChasmRoom: public StdRoom
{
public:
    ChasmRoom(R r) :
        StdRoom(r, RoomType::chasm) {}

    ~ChasmRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class ForestRoom: public StdRoom
{
public:
    ForestRoom(R r) :
        StdRoom(r, RoomType::forest) {}

    ~ForestRoom() {}

    bool is_allowed() const override;

protected:
    std::vector<RoomAutoFeatureRule> auto_features_allowed() const override;

    int base_pct_chance_drk() const override;

    void on_pre_connect_hook(bool door_proposals[map_w][map_h]) override;

    void on_post_connect_hook(bool door_proposals[map_w][map_h]) override;
};

class CorrLinkRoom: public Room
{
public:
    CorrLinkRoom(const R& r) :
        Room(r, RoomType::corr_link) {}

    ~CorrLinkRoom() {}

    void on_pre_connect(bool door_proposals[map_w][map_h]) override
    {
        (void)door_proposals;
    }

    void on_post_connect(bool door_proposals[map_w][map_h]) override
    {
        (void)door_proposals;
    }
};

class CrumbleRoom: public Room
{
public:
    CrumbleRoom(const R& r) :
        Room(r, RoomType::crumble_room) {}

    ~CrumbleRoom() {}

    void on_pre_connect(bool door_proposals[map_w][map_h]) override
    {
        (void)door_proposals;
    }

    void on_post_connect(bool door_proposals[map_w][map_h]) override
    {
        (void)door_proposals;
    }
};

class RiverRoom: public Room
{
public:
    RiverRoom(const R& r) :
        Room    (r, RoomType::river),
        axis_   (Axis::hor) {}

    ~RiverRoom() {}

    void on_pre_connect(bool door_proposals[map_w][map_h]) override;

    void on_post_connect(bool door_proposals[map_w][map_h]) override
    {
        (void)door_proposals;
    }

    Axis axis_;
};

#endif
