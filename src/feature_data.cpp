#include "feature_data.hpp"

#include "init.hpp"

#include "colors.hpp"
#include "actor.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "feature_trap.hpp"
#include "feature_event.hpp"
#include "feature_door.hpp"

using namespace std;

//--------------------------------------------------------- FEATURE ROOM SPAWN RULES
Feature_room_spawn_rules::Feature_room_spawn_rules() :
    max_nr_in_room_(-1),
    dlvls_allowed_(Range(-1, -1)),
    placement_rule_(Placement_rule::adj_to_walls)
{
    room_types_native_.clear();
}

void Feature_room_spawn_rules::reset() {*this = Feature_room_spawn_rules();}

void Feature_room_spawn_rules::set(const int MAX_NR_IN_ROOM,
                                   const Range& dlvls_allowed,
                                   const Placement_rule placement_rule,
                                   std::initializer_list<Room_type> room_types)
{
    max_nr_in_room_    = MAX_NR_IN_ROOM;
    dlvls_allowed_   = dlvls_allowed;
    placement_rule_  = placement_rule;

    room_types_native_.clear();

    for (Room_type id : room_types)
    {
        room_types_native_.push_back(id);
    }
}

bool Feature_room_spawn_rules::is_belonging_to_room_type(const Room_type type) const
{
    return find(begin(room_types_native_), end(room_types_native_), type) !=
           end(room_types_native_);
}

Placement_rule Feature_room_spawn_rules::get_placement_rule() const
{
    return placement_rule_;
}

int Feature_room_spawn_rules::get_max_nr_in_room() const
{
    return max_nr_in_room_;
}

Range Feature_room_spawn_rules::get_dlvls_allowed() const
{
    return dlvls_allowed_;
}

