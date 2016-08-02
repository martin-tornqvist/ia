#include "feature_data.hpp"

#include "init.hpp"

#include "colors.hpp"
#include "actor.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "feature_trap.hpp"
#include "feature_event.hpp"
#include "feature_door.hpp"
#include "feature_monolith.hpp"
#include "game_time.hpp"

//--------------------------------------------------------- FEATURE ROOM SPAWN RULES
FeatureRoomSpawnRules::FeatureRoomSpawnRules() :
    max_nr_in_room_(-1),
    dlvls_allowed_(Range(-1, -1)),
    placement_rule_(PlacementRule::adj_to_walls)
{
    room_types_native_.clear();
}

void FeatureRoomSpawnRules::reset()
{
    *this = FeatureRoomSpawnRules();
}

void FeatureRoomSpawnRules::set(const int max_nr_in_room,
                                   const Range& dlvls_allowed,
                                   const PlacementRule placement_rule,
                                   std::initializer_list<RoomType> room_types)
{
    max_nr_in_room_     = max_nr_in_room;
    dlvls_allowed_      = dlvls_allowed;
    placement_rule_     = placement_rule;

    room_types_native_.clear();

    for (RoomType id : room_types)
    {
        room_types_native_.push_back(id);
    }
}

bool FeatureRoomSpawnRules::is_belonging_to_room_type(const RoomType type) const
{
    return find(begin(room_types_native_), end(room_types_native_), type) !=
           end(room_types_native_);
}

PlacementRule FeatureRoomSpawnRules::placement_rule() const
{
    return placement_rule_;
}

int FeatureRoomSpawnRules::max_nr_in_room() const
{
    return max_nr_in_room_;
}

Range FeatureRoomSpawnRules::dlvls_allowed() const
{
    return dlvls_allowed_;
}

