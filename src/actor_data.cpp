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

    ability_vals.reset();

    spell_skill = SpellSkill::basic;

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
    d.color = clr_white_lgt;
    d.tile = TileId::player_firearm;
    d.is_auto_spawn_allowed = false;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Reanimated Corpse";
    d.name_the = "the Reanimated Corpse";
    d.corpse_name_a = "a lifeless gruesome corpse";
    d.corpse_name_the = "the lifeless gruesome corpse";
    d.id = ActorId::zombie;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'Z';
    d.color = clr_brown;
    d.tile = TileId::zombie_unarmed;
    d.hp = 14;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 10);
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  20),
        MonGroupSpawnRule(MonGroupSize::few,    70),
        MonGroupSpawnRule(MonGroupSize::pack,   10),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::humanoid;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 7;
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

    d.name_a = "a Reanimated Corpse";
    d.name_the = "the Reanimated Corpse";
    d.corpse_name_a = "a lifeless gruesome corpse";
    d.corpse_name_the = "the lifeless gruesome corpse";
    d.id = ActorId::zombie_axe;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'Z';
    d.color = clr_gray;
    d.tile = TileId::zombie_armed;
    d.hp = 14;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 10);
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  20),
        MonGroupSpawnRule(MonGroupSize::few,    70),
        MonGroupSpawnRule(MonGroupSize::pack,   10),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
    });
    d.actor_size = ActorSize::humanoid;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 7;
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

    d.name_a = "a Bloated Corpse";
    d.name_the = "the Bloated Corpse";
    d.corpse_name_a = "a huge gruesome corpse";
    d.corpse_name_the = "the huge gruesome corpse";
    d.id = ActorId::bloated_zombie;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow - 20;
    d.ranged_cooldown_turns = 20;
    d.glyph = 'Z';
    d.color = clr_white_lgt;
    d.tile = TileId::zombie_bloated;
    d.hp = 30;
    d.dmg_melee = min_dmg_to_wound + 3;
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
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.corpse_name_a = "the corpse of Major Clapham-Lee";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::major_clapham_lee;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.glyph = 'Z';
    d.color = clr_cyan_lgt;
    d.tile = TileId::zombie_unarmed;
    d.hp = 26;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 10);
    d.spawn_min_dlvl = 4;
    d.actor_size = ActorSize::humanoid;
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.corpse_name_a = "the corpse of Dean Halsey";
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
    d.ability_vals.set_val(AbilityId::dodging, 10);
    d.is_auto_spawn_allowed = false;
    d.spawn_min_dlvl = 4;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.name_the = "the Crawling Intestines";
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
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 2;
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

    d.name_a = "a Crawling Hand";
    d.name_the = "the Crawling Hand";
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
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 2;
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
    d.spell_skill = SpellSkill::basic;
    d.glyph = 'z';
    d.color = clr_magenta;
    d.tile = TileId::crawling_hand;
    d.hp = 28;
    d.spi = 25;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 60);
    d.ability_vals.set_val(AbilityId::stealth, 90);
    d.nr_left_allowed_to_spawn = 1;
    d.is_unique = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 8;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 1;
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

    d.name_a = "a Floating Skull";
    d.name_the = "the Floating Skull";
    d.corpse_name_a = "a Skull";
    d.corpse_name_the = "the Skull";
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
    d.color = clr_white_lgt;
    d.tile = TileId::floating_skull;
    d.hp = 6;
    d.spi = 16;
    d.dmg_melee = min_dmg_to_wound - 2;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = dlvl_last_early_game;
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

    d.name_a = "a Cultist";
    d.name_the = "the Cultist";
    d.corpse_name_a = "a Cultist corpse";
    d.corpse_name_the = "the Cultist corpse";
    d.id = ActorId::cultist;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.ranged_cooldown_turns = 1;
    d.spell_skill = SpellSkill::basic;
    d.glyph = 'P';
    d.color = clr_gray;
    d.tile = TileId::cultist_firearm;
    d.hp = 4;
    d.spi = 12;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::ranged, 65);
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_open_doors = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 16;
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

    d.name_a = "a Bog Tcher";
    d.name_the = "the Bog Tcher";
    d.corpse_name_a = "a Bog Tcher corpse";
    d.corpse_name_the = "the Bog Tcher corpse";
    d.id = ActorId::bog_tcher;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 10;
    d.ranged_cooldown_turns = 1;
    d.spell_skill = SpellSkill::basic;
    d.glyph = 'p';
    d.color = clr_brown_drk;
    d.tile = TileId::bog_tcher;
    d.hp = 6;
    d.spi = 12;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::ranged, 60);
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
    d.corpse_name_a = "the corpse of Keziah Mason";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::keziah_mason;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_skill = SpellSkill::expert;
    d.glyph = 'P';
    d.color = clr_magenta;
    d.tile = TileId::witch_or_warlock;
    d.hp = 22;
    d.spi = 60;
    d.ability_vals.set_val(AbilityId::melee, 40);
    d.ability_vals.set_val(AbilityId::ranged, 40);
    d.nr_left_allowed_to_spawn = 1;
    d.is_unique = true;
    d.spawn_min_dlvl = 3;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_open_doors = true;
    d.can_bash_doors = true;
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
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::ritual);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Brown Jenkin";
    d.name_the = "Brown Jenkin";
    d.corpse_name_a = "the corpse of Brown Jenkin";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::brown_jenkin;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.spell_skill = SpellSkill::basic;
    d.glyph = 'r';
    d.color = clr_magenta;
    d.tile = TileId::rat_thing;
    d.hp = 10;
    d.spi = 25;
    d.dmg_melee = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::stealth, 90);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.is_auto_spawn_allowed = false;
    d.nr_left_allowed_to_spawn = 0;
    d.is_unique = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.name_the = "the " + d.name_a;
    d.corpse_name_a = "the corpse of The Elder Hierophant";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::leng_elder;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::slow;
    d.spell_skill = SpellSkill::master;
    d.glyph = 'P';
    d.color = clr_yellow;
    d.tile = TileId::leng_elder;
    d.hp = 200;
    d.spi = 200;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::ranged, 40);
    d.is_auto_spawn_allowed = false;
    d.is_unique = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
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

    d.name_a = "a Cultist Priest";
    d.name_the = "the Cultist Priest";
    d.corpse_name_a = "a Cultist Priest corpse";
    d.corpse_name_the = "the Cultist Priest corpse";
    d.id = ActorId::cultist_priest;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_skill = SpellSkill::basic;
    d.glyph = 'P';
    d.color = clr_cyan_lgt;
    d.tile = TileId::cultist_dagger;
    d.hp = 6;
    d.spi = 30;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.spawn_min_dlvl = 3;
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

    d.name_a = "a Wizard";
    d.name_the = "the Wizard";
    d.corpse_name_a = "a Wizard corpse";
    d.corpse_name_the = "the Wizard corpse";
    d.id = ActorId::cultist_wizard;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_skill = SpellSkill::expert;
    d.glyph = 'P';
    d.color = clr_yellow;
    d.tile = TileId::witch_or_warlock;
    d.hp = 22;
    d.spi = 40;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.spawn_min_dlvl = dlvl_first_mid_game;
    d.spawn_max_dlvl = (dlvl_last_mid_game + dlvl_last) / 2;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_open_doors = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 14;
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

    d.name_a = "an Arch Wizard";
    d.name_the = "the Arch Wizard";
    d.corpse_name_a = "an Arch Wizard corpse";
    d.corpse_name_the = "the Arch Wizard corpse";
    d.id = ActorId::cultist_arch_wizard;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_skill = SpellSkill::master;
    d.glyph = 'P';
    d.color = clr_blue_lgt;
    d.tile = TileId::witch_or_warlock;
    d.hp = 40;
    d.spi = 50;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.spawn_min_dlvl = dlvl_first_late_game;
    d.spawn_max_dlvl = dlvl_last - 1;
    d.actor_size = ActorSize::humanoid;
    d.is_humanoid = true;
    d.can_open_doors = true;
    d.can_bash_doors = true;
    d.nr_turns_aware = 16;
    d.descr =
        "A fanatic cultist of the Arch Wizard rank, madly gibbering in some "
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

    d.name_a = "a Huge Green Spider";
    d.name_the = "the Huge Green Spider";
    d.corpse_name_a = "a Green Spider corpse";
    d.corpse_name_the = "the Green Spider corpse";
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
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.spawn_min_dlvl = dlvl_first_mid_game - 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  3),
        MonGroupSpawnRule(MonGroupSize::few,    1),
    });
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.descr = "An enormous green spider.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.can_be_summoned = true;
    d.is_spider = true;
    d.native_rooms.push_back(RoomType::spider);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Huge White Spider";
    d.name_the = "the Huge White Spider";
    d.corpse_name_a = "a White Spider corpse";
    d.corpse_name_the = "the White Spider corpse";
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
    d.color = clr_white_lgt;
    d.tile = TileId::giant_spider;
    d.hp = 16;
    d.dmg_melee = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.spawn_min_dlvl = dlvl_first_mid_game - 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  3),
        MonGroupSpawnRule(MonGroupSize::few,    1),
    });
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.nr_turns_aware = 5;
    d.actor_size = ActorSize::humanoid;
    d.descr = "An enormous white spider.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.can_be_summoned = true;
    d.is_spider = true;
    d.native_rooms.push_back(RoomType::spider);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Huge Red Spider";
    d.name_the = "the Huge Red Spider";
    d.corpse_name_a = "a Red Spider corpse";
    d.corpse_name_the = "the Red Spider corpse";
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
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.spawn_min_dlvl = dlvl_first_mid_game - 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  3),
        MonGroupSpawnRule(MonGroupSize::few,    1),
    });
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.nr_turns_aware = 5;
    d.actor_size = ActorSize::humanoid;
    d.descr = "An enormous red spider.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.can_be_summoned = true;
    d.is_spider = true;
    d.native_rooms.push_back(RoomType::spider);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Shadow Spider";
    d.name_the = "the Shadow Spider";
    d.id = ActorId::shadow_spider;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'a';
    d.color = clr_gray;
    d.tile = TileId::giant_spider;
    d.hp = 8;
    d.dmg_melee = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::stealth, 95);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.natural_props[(size_t)PropId::lgt_sens] = true;
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    4),
        MonGroupSpawnRule(MonGroupSize::pack,   8),
        MonGroupSpawnRule(MonGroupSize::swarm , 1)
    });
    d.nr_turns_aware = 5;
    d.actor_size = ActorSize::humanoid;
    d.is_spider = true;
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

    d.name_a = "a Leng Spider";
    d.name_the = "the Leng Spider";
    d.corpse_name_a = "a Leng Spider corpse";
    d.corpse_name_the = "the Leng Spider corpse";
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
    d.hp = 32;
    d.dmg_melee = min_dmg_to_wound + 3;
