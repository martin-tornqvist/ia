#include "actor_data.hpp"

#include "init.hpp"

#include <string>
#include <vector>
#include <math.h>

#include "item.hpp"
#include "saving.hpp"
#include "game_time.hpp"

void ActorDataT::reset()
{
    id = ActorId::END;
    name_a = "";
    name_the = "";
    corpse_name_a = "";
    corpse_name_the = "";
    tile = TileId::empty;
    glyph = 'X';
    color = clr_yellow;

    // Default spawn group size is "alone"
    group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone, 1),
    });

    hp = dmg_melee = dmg_ranged = 1;
    spi = 12;
    speed_pct = (int)ActorSpeed::normal;

    for (size_t i = 0; i < (size_t)PropId::END; ++i)
    {
        natural_props[i] = false;
    }

    ranged_cooldown_turns = spell_cooldown_turns = 0;

    ability_vals.reset();

    spell_skill = 0;

    for (size_t i = 0; i < (size_t)AiId::END; ++i)
    {
        ai[i] = false;
    }

    ai[(size_t)AiId::moves_to_random_when_unaware] = true;

    nr_turns_aware = 0;
    spawn_min_dlvl = 999;
    spawn_max_dlvl = dlvl_last - 2;
    actor_size = ActorSize::humanoid;
    is_humanoid = false;
    is_infra_visible = true;
    is_auto_descr_allowed = true;
    nr_kills = 0;
    has_player_seen = false;
    can_open_doors = can_bash_doors = false;
    prevent_knockback = false;
    nr_left_allowed_to_spawn = -1;
    is_unique = false;
    is_auto_spawn_allowed = true;
    wary_msg = "";
    aggro_msg_mon_seen = "";
    aggro_msg_mon_hidden = "";
    aggro_sfx_mon_seen = SfxId::END;
    aggro_sfx_mon_hidden = SfxId::END;
    spell_cast_msg = "";
    erratic_move_pct = ActorErraticFreq::rare;
    mon_shock_lvl = ShockLvl::none;
    is_rat = false;
    is_canine = false;
    is_spider = false;
    is_undead = false;
    is_ghost = false;
    is_snake = false;
    is_reptile = false;
    is_amphibian = false;
    can_be_summoned = false;
    can_bleed = true;
    can_leave_corpse = true;
    prio_corpse_bash = false;
    native_rooms.clear();
    descr = "";
}