//--------------------------------------------------------- MOVE RULES
bool MoveRules::can_move(Actor& actor) const
{
    if (can_move_cmn_)
    {
        return true;
    }

    auto& prop_handler = actor.prop_handler();

    //If not allowing normal move, check if any property overrides this
    for (size_t i = 0; i < size_t(PropId::END); ++i)
    {
        const PropId id = PropId(i);

        if (prop_handler.has_prop(id) && can_move_if_have_prop_[i])
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------- FEATURE DATA
namespace feature_data
{

FeatureDataT data_list[int(FeatureId::END)];

namespace
{

void reset_data(FeatureDataT& d)
{
    d.mk_obj = [](const P & p) {(void)p; return nullptr;};
    d.id = FeatureId::END;
    d.glyph = ' ';
    d.tile = TileId::empty;
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

void add_to_list_and_reset(FeatureDataT& d)
{
    data_list[int(d.id)] = d;
    reset_data(d);
}

void init_data_list()
{
    FeatureDataT d;
    reset_data(d);

    //---------------------------------------------------------------------------
    d.id = FeatureId::floor;
    d.mk_obj = [](const P & p)
    {
        return new Floor(p);
    };
    d.glyph = '.';
    d.tile = TileId::floor;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::stone;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::bridge;
    d.mk_obj = [](const P & p)
    {
        return new Bridge(p);
    };
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::wood;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::wall;
    d.mk_obj = [](const P & p)
    {
        return new Wall(p);
    };
    d.glyph = config::is_text_mode_wall_full_square() ? 10 : '#';
    d.tile = TileId::wall_top;
    d.move_rules.set_prop_can_move(PropId::ethereal);
    d.move_rules.set_prop_can_move(PropId::burrowing);
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
    d.id = FeatureId::tree;
    d.mk_obj = [](const P & p)
    {
        return new Tree(p);
    };
    d.glyph = '|';
    d.tile = TileId::tree;
    d.move_rules.set_prop_can_move(PropId::ethereal);
    d.is_sound_passable = false;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.msg_on_player_blocked = "There is a tree in the way.";
    d.matl_type = Matl::wood;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::grass;
    d.mk_obj = [](const P & p)
    {
        return new Grass(p);
    };
    d.glyph = '.';
    d.tile = TileId::floor;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::plant;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::bush;
    d.mk_obj = [](const P & p)
    {
        return new Bush(p);
    };
    d.glyph = '"';
    d.tile = TileId::bush;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::plant;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::stairs;
    d.mk_obj = [](const P & p)
    {
        return new Stairs(p);
    };
    d.glyph = '>';
    d.tile = TileId::stairs_down;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::monolith;
    d.mk_obj = [](const P & p)
    {
        return new Monolith(p);
    };
    d.glyph = '|';
    d.tile = TileId::monolith;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false; //We don't want to mess with the color
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 3;
    d.matl_type = Matl::stone;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::lever;
    d.mk_obj = [](const P & p)
    {
        return new Lever(p);
    };
    d.glyph = '%';
    d.tile = TileId::lever_left;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::metal;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::brazier;
    d.mk_obj = [](const P & p)
    {
        return new Brazier(p);
    };
    d.glyph = '0';
    d.tile = TileId::brazier;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::metal;
    d.room_spawn_rules.set(3, {0, dlvl_last_mid_game}, PlacementRule::away_from_walls,
    {
        RoomType::ritual, RoomType::plain,
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::liquid_shallow;
    d.mk_obj = [](const P & p)
    {
        return new LiquidShallow(p);
    };
    d.glyph = '~';
    d.tile = TileId::water1;
    d.move_rules.set_can_move_cmn();
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_rigid = false;
    d.dodge_modifier = -10;
    d.matl_type = Matl::fluid;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::liquid_deep;
    d.mk_obj = [](const P & p)
    {
        return new LiquidDeep(p);
    };
    d.glyph = '~';
    d.tile = TileId::water1;
    d.move_rules.set_prop_can_move(PropId::ethereal);
    d.move_rules.set_prop_can_move(PropId::flying);
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_rigid = false;
    d.matl_type = Matl::fluid;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::chasm;
    d.mk_obj = [](const P & p)
    {
        return new Chasm(p);
    };
    d.glyph = ' ';
    d.move_rules.set_prop_can_move(PropId::ethereal);
    d.move_rules.set_prop_can_move(PropId::flying);
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.is_bottomless = true;
    d.msg_on_player_blocked = "A chasm lies in my way.";
    d.msg_on_player_blocked_blind =
        "I realize I am standing on the edge of a chasm.";
    d.matl_type = Matl::empty;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::gravestone;
    d.mk_obj = [](const P & p)
    {
        return new GraveStone(p);
    };
    d.glyph = ']';
    d.tile = TileId::grave_stone;
    d.move_rules.set_prop_can_move(PropId::ethereal);
    d.move_rules.set_prop_can_move(PropId::flying);
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 2;
    d.matl_type = Matl::stone;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::church_bench;
    d.mk_obj = [](const P & p)
    {
        return new ChurchBench(p);
    };
    d.glyph = '[';
    d.tile = TileId::church_bench;
    d.move_rules.set_prop_can_move(PropId::ethereal);
    d.move_rules.set_prop_can_move(PropId::flying);
    d.move_rules.set_prop_can_move(PropId::ooze);
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
    d.id = FeatureId::carpet;
    d.mk_obj = [](const P & p)
    {
        return new Carpet(p);
    };
    d.glyph = '.';
    d.tile = TileId::floor;
    d.can_have_rigid = false;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::cloth;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::rubble_high;
    d.mk_obj = [](const P & p)
    {
        return new RubbleHigh(p);
    };
    d.glyph = 8;
    d.tile = TileId::rubble_high;
    d.move_rules.set_prop_can_move(PropId::ethereal);
    d.move_rules.set_prop_can_move(PropId::ooze);
    d.move_rules.set_prop_can_move(PropId::burrowing);
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
    d.id = FeatureId::rubble_low;
    d.mk_obj = [](const P & p)
    {
        return new RubbleLow(p);
    };
    d.glyph = ',';
    d.tile = TileId::rubble_low;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(4, {0, dlvl_last}, PlacementRule::either,
    {
        RoomType::plain, RoomType::crypt, RoomType::monster, RoomType::cave
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::bones;
    d.mk_obj = [](const P & p)
    {
        return new Bones(p);
    };
    d.glyph = '&';
    d.tile = TileId::corpse2;
    d.move_rules.set_can_move_cmn();
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(3, {0, dlvl_last}, PlacementRule::either,
    {
        RoomType::monster, RoomType::cave
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::statue;
    d.mk_obj = [](const P & p)
    {
        return new Statue(p);
    };
    d.glyph = 5; //Paragraph sign
    d.tile = TileId::witch_or_warlock;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(6, {0, dlvl_last_mid_game}, PlacementRule::either,
    {
        RoomType::plain, RoomType::human, RoomType::forest
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::cocoon;
    d.mk_obj = [](const P & p)
    {
        return new Cocoon(p);
    };
    d.glyph = '8';
    d.tile = TileId::cocoon_closed;
    d.is_projectile_passable = true;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 1;
    d.matl_type = Matl::cloth;
    d.room_spawn_rules.set(3, {0, dlvl_last}, PlacementRule::either, {RoomType::spider});
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::chest;
    d.mk_obj = [](const P & p)
    {
        return new Chest(p);
    };
    d.glyph = '+';
    d.tile = TileId::chest_closed;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.room_spawn_rules.set(2, {0, dlvl_last_mid_game}, PlacementRule::adj_to_walls,
    {
        RoomType::human
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::cabinet;
    d.mk_obj = [](const P & p)
    {
        return new Cabinet(p);
    };
    d.glyph = '7';
    d.tile = TileId::cabinet_closed;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::wood;
    d.room_spawn_rules.set(1, {0, dlvl_last_mid_game}, PlacementRule::adj_to_walls,
    {
        RoomType::human
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::fountain;
    d.mk_obj = [](const P & p)
    {
        return new Fountain(p);
    };
    d.glyph = '1';
    d.tile = TileId::fountain;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(1, {0, dlvl_last_mid_game}, PlacementRule::away_from_walls,
    {
        RoomType::plain, RoomType::forest
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::pillar;
    d.mk_obj = [](const P & p)
    {
        return new Pillar(p);
    };
    d.glyph = '|';
    d.tile = TileId::pillar;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(3, {0, dlvl_last}, PlacementRule::away_from_walls,
    {
        RoomType::plain, RoomType::crypt, RoomType::ritual, RoomType::monster
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::stalagmite;
    d.mk_obj = [](const P & p)
    {
        return new Stalagmite(p);
    };
    d.glyph = ':';
    d.tile = TileId::stalagmite;
    d.is_projectile_passable = false;
    d.is_los_passable = false;
    d.can_have_blood = true;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(10, {0, dlvl_last}, PlacementRule::either,
    {
        RoomType::cave
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::altar;
    d.mk_obj = [](const P & p)
    {
        return new Altar(p);
    };
    d.glyph = '_';
    d.tile = TileId::altar;
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 3;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(1, {0, dlvl_last_mid_game}, PlacementRule::either,
    {
        RoomType::ritual, RoomType::forest
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::tomb;
    d.mk_obj = [](const P & p)
    {
        return new Tomb(p);
    };
    d.glyph = ']';
    d.tile = TileId::tomb_closed;
    d.move_rules.set_prop_can_move(PropId::ethereal);
    d.move_rules.set_prop_can_move(PropId::flying);
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    d.shock_when_adjacent = 3;
    d.matl_type = Matl::stone;
    d.room_spawn_rules.set(2, {0, dlvl_last_mid_game}, PlacementRule::either,
    {
        RoomType::crypt, RoomType::forest
    });
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::door;
    d.mk_obj = [](const P & p)
    {
        return new Door(p);
    };
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_rigid = false;
    d.can_have_item = false;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::trap;
    d.mk_obj = [](const P & p)
    {
        return new Trap(p);
    };
    d.move_rules.set_can_move_cmn();
    d.can_have_rigid = false;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::lit_dynamite;
    d.mk_obj = [](const P & p)
    {
        return new LitDynamite(p);
    };
    d.glyph = '/';
    d.tile = TileId::dynamite_lit;
    d.move_rules.set_can_move_cmn();
    d.can_have_blood = false;
    d.can_have_gore = false;
    d.can_have_corpse = false;
    d.can_have_item = false;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::lit_flare;
    d.mk_obj = [](const P & p)
    {
        return new LitFlare(p);
    };
    d.glyph = '/';
    d.tile = TileId::flare_lit;
    d.move_rules.set_can_move_cmn();
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::smoke;
    d.mk_obj = [](const P & p)
    {
        return new Smoke(p);
    };
    d.glyph = '*';
    d.tile = TileId::smoke;
    d.move_rules.set_can_move_cmn();
    d.is_los_passable = false;
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::event_wall_crumble;
    d.mk_obj = [](const P & p)
    {
        return new EventWallCrumble(p);
    };
    d.move_rules.set_can_move_cmn();
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::event_snake_emerge;
    d.mk_obj = [](const P & p)
    {
        return new EventSnakeEmerge(p);
    };
    d.move_rules.set_can_move_cmn();
    add_to_list_and_reset(d);
    //---------------------------------------------------------------------------
    d.id = FeatureId::event_rats_in_the_walls_discovery;
    d.mk_obj = [](const P & p)
    {
        return new EventRatsInTheWallsDiscovery(p);
    };
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

const FeatureDataT& data(const FeatureId id)
{
    ASSERT(id != FeatureId::END);
    ASSERT(id != FeatureId::END);
    return data_list[int(id)];
}

} //FeatureData