//--------------------------------------------------------- MOVE RULES
bool Move_rules::can_move(const bool actor_prop_ids[size_t(Prop_id::END)]) const
{
    if (can_move_cmn_)
    {
        return true;
    }

    //If not allowing normal move, check if any property overrides this
    for (int i = 0; i < int(Prop_id::END); ++i)
    {
        if (actor_prop_ids[i] && can_move_if_have_prop_[i])
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------- FEATURE DATA
namespace feature_data
{

Feature_data_t data[int(Feature_id::END)];

namespace
{

void reset_data(Feature_data_t& d)
{
    d.mk_obj = [](const Pos & p) {(void)p; return nullptr;};
    d.id = Feature_id::END;
    d.glyph = ' ';
    d.tile = Tile_id::empty;
    d.move_rules.reset();
    d.is_sound_passable = true;
    d.is_projectile_passable = true;
    d.is_los_passable = true;
    d.is_smoke_passable = true;
    d.can_have_blood = true;
    d.can_have_gore = true;
    d.can_have_corpse = true;
    d.can_have_rigid = true;
    d.can_have_item = true;
    d.is_bottomless = false;
    d.matl_type = Matl::stone;
    d.msg_on_player_blocked = "The way is blocked.";
    d.msg_on_player_blocked_blind = "I bump into something.";
    d.dodge_modifier = 0;
    d.shock_when_adjacent = 0;
    d.room_spawn_rules.reset();
}

void add_to_list_and_reset(Feature_data_t& d)
{
    data[int(d.id)] = d;
    reset_data(d);
}

void init_data_list()
{
    Feature_data_t d;
    reset_data(d);

    //---------------------------------------------------------------------------
    d.id = Feature_id::floor;
    d.mk_obj = [](const Pos & p) {return new Floor(p);};
    d.glyph = '.';
    d.tile = Tile_id::floor;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::stone;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::bridge;
    d.mk_obj = [](const Pos & p) {return new Bridge(p);};
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::wood;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::wall;
    d.mk_obj = [](const Pos & p) {return new Wall(p);};
    d.glyph = config::is_ascii_wall_full_square() ? 10 : '#';
    d.tile = Tile_id::wall_top;
    d.move_rules.set_prop_can_move(Prop_id::ethereal);
    d.move_rules.set_prop_can_move(Prop_id::burrowing);
    d.is_sound_passable = false;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.is_smoke_passable = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::tree;
    d.mk_obj = [](const Pos & p) {return new Tree(p);};
    d.glyph = '|';
    d.tile = Tile_id::tree;
    d.move_rules.set_prop_can_move(Prop_id::ethereal);
    //d.move_rules.set_prop_can_move(Prop_id::flying); //Its hard to notice some monsters
    d.is_sound_passable = false;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 1;
    d.msg_on_player_blocked = "There is a tree in the way.";
    d.matl_type = Matl::wood;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::grass;
    d.mk_obj = [](const Pos & p) {return new Grass(p);};
    d.glyph = '.';
    d.tile = Tile_id::floor;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::plant;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::bush;
    d.mk_obj = [](const Pos & p) {return new Bush(p);};
    d.glyph = '"';
    d.tile = Tile_id::bush;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::plant;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::stairs;
    d.mk_obj = [](const Pos & p) {return new Stairs(p);};
    d.glyph = '>';
    d.tile = Tile_id::stairs_down;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::lever;
    d.mk_obj = [](const Pos & p) {return new Lever(p);};
    d.glyph = '%';
    d.tile = Tile_id::lever_left;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::metal;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::brazier;
    d.mk_obj = [](const Pos & p) {return new Brazier(p);};
    d.glyph = '0';
    d.tile = Tile_id::brazier;
    d.move_rules.set_can_move_cmn();
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::metal;
    d.room_spawn_rules.set(3, {0, DLVL_LAST_MID_GAME}, Placement_rule::either,
    {
        Room_type::ritual
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::liquid_shallow;
    d.mk_obj = [](const Pos & p) {return new Liquid_shallow(p);};
    d.glyph = '~';
    d.tile = Tile_id::water1;
    d.move_rules.set_can_move_cmn();
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_rigid = false;
    d.dodge_modifier = -10;
    d.matl_type = Matl::fluid;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::liquid_deep;
    d.mk_obj = [](const Pos & p) {return new Liquid_deep(p);};
    d.glyph = '~';
    d.tile = Tile_id::water1;
    d.move_rules.set_prop_can_move(Prop_id::ethereal);
    d.move_rules.set_prop_can_move(Prop_id::flying);
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_rigid = false;
    d.matl_type = Matl::fluid;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::chasm;
    d.mk_obj = [](const Pos & p) {return new Chasm(p);};
    d.glyph = ' ';
    d.move_rules.set_prop_can_move(Prop_id::ethereal);
    d.move_rules.set_prop_can_move(Prop_id::flying);
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.is_bottomless = true;
    d.msg_on_player_blocked = "A chasm lies in my way.";
    d.msg_on_player_blocked_blind =
        "I realize I am standing on the edge of a chasm.";
    d.shock_when_adjacent = 3;
    d.matl_type = Matl::empty;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::gravestone;
    d.mk_obj = [](const Pos & p) {return new Grave_stone(p);};
    d.glyph = '&';
    d.tile = Tile_id::grave_stone;
    d.move_rules.set_prop_can_move(Prop_id::ethereal);
    d.move_rules.set_prop_can_move(Prop_id::flying);
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 2;
    d.matl_type = Matl::stone;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::church_bench;
    d.mk_obj = [](const Pos & p) {return new Church_bench(p);};
    d.glyph = '[';
    d.tile = Tile_id::church_bench;
    d.move_rules.set_prop_can_move(Prop_id::ethereal);
    d.move_rules.set_prop_can_move(Prop_id::flying);
    d.move_rules.set_prop_can_move(Prop_id::ooze);
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::wood;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::carpet;
    d.mk_obj = [](const Pos & p) {return new Carpet(p);};
    d.glyph = '.';
    d.tile = Tile_id::floor;
    d.can_have_rigid = false;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::cloth;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::rubble_high;
    d.mk_obj = [](const Pos & p) {return new Rubble_high(p);};
    d.glyph = 8;
    d.tile = Tile_id::rubble_high;
    d.move_rules.set_prop_can_move(Prop_id::ethereal);
    d.move_rules.set_prop_can_move(Prop_id::ooze);
    d.move_rules.set_prop_can_move(Prop_id::burrowing);
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.is_smoke_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::rubble_low;
    d.mk_obj = [](const Pos & p) {return new Rubble_low(p);};
    d.glyph = ',';
    d.tile = Tile_id::rubble_low;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(4, {0, DLVL_LAST}, Placement_rule::either,
    {
        Room_type::plain, Room_type::crypt, Room_type::monster, Room_type::cave
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::bones;
    d.mk_obj = [](const Pos & p) {return new Bones(p);};
    d.glyph = '&';
    d.tile = Tile_id::corpse2;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(3, {0, DLVL_LAST}, Placement_rule::either,
    {
        Room_type::monster, Room_type::cave
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::statue;
    d.mk_obj = [](const Pos & p) {return new Statue(p);};
    d.glyph = 5; //Paragraph sign
    d.tile = Tile_id::witch_or_warlock;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(6, {0, DLVL_LAST_MID_GAME}, Placement_rule::either,
    {
        Room_type::plain, Room_type::human, Room_type::forest
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::cocoon;
    d.mk_obj = [](const Pos & p) {return new Cocoon(p);};
    d.glyph = '8';
    d.tile = Tile_id::cocoon_closed;
    d.is_projectile_passable = true;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 3;
    d.matl_type = Matl::cloth;
    d.room_spawn_rules.set(3, {0, DLVL_LAST}, Placement_rule::either, {Room_type::spider});
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::chest;
    d.mk_obj = [](const Pos & p) {return new Chest(p);};
    d.glyph = '+';
    d.tile = Tile_id::chest_closed;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.room_spawn_rules.set(2, {0, DLVL_LAST_MID_GAME}, Placement_rule::adj_to_walls,
    {
        Room_type::human
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::cabinet;
    d.mk_obj = [](const Pos & p) {return new Cabinet(p);};
    d.glyph = '7';
    d.tile = Tile_id::cabinet_closed;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::wood;
    d.room_spawn_rules.set(1, {0, DLVL_LAST_MID_GAME}, Placement_rule::adj_to_walls,
    {
        Room_type::human
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::fountain;
    d.mk_obj = [](const Pos & p) {return new Fountain(p);};
    d.glyph = '%';
    d.tile = Tile_id::fountain;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(1, {0, DLVL_LAST_MID_GAME}, Placement_rule::away_from_walls,
    {
        Room_type::plain, Room_type::forest
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::pillar;
    d.mk_obj = [](const Pos & p) {return new Pillar(p);};
    d.glyph = '|';
    d.tile = Tile_id::pillar;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(3, {0, DLVL_LAST}, Placement_rule::away_from_walls,
    {
        Room_type::plain, Room_type::crypt, Room_type::ritual, Room_type::monster
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::monolith;
    d.mk_obj = [](const Pos & p) {return new Monolith(p);};
    d.glyph = '|';
    d.tile = Tile_id::monolith;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = true;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(1, {0, DLVL_LAST}, Placement_rule::away_from_walls,
    {
        Room_type::cave, Room_type::forest
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::stalagmite;
    d.mk_obj = [](const Pos & p) {return new Stalagmite(p);};
    d.glyph = ':';
    d.tile = Tile_id::stalagmite;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = true;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(10, {0, DLVL_LAST}, Placement_rule::either,
    {
        Room_type::cave
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::altar;
    d.mk_obj = [](const Pos & p) {return new Altar(p);};
    d.glyph = '_';
    d.tile = Tile_id::altar;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 10;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(1, {0, DLVL_LAST_MID_GAME}, Placement_rule::either,
    {
        Room_type::ritual, Room_type::forest
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::tomb;
    d.mk_obj = [](const Pos & p) {return new Tomb(p);};
    d.glyph = '&';
    d.tile = Tile_id::tomb_closed;
    d.move_rules.set_prop_can_move(Prop_id::ethereal);
    d.move_rules.set_prop_can_move(Prop_id::flying);
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 10;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(2, {0, DLVL_LAST_MID_GAME}, Placement_rule::either,
    {
        Room_type::crypt, Room_type::forest
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::door;
    d.mk_obj = [](const Pos & p) {return new Door(p);};
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::trap;
    d.mk_obj = [](const Pos & p) {return new Trap(p);};
    d.move_rules.set_can_move_cmn();
    d.can_have_rigid = false;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::lit_dynamite;
    d.mk_obj = [](const Pos & p) {return new Lit_dynamite(p);};
    d.glyph = '/';
    d.tile = Tile_id::dynamite_lit;
    d.move_rules.set_can_move_cmn();
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_item = false;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::lit_flare;
    d.mk_obj = [](const Pos & p) {return new Lit_flare(p);};
    d.glyph = '/';
    d.tile = Tile_id::flare_lit;
    d.move_rules.set_can_move_cmn();
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::smoke;
    d.mk_obj = [](const Pos & p) {return new Smoke(p);};
    d.glyph = '*';
    d.tile = Tile_id::smoke;
    d.move_rules.set_can_move_cmn();
    d.is_los_passable = false;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::event_wall_crumble;
    d.mk_obj = [](const Pos & p) {return new Event_wall_crumble(p);};
    d.move_rules.set_can_move_cmn();
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = Feature_id::event_rats_in_the_walls_discovery;
    d.mk_obj = [](const Pos & p) {return new Event_rats_in_the_walls_discovery(p);};
    d.move_rules.set_can_move_cmn();
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
}

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;
    init_data_list();
    TRACE_FUNC_END;
}

const Feature_data_t& get_data(const Feature_id id)
{
    assert(id != Feature_id::END);
    assert(id != Feature_id::END);
    return data[int(id)];
}

} //Feature_data