namespace actor_data
{

ActorDataT data[(size_t)ActorId::END];

namespace
{

void init_data_list()
{
    ActorDataT d;
    d.reset();

    d.name_a = "Player";
    d.name_the = "Player";
    d.id = ActorId::player;
    d.hp = player_start_hp;
    d.spi = player_start_spi;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = '@';
    d.color = clr_white_high;
    d.tile = TileId::player_firearm;
    d.is_auto_spawn_allowed = false;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Reanimated Corpse";
    d.name_the = "The Reanimated Corpse";
    d.corpse_name_a = "A lifeless gruesome corpse";
    d.corpse_name_the = "The lifeless gruesome corpse";
    d.id = ActorId::zombie;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'Z';
    d.color = clr_brown;
    d.tile = TileId::zombie_unarmed;
    d.hp = 16;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    40),
        MonGroupSpawnRule(MonGroupSize::pack,   10),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::humanoid;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 8;
    d.descr =
        "This rotting thing appears to have been brought back to life "
        "through some abominable process. It has grown sharp claws to "
        "attack with.";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_seen = d.name_the + " growls.";
    d.aggro_msg_mon_hidden = "I hear a growling voice.";
    d.aggro_sfx_mon_seen = SfxId::zombie_growl;
    d.aggro_sfx_mon_hidden = SfxId::zombie_growl;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_undead = true;
    d.is_humanoid = true;
    d.prio_corpse_bash = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::crypt);
    d.native_rooms.push_back(RoomType::jail);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Reanimated Corpse";
    d.name_the = "The Reanimated Corpse";
    d.corpse_name_a = "A lifeless gruesome corpse";
    d.corpse_name_the = "The lifeless gruesome corpse";
    d.id = ActorId::zombie_axe;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'Z';
    d.color = clr_gray;
    d.tile = TileId::zombie_armed;
    d.hp = 16;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    40),
        MonGroupSpawnRule(MonGroupSize::pack,   10),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::humanoid;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 8;
    d.descr =
        "This rotting thing appears to have been brought back to life "
        "through some abominable process. It is wielding a rusty axe.";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_seen = d.name_the + " growls.";
    d.aggro_msg_mon_hidden = "I hear a growling voice.";
    d.aggro_sfx_mon_seen = SfxId::zombie_growl;
    d.aggro_sfx_mon_hidden = SfxId::zombie_growl;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_undead = true;
    d.is_humanoid = true;
    d.prio_corpse_bash = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::crypt);
    d.native_rooms.push_back(RoomType::jail);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Bloated Corpse";
    d.name_the = "The Bloated Corpse";
    d.corpse_name_a = "A huge gruesome corpse";
    d.corpse_name_the = "The huge gruesome corpse";
    d.id = ActorId::bloated_zombie;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.ranged_cooldown_turns = 20;
    d.glyph = 'Z';
    d.color = clr_white_high;
    d.tile = TileId::zombie_bloated;
    d.hp = 24;
    d.dmg_melee = min_dmg_to_wound + 2;
    d.dmg_ranged = 3;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::ranged, 50);
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  3),
        MonGroupSpawnRule(MonGroupSize::few,    1)
    });
    d.actor_size = ActorSize::humanoid;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 16;
    d.descr =
        "This lumbering giant corpse seems to be artificially bloated "
        "somehow. It is constantly oozing putrid liquid that it can spit "
        "to attack with.";
    d.wary_msg = d.name_the + " looks wary.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_undead = true;
    d.is_humanoid = true;
    d.prio_corpse_bash = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::crypt);
    d.native_rooms.push_back(RoomType::jail);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Major Clapham-Lee";
    d.name_the = "Major Clapham-Lee";
    d.corpse_name_a = "The corpse of Major Clapham-Lee";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::major_clapham_lee;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'Z';
    d.color = clr_cyan_lgt;
    d.tile = TileId::zombie_unarmed;
    d.hp = 26;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.spawn_min_dlvl = 4;
    d.actor_size = ActorSize::humanoid;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 20;
    d.descr =
        "Major Sir Eric Moreland Clapham-Lee was once a commanding officer "
        "during the Great War. Shortly after his plane was shot down, his "
        "body was stolen. Now he roams these halls as a resurrected "
        "warrior in service of the cult, commanding a squad of undead.";
    d.wary_msg = d.name_the + " looks wary.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_undead = true;
    d.is_humanoid = true;
    d.is_unique = true;
    d.prio_corpse_bash = true;
    d.nr_left_allowed_to_spawn = 1;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::crypt);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Dean Halsey";
    d.name_the = "Dean Halsey";
    d.corpse_name_a = "The corpse of Dean Halsey";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::dean_halsey;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'Z';
    d.color = clr_cyan;
    d.tile = TileId::zombie_unarmed;
    d.hp = 26;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.is_auto_spawn_allowed = false;
    d.spawn_min_dlvl = 4;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.nr_turns_aware = 20;
    d.descr =
        "Alan Halsey was the dean of the Miskatonic University in New "
        "England. He must have gotten into the hands of the Cult, who "
        "turned him into the hellish zombie warrior I now see before me.";
    d.wary_msg = d.name_the + " looks wary.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_undead = true;
    d.is_humanoid = true;
    d.is_unique = true;
    d.prio_corpse_bash = true;
    d.nr_left_allowed_to_spawn = 0;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::crypt);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Crawling Intestines";
    d.name_the = "The Crawling Intestines";
    d.corpse_name_a = "";
    d.corpse_name_the = "";
    d.id = ActorId::crawling_intestines;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'z';
    d.color = clr_red_lgt;
    d.tile = TileId::crawling_intestines;
    d.hp = 4;
    d.spi = 8;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 15;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  2),
        MonGroupSpawnRule(MonGroupSize::few,    1)
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 5;
    d.descr = "A writhing mass of crawling intestines.";
    d.wary_msg = d.name_the + " starts groping and feeling around.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.can_be_summoned = false;
    d.is_auto_descr_allowed = false;
    d.can_leave_corpse = false;
    d.is_undead = true;
    d.native_rooms.push_back(RoomType::monster);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Crawling Hand";
    d.name_the = "The Crawling Hand";
    d.corpse_name_a = "";
    d.corpse_name_the = "";
    d.id = ActorId::crawling_hand;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'z';
    d.color = clr_brown;
    d.tile = TileId::crawling_hand;
    d.hp = 4;
    d.spi = 8;
    d.dmg_melee = 3;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 15;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  2),
        MonGroupSpawnRule(MonGroupSize::few,    1)
    });
    d.actor_size = ActorSize::floor;
    d.prevent_knockback = true;
    d.nr_turns_aware = 5;
    d.descr =
        "A disembodied hand, crawling around frantically on its fingertips "
        "like a spider.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::very;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.can_be_summoned = false;
    d.is_auto_descr_allowed = false;
    d.can_leave_corpse = false;
    d.is_undead = true;
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::plain);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Thing";
    d.name_the = "Thing";
    d.corpse_name_a = "";
    d.corpse_name_the = "";
    d.id = ActorId::thing;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 20;
    d.glyph = 'z';
    d.color = clr_magenta;
    d.tile = TileId::crawling_hand;
    d.hp = 25;
    d.spi = 25;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 60);
    d.ability_vals.set_val(AbilityId::stealth, 90);
    d.nr_left_allowed_to_spawn = 1;
    d.is_unique = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 5;
    d.spawn_max_dlvl = dlvl_last;
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 999;
    d.descr =
        "A particularly nasty disembodied hand, crawling around frantically on "
        "its fingertips like a spider. Beware of the Thing.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.can_be_summoned = false;
    d.can_leave_corpse = false;
    d.is_auto_descr_allowed = false;
    d.is_undead = true;
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::human);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Floating Skull";
    d.name_the = "The Floating Skull";
    d.corpse_name_a = "A Skull";
    d.corpse_name_the = "The Skull";
    d.id = ActorId::floating_skull;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'z';
    d.color = clr_white_high;
    d.tile = TileId::floating_skull;
    d.hp = 6;
    d.spi = 16;
    d.dmg_melee = min_dmg_to_wound - 2;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 60);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 10;
    d.descr = "A severed head floating through the air.";
    d.wary_msg = d.name_the + " looks wary.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.can_be_summoned = true;
    d.is_undead = true;
    d.is_auto_descr_allowed = false;
    d.can_bleed = false;
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::chasm);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Cultist";
    d.name_the = "The Cultist";
    d.corpse_name_a = "A Cultist corpse";
    d.corpse_name_the = "The Cultist corpse";
    d.id = ActorId::cultist;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.ranged_cooldown_turns = 2;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 10;
    d.glyph = 'P';
    d.color = clr_gray;
    d.tile = TileId::cultist_firearm;
    d.hp = 4;
    d.spi = 12;
    d.ability_vals.set_val(AbilityId::melee, 45);
    d.ability_vals.set_val(AbilityId::ranged, 50);
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_open_doors = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 12;
    d.descr =
        "A fanatic cultist of the lowest rank, madly gibbering in some "
        "half-lost language. They may occasionally have learned to cast a "
        "spell or two, although never with any great skill. "
        "Therefore they tend to rely on mundane firearms to handle threats.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "mutters incantations.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::human);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::jail);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Bog Tcher";
    d.name_the = "The Bog Tcher";
    d.corpse_name_a = "A Bog Tcher corpse";
    d.corpse_name_the = "The Bog Tcher corpse";
    d.id = ActorId::bog_tcher;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.ranged_cooldown_turns = 2;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 10;
    d.glyph = 'p';
    d.color = clr_brown_drk;
    d.tile = TileId::bog_tcher;
    d.hp = 6;
    d.spi = 12;
    d.ability_vals.set_val(AbilityId::melee, 45);
    d.ability_vals.set_val(AbilityId::ranged, 50);
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  4),
        MonGroupSpawnRule(MonGroupSize::few,    1),
    });
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_open_doors = true;
    d.can_bash_doors = false;
    d.nr_turns_aware = 16;
    d.descr =
        "A vile dwarfish creature, clad in coarse brown robes. It has a "
        "cruel grin on its pale face, and its eyes are gleaming with hatred. "
        "The Bog Tcher are mechanically inclined, which they put to use by "
        "crafting sadistic weapons and traps. Their favored weapon is their "
        "\"spike gun\", which can pin the target against solid objects for "
        "prolonged suffering.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "mutters incantations.";
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::jail);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Keziah Mason";
    d.name_the = "Keziah Mason";
    d.corpse_name_a = "The corpse of Keziah Mason";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::keziah_mason;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 50;
    d.glyph = 'P';
    d.color = clr_magenta;
    d.tile = TileId::witch_or_warlock;
    d.hp = 10;
    d.spi = 60;
    d.ability_vals.set_val(AbilityId::melee, 40);
    d.ability_vals.set_val(AbilityId::ranged, 40);
    d.nr_left_allowed_to_spawn = 1;
    d.is_unique = true;
    d.spawn_min_dlvl = 3;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.descr =
        "During the Salem witch trials of 1692 an old woman by the name of "
        "Keziah Mason was arrested for her suspicious behavior and seeming "
        "insight into other worlds. In her testimony to the judge, she "
        "confessed to having signed her name in the Black Book of "
        "Azathoth, and of her secret name of Nahab. She later disappeared "
        "mysteriously from Salem. She is described as having a \"bent "
        "back, long nose, and shrivelled chin\" and has a \"croaking "
        "voice\". She has an animal familiar; the rat Brown Jenkin, which "
        "she trusts to carry messages between her and the devil. She feeds "
        "this creature on her blood.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "croaks incantations.";
    d.aggro_msg_mon_seen = d.name_the + " chortles in a croaking voice.";
    d.aggro_msg_mon_hidden = "I hear a repulsive croaking voice.";
    d.nr_turns_aware = 999;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Brown Jenkin";
    d.name_the = "Brown Jenkin";
    d.corpse_name_a = "The corpse of Brown Jenkin";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::brown_jenkin;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_cooldown_turns = 3;
    d.spell_skill = 10;
    d.glyph = 'r';
    d.color = clr_magenta;
    d.tile = TileId::rat_thing;
    d.hp = 10;
    d.spi = 25;
    d.dmg_melee = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::stealth, 90);
    d.is_auto_spawn_allowed = false;
    d.nr_left_allowed_to_spawn = 0;
    d.is_unique = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = data[(size_t)ActorId::keziah_mason].spawn_min_dlvl;
    d.actor_size = ActorSize::floor;
    d.descr =
        "\"That object - no larger than a good sized rat and quaintly "
        "called by the townspeople, \"Brown Jenkin\" - seemed to have been "
        "the fruit of a remarkable case of sympathetic herd-delusion, for "
        "in 1692, no less than eleven persons had testified to glimpsing it. "
        "There were recent rumors, too, with a baffling and disconcerting "
        "amount of agreement. Witnesses said it had long hair and the shape "
        "of a rat, but that its sharp-toothed, bearded face was evilly human "
        "while its paws were like tiny human hands. It took messages betwixt "
        "old Keziah and the devil, and was nursed on the witch's blood, which "
        "it sucked like a vampire. Its voice was a kind of loathsome titter, "
        "and could speak all languages.\" "
        "H.P.Lovecraft -\"Dreams in the witch house\".";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.spell_cast_msg = "";
    d.aggro_msg_mon_seen = d.name_the + " titters in a loathsome voice.";
    d.aggro_msg_mon_hidden = "I hear a loathsome titter.";
    d.nr_turns_aware = 999;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_rat = true;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Elder Hierophant";
    d.name_the = "The " + d.name_a;
    d.corpse_name_a = "The corpse of The Elder Hierophant";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::leng_elder;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::slow;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 100;
    d.glyph = 'P';
    d.color = clr_yellow;
    d.tile = TileId::leng_elder;
    d.hp = 200;
    d.spi = 200;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::ranged, 40);
    d.is_auto_spawn_allowed = false;
    d.is_unique = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.descr = "[DESCRIPTION MISSING]";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "makes strange gestures in the air.";
    d.nr_turns_aware = 999;
    d.erratic_move_pct = ActorErraticFreq::never;
    d.mon_shock_lvl = ShockLvl::mind_shattering;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Cultist Priest";
    d.name_the = "The Cultist Priest";
    d.corpse_name_a = "A Cultist Priest corpse";
    d.corpse_name_the = "The Cultist Priest corpse";
    d.id = ActorId::cultist_priest;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 30;
    d.glyph = 'P';
    d.color = clr_cyan_lgt;
    d.tile = TileId::cultist_dagger;
    d.hp = 6;
    d.spi = 30;
    d.ability_vals.set_val(AbilityId::melee, 45);
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_open_doors = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 12;
    d.descr =
        "A fanatic cultist of the Priest rank, madly gibbering in some "
        "half-lost language.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "mutters incantations.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::human);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Wizard";
    d.name_the = "The Wizard";
    d.corpse_name_a = "A Wizard corpse";
    d.corpse_name_the = "The Wizard corpse";
    d.id = ActorId::cultist_wizard;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 60;
    d.glyph = 'P';
    d.color = clr_yellow;
    d.tile = TileId::witch_or_warlock;
    d.hp = 18;
    d.spi = 40;
    d.ability_vals.set_val(AbilityId::melee, 45);
    d.spawn_min_dlvl = dlvl_first_mid_game;
    d.spawn_max_dlvl = (dlvl_last_mid_game + dlvl_last) / 2;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_open_doors = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 12;
    d.descr =
        "A fanatic cultist of the Wizard rank, madly gibbering in some "
        "half-lost language.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "mutters incantations.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::human);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Grand Wizard";
    d.name_the = "The Grand Wizard";
    d.corpse_name_a = "A Grand Wizard corpse";
    d.corpse_name_the = "The Grand Wizard corpse";
    d.id = ActorId::cultist_grand_wizard;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 90;
    d.glyph = 'P';
    d.color = clr_white;
    d.tile = TileId::witch_or_warlock;
    d.hp = 36;
    d.spi = 50;
    d.ability_vals.set_val(AbilityId::melee, 45);
    d.spawn_min_dlvl = dlvl_first_late_game;
    d.spawn_max_dlvl = 999;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_open_doors = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 12;
    d.descr =
        "A fanatic cultist of the Grand Wizard rank, madly gibbering in some "
        "half-lost language.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "mutters incantations.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::human);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Huge Green Spider";
    d.name_the = "The Huge Green Spider";
    d.corpse_name_a = "A Green Spider corpse";
    d.corpse_name_the = "The Green Spider corpse";
    d.id = ActorId::green_spider;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'A';
    d.color = clr_green_lgt;
    d.tile = TileId::giant_spider;
    d.hp = 16;
    d.dmg_melee = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  3),
        MonGroupSpawnRule(MonGroupSize::few,    1),
    });
    d.natural_props[(size_t)PropId::infravis] = true;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.descr = "An enormous green spider.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.can_be_summoned = true;
    d.is_spider = true;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::spider);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Huge White Spider";
    d.name_the = "The Huge White Spider";
    d.corpse_name_a = "A White Spider corpse";
    d.corpse_name_the = "The White Spider corpse";
    d.id = ActorId::white_spider;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'A';
    d.color = clr_white_high;
    d.tile = TileId::giant_spider;
    d.hp = 16;
    d.dmg_melee = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  3),
        MonGroupSpawnRule(MonGroupSize::few,    1),
    });
    d.natural_props[(size_t)PropId::infravis] = true;
    d.nr_turns_aware = 5;
    d.actor_size = ActorSize::humanoid;
    d.descr = "An enormous white spider.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.can_be_summoned = true;
    d.is_spider = true;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::spider);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Huge Red Spider";
    d.name_the = "The Huge Red Spider";
    d.corpse_name_a = "A Red Spider corpse";
    d.corpse_name_the = "The Red Spider corpse";
    d.id = ActorId::red_spider;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'A';
    d.color = clr_red_lgt;
    d.tile = TileId::giant_spider;
    d.hp = 16;
    d.dmg_melee = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  3),
        MonGroupSpawnRule(MonGroupSize::few,    1),
    });
    d.natural_props[(size_t)PropId::infravis] = true;
    d.nr_turns_aware = 5;
    d.actor_size = ActorSize::humanoid;
    d.descr = "An enormous red spider.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.can_be_summoned = true;
    d.is_spider = true;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::spider);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Shadow Spider";
    d.name_the = "The Shadow Spider";
    d.id = ActorId::shadow_spider;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'a';
    d.color = clr_gray;
    d.tile = TileId::giant_spider;
    d.hp = 8;
    d.dmg_melee = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::stealth, 90);
    d.natural_props[(size_t)PropId::lgt_sens] = true;
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   2)
    });
    d.nr_turns_aware = 5;
    d.actor_size = ActorSize::humanoid;
    d.is_spider = true;
    d.is_infra_visible = false;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.descr = "A faint shadow of a huge spider, lurking in the periphery.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::spider);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Leng Spider";
    d.name_the = "The Leng Spider";
    d.corpse_name_a = "A Leng Spider corpse";
    d.corpse_name_the = "The Leng Spider corpse";
    d.id = ActorId::leng_spider;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'A';
    d.color = clr_magenta;
    d.tile = TileId::giant_spider;
    d.hp = 30;
    d.dmg_melee = min_dmg_to_wound + 3;