//  d.dmg_ranged = 1;
    d.ability_vals.set_val(AbilityId::melee, 80);
//  d.ability_vals.set_val(AbilityId::ranged, 40);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.spawn_min_dlvl = dlvl_first_mid_game;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    4),
        MonGroupSpawnRule(MonGroupSize::pack,   1)
    });
    d.nr_turns_aware = 16;
    d.actor_size = ActorSize::giant;
    d.descr =
        "Leng spiders are huge, purplish arachnids, with pustulent bloated "
        "bodies and long, bristly legs. Native to the Dreamlands, the "
        "spiders of Leng are intelligent, dangerous, and gigantic.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.is_spider = true;
    d.can_be_summoned = true;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::spider);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Pit Viper";
    d.name_the = "the Pit Viper";
    d.corpse_name_a = "a Pit Viper corpse";
    d.corpse_name_the = "the Pit Viper corpse";
    d.id = ActorId::pit_viper;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.glyph = 'S';
    d.color = clr_green_lgt;
    d.tile = TileId::snake;
    d.hp = 6;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = dlvl_last_early_game;
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.is_snake = true;
    d.is_reptile = true;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    d.native_rooms.push_back(RoomType::ritual);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Spitting Cobra";
    d.name_the = "the Spitting Cobra";
    d.corpse_name_a = "a Spitting Cobra corpse";
    d.corpse_name_the = "the Spitting Cobra corpse";
    d.id = ActorId::spitting_cobra;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.glyph = 'S';
    d.color = clr_red;
    d.tile = TileId::snake;
    d.hp = 6;
    d.dmg_melee = 1;
    d.dmg_ranged = 1;
    d.ranged_cooldown_turns = 150;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::ranged, 85);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = dlvl_last_early_game;
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.is_snake = true;
    d.is_reptile = true;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    d.native_rooms.push_back(RoomType::ritual);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Black Mamba";
    d.name_the = "the Black Mamba";
    d.corpse_name_a = "a Black Mamba corpse";
    d.corpse_name_the = "the Black Mamba corpse";
    d.id = ActorId::black_mamba;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'S';
    d.color = clr_gray;
    d.tile = TileId::snake;
    d.hp = 6;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = dlvl_last_early_game;
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.is_snake = true;
    d.is_reptile = true;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    d.native_rooms.push_back(RoomType::ritual);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Fire Hound";
    d.name_the = "the Fire Hound";
    d.id = ActorId::fire_hound;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.ranged_cooldown_turns = 3;
    d.glyph = 'd';
    d.color = clr_red;
    d.tile = TileId::hound;
    d.hp = 22;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.dmg_ranged = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::ranged, 65);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 8;
    d.spawn_max_dlvl = dlvl_last - 3;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    3),
        MonGroupSpawnRule(MonGroupSize::pack,   1),
    });
    d.actor_size = ActorSize::floor;
    d.can_bash_doors = true;
    d.nr_turns_aware = 12;
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
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "an Energy Hound";
    d.name_the = "the Energy Hound";
    d.id = ActorId::energy_hound;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.ranged_cooldown_turns = 2;
    d.glyph = 'd';
    d.color = clr_yellow;
    d.tile = TileId::hound;
    d.hp = 32;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.dmg_ranged = min_dmg_to_wound + 3;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.ability_vals.set_val(AbilityId::ranged, 75);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::r_elec] = true;
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = (dlvl_first_mid_game + dlvl_first_late_game) / 2;
    d.spawn_max_dlvl = dlvl_last - 2;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    3),
        MonGroupSpawnRule(MonGroupSize::pack,   1),
    });
    d.actor_size = ActorSize::floor;
    d.can_bash_doors = true;
    d.nr_turns_aware = 12;
    d.descr =
        "This extremely aggressive canine seems to be part corporeal and "
        "part energy. It breathes lightning.";
    d.wary_msg = d.name_the + " seems aggravated.";
    d.aggro_msg_mon_seen = d.name_the + " snarls.";
    d.aggro_msg_mon_hidden = "I hear a snarl.";
    d.aggro_sfx_mon_seen = SfxId::dog_snarl;
    d.aggro_sfx_mon_hidden = SfxId::dog_snarl;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_canine = true;
    d.can_be_summoned = true;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Zuul the Gatekeeper";
    d.name_the = "Zuul the Gatekeeper";
    d.corpse_name_a = "the corpse of Zuul the Gatekeeper";
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
    d.hp = 80;
    d.spi = 40;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 90);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
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

    d.name_a = "a Ghost";
    d.name_the = "the Ghost";
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
    d.color = clr_white_lgt;
    d.tile = TileId::ghost;
    d.hp = 4;
    d.spi = 12;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.natural_props[(size_t)PropId::ethereal] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.can_be_summoned = false;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::crypt);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Phantasm";
    d.name_the = "the Phantasm";
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
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::ethereal] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.natural_props[(size_t)PropId::cloaked] = true;
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
    d.can_be_summoned = false;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::crypt);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Wraith";
    d.name_the = "the Wraith";
    d.id = ActorId::wraith;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.spell_skill = SpellSkill::basic;
    d.glyph = 'G';
    d.color = clr_red;
    d.tile = TileId::wraith;
    d.hp = 15;
    d.spi = 30;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.natural_props[(size_t)PropId::ethereal] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.natural_props[(size_t)PropId::invis] = true;
    d.spawn_min_dlvl = dlvl_first_late_game;
    d.spawn_max_dlvl = dlvl_last - 3;
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.descr = "A powerful spirit.";
    d.wary_msg = d.name_the + " looks disturbed.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_undead = true;
    d.is_ghost = true;
    d.can_be_summoned = false;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::crypt);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Void Traveler";
    d.name_the = "the Void Traveler";
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
    d.color = clr_white_lgt;
    d.tile = TileId::void_traveler;
    d.hp = 50;
    d.spi = 30;
    d.dmg_melee = min_dmg_to_wound + 4;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.spawn_min_dlvl = dlvl_first_mid_game;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 12;
    d.descr =
        "A nightmarish creature of humanoid shape. It has a hideous gangly "
        "physique and sickly pale skin. Its arms are as long as its body is "
        "tall, and it attacks with ripping claws. It can appear anywhere it "
        "desires at any time, and can also snatch teleporting creatures from "
        "their voyage and draw them to it.";
    d.wary_msg = d.name_the + " looks wary.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.can_bash_doors = false;
    d.can_open_doors = false;
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

    d.name_a = "an Elder Void Traveler";
    d.name_the = "the Elder Void Traveler";
    d.id = ActorId::elder_void_traveler;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.glyph = 'Y';
    d.color = clr_orange;
    d.tile = TileId::void_traveler;
    d.hp = 70;
    d.spi = 30;
    d.spell_skill = SpellSkill::expert;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 90);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.spawn_min_dlvl = dlvl_first_late_game;
    d.spawn_max_dlvl = dlvl_last - 2;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 12;
    d.descr =
        "A nightmarish creature of humanoid shape. It has a hideous gangly "
        "physique and sickly pale skin. Its arms are as long as its body is "
        "tall, and it attacks with ripping claws. It can appear anywhere it "
        "desires at any time, and can also snatch teleporting creatures from "
        "their voyage and draw them to it. This ancient individual has learned "
        "the art of spellcasting.";
    d.wary_msg = d.name_the + " looks wary.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::mind_shattering;
    d.can_bash_doors = false;
    d.can_open_doors = false;
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

    d.name_a = "a Rat";
    d.name_the = "the Rat";
    d.corpse_name_a = "a Rat corpse";
    d.corpse_name_the = "the Rat corpse";
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
    d.natural_props[(size_t)PropId::darkvis] = true;
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

    d.name_a = "a Rat-thing";
    d.name_the = "the Rat-thing";
    d.corpse_name_a = "a Rat-thing corpse";
    d.corpse_name_the = "the Rat-thing corpse";
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
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = 3;
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

    d.name_a = "a Wolf";
    d.name_the = "the Wolf";
    d.corpse_name_a = "a Wolf carcass";
    d.corpse_name_the = "the Wolf carcass";
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
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 0;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 2;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    1),
        MonGroupSpawnRule(MonGroupSize::pack,   2)
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

    d.name_a = "a Raven";
    d.name_the = "the Raven";
    d.corpse_name_a = "a Raven carcass";
    d.corpse_name_the = "the Raven carcass";
    d.id = ActorId::raven;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.glyph = 'b';
    d.color = clr_gray;
    d.tile = TileId::raven;
    d.hp = 2;
    d.spi = 6;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 35);
    d.ability_vals.set_val(AbilityId::dodging, 50);
    d.natural_props[(size_t)PropId::flying] = true;
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = dlvl_last_early_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    1)
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 7;
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

    d.name_a = "a Giant Bat";
    d.name_the = "the Giant Bat";
    d.corpse_name_a = "a Giant Bat corpse";
    d.corpse_name_the = "the Giant Bat corpse";
    d.id = ActorId::giant_bat;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
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
    d.ability_vals.set_val(AbilityId::dodging, 60);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    4),
        MonGroupSpawnRule(MonGroupSize::pack,   2),
    });
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.nr_turns_aware = 7;
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

    d.name_a = "a Vampire Bat";
    d.name_the = "the Vampire Bat";
    d.corpse_name_a = "a Vampire Bat corpse";
    d.corpse_name_the = "the Vampire Bat corpse";
    d.id = ActorId::vampire_bat;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.glyph = 'B';
    d.color = clr_red_lgt;
    d.tile = TileId::bat;
    d.hp = 12;
    d.spi = 8;
    d.dmg_melee = 3;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 60);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 6;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    3),
    });
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.nr_turns_aware = 7;
    d.descr =
        "An unknown species, perhaps stemming from some hidden bowels of "
        "the earth. It is about as tall as a full-grown human. Its attack "
        "drains health from the victim.";
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

    d.name_a = "Abaxu the Destroyer";
    d.name_the = "Abaxu the Destroyer";
    d.corpse_name_a = "the corpse of Abaxu";
    d.corpse_name_the = "the corpse of Abaxu";
    d.id = ActorId::abaxu;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fastest;
    d.glyph = 'B';
    d.color = clr_white_lgt;
    d.tile = TileId::bat;
    d.hp = 120;
    d.spi = 36;
    d.dmg_melee = min_dmg_to_wound + 6;
    d.ability_vals.set_val(AbilityId::melee, 90);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.spawn_min_dlvl = dlvl_first_late_game;
    d.spawn_max_dlvl = 999;
    d.actor_size = ActorSize::giant;
    d.can_bash_doors = true;
    d.nr_turns_aware = 24;
    d.descr =
        "A colossal white bat, spoken of in ancient traditions as \"Abaxu "
        "the Destroyer\" (or a variation thereof). It has eternal life, which "
        "it sustains by feeding on other creatures - allegedly including "
        "human sacrifice. This has surely given rise to certain vampire myths "
        "over the years.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_hidden = "I hear the flapping of great wings.";
    d.aggro_sfx_mon_seen = SfxId::flapping_wings;
    d.aggro_sfx_mon_hidden = SfxId::flapping_wings;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::mind_shattering;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 1;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Byakhee";
    d.name_the = "the Byakhee";
    d.corpse_name_a = "a Byakhee corpse";
    d.corpse_name_the = "the Byakhee corpse";
    d.id = ActorId::byakhee;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'B';
    d.color = clr_brown_drk;
    d.tile = TileId::byakhee;
    d.hp = 32;
    d.spi = 20;
    d.dmg_melee = min_dmg_to_wound + 4;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 8;
    d.spawn_max_dlvl = dlvl_last - 3;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  4),
        MonGroupSpawnRule(MonGroupSize::few,    1),
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
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.can_be_summoned = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Giant Mantis";
    d.name_the = "the Giant Mantis";
    d.corpse_name_a = "a Giant Mantis corpse";
    d.corpse_name_the = "the Giant Mantis corpse";
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
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::stealth, 20);
    d.ability_vals.set_val(AbilityId::dodging, 30);
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 8;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.nr_turns_aware = 5;
    d.descr = "A huge predatory insect.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.can_be_summoned = false;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Giant Locust";
    d.name_the = "the Giant Locust";
    d.corpse_name_a = "a Giant Locust corpse";
    d.corpse_name_the = "the Giant Locust corpse";
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
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.can_be_summoned = false;
    d.native_rooms.push_back(RoomType::plain);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Mi-go";
    d.name_the = "the Mi-go";
    d.corpse_name_a = "a Mi-go corpse";
    d.corpse_name_the = "the Mi-go corpse";
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
    d.spell_skill = SpellSkill::basic;
    d.glyph = 'E';
    d.color = clr_brown_drk;
    d.tile = TileId::mi_go;
    d.hp = 10;
    d.spi = 20;
    d.dmg_melee = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::ranged, 60);
    d.natural_props[(size_t)PropId::flying] = true;
    d.spawn_min_dlvl = 6;
    d.spawn_max_dlvl = dlvl_last - 6;
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
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Mi-go Commander";
    d.name_the = "the Mi-go Commander";
    d.corpse_name_a = "a Mi-go corpse";
    d.corpse_name_the = "the Mi-go corpse";
    d.id = ActorId::mi_go_commander;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::fast;
    d.ranged_cooldown_turns = 3;
    d.spell_skill = SpellSkill::basic;
    d.glyph = 'E';
    d.color = clr_green;
    d.tile = TileId::mi_go;
    d.hp = 24;
    d.spi = 20;
    d.dmg_melee = min_dmg_to_wound + 3;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.ability_vals.set_val(AbilityId::ranged, 80);
    d.natural_props[(size_t)PropId::flying] = true;
    d.spawn_min_dlvl = 12;
    d.spawn_max_dlvl = dlvl_last - 2;
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
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Flying Polyp";
    d.name_the = "the Flying Polyp";
    d.corpse_name_a = "a dead Polyp";
    d.corpse_name_the = "the dead Polyp";
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
    d.ability_vals.set_val(AbilityId::melee, 80);
    //
    // NOTE: Polyps should be resistant against all damage except electricity
    //
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
    d.spawn_min_dlvl = 6;
    d.spawn_max_dlvl = dlvl_last_mid_game;
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
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Greater Polyp";
    d.name_the = "the Greater Polyp";
    d.corpse_name_a = "a dead Polyp";
    d.corpse_name_the = "the dead Polyp";
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
    d.color = clr_cyan_lgt;
    d.tile = TileId::polyp;
    d.hp = 70;
    d.spi = 30;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 90);
    //
    // NOTE: Polyps should be resistant against all damage except electricity
    //
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
    d.spawn_min_dlvl = dlvl_first_mid_game;
    d.spawn_max_dlvl = dlvl_last - 3;
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
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Mind Leech";
    d.name_the = "the Mind Leech";
    d.corpse_name_a = "a Mind Leech corpse";
    d.corpse_name_the = "the Mind Leech corpse";
    d.id = ActorId::mind_leech;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.glyph = 'L';
    d.color = clr_yellow_drk;
    d.tile = TileId::leech;
    d.hp = 32;
    d.spi = 40;
    d.dmg_melee = 2;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.spawn_min_dlvl = 5;
    d.spawn_max_dlvl = (dlvl_last_mid_game + dlvl_last) / 2;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = false;
    d.can_open_doors = false;
    d.nr_turns_aware = 16;
    d.descr =
        "A tentacled alien creature, mysteriously floating through the air "
        "in a ghostly manner. They somehow sustain themselves on the brain "
        "activity of other beings. This will usually not kill the victim, "
        "but the experience is horrific beyond words. They only feed on "
        "sound minds however - a strange or chaotic mind will cause them "
        "great distress.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Life Leech";
    d.name_the = "the Life Leech";
    d.corpse_name_a = "a Life Leech corpse";
    d.corpse_name_the = "the Life Leech corpse";
    d.id = ActorId::life_leech;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.glyph = 'L';
    d.color = clr_red_lgt;
    d.tile = TileId::leech;
    d.hp = 32;
    d.spi = 40;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.spawn_min_dlvl = 5;
    d.spawn_max_dlvl = (dlvl_last_mid_game + dlvl_last) / 2;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = false;
    d.can_open_doors = false;
    d.nr_turns_aware = 16;
    d.descr =
        "A tentacled alien creature, mysteriously floating through the air "
        "in a ghostly manner. They sustain themselves by consuming the life "
        "force of other beings.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Spirit Leech";
    d.name_the = "the Spirit Leech";
    d.corpse_name_a = "a Spirit Leech corpse";
    d.corpse_name_the = "the Spirit Leech corpse";
    d.id = ActorId::spirit_leech;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.glyph = 'L';
    d.color = clr_violet;
    d.tile = TileId::leech;
    d.hp = 32;
    d.spi = 40;
    d.dmg_melee = 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.spawn_min_dlvl = 5;
    d.spawn_max_dlvl = (dlvl_last_mid_game + dlvl_last) / 2;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = false;
    d.can_open_doors = false;
    d.nr_turns_aware = 16;
    d.descr =
        "A tentacled alien creature, mysteriously floating through the air "
        "in a ghostly manner. They sustain themselves by consuming the Spirit "
        "of other beings.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Ghoul";
    d.name_the = "the Ghoul";
    d.corpse_name_a = "a Ghoul corpse";
    d.corpse_name_the = "the Ghoul corpse";
    d.id = ActorId::ghoul;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    //
    // NOTE: This should be the same as the player Ghoul background bonus
    //
    d.speed_pct = (int)ActorSpeed::normal + 10;
    d.glyph = 'M';
    d.color = clr_green;
    d.tile = TileId::ghoul;
    d.hp = 30;
    d.spi = 20;
    d.dmg_melee = min_dmg_to_wound + 3;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 6;
    d.spawn_max_dlvl = dlvl_last_mid_game;
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
    d.is_humanoid = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::monster);
    d.native_rooms.push_back(RoomType::crypt);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Shadow";
    d.name_the = "the Shadow";
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
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::stealth, 95);
    d.natural_props[(size_t)PropId::lgt_sens] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
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
    d.is_humanoid = true;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "an Invisible Stalker";
    d.name_the = "the Invisible Stalker";
    d.id = ActorId::invis_stalker;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 10;
    d.glyph = 'M';
    d.color = clr_yellow;
    d.tile = TileId::shadow;
    d.hp = 18;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::invis] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = dlvl_last_early_game;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.nr_turns_aware = 24;
    d.descr =
        "A mysterious humanoid figure stalking for prey. It is invisible to "
        "normal eyes.";
    d.wary_msg = d.name_the + " looks wary.";
    d.is_auto_descr_allowed = true;
    d.is_humanoid = true;
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::human);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Mummy";
    d.name_the = "the Mummy";
    d.corpse_name_a = "a lifeless Mummy";
    d.corpse_name_the = "the lifeless Mummy";
    d.id = ActorId::mummy;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow + 10;
    d.spell_skill = SpellSkill::expert;
    d.glyph = 'P';
    d.color = clr_magenta;
    d.tile = TileId::mummy;
    d.hp = 36;
    d.spi = 30;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 65);
    d.spawn_min_dlvl = dlvl_first_mid_game;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 24;
    d.descr = "A mummified human being, possibly dating back millennia.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.is_undead = true;
    d.is_humanoid = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Crocodile Head Mummy";
    d.name_the = "the Crocodile Head Mummy";
    d.corpse_name_a = "a lifeless Mummy";
    d.corpse_name_the = "the lifeless Mummy";
    d.id = ActorId::croc_head_mummy;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::slow + 10;
    d.glyph = 'P';
    d.color = clr_gray;
    d.tile = TileId::croc_head_mummy;
    d.hp = 36;
    d.spi = 30;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.spawn_min_dlvl = dlvl_first_mid_game + 4;
    d.spawn_max_dlvl = dlvl_last - 3;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  20),
        MonGroupSpawnRule(MonGroupSize::few,    60),
        MonGroupSpawnRule(MonGroupSize::pack,   10),
        MonGroupSpawnRule(MonGroupSize::swarm,  1)
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
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Khephren";
    d.name_the = "Khephren";
    d.corpse_name_a = "the lifeless Mummy of Khephren";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::khephren;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_skill = SpellSkill::expert;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 0;
    d.glyph = 'P';
    d.color = clr_red;
    d.tile = TileId::mummy;
    d.hp = 80;
    d.spi = 60;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.is_auto_spawn_allowed = false;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 9999;
    d.descr =
        "The mummified fourth dynasty Egyptian pharaoh Khephren. How he came "
        "to dwell here is beyond anyone's guess.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_undead = true;
    d.is_humanoid = true;
    d.native_rooms.push_back(RoomType::plain);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Nitokris";
    d.name_the = "Nitokris";
    d.corpse_name_a = "the lifeless Mummy of Nitokris";
    d.corpse_name_the = d.corpse_name_a;
    d.id = ActorId::nitokris;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_skill = SpellSkill::expert;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 1;
    d.glyph = 'P';
    d.color = clr_red_lgt;
    d.tile = TileId::mummy;
    d.hp = 80;
    d.spi = 60;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.spawn_min_dlvl = 11;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.nr_turns_aware = 9999;
    d.descr =
        "The mummified sixth dynasty Egyptian pharaoh Nitokris. How she "
        "came to dwell here is beyond anyone's guess. Her name is found in the "
        "histories of Herodotus and writings of Manetho. According to the "
        "writings of the former, she took the throne by inviting her "
        "brother's murderers to a banquet. Then killed them by flooding the "
        "sealed room with the Nile.";
    d.wary_msg = d.name_the + " looks wary.";
    d.spell_cast_msg = "speaks incantations in a deep hollow voice.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_undead = true;
    d.is_humanoid = true;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Deep One";
    d.name_the = "the Deep One";
    d.corpse_name_a = "a corpse of a Deep One";
    d.corpse_name_the = "the corpse of a Deep One";
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
    d.hp = 22;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound + 4;
    d.dmg_ranged = min_dmg_to_wound;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::ranged, 70);
    d.spawn_min_dlvl = 6;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::pack,   3),
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
        "water. In the timeless depths of the sea, the deep ones' alien, "
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

    d.name_a = "a Carnivorous Ape";
    d.name_the = "the Carnivorous Ape";
    d.corpse_name_a = "an Ape corpse";
    d.corpse_name_the = "the Ape corpse";
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
    d.hp = 22;
    d.spi = 6;
    d.dmg_melee = min_dmg_to_wound + 1;
    d.ability_vals.set_val(AbilityId::melee, 75);
    d.ability_vals.set_val(AbilityId::dodging, 20);
    d.natural_props[(size_t)PropId::darkvis] = true;
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

    d.name_a = "a Worm Mass";
    d.name_the = "the Worm Mass";
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
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 1;
    d.spawn_max_dlvl = dlvl_last_early_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    8),
        MonGroupSpawnRule(MonGroupSize::pack,   1),
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
    d.name_the = "the Mind Worms";
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
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 6;
    d.spawn_max_dlvl = dlvl_last_mid_game;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::few,    8),
        MonGroupSpawnRule(MonGroupSize::pack,   1),
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

    d.name_a = "a Dust Vortex";
    d.name_the = "the Dust Vortex";
    d.id = ActorId::dust_vortex;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'V';
    d.color = clr_gray;
    d.tile = TileId::vortex;
    d.hp = 10;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 10;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone, 1)
    });
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.is_auto_descr_allowed = true;
    d.descr = "A spinning force of destruction.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_hidden = "I hear a howling wind.";
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Fire Vortex";
    d.name_the = "the Fire Vortex";
    d.id = ActorId::fire_vortex;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'V';
    d.color = clr_red;
    d.tile = TileId::vortex;
    d.hp = 16;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = 8;
    d.spawn_max_dlvl = dlvl_last - 3;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone, 2)
    });
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 5;
    d.is_auto_descr_allowed = true;
    d.descr = "A spinning force of fire and destruction.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_hidden = "I hear a howling wind.";
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "an Energy Vortex";
    d.name_the = "the Energy Vortex";
    d.id = ActorId::energy_vortex;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'V';
    d.color = clr_yellow;
    d.tile = TileId::vortex;
    d.hp = 24;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound + 3;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = (dlvl_first_mid_game + dlvl_first_late_game) / 2;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 9;
    d.is_auto_descr_allowed = true;
    d.descr = "A spinning force of pure energy.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.aggro_msg_mon_hidden = "I hear a howling wind.";
    d.can_be_summoned = true;
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::cave);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Black Ooze";
    d.name_the = "the Black Ooze";
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
    d.color = clr_gray;
    d.tile = TileId::ooze;
    d.hp = 16;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::ooze] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::r_blind] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 1;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    3),
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 8;
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
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Clear Ooze";
    d.name_the = "the Clear Ooze";
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
    d.color = clr_white_lgt;
    d.tile = TileId::ooze;
    d.hp = 12;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 2;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.ability_vals.set_val(AbilityId::stealth, 95);
    d.natural_props[(size_t)PropId::ooze] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::r_blind] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 4;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 2;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    3),
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 8;
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
    d.mon_shock_lvl = ShockLvl::frightening;
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Putrid Ooze";
    d.name_the = "the Putrid Ooze";
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
    d.hp = 18;
    d.spi = 12;
    d.dmg_melee = min_dmg_to_wound - 1;
    d.ability_vals.set_val(AbilityId::melee, 60);
    d.natural_props[(size_t)PropId::ooze] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::r_blind] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 5;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 2;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    3),
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 8;
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
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Poison Ooze";
    d.name_the = "the Poison Ooze";
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
    d.natural_props[(size_t)PropId::r_blind] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 7;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 2;
    d.group_sizes.assign(
    {
        MonGroupSpawnRule(MonGroupSize::alone,  1),
        MonGroupSpawnRule(MonGroupSize::few,    3),
    });
    d.actor_size = ActorSize::floor;
    d.nr_turns_aware = 8;
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
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Strange Color";
    d.name_the = "the Strange Color";
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
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.natural_props[(size_t)PropId::ooze] = true;
    d.natural_props[(size_t)PropId::ethereal] = true;
    d.natural_props[(size_t)PropId::r_phys] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::r_blind] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 14;
    d.spawn_max_dlvl = dlvl_last - 6;
    d.actor_size = ActorSize::humanoid;
    d.nr_turns_aware = 8;
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
    d.can_bleed = false;
    d.can_leave_corpse = false;
    d.erratic_move_pct = ActorErraticFreq::very;
    d.mon_shock_lvl = ShockLvl::frightening;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Chthonian";
    d.name_the = "the Chthonian";
    d.corpse_name_a = "a Chthonian corpse";
    d.corpse_name_the = "the Chthonian corpse";
    d.id = ActorId::chthonian;
    d.ai[(size_t)AiId::looks] = true;
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
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.natural_props[(size_t)PropId::burrowing] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = (dlvl_first_mid_game + dlvl_first_late_game) / 2;
    d.actor_size = ActorSize::giant;
    d.nr_turns_aware = 16;
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

    d.id = ActorId::animated_wpn;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = true;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal;
    d.hp = 16;
    d.spi = 40;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::dodging, 20);
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
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::unsettling;
    data[(size_t)d.id] = d;
    d.reset();

    d.id = ActorId::mold;
    d.name_a = "Mold";
    d.name_the = "the Mold";
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
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_blind] = true;
    d.speed_pct = (int)ActorSpeed::sluggish;
    d.ai[(size_t)AiId::looks] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::moves_to_random_when_unaware] = false;
    d.prevent_knockback = true;
    d.nr_turns_aware = 5;
    d.spawn_min_dlvl = 3;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
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
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Gas Spore";
    d.name_the = "the Gas Spore";
    d.id = ActorId::gas_spore;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = false;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::sluggish;
    d.glyph = 'E';
    d.color = clr_red;
    d.tile = TileId::gas_spore;
    d.hp = 1;
    d.spi = 1;
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_blind] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.spawn_min_dlvl = 2;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 5;
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
    d.erratic_move_pct = ActorErraticFreq::somewhat;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::cave);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::forest);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Sentry Drone";
    d.name_the = "the Sentry Drone";
    d.corpse_name_a = "a broken Sentry Drone";
    d.corpse_name_the = "the broken Sentry Drone";
    d.id = ActorId::sentry_drone;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = false;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = false;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.ranged_cooldown_turns = 3;
    d.spell_skill = SpellSkill::expert;
    d.glyph = 'T';
    d.color = clr_red;
    d.tile = TileId::device2;
    d.hp = 18;
    d.spi = 40;
    d.ability_vals.set_val(AbilityId::melee, 50);
    d.ability_vals.set_val(AbilityId::ranged, 65);
    d.ability_vals.set_val(AbilityId::dodging, 35);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.natural_props[(size_t)PropId::r_poison] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_disease] = true;
    d.natural_props[(size_t)PropId::r_blind] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = dlvl_first_mid_game;
    d.spawn_max_dlvl = d.spawn_min_dlvl + 4;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = false;
    d.can_open_doors = false;
    d.nr_turns_aware = 16;
    d.descr =
        "An infernal piece of technology, seemingly designed to serve as a "
        "sort of guard. It hovers around, searching the area for interlopers "
        "to blast on sight. It appears to have some organic parts, and may "
        "even be a conscious living being.";
    d.wary_msg = d.name_the + " seems disturbed.";
    d.spell_cast_msg = "makes buzzing sounds in peculiar frequencies.";
    d.aggro_msg_mon_seen = d.name_the + " makes a deep buzzing sound.";
    d.aggro_msg_mon_hidden = "I hear a deep buzzing sound.";
    d.erratic_move_pct = ActorErraticFreq::rare;
    d.mon_shock_lvl = ShockLvl::unsettling;
    d.native_rooms.push_back(RoomType::plain);
    d.native_rooms.push_back(RoomType::flooded);
    d.native_rooms.push_back(RoomType::muddy);
    d.native_rooms.push_back(RoomType::jail);
    d.native_rooms.push_back(RoomType::chasm);
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "a Hunting Horror";
    d.name_the = "the Hunting Horror";
    d.corpse_name_a = "a Hunting Horror corpse";
    d.corpse_name_the = "the Hunting Horror corpse";
    d.id = ActorId::hunting_horror;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = false;
    d.ai[(size_t)AiId::moves_to_leader] = false;
    d.speed_pct = (int)ActorSpeed::fast;
    d.glyph = 'W';
    d.color = clr_gray;
    d.tile = TileId::hunting_horror;
    d.hp = 90;
    d.spi = 40;
    d.dmg_melee = min_dmg_to_wound + 7;
    d.ability_vals.set_val(AbilityId::melee, 90);
    d.ability_vals.set_val(AbilityId::dodging, 40);
    d.natural_props[(size_t)PropId::flying] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
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

    d.name_a = "a Death Fiend";
    d.name_the = "the Death Fiend";
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
    d.speed_pct = (int)ActorSpeed::normal;
    d.spell_skill = SpellSkill::expert;
    d.glyph = 'Y';
    d.color = clr_gray;
    d.tile = TileId::fiend;
    d.hp = 90;
    d.spi = 40;
    d.dmg_melee = 200;
    d.ability_vals.set_val(AbilityId::melee, 80);
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_sleep] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::darkvis] = true;
    d.prevent_knockback = true;
    d.spawn_min_dlvl = dlvl_last - 6;
    d.spawn_max_dlvl = dlvl_last - 2;
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
    d.speed_pct = (int)ActorSpeed::normal + 10;
    d.spell_skill = SpellSkill::master;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 0;
    d.glyph = 'P';
    d.color = clr_red;
    d.tile = TileId::the_high_priest;
    d.hp = 110;
    d.spi = 999;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.natural_props[(size_t)PropId::r_fire] = true;
    d.dmg_melee = min_dmg_to_wound + 3;
    d.ability_vals.set_val(AbilityId::melee, 70);
    d.ability_vals.set_val(AbilityId::dodging, 25);
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
    d.is_undead = true;
    d.can_leave_corpse = false;
    d.can_bleed = true;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Balastha";
    d.corpse_name_a = "the corpse of Balastha";
    d.corpse_name_the = d.corpse_name_a;
    d.name_the = d.name_a;
    d.id = ActorId::high_priest_guard_war_vet;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 0;
    d.glyph = 'P';
    d.color = clr_green;
    d.tile = TileId::player_firearm;
    d.hp = 100;
    d.spi = 999;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.ability_vals.set_val(AbilityId::melee, 100);
    d.ability_vals.set_val(AbilityId::dodging, 25);
    d.ability_vals.set_val(AbilityId::ranged, 100);
    d.is_auto_spawn_allowed = false;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.prevent_knockback = true;
    d.nr_turns_aware = 9999;
    d.descr =
        "Balastha is an undead master of arms and warfare, bound to guard the "
        "High Priest for all eternity. He is wielding a Tommy Gun.";
    d.wary_msg = d.name_the + " looks wary.";
    d.is_auto_descr_allowed = false;
    d.spell_cast_msg = "mutters incantations.";
    d.erratic_move_pct = ActorErraticFreq::never;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_humanoid = true;
    d.is_undead = true;
    d.can_leave_corpse = true;
    d.can_bleed = true;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Uraga";
    d.corpse_name_a = "the corpse of Uraga";
    d.corpse_name_the = d.corpse_name_a;
    d.name_the = d.name_a;
    d.id = ActorId::high_priest_guard_rogue;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 0;
    d.glyph = 'P';
    d.color = clr_cyan;
    d.tile = TileId::player_melee;
    d.hp = 80;
    d.spi = 999;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.ability_vals.set_val(AbilityId::melee, 90);
    d.ability_vals.set_val(AbilityId::dodging, 50);
    d.ability_vals.set_val(AbilityId::stealth, 120);
    d.is_auto_spawn_allowed = false;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.prevent_knockback = true;
    d.nr_turns_aware = 9999;
    d.descr =
        "Uraga is an undead master of deception and assassination, bound "
        "to guard the High Priest for all eternity.";
    d.wary_msg = d.name_the + " looks wary.";
    d.is_auto_descr_allowed = false;
    d.spell_cast_msg = "mutters incantations.";
    d.erratic_move_pct = ActorErraticFreq::never;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_humanoid = true;
    d.is_undead = true;
    d.can_leave_corpse = true;
    d.can_bleed = true;
    data[(size_t)d.id] = d;
    d.reset();

    d.name_a = "Vrka";
    d.corpse_name_a = "the corpse of Vrka";
    d.corpse_name_the = d.corpse_name_a;
    d.name_the = d.name_a;
    d.id = ActorId::high_priest_guard_ghoul;
    d.ai[(size_t)AiId::looks] = true;
    d.ai[(size_t)AiId::makes_room_for_friend] = false;
    d.ai[(size_t)AiId::attacks] = true;
    d.ai[(size_t)AiId::paths_to_tgt_when_aware] = true;
    d.ai[(size_t)AiId::moves_to_tgt_when_los] = true;
    d.ai[(size_t)AiId::moves_to_lair] = true;
    d.ai[(size_t)AiId::moves_to_leader] = true;
    d.speed_pct = (int)ActorSpeed::normal + 20;
    d.is_unique = true;
    d.nr_left_allowed_to_spawn = 0;
    d.glyph = 'P';
    d.color = clr_brown;
    d.tile = TileId::ghoul;
    d.hp = 140;
    d.spi = 999;
    d.natural_props[(size_t)PropId::r_conf] = true;
    d.natural_props[(size_t)PropId::r_fear] = true;
    d.natural_props[(size_t)PropId::r_breath] = true;
    d.dmg_melee = min_dmg_to_wound + 5;
    d.ability_vals.set_val(AbilityId::melee, 90);
    d.ability_vals.set_val(AbilityId::dodging, 25);
    d.is_auto_spawn_allowed = false;
    d.actor_size = ActorSize::humanoid;
    d.can_bash_doors = true;
    d.can_open_doors = true;
    d.prevent_knockback = true;
    d.nr_turns_aware = 9999;
    d.descr =
        "Vrka is a monstrous wolf-like creature dwelling in the High Priest's "
        "chambers, serving as a guardian.";
    d.wary_msg = d.name_the + " looks wary.";
    d.is_auto_descr_allowed = false;
    d.spell_cast_msg = "mutters incantations.";
    d.erratic_move_pct = ActorErraticFreq::never;
    d.mon_shock_lvl = ShockLvl::terrifying;
    d.is_humanoid = true;
    d.can_leave_corpse = true;
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