//  d.dmg_ranged = 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
//  d.ability_vals.set_val(AbilityId::ranged, 40);
    d.spawn_min_dlvl = 10;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    4),
        MonGroupSpawnRule(MonGroupSize::pack,   1)
    });
    d.nr_turns_aware = 16;
    d.actor_size = ActorSize::humanoid;
    d.descr =
        "Leng spiders are huge, purplish arachnids, with pustulent bloated "
        "bodies and long, bristly legs. Native to the Dreamlands, the "
        "spiders of Leng are intelligent, dangerous, and gigantic.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.is_spider = true;
    d.is_infra_visible = false;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::spider);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Pit Viper";
    d.name_the = "The Pit Viper";
    d.corpse_name_a = "A Pit Viper corpse";
    d.corpse_name_the = "The Pit Viper corpse";
    d.id = ActorId::pit_viper;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'S';
    d.color = clr_green_lgt;
    d.tile = TileId::snake;
    d.hp = 6;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 5;
    d.descr =
        "A fierce predator. It hisses menacingly, and venom drips from its "
        "fangs.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.aggro_msg_mon_seen = d.name_the + " hisses.";
    d.aggro_msg_mon_hidden = "I hear hissing.";
    d.aggro_sfx_mon_seen = SfxId::hiss;
    d.aggro_sfx_mon_hidden = SfxId::hiss;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.is_infra_visible = false;
    d.is_snake = true;
    d.is_reptile = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    d.native_rooms.push_back(RoomType::ritual);
    d.can_be_summoned = true;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Spitting Cobra";
    d.name_the = "The Spitting Cobra";
    d.corpse_name_a = "A Spitting Cobra corpse";
    d.corpse_name_the = "The Spitting Cobra corpse";
    d.id = ActorId::spitting_cobra;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'S';
    d.color = clr_red;
    d.tile = TileId::snake;
    d.hp = 6;
    d.dmg_melee = 1;
    d.dmg_ranged = 1;
    d.ranged_cooldown_turns = 150;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::ranged, 85);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 5;
    d.descr =
        "A fierce predator. It hisses menacingly, and venom drips from its "
        "fangs.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.aggro_msg_mon_seen = d.name_the + " hisses.";
    d.aggro_msg_mon_hidden = "I hear hissing.";
    d.aggro_sfx_mon_seen = SfxId::hiss;
    d.aggro_sfx_mon_hidden = SfxId::hiss;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.is_infra_visible = false;
    d.is_snake = true;
    d.is_reptile = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    d.native_rooms.push_back(RoomType::ritual);
    d.can_be_summoned = true;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Black Mamba";
    d.name_the = "The Black Mamba";
    d.corpse_name_a = "A Black Mamba corpse";
    d.corpse_name_the = "The Black Mamba corpse";
    d.id = ActorId::black_mamba;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.glyph = 'S';
    d.color = clr_gray;
    d.tile = TileId::snake;
    d.hp = 6;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 5;
    d.descr =
        "A fierce predator. It hisses menacingly, and venom drips from its "
        "fangs.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.aggro_msg_mon_seen = d.name_the + " hisses.";
    d.aggro_msg_mon_hidden = "I hear hissing.";
    d.aggro_sfx_mon_seen = SfxId::hiss;
    d.aggro_sfx_mon_hidden = SfxId::hiss;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.is_infra_visible = false;
    d.is_snake = true;
    d.is_reptile = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    d.native_rooms.push_back(RoomType::ritual);
    d.can_be_summoned = true;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Fire Hound";
    d.name_the = "The Fire Hound";
    d.corpse_name_a = "A Fire Hound corpse";
    d.corpse_name_the = "The Fire Hound corpse";
    d.id = ActorId::fire_hound;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.ranged_cooldown_turns = 6;
    d.glyph = 'd';
    d.color = clr_red;
    d.tile = TileId::hound;
    d.hp = 22;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.dmg_ranged = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::ranged, 65);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 9;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  4),
        MonGroupSpawnRule(MonGroupSize::few,    10),
        MonGroupSpawnRule(MonGroupSize::pack,   1)
    });
    d.actor_size = ActorSize::floor;
    d.can_bash_doors = true;
    d.nr_turns_aware = 16;
    d.descr =
        "This extremely aggressive canine seems to be part corporeal and "
        "part fire. It breathes searing flames.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.aggro_msg_mon_seen = d.name_the + " snarls.";
    d.aggro_msg_mon_hidden = "I hear a snarl.";
    d.aggro_sfx_mon_seen = SfxId::dog_snarl;
    d.aggro_sfx_mon_hidden = SfxId::dog_snarl;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_canine = true;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Zuul the Gatekeeper";
    d.name_the = "Zuul the Gatekeeper";
    d.corpse_name_a = "The corpse of Zuul the Gatekeeper";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::zuul;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'D';
    d.color = clr_magenta;
    d.tile = TileId::hound;
    d.can_leave_corpse = false;
    d.is_auto_descr_allowed = false;
    d.hp = 65;
    d.spi = 40;
    d.dmg_melee = min_dmg_to_wound + 3;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 25);
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 13;
    d.spawn_max_dlvl = 999;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.nr_turns_aware = 50;
    d.descr =
        "Zuul the Gatekeeper of Gozer is a demigod and minion of Gozer. It "
        "was worshiped by the Sumerians and Hittites in 6000 BC, along "
        "with Gozer.";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_seen = d.name_the + " snarls.";
    d.aggro_msg_mon_hidden = "I hear a snarl.";
    d.aggro_sfx_mon_seen = SfxId::dog_snarl;
    d.aggro_sfx_mon_hidden = SfxId::dog_snarl;
    d.erratic_move_pct = ActorErraticFreq::never;
    d.mon_shock_lvl = ShockLvl::mind_shattering;
    d.is_canine = true;
    d.can_be_summoned = false;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 1;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::human);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Ghost";
    d.name_the = "The Ghost";
    d.id = ActorId::ghost;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'G';
    d.color = clr_white_high;
    d.tile = TileId::ghost;
    d.hp = 4;
    d.spi = 12;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.natural_props[(size_t)PropId::ethereal] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.natural_props[(size_t)PropId::invis] = true;
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.descr = "A restless spirit.";
    d.wary_msg = d.name_the + " looks disturbed.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_undead = true;
    d.is_ghost = true;
    d.is_infra_visible = false;
    d.can_be_summoned = false;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::crypt);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Phantasm";
    d.name_the = "The Phantasm";
    d.id = ActorId::phantasm;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'G';
    d.color = clr_gray;
    d.tile = TileId::phantasm;
    d.hp = 9;
    d.spi = 12;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.natural_props[(size_t)PropId::ethereal] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.natural_props[(size_t)PropId::invis] = true;
    d.spawn_min_dlvl = dlvl_first_mid_game;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.descr =
        "It exists between the land of the dead and the living. It "
        "resembles a grim reaper, including the cloak, scythe, and "
        "skeletal appearance.";
    d.wary_msg = d.name_the + " looks disturbed.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_undead = true;
    d.is_ghost = true;
    d.is_infra_visible = false;
    d.can_be_summoned = false;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::crypt);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Wraith";
    d.name_the = "The Wraith";
    d.id = ActorId::wraith;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.spell_cooldown_turns = 4;
    d.spell_skill = 50;
    d.glyph = 'G';
    d.color = clr_red;
    d.tile = TileId::wraith;
    d.hp = 15;
    d.spi = 30;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.natural_props[(size_t)PropId::ethereal] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.natural_props[(size_t)PropId::invis] = true;
    d.spawn_min_dlvl = dlvl_first_late_game;
    d.spawn_max_dlvl = 999;
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.descr = "A powerful spirit.";
    d.wary_msg = d.name_the + " looks disturbed.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_undead = true;
    d.is_ghost = true;
    d.is_infra_visible = false;
    d.can_be_summoned = false;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::crypt);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Void Traveler";
    d.name_the = "The Void Traveler";
    d.id = ActorId::void_traveler;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'Y';
    d.color = clr_white_high;
    d.tile = TileId::void_traveler;
    d.hp = 50;
    d.spi = 30;
    d.dmg_melee = min_dmg_to_wound + 4;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.spawn_min_dlvl = dlvl_first_mid_game + 2;
    d.spawn_max_dlvl = 999;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 12;
    d.descr =
        "A nightmarish creature of humanoid shape. It has a hideous gangly "
        "physique and sickly pale skin. Its arms are as long as its body is "
        "tall, and it attacks with ripping claws. It has the ability to "
        "appear anywhere it desires at any time.";
    d.wary_msg = d.name_the + " looks wary.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.can_bash_doors = false;
    d.can_open_doors = false;
    d.is_infra_visible = false;
    d.can_be_summoned = false;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::plain);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Rat";
    d.name_the = "The Rat";
    d.corpse_name_a = "A Rat corpse";
    d.corpse_name_the = "The Rat corpse";
    d.id = ActorId::rat;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'r';
    d.color = clr_gray;
    d.tile = TileId::rat;
    d.hp = 2;
    d.spi = 1;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 30);
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    5),
        MonGroupSpawnRule(MonGroupSize::pack,   10),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 5;
    d.descr = "An large aggressive rodent.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.is_rat = true;
    d.is_auto_spawn_allowed = false;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Rat-thing";
    d.name_the = "The Rat-thing";
    d.corpse_name_a = "A Rat-thing corpse";
    d.corpse_name_the = "The Rat-thing corpse";
    d.id = ActorId::rat_thing;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'r';
    d.color = clr_brown_drk;
    d.tile = TileId::rat_thing;
    d.hp = 4;
    d.spi = 8;
    d.dmg_melee = 3;
    d.ability_vals.set_val(AbilityId::melee, 40);
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = dlvl_last_early_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    20),
        MonGroupSpawnRule(MonGroupSize::pack,   5),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 5;
    d.descr =
        "At first sight, a Rat-thing can easily be mistaken for a large, "
        "dark rat. On closer examination, however, their human hands and "
        "evil caricatures of human heads reveal their unnatural nature. "
        "Rat-things are said to be created from dead cultists by foul "
        "witchcraft. Thus, they are found where the witch who created them "
        "had her lair.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.is_rat = true;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::human);
    d.native_rooms.push_back(RoomType::crypt);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Wolf";
    d.name_the = "The Wolf";
    d.corpse_name_a = "A Wolf carcass";
    d.corpse_name_the = "The Wolf carcass";
    d.id = ActorId::wolf;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'd';
    d.color = clr_gray;
    d.tile = TileId::wolf;
    d.hp = 4;
    d.spi = 2;
    d.dmg_melee = 3;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 0;
    d.spawn_max_dlvl = 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   5)
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 5;
    d.descr = "A large wolf with eyes full of cunning.";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_seen = d.name_the + " snarls.";
    d.aggro_msg_mon_hidden = "I hear a chilling howl.";
    d.aggro_sfx_mon_seen = SfxId::dog_snarl;
    d.aggro_sfx_mon_hidden = SfxId::wolf_howl;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.is_canine = true;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Raven";
    d.name_the = "The Raven";
    d.corpse_name_a = "A Raven carcass";
    d.corpse_name_the = "The Raven carcass";
    d.id = ActorId::raven;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.glyph = 'b';
    d.color = clr_gray_drk;
    d.tile = TileId::raven;
    d.hp = 2;
    d.spi = 6;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 40);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.natural_props[(size_t)PropId::flying] = true;
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    1)
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 3;
    d.descr = "A vicious black bird.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_hidden = "I hear the flapping of wings.";
    d.aggro_sfx_mon_seen = SfxId::flapping_wings;
    d.aggro_sfx_mon_hidden = SfxId::flapping_wings;
    d.erratic_move_pct = ActorErraticFreq::very;
    d.mon_shock_lvl = ShockLvl::none;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Giant Bat";
    d.name_the = "The Giant Bat";
    d.corpse_name_a = "A Giant Bat corpse";
    d.corpse_name_the = "The Giant Bat corpse";
    d.id = ActorId::giant_bat;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.glyph = 'B';
    d.color = clr_gray;
    d.tile = TileId::bat;
    d.hp = 8;
    d.spi = 6;
    d.dmg_melee = 2;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 70);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 4;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    10),
        MonGroupSpawnRule(MonGroupSize::pack,   20),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.nr_turns_aware = 5;
    d.descr =
        "An unknown species, perhaps stemming from some hidden bowels of "
        "the earth. It is about as tall as a full-grown human.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_hidden = "I hear the flapping of great wings.";
    d.aggro_sfx_mon_seen = SfxId::flapping_wings;
    d.aggro_sfx_mon_hidden = SfxId::flapping_wings;
    d.erratic_move_pct = ActorErraticFreq::very;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Byakhee";
    d.name_the = "The Byakhee";
    d.corpse_name_a = "A Byakhee corpse";
    d.corpse_name_the = "The Byakhee corpse";
    d.id = ActorId::byakhee;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.glyph = 'B';
    d.color = clr_brown_drk;
    d.tile = TileId::byakhee;
    d.hp = 14;
    d.spi = 20;
    d.dmg_melee = min_dmg_to_wound + 3;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 6;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    40),
        MonGroupSpawnRule(MonGroupSize::pack,   5),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.nr_turns_aware = 5;
    d.descr =
        "\"There flapped rhythmically a horde of tame, trained, hybrid "
        "winged things ... not altogether crows, nor moles, nor buzzards, "
        "nor ants, nor decomposed human beings, but something I cannot and "
        "must not recall.\"-H.P. Lovecraft, \"The Festival\". A Byakhee "
        "appears as a disturbing mixture of creatures composing a humanoid "
        "form. Insect traits vie with predatory birds, digging mammals, "
        "and possibly the necrotic flesh of human victims. At home in "
        "lonely voids of interstellar space, Byakhee are a created race, "
        "though their origin is buried in the distant past.";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_hidden = "I hear the flapping of great wings.";
    d.aggro_sfx_mon_seen = SfxId::flapping_wings;
    d.aggro_sfx_mon_hidden = SfxId::flapping_wings;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Giant Mantis";
    d.name_the = "The Giant Mantis";
    d.corpse_name_a = "A Giant Mantis corpse";
    d.corpse_name_the = "The Giant Mantis corpse";
    d.id = ActorId::giant_mantis;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.glyph = 'I';
    d.color = clr_green_lgt;
    d.tile = TileId::mantis;
    d.hp = 25;
    d.spi = 20;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::stealth, 20);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 8;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.nr_turns_aware = 5;
    d.descr = "A huge predatory insect.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.is_infra_visible = false;
    d.can_be_summoned = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Giant Locust";
    d.name_the = "The Giant Locust";
    d.corpse_name_a = "A Giant Locust corpse";
    d.corpse_name_the = "The Giant Locust corpse";
    d.id = ActorId::locust;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'i';
    d.color = clr_brown;
    d.tile = TileId::locust;
    d.hp = 3;
    d.spi = 2;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 30);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 7;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::pack,   1),
        MonGroupSpawnRule(MonGroupSize::swarm,  4)
    });
    d.actor_size = ActorSize::floor;
    d.can_bash_doors = false;
    d.nr_turns_aware = 5;
    d.descr = "A huge swarming insect. They breed rapidly.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.erratic_move_pct = ActorErraticFreq::very;
    d.mon_shock_lvl = ShockLvl::none;
    d.is_infra_visible = false;
    d.can_be_summoned = false;
    d.native_rooms.push_back(RoomType::plain);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Fungus from Yuggoth";
    d.name_the = "The Fungus from Yuggoth";
    d.corpse_name_a = "A dead Fungus from Yuggoth";
    d.corpse_name_the = "The dead Fungus from Yuggoth";
    d.id = ActorId::mi_go;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.ranged_cooldown_turns = 4;
    d.spell_cooldown_turns = 3;
    d.spell_skill = 30;
    d.glyph = 'E';
    d.color = clr_brown_drk;
    d.tile = TileId::mi_go;
    d.hp = 10;
    d.spi = 20;
    d.dmg_melee = 3;
    d.ability_vals.set_val(AbilityId::melee, 45);
    d.ability_vals.set_val(AbilityId::ranged, 60);
    d.natural_props[(size_t)PropId::flying] = true;
    d.spawn_min_dlvl = 6;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  5),
        MonGroupSpawnRule(MonGroupSize::few,    10)
    });
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 16;
    d.descr =
        "Fungi are more closely related to animals than plants, so it's no "
        "wonder that on some worlds, fungal life evolved to dominate "
        "animal based intelligences. The Mi-go, as they are called, come "
        "from such a world. More like crustaceans than any other "
        "terrestrial life form, they find themselves somewhat hampered by "
        "our planet's thicker atmosphere and heavy gravity. They are a "
        "highly scientific race with great aptitude for surgery. Typical "
        "Mi-go have heads like large exposed brains, with many "
        "convolutions on the wrinkled surfaces; six to eight limbs, a "
        "single pair of vast membranous wings, and a long semi-prehensile "
        "tail trailing behind. They have visited Earth for centuries to "
        "mine certain minerals not available on their icy world, and more "
        "recently, to study its odd inhabitants. The mi-go are responsible "
        "for many \"alien abduction\" kidnappings. Physically they are not "
        "much more robust than the average human, so they rely upon their "
        "superior science to subdue any primitives who stumble upon their "
        "mines and outposts.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "speaks incantations in a droning voice.";
    d.aggro_msg_mon_seen = d.name_the + " speaks in a droning voice.";
    d.aggro_msg_mon_hidden = "I hear a droning voice.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Mi-go Commander";
    d.name_the = "The Mi-go Commander";
    d.corpse_name_a = "A dead Fungus from Yuggoth";
    d.corpse_name_the = "The dead Fungus from Yuggoth";
    d.id = ActorId::mi_go_commander;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.ranged_cooldown_turns = 4;
    d.spell_cooldown_turns = 3;
    d.spell_skill = 50;
    d.glyph = 'E';
    d.color = clr_red;
    d.tile = TileId::mi_go;
    d.hp = 24;
    d.spi = 20;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 55);
    d.ability_vals.set_val(AbilityId::ranged, 60);
    d.natural_props[(size_t)PropId::flying] = true;
    d.spawn_min_dlvl = 12;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 16;
    d.descr = data[(size_t)ActorId::mi_go].descr;
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "speaks incantations in a droning voice.";
    d.aggro_msg_mon_seen = d.name_the + " speaks in a droning voice.";
    d.aggro_msg_mon_hidden = "I hear a droning voice.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Flying Polyp";
    d.name_the = "The Flying Polyp";
    d.corpse_name_a = "A dead Polyp";
    d.corpse_name_the = "The dead Polyp";
    d.id = ActorId::flying_polyp;
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.ai[(size_t)AiId::looks] = false;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'E';
    d.color = clr_magenta;
    d.tile = TileId::polyp;
    d.hp = 12;
    d.spi = 20;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    // NOTE: Polyps should be resistant against all damage except electricity
    d.natural_props[(size_t)PropId::r_phys] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_acid] = true;
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::invis] = true;
    d.natural_props[(size_t)PropId::see_invis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = 7;
    d.spawn_max_dlvl = 999;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = false;
    d.can_open_doors = false;
    d.nr_turns_aware = 5;
    d.descr =
        "Flying polyps are a horrible elder race of half polypous, utterly "
        "alien entities. They are only partly material and have the power of "
        "aerial motion, despite the absence of wings. Their senses does not "
        "include sight, but what senses they have can penetrate all material "
        "obstructions. They are invulnerable to normal means of damage, "
        "though they can be destroyed by certain forms of electrical energy.";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_seen = d.name_the + " makes shrill whistling sounds.";
    d.aggro_msg_mon_hidden = "I hear a shrill whistling.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Greater Polyp";
    d.name_the = "The Greater Polyp";
    d.corpse_name_a = "A dead Polyp";
    d.corpse_name_the = "The dead Polyp";
    d.id = ActorId::greater_polyp;
    d.can_be_summoned = false;
    d.can_bleed = false;
    d.ai[(size_t)AiId::looks] = false;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'E';
    d.color = clr_red;
    d.tile = TileId::polyp;
    d.hp = 70;
    d.spi = 30;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 70);
    // NOTE: Polyps should be resistant against all damage except electricity
    d.natural_props[(size_t)PropId::r_phys] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_acid] = true;
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::invis] = true;
    d.natural_props[(size_t)PropId::see_invis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = dlvl_first_late_game;
    d.spawn_max_dlvl = 999;
    d.actor_size = ActorSize::giant;
    d.can_bash_doors = false;
    d.can_open_doors = false;
    d.nr_turns_aware = 5;
    d.descr = data[(size_t)ActorId::flying_polyp].descr;
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_seen = d.name_the + " makes shrill whistling sounds.";
    d.aggro_msg_mon_hidden = "I hear a shrill whistling.";
    d.erratic_move_pct = ActorErraticFreq::very;
    d.mon_shock_lvl = ShockLvl::mind_shattering;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Mind Eater";
    d.name_the = "The Mind Eater";
    d.corpse_name_a = "A Mind Eater corpse";
    d.corpse_name_the = "The Mind Eater corpse";
    d.id = ActorId::mind_eater;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'E';
    d.color = clr_orange;
    d.tile = TileId::mind_eater;
    d.hp = 40;
    d.spi = 40;
    d.dmg_melee = 2;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.spawn_min_dlvl = 6;
    d.spawn_max_dlvl = dlvl_last - 4;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = false;
    d.can_open_doors = false;
    d.nr_turns_aware = 7;
    d.descr =
        "A large tentacled alien creature, mysteriously floating through the "
        "air in a ghostly manner. They somehow sustain themselves on the "
        "brain activity of other beings. Their preferred strategy is to subdue "
        "their victim with a paralyzing sting, and once immobilized, a second "
        "appendage probes the brain. This will usually not kill the victim, "
        "but the experience is horrific beyond words.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.is_infra_visible = false;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Ghoul";
    d.name_the = "The Ghoul";
    d.corpse_name_a = "A Ghoul corpse";
    d.corpse_name_the = "The Ghoul corpse";
    d.id = ActorId::ghoul;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'M';
    d.color = clr_green;
    d.tile = TileId::ghoul;
    d.hp = 26;
    d.spi = 20;
    d.dmg_melee = min_dmg_to_wound + 2;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 5;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    20),
        MonGroupSpawnRule(MonGroupSize::pack,   10),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 7;
    d.descr =
        "\"These figures were seldom completely human, but often approached "
        "humanity in varying degrees. Most of the bodies, while roughly "
        "bipedal, had a forward slumping, and a vaguely canine cast. The "
        "texture of the majority was a kind of unpleasant rubberiness\" "
        "-H.P.Lovecraft \"Pickman's Model\". Ghouls are rubbery, loathsome "
        "humanoids with hooflike feet, canine features, and claws. They are "
        "often encrusted with grave mold collected as they feed.";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_seen = d.name_the + " growls.";
    d.aggro_msg_mon_hidden = "I hear a chilling howl.";
    d.aggro_sfx_mon_seen = SfxId::ghoul_growl;
    d.aggro_sfx_mon_hidden = SfxId::wolf_howl;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_infra_visible = false;
    d.is_humanoid = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::crypt);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Shadow";
    d.name_the = "The Shadow";
    d.id = ActorId::shadow;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'M';
    d.color = clr_gray;
    d.tile = TileId::shadow;
    d.hp = 4;
    d.spi = 12;
    d.dmg_melee = 3;
    d.ability_vals.set_val(AbilityId::melee, 45);
    d.ability_vals.set_val(AbilityId::stealth, 90);
    d.natural_props[(size_t)PropId::lgt_sens] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   2)
    });
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = false;
    d.can_open_doors = false;
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.nr_turns_aware = 7;
    d.descr = "A living shadow.";
    d.wary_msg = d.name_the + " looks wary.";
    d.is_auto_descr_allowed = false;
    d.is_infra_visible = false;
    d.is_humanoid = true;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "An Invisible Stalker";
    d.name_the = "The Invisible Stalker";
    d.id = ActorId::invis_stalker;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'M';
    d.color = clr_yellow;
    d.tile = TileId::shadow;
    d.hp = 15;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::invis] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.nr_turns_aware = 32;
    d.descr =
        "A mysterious humanoid figure stalking for prey. It is invisible to "
        "normal eyes.";
    d.wary_msg = d.name_the + " looks wary.";
    d.is_auto_descr_allowed = true;
    d.is_infra_visible = false;
    d.is_humanoid = true;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Mummy";
    d.name_the = "The Mummy";
    d.corpse_name_a = "A lifeless Mummy";
    d.corpse_name_the = "The lifeless Mummy";
    d.id = ActorId::mummy;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 50;
    d.glyph = 'P';
    d.color = clr_magenta;
    d.tile = TileId::mummy;
    d.hp = 25;
    d.spi = 30;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.spawn_min_dlvl = dlvl_first_late_game - 3;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 7;
    d.descr = "A mummified human being, possibly dating back millennia.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_undead = true;
    d.is_humanoid = true;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Crocodile Head Mummy";
    d.name_the = "The Crocodile Head Mummy";
    d.corpse_name_a = "A lifeless Mummy";
    d.corpse_name_the = "The lifeless Mummy";
    d.id = ActorId::croc_head_mummy;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'P';
    d.color = clr_gray;
    d.tile = TileId::croc_head_mummy;
    d.hp = 25;
    d.spi = 30;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.spawn_min_dlvl = dlvl_first_late_game - 3;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::pack,   1),
        MonGroupSpawnRule(MonGroupSize::swarm,  2)
    });
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 7;
    d.descr =
        "A grotesque mummified hybrid creature, with the body of a human and "
        "the head of a crocodile. It is wielding a spear.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "casts a spell.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_undead = true;
    d.is_humanoid = true;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Khephren";
    d.name_the = "Khephren";
    d.corpse_name_a = "The lifeless Mummy of Khephren";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::khephren;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 80;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 0;
    d.glyph = 'P';
    d.color = clr_red;
    d.tile = TileId::mummy;
    d.hp = 70;
    d.spi = 60;
    d.dmg_melee = min_dmg_to_wound + 2;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.is_auto_spawn_allowed = false;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 9999;
    d.descr =
        "The mummified fourth dynasty Egyptian pharaoh Khephren. How he came "
        "to dwell here is beyond anyones guess.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_undead = true;
    d.is_humanoid = true;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::plain);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Nitokris";
    d.name_the = "Nitokris";
    d.corpse_name_a = "The lifeless Mummy of Nitokris";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::nitokris;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 80;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 1;
    d.glyph = 'P';
    d.color = clr_red_lgt;
    d.tile = TileId::mummy;
    d.hp = 70;
    d.spi = 60;
    d.dmg_melee = min_dmg_to_wound + 2;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.spawn_min_dlvl = 11;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 9999;
    d.descr =
        "The mummified sixth dynasty Egyptian pharaoh Nitokris. How she "
        "came to dwell here is beyond anyones guess. Her name is found in the "
        "histories of Herodotus and writings of Manetho. According to the "
        "writings of the former, she took the throne by inviting her "
        "brothers murderers to a banquet. Then killed them by flooding the "
        "sealed room with the Nile.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_undead = true;
    d.is_humanoid = true;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Deep One";
    d.name_the = "The Deep One";
    d.corpse_name_a = "A corpse of a Deep One";
    d.corpse_name_the = "The corpse of a Deep One";
    d.id = ActorId::deep_one;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.ranged_cooldown_turns = 15;
    d.glyph = 'F';
    d.color = clr_white;
    d.tile = TileId::deep_one;
    d.hp = 18;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound + 4;
    d.dmg_ranged = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::ranged, 50);
    d.spawn_min_dlvl = 6;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::pack,   6),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.is_amphibian = true;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 7;
    d.descr =
        "Deep ones are misbegotten creatures of the deep. A deep one "
        "appears as an abominable crossbreed of a human and amphibian. Its "
        "fins are merged with twisted arms and legs; its bent back is "
        "crowned with a long, spiny frill. They can breathe both air and "
        "water. In the timeless depths of the sea, the deep one's alien, "
        "arrogant lives are coldly beautiful, unbelievably cruel, and "
        "effectively immortal. Deep ones may be worshipped by humans with "
        "whom they regularly interbreed.";
    d.wary_msg = d.name_the + " looks wary.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Carnivorous Ape";
    d.name_the = "The Carnivorous Ape";
    d.corpse_name_a = "An Ape corpse";
    d.corpse_name_the = "The Ape corpse";
    d.id = ActorId::ape;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'M';
    d.color = clr_white;
    d.tile = TileId::ape;
    d.hp = 18;
    d.spi = 6;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = dlvl_first_late_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   1)
    });
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_bash_doors = true;
    d.can_open_doors = false;
    d.nr_turns_aware = 7;
    d.descr =
        "An extremely aggressive species of great ape. Their pale skin and "
        "excellent night vision indicates that they adapted to subterranean "
        "life a very long time ago.";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_hidden = "I hear a beast howling.";
    d.aggro_msg_mon_seen = d.name_the + " howls.";
    d.aggro_sfx_mon_seen = SfxId::ape;
    d.aggro_sfx_mon_hidden = SfxId::ape;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::monster);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Worm Mass";
    d.name_the = "The Worm Mass";
    d.id = ActorId::worm_mass;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'w';
    d.color = clr_white;
    d.tile = TileId::mass_of_worms;
    d.hp = 2;
    d.spi = 1;
    d.dmg_melee = 2;
    d.ability_vals.set_val(AbilityId::melee, 35);
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    2),
        MonGroupSpawnRule(MonGroupSize::pack,   8),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::floor;
    d.prevent_knockback = true;
    d.nr_turns_aware = 5;
    d.descr = "A slithering conglomeration of carnivorous worms.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.can_be_summoned = true;
    d.is_auto_descr_allowed = true;
    d.erratic_move_pct = ActorErraticFreq::very;
    d.can_bleed = true;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::forest);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Mind Worms";
    d.name_the = "The Mind Worms";
    d.id = ActorId::mind_worms;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'w';
    d.color = clr_violet;
    d.tile = TileId::mass_of_worms;
    d.hp = 4;
    d.spi = 2;
    d.dmg_melee = 3;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 6;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    2),
        MonGroupSpawnRule(MonGroupSize::pack,   8),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::floor;
    d.prevent_knockback = true;
    d.nr_turns_aware = 5;
    d.descr =
        "A slithering conglomeration of carnivorous worms. Their bite causes "
        "heavy disorientation.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.can_be_summoned = true;
    d.is_auto_descr_allowed = true;
    d.erratic_move_pct = ActorErraticFreq::very;
    d.can_bleed = true;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::forest);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Dust Vortex";
    d.name_the = "The Dust Vortex";
    d.id = ActorId::dust_vortex;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'V';
    d.color = clr_gray;
    d.tile = TileId::vortex;
    d.hp = 10;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few, 1)
    });
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.is_auto_descr_allowed = true;
    d.descr =
        "A spinning force of destruction. It seems to move with some "
        "manner of purpose.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_hidden = "I hear a howling wind.";
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.is_infra_visible = false;
    d.erratic_move_pct = ActorErraticFreq::very;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Fire Vortex";
    d.name_the = "The Fire Vortex";
    d.id = ActorId::fire_vortex;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'V';
    d.color = clr_red;
    d.tile = TileId::vortex;
    d.hp = 16;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = 8;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few, 2)
    });
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.is_auto_descr_allowed = true;
    d.descr =
        "A spinning force of fire and destruction. It seems to move with "
        "some manner of purpose.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_hidden = "I hear a howling wind.";
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.erratic_move_pct = ActorErraticFreq::very;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Black Ooze";
    d.name_the = "The Black Ooze";
    d.id = ActorId::ooze_black;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::sluggish;
    d.glyph = 'o';
    d.color = clr_gray_drk;
    d.tile = TileId::ooze;
    d.hp = 20;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::ooze] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   1),
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 32;
    d.is_auto_descr_allowed = true;
    d.can_open_doors = false;
    d.can_bash_doors = false;
    d.descr = "It's just a mass of gunk, but it seems sentient.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_seen = d.name_the + " makes a gurgling noise.";
    d.aggro_msg_mon_hidden = "I hear a gurgling noise.";
    d.aggro_sfx_mon_seen = SfxId::ooze_gurgle;
    d.aggro_sfx_mon_hidden = SfxId::ooze_gurgle;
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.is_infra_visible = false;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Clear Ooze";
    d.name_the = "The Clear Ooze";
    d.id = ActorId::ooze_clear;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::sluggish;
    d.glyph = 'o';
    d.color = clr_white_high;
    d.tile = TileId::ooze;
    d.hp = 12;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 2;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::stealth, 90);
    d.natural_props[(size_t)PropId::ooze] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   1)
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 32;
    d.is_auto_descr_allowed = true;
    d.can_open_doors = false;
    d.can_bash_doors = false;
    d.descr = "It's a clear mass of gunk. They can be hard to spot.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_seen = d.name_the + " makes a gurgling noise.";
    d.aggro_msg_mon_hidden = "I hear a gurgling noise.";
    d.aggro_sfx_mon_seen = SfxId::ooze_gurgle;
    d.aggro_sfx_mon_hidden = SfxId::ooze_gurgle;
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.is_infra_visible = false;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Putrid Ooze";
    d.name_the = "The Putrid Ooze";
    d.id = ActorId::ooze_putrid;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::sluggish;
    d.glyph = 'o';
    d.color = clr_green;
    d.tile = TileId::ooze;
    d.hp = 20;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::ooze] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 5;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   1),
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 32;
    d.is_auto_descr_allowed = true;
    d.can_open_doors = false;
    d.can_bash_doors = false;
    d.descr =
        "It's just a mass of gunk, but it seems sentient. It stinks like "
        "rotting cadavers.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_seen = d.name_the + " makes a gurgling noise.";
    d.aggro_msg_mon_hidden = "I hear a gurgling noise.";
    d.aggro_sfx_mon_seen = SfxId::ooze_gurgle;
    d.aggro_sfx_mon_hidden = SfxId::ooze_gurgle;
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.is_infra_visible = false;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Poison Ooze";
    d.name_the = "The Poison Ooze";
    d.id = ActorId::ooze_poison;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::sluggish;
    d.glyph = 'o';
    d.color = clr_green_lgt;
    d.tile = TileId::ooze;
    d.hp = 20;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::ooze] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 9;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   1),
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 32;
    d.is_auto_descr_allowed = true;
    d.can_open_doors = false;
    d.can_bash_doors = false;
    d.descr =
        "It's just a mass of gunk, but it seems sentient. It drips and "
        "sizzles with poison.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_seen = d.name_the + " makes a gurgling noise.";
    d.aggro_msg_mon_hidden = "I hear a gurgling noise.";
    d.aggro_sfx_mon_seen = SfxId::ooze_gurgle;
    d.aggro_sfx_mon_hidden = SfxId::ooze_gurgle;
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.is_infra_visible = false;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Strange Color";
    d.name_the = "The Strange Color";
    d.id = ActorId::strange_color;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'O';
    d.color = clr_green_lgt;
    d.tile = TileId::ooze;
    d.hp = 35;
    d.spi = 40;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::ooze] = true;
    d.natural_props[(size_t)PropId::ethereal] = true;
    d.natural_props[(size_t)PropId::r_phys] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 14;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 16;
    d.can_open_doors = false;
    d.can_bash_doors = false;
    d.descr =
        "A peculiar floating speck of strange shifting colors. It is very "
        "confusing to look at. Wherever they pass, they corrupt the "
        "environment around them. They are invulnerable to attacks from "
        "common materials like iron or steel, but can be destroyed by other "
        "forms of damage, such as fire or electricity.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.is_auto_descr_allowed = true;
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.is_infra_visible = false;
    d.erratic_move_pct = ActorErraticFreq::very;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Chthonian";
    d.name_the = "The Chthonian";
    d.corpse_name_a = "A Chthonian corpse";
    d.corpse_name_the = "The Chthonian corpse";
    d.id = ActorId::chthonian;
    d.ai[(size_t)AiId::looks] = false;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::sluggish;
    d.glyph = 'W';
    d.color = clr_brown_drk;
    d.tile = TileId::chthonian;
    d.hp = 200;
    d.spi = 40;
    d.dmg_melee = min_dmg_to_wound + 10;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::burrowing] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = 12;
    d.actor_size = ActorSize::giant;
    d.nr_turns_aware = 5;
    d.is_auto_descr_allowed = true;
    d.can_open_doors = false;
    d.can_bash_doors = false;
    d.descr =
        "It resembles an immense squid, with reaching, groping tentacles, "
        "and a pulpy, elongated sack of a body. Chthonians are powerful "
        "burrowers, and live for more than a thousand years.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_seen = "";
    d.aggro_msg_mon_hidden = "";
    d.aggro_sfx_mon_seen = SfxId::END;
    d.aggro_sfx_mon_hidden = SfxId::END;
    d.can_be_summoned = false;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Floating Skull";
    d.name_the = "The Floating Skull";
    d.corpse_name_a = "A Skull";
    d.corpse_name_the = "The Skull";
    d.id = ActorId::floating_skull;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'z';
    d.color = clr_white_high;
    d.tile = TileId::floating_skull;
    d.hp = 6;
    d.spi = 16;
    d.dmg_melee = min_dmg_to_wound - 2;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 60);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 10;
    d.descr = "A severed head floating through the air.";
    d.wary_msg = d.name_the + " looks wary.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.can_be_summoned = true;
    d.is_undead = true;
    d.is_auto_descr_allowed = false;
    d.can_bleed = false;
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::chasm);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.id = ActorId::animated_wpn;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.hp = 18;
    d.spi = 40;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::r_elec] = true;
    d.natural_props[(size_t)PropId::r_blind] = true;
    d.actor_size = ActorSize::humanoid;
    d.is_auto_descr_allowed = false;
    d.can_leave_corpse = false;
    d.can_bash_doors = true;
    d.can_open_doors = false;
    d.nr_turns_aware = 16;
    d.aggro_msg_mon_seen = "";
    d.aggro_msg_mon_hidden = "";
    d.is_infra_visible = false;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::unsettling;
    data[(size_t)d.id] = d;
    d.reset();

    d.id = ActorId::mold;
    d.name_a = "Mold";
    d.name_the = "The Mold";
    d.corpse_name_a = "";
    d.corpse_name_the = "";
    d.tile = TileId::fungi;
    d.glyph = 'e';
    d.color = clr_green_lgt;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few, 1)
    });
    d.hp = 1;
    d.spi = 1;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.speed_pct = (int)ActorSpeed::sluggish;
    d.ai[(size_t)AiId::looks] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::moves_to_random_when_unaware] = false;
    d.prevent_knockback = true;
    d.nr_turns_aware = 5;
    d.spawn_min_dlvl = 3;
    d.actor_size = ActorSize::floor;
    d.is_auto_descr_allowed = false;
    d.erratic_move_pct = ActorErraticFreq::never;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.descr =
        "A sickly growth thriving in damp areas. It produces toxic spores "
        "which it stores in its slimy heads.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_seen = "";
    d.aggro_msg_mon_hidden = "";
    d.aggro_sfx_mon_seen = SfxId::END;
    d.aggro_sfx_mon_hidden = SfxId::END;
    d.is_infra_visible = false;
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Gas Spore";
    d.name_the = "The Gas Spore";
    d.id = ActorId::gas_spore;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::slow;
    d.glyph = 'E';
    d.color = clr_red;
    d.tile = TileId::gas_spore;
    d.hp = 1;
    d.spi = 1;
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.spawn_min_dlvl = 3;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   4)
    });
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.is_auto_descr_allowed = true;
    d.can_open_doors = false;
    d.can_bash_doors = false;
    d.descr =
        "A vaguely intelligent floating fungus-based organism. It contains a "
        "highly explosive gas and will detonate very easily, which it seems to "
        "desire.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_hidden = "";
    d.can_be_summoned = false;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.is_infra_visible = false;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::none;
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Hunting Horror";
    d.name_the = "The Hunting Horror";
    d.corpse_name_a = "A Hunting Horror corpse";
    d.corpse_name_the = "The Hunting Horror corpse";
    d.id = ActorId::hunting_horror;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.glyph = 'W';
    d.color = clr_gray_drk;
    d.tile = TileId::hunting_horror;
    d.hp = 90;
    d.spi = 40;
    d.dmg_melee = min_dmg_to_wound + 7;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = dlvl_last - 6;
    d.spawn_max_dlvl = 999;
    d.actor_size = ActorSize::giant;
    d.nr_turns_aware = 5;
    d.is_auto_descr_allowed = true;
    d.can_open_doors = false;
    d.can_bash_doors = true;
    d.descr =
        "It resembles an enormous black ropy worm, like a legless dragon "
        "seen in a nightmare. A great gaping maw filled with jagged, "
        "irregular teeth gapes open hungrily as it flies in search of "
        "prey. Hunting horrors haunt the dark places of the universe, from "
        "which they are called up at the whim of their lord Nyarlathotep.";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_msg_mon_hidden = "I hear the flapping of great wings.";
    d.aggro_sfx_mon_seen = SfxId::flapping_wings;
    d.aggro_sfx_mon_hidden = SfxId::flapping_wings;
    d.can_bleed = true;
    d.can_be_summoned = false;
    d.mon_shock_lvl = ShockLvl::mind_shattering;
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "A Death Fiend";
    d.name_the = "The Death Fiend";
    d.corpse_name_a = "";
    d.corpse_name_the = "";
    d.id = ActorId::death_fiend;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow;
    d.spell_cooldown_turns = 3;
    d.spell_skill = 70;
    d.glyph = 'Y';
    d.color = clr_red;
    d.tile = TileId::fiend;
    d.hp = 90;
    d.spi = 40;
    d.dmg_melee = 200;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::infravis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = dlvl_last - 6;
    d.spawn_max_dlvl = 999;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.is_auto_descr_allowed = true;
    d.can_open_doors = false;
    d.can_bash_doors = true;
    d.descr =
        "A living incarnation of death. With its goat-like features, it is "
        "eerily similar to the imagery of Devils and Demons seen in medieval "
        "woodcuts. The attack of this entity can bypass any armor, and means "
        "almost certain death!";
    d.wary_msg = d.name_the + " looks wary.";
    d.aggro_sfx_mon_seen = SfxId::END;
    d.aggro_sfx_mon_hidden = SfxId::END;
    d.can_be_summoned = false;
    d.is_infra_visible = false;
    d.is_humanoid = true;
    d.can_leave_corpse = false;
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.mon_shock_lvl = ShockLvl::mind_shattering;
    d.erratic_move_pct = ActorErraticFreq::never;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "The High Priest";
    d.name_the = d.name_a;
    d.id = ActorId::the_high_priest;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.spell_cooldown_turns = 2;
    d.spell_skill = 100;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 0;
    d.glyph = 'P';
    d.color = clr_red;
    d.tile = TileId::the_high_priest;
    d.hp = 160;
    d.spi = 99999;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.is_auto_spawn_allowed = false;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.prevent_knockback = true;
    d.nr_turns_aware = 9999;
    d.descr =
        "The Supreme Wizard of the Cult of Starry Wisdom. He appears "
        "incredibly old, like a hideous ancient vampire or a grim reaper. "
        "Tattered crimson robes flow from his skeletal frame, his bony "
        "fingers extends into razor sharp claws, and he has a grim expression "
        "on his face.";
    d.wary_msg = d.name_the + " looks wary.";
    d.is_auto_descr_allowed = false;
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.erratic_move_pct = ActorErraticFreq::never;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_humanoid = true;
    d.can_leave_corpse = false;
    d.can_bleed = true;
    data[(size_t)d.id] = d;
    d.reset();
}

} // namespace

void init()
{
    TRACE_FUNC_BEGIN;
    init_data_list();
    TRACE_FUNC_END;
}

void save()
{
    for (int i = 0; i < (int)ActorId::END; ++i)
    {
        const auto& d = data[i];

        saving::put_int(d.nr_left_allowed_to_spawn);
        saving::put_int(d.nr_kills);
        saving::put_bool(d.has_player_seen);
    }
}

void load()
{
    for (int i = 0; i < (int)ActorId::END; ++i)
    {
        auto& d = data[i];

        d.nr_left_allowed_to_spawn = saving::get_int();
        d.nr_kills = saving::get_int();
        d.has_player_seen = saving::get_bool();
    }
}

} // actor_data
