#include "actor_data.hpp"

#include <string>
#include <vector>
#include <math.h>
#include <unordered_map>

#include "xml.hpp"
#include "saving.hpp"
#include "item_data.hpp"
#include "property_factory.hpp"
#include "item_att_property.hpp"
#include "property.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
// *****************************************************************************
// TODO: TEMPORARY CODE
// *****************************************************************************
static std::unordered_map<std::string, ActorId> str_to_actor_id_map = {
        {"player", ActorId::player},
        {"zombie", ActorId::zombie},
        {"zombie_axe", ActorId::zombie_axe},
        {"bloated_zombie", ActorId::bloated_zombie},
        {"crawling_intestines", ActorId::crawling_intestines},
        {"crawling_hand", ActorId::crawling_hand},
        {"thing", ActorId::thing},
        {"floating_skull", ActorId::floating_skull},
        {"cultist", ActorId::cultist},
        {"cultist_priest", ActorId::cultist_priest},
        {"cultist_wizard", ActorId::cultist_wizard},
        {"cultist_arch_wizard", ActorId::cultist_arch_wizard},
        {"bog_tcher", ActorId::bog_tcher},
        {"rat", ActorId::rat},
        {"rat_thing", ActorId::rat_thing},
        {"green_spider", ActorId::green_spider},
        {"white_spider", ActorId::white_spider},
        {"red_spider", ActorId::red_spider},
        {"shadow_spider", ActorId::shadow_spider},
        {"leng_spider", ActorId::leng_spider},
        {"pit_viper", ActorId::pit_viper},
        {"spitting_cobra", ActorId::spitting_cobra},
        {"black_mamba", ActorId::black_mamba},
        {"mi_go", ActorId::mi_go},
        {"mi_go_commander", ActorId::mi_go_commander},
        {"flying_polyp", ActorId::flying_polyp},
        {"greater_polyp", ActorId::greater_polyp},
        {"mind_leech", ActorId::mind_leech},
        {"spirit_leech", ActorId::spirit_leech},
        {"life_leech", ActorId::life_leech},
        {"ghoul", ActorId::ghoul},
        {"shadow", ActorId::shadow},
        {"invis_stalker", ActorId::invis_stalker},
        {"wolf", ActorId::wolf},
        {"fire_hound", ActorId::fire_hound},
        {"energy_hound", ActorId::energy_hound},
        {"zuul", ActorId::zuul},
        {"ghost", ActorId::ghost},
        {"void_traveler", ActorId::void_traveler},
        {"elder_void_traveler", ActorId::elder_void_traveler},
        {"raven", ActorId::raven},
        {"giant_bat", ActorId::giant_bat},
        {"vampire_bat", ActorId::vampire_bat},
        {"abaxu", ActorId::abaxu},
        {"byakhee", ActorId::byakhee},
        {"giant_mantis", ActorId::giant_mantis},
        {"locust", ActorId::locust},
        {"mummy", ActorId::mummy},
        {"croc_head_mummy", ActorId::croc_head_mummy},
        {"khephren", ActorId::khephren},
        {"nitokris", ActorId::nitokris},
        {"deep_one", ActorId::deep_one},
        {"ape", ActorId::ape},
        {"keziah_mason", ActorId::keziah_mason},
        {"brown_jenkin", ActorId::brown_jenkin},
        {"major_clapham_lee", ActorId::major_clapham_lee},
        {"dean_halsey", ActorId::dean_halsey},
        {"worm_mass", ActorId::worm_mass},
        {"mind_worms", ActorId::mind_worms},
        {"dust_vortex", ActorId::dust_vortex},
        {"fire_vortex", ActorId::fire_vortex},
        {"energy_vortex", ActorId::energy_vortex},
        {"ooze_black", ActorId::ooze_black},
        {"ooze_clear", ActorId::ooze_clear},
        {"ooze_putrid", ActorId::ooze_putrid},
        {"ooze_poison", ActorId::ooze_poison},
        {"strange_color", ActorId::strange_color},
        {"chthonian", ActorId::chthonian},
        {"hunting_horror", ActorId::hunting_horror},
        {"sentry_drone", ActorId::sentry_drone},
        {"animated_wpn", ActorId::animated_wpn},
        {"mold", ActorId::mold},
        {"gas_spore", ActorId::gas_spore},
        {"death_fiend", ActorId::death_fiend},
        {"the_high_priest", ActorId::the_high_priest},
        {"high_priest_guard_war_vet", ActorId::high_priest_guard_war_vet},
        {"high_priest_guard_rogue", ActorId::high_priest_guard_rogue},
        {"high_priest_guard_ghoul", ActorId::high_priest_guard_ghoul}
};

// *****************************************************************************
// TODO: TEMPORARY CODE
// *****************************************************************************
static std::unordered_map<ActorId, std::string> actor_id_to_str_map = {
        {ActorId::player, "player"},
        {ActorId::zombie, "zombie"},
        {ActorId::zombie_axe, "zombie_axe"},
        {ActorId::bloated_zombie, "bloated_zombie"},
        {ActorId::crawling_intestines, "crawling_intestines"},
        {ActorId::crawling_hand, "crawling_hand"},
        {ActorId::thing, "thing"},
        {ActorId::floating_skull, "floating_skull"},
        {ActorId::cultist, "cultist"},
        {ActorId::cultist_priest, "cultist_priest"},
        {ActorId::cultist_wizard, "cultist_wizard"},
        {ActorId::cultist_arch_wizard, "cultist_arch_wizard"},
        {ActorId::bog_tcher, "bog_tcher"},
        {ActorId::rat, "rat"},
        {ActorId::rat_thing, "rat_thing"},
        {ActorId::green_spider, "green_spider"},
        {ActorId::white_spider, "white_spider"},
        {ActorId::red_spider, "red_spider"},
        {ActorId::shadow_spider, "shadow_spider"},
        {ActorId::leng_spider, "leng_spider"},
        {ActorId::pit_viper, "pit_viper"},
        {ActorId::spitting_cobra, "spitting_cobra"},
        {ActorId::black_mamba, "black_mamba"},
        {ActorId::mi_go, "mi_go"},
        {ActorId::mi_go_commander, "mi_go_commander"},
        {ActorId::flying_polyp, "flying_polyp"},
        {ActorId::greater_polyp, "greater_polyp"},
        {ActorId::mind_leech, "mind_leech"},
        {ActorId::spirit_leech, "spirit_leech"},
        {ActorId::life_leech, "life_leech"},
        {ActorId::ghoul, "ghoul"},
        {ActorId::shadow, "shadow"},
        {ActorId::invis_stalker, "invis_stalker"},
        {ActorId::wolf, "wolf"},
        {ActorId::fire_hound, "fire_hound"},
        {ActorId::energy_hound, "energy_hound"},
        {ActorId::zuul, "zuul"},
        {ActorId::ghost, "ghost"},
        {ActorId::void_traveler, "void_traveler"},
        {ActorId::elder_void_traveler, "elder_void_traveler"},
        {ActorId::raven, "raven"},
        {ActorId::giant_bat, "giant_bat"},
        {ActorId::vampire_bat, "vampire_bat"},
        {ActorId::abaxu, "abaxu"},
        {ActorId::byakhee, "byakhee"},
        {ActorId::giant_mantis, "giant_mantis"},
        {ActorId::locust, "locust"},
        {ActorId::mummy, "mummy"},
        {ActorId::croc_head_mummy, "croc_head_mummy"},
        {ActorId::khephren, "khephren"},
        {ActorId::nitokris, "nitokris"},
        {ActorId::deep_one, "deep_one"},
        {ActorId::ape, "ape"},
        {ActorId::keziah_mason, "keziah_mason"},
        {ActorId::brown_jenkin, "brown_jenkin"},
        {ActorId::major_clapham_lee, "major_clapham_lee"},
        {ActorId::dean_halsey, "dean_halsey"},
        {ActorId::worm_mass, "worm_mass"},
        {ActorId::mind_worms, "mind_worms"},
        {ActorId::dust_vortex, "dust_vortex"},
        {ActorId::fire_vortex, "fire_vortex"},
        {ActorId::energy_vortex, "energy_vortex"},
        {ActorId::ooze_black, "ooze_black"},
        {ActorId::ooze_clear, "ooze_clear"},
        {ActorId::ooze_putrid, "ooze_putrid"},
        {ActorId::ooze_poison, "ooze_poison"},
        {ActorId::strange_color, "strange_color"},
        {ActorId::chthonian, "chthonian"},
        {ActorId::hunting_horror, "hunting_horror"},
        {ActorId::sentry_drone, "sentry_drone"},
        {ActorId::animated_wpn, "animated_wpn"},
        {ActorId::mold, "mold"},
        {ActorId::gas_spore, "gas_spore"},
        {ActorId::death_fiend, "death_fiend"},
        {ActorId::the_high_priest, "the_high_priest"},
        {ActorId::high_priest_guard_war_vet, "high_priest_guard_war_vet"},
        {ActorId::high_priest_guard_rogue, "high_priest_guard_rogue"},
        {ActorId::high_priest_guard_ghoul, "high_priest_guard_ghoul"}
};

static ActorId get_id(xml::Element* mon_e)
{
        const auto id_search = str_to_actor_id_map.find(
                xml::get_attribute_str(mon_e, "id"));

        ASSERT(id_search != end(str_to_actor_id_map));

        return id_search->second;
}

static void dump_text(xml::Element* text_e, ActorData& data)
{
        data.name_a = xml::get_text_str(
                xml::first_child(text_e, "name_a"));

        data.name_the = xml::get_text_str(
                xml::first_child(text_e, "name_the"));

        data.corpse_name_a = xml::get_text_str(
                xml::first_child(text_e, "corpse_name_a"));

        data.corpse_name_the = xml::get_text_str(
                xml::first_child(text_e, "corpse_name_the"));

        data.descr = xml::get_text_str(
                xml::first_child(text_e, "description"));

        data.allow_generated_descr = xml::get_text_bool(
                xml::first_child(text_e, "allow_generated_description"));

        data.wary_msg = xml::get_text_str(
                xml::first_child(text_e, "wary_message"));

        auto aware_msg_seen_e =
                xml::first_child(text_e, "aware_message_seen");

        data.aware_msg_mon_seen = xml::get_text_str(aware_msg_seen_e);

        xml::try_get_attribute_bool(
                aware_msg_seen_e,
                "use_cultist_messages",
                data.use_cultist_aware_msg_mon_seen);

        auto aware_msg_hidden_e =
                xml::first_child(text_e, "aware_message_hidden");

        data.aware_msg_mon_hidden = xml::get_text_str(aware_msg_hidden_e);

        xml::try_get_attribute_bool(
                aware_msg_hidden_e,
                "use_cultist_messages",
                data.use_cultist_aware_msg_mon_hidden);

        data.spell_cast_msg = xml::get_text_str(
                xml::first_child(text_e, "spell_message"));
}

static void dump_gfx(xml::Element* gfx_e, ActorData& data)
{
        data.tile = str_to_tile_id_map.at(
                xml::get_text_str(
                        xml::first_child(gfx_e, "tile")));

        const std::string char_str = xml::get_text_str(
                xml::first_child(gfx_e, "character"));

        ASSERT(char_str.length() == 1);

        data.character = char_str[0];

        data.color = colors::name_to_color(
                xml::get_text_str(
                        xml::first_child(gfx_e, "color")));
}

static void dump_audio(xml::Element* audio_e, ActorData& data)
{
        data.aware_sfx_mon_seen = str_to_sfx_id_map.at(
                xml::get_text_str(
                        xml::first_child(audio_e, "aware_sfx_seen")));

        data.aware_sfx_mon_hidden = str_to_sfx_id_map.at(
                xml::get_text_str(
                        xml::first_child(audio_e, "aware_sfx_hidden")));
}

static void dump_attributes(xml::Element* attrib_e, ActorData& data)
{
        data.hp = xml::get_text_int(
                xml::first_child(attrib_e, "hit_points"));

        data.spi = xml::get_text_int(
                xml::first_child(attrib_e, "spirit"));

        data.speed_pct = xml::get_text_int(
                xml::first_child(attrib_e, "speed_percent"));

        data.mon_shock_lvl = str_to_shock_lvl_map.at(
                xml::get_text_str(
                        xml::first_child(attrib_e, "shock_level")));

        data.ability_values.set_val(
                AbilityId::melee,
                xml::get_text_int(
                        xml::first_child(attrib_e, "melee")));

        data.ability_values.set_val(
                AbilityId::ranged,
                xml::get_text_int(
                        xml::first_child(attrib_e, "ranged")));

        data.ability_values.set_val(
                AbilityId::dodging,
                xml::get_text_int(
                        xml::first_child(attrib_e, "dodging")));

        data.ability_values.set_val(
                AbilityId::stealth,
                xml::get_text_int(
                        xml::first_child(attrib_e, "stealth")));

        data.ability_values.set_val(
                AbilityId::searching,
                xml::get_text_int(
                        xml::first_child(attrib_e, "searching")));

        data.can_open_doors = xml::get_text_bool(
                xml::first_child(attrib_e, "can_open_doors"));

        data.can_bash_doors = xml::get_text_bool(
                xml::first_child(attrib_e, "can_bash_doors"));

        data.actor_size = str_to_actor_size_map.at(
                xml::get_text_str(xml::first_child(attrib_e, "size")));

        data.prevent_knockback = xml::get_text_bool(
                xml::first_child(attrib_e, "always_prevent_knockback"));

        data.is_humanoid = xml::has_child(attrib_e, "humanoid");

        data.is_rat = xml::has_child(attrib_e, "rat");

        data.is_canine = xml::has_child(attrib_e, "canine");

        data.is_spider = xml::has_child(attrib_e, "spider");

        data.is_undead = xml::has_child(attrib_e, "undead");

        data.is_ghost = xml::has_child(attrib_e, "ghost");

        data.is_ghoul = xml::has_child(attrib_e, "ghoul");

        data.is_snake = xml::has_child(attrib_e, "snake");

        data.is_reptile = xml::has_child(attrib_e, "reptile");

        data.is_amphibian = xml::has_child(attrib_e, "amphibian");

        data.can_bleed = xml::get_text_bool(
                xml::first_child(attrib_e, "can_bleed"));

        data.can_leave_corpse = xml::get_text_bool(
                xml::first_child(attrib_e, "can_leave_corpse"));

        data.prio_corpse_bash = xml::get_text_bool(
                xml::first_child(attrib_e, "prioritize_destroying_corpse"));
}

static void dump_intr_attack_property(xml::Element* property_e,
                                      IntrAttData& attack_data)
{
        const auto prop_id =
                str_to_prop_id_map.at(
                        xml::get_text_str(property_e));

        ItemAttProp& attack_prop = attack_data.prop_applied;

        attack_prop.prop = property_factory::make(prop_id);

        xml::try_get_attribute_int(
                property_e,
                "percent_chance",
                attack_prop.pct_chance_to_apply);

        if ((attack_prop.pct_chance_to_apply <= 0) ||
            (attack_prop.pct_chance_to_apply > 100))
        {
                TRACE_ERROR_RELEASE
                        << "Invalid attack property chance: "
                        << attack_prop.pct_chance_to_apply
                        << std::endl;

                PANIC;
        }

        int duration;

        if (xml::try_get_attribute_int(property_e, "duration", duration))
        {
                attack_prop.prop->set_duration(duration);
        }
        else // Duration not specified as integer
        {
                // Check if duration is specified as string ("indefinite")

                std::string duration_str;

                if (xml::try_get_attribute_str(property_e,
                                               "duration",
                                               duration_str))
                {
                        if (duration_str == "indefinite")
                        {
                                attack_prop.prop->set_indefinite();
                        }
                }
        }
}

static void dump_items(xml::Element* items_e, ActorData& data)
{
        for (auto item_set_e = xml::first_child(items_e);
             item_set_e;
             item_set_e = xml::next_sibling(item_set_e))
        {
                ActorItemSetData item_set;

                const std::string id_str = xml::get_text_str(item_set_e);

                item_set.item_set_id = str_to_item_set_id_map.at(id_str);

                xml::try_get_attribute_int(
                        item_set_e,
                        "percent_chance",
                        item_set.pct_chance_to_spawn);

                xml::try_get_attribute_int(
                        item_set_e,
                        "min",
                        item_set.nr_spawned_range.min);

                xml::try_get_attribute_int(
                        item_set_e,
                        "max",
                        item_set.nr_spawned_range.max);

                data.item_sets.push_back(item_set);
        }
}

static void dump_intr_attacks(xml::Element* attacks_e, ActorData& data)
{
        for (auto attack_e = xml::first_child(attacks_e);
             attack_e;
             attack_e = xml::next_sibling(attack_e))
        {
                IntrAttData attack_data;

                const std::string id_str =
                        xml::get_attribute_str(attack_e, "id");

                attack_data.item_id = str_to_intr_item_id_map.at(id_str);

                auto e = xml::first_child(attack_e);

                attack_data.dmg = xml::get_text_int(e);

                // Propertyies applied
                for (e = xml::next_sibling(e);
                     e;
                     e = xml::next_sibling(e))
                {
                        dump_intr_attack_property(e, attack_data);
                }

                data.intr_attacks.push_back(attack_data);
        }
}

static void dump_spells(xml::Element* spells_e, ActorData& data)
{
        for (auto spell_e = xml::first_child(spells_e);
             spell_e;
             spell_e = xml::next_sibling(spell_e))
        {
                ActorSpellData spell_data;

                const std::string id_str = xml::get_text_str(spell_e);

                spell_data.spell_id = str_to_spell_id_map.at(id_str);

                const std::string skill_str =
                        xml::get_attribute_str(spell_e, "skill");

                spell_data.spell_skill = str_to_spell_skill_map.at(skill_str);

                xml::try_get_attribute_int(
                        spell_e,
                        "percent_chance",
                        spell_data.pct_chance_to_know);

                data.spells.push_back(spell_data);
        }
}

static void dump_properties(xml::Element* properties_e, ActorData& data)
{
        for (auto e = xml::first_child(properties_e);
             e;
             e = xml::next_sibling(e))
        {
                const auto prop_id = str_to_prop_id_map.at(
                        xml::get_text_str(e));

                data.natural_props[(size_t)prop_id] = true;
        }
}

static void dump_ai(xml::Element* ai_e, ActorData& data)
{
        data.erratic_move_pct = xml::get_text_int(
                xml::first_child(ai_e, "erratic_move_percent"));

        data.nr_turns_aware = xml::get_text_int(
                xml::first_child(ai_e, "turns_aware"));

        data.ranged_cooldown_turns = xml::get_text_int(
                xml::first_child(ai_e, "ranged_cooldown_turns"));

        for (size_t i = 0; i < (size_t)AiId::END; ++i)
        {
                const std::string ai_id_str = ai_id_to_str_map.at((AiId)i);

                data.ai[i] = xml::get_text_bool(
                        xml::first_child(ai_e, ai_id_str));
        }
}

static void dump_group_size(xml::Element* group_e, ActorData& data)
{
        const auto group_size = str_to_group_size_map.at(
                xml::get_text_str(group_e));

        int weight = 1;

        xml::try_get_attribute_int(group_e, "weight", weight);

        data.group_sizes.push_back({group_size, weight});
}

static void dump_native_room(xml::Element* native_room_e, ActorData& data)
{
        const auto room_type = str_to_room_type_map.at(
                xml::get_text_str(native_room_e));

        data.native_rooms.push_back(room_type);
}

static void dump_spawning(xml::Element* spawn_e, ActorData& data)
{
        data.spawn_min_dlvl = xml::get_text_int(
                xml::first_child(spawn_e, "min_dungeon_level"));

        data.spawn_max_dlvl = xml::get_text_int(
                xml::first_child(spawn_e, "max_dungeon_level"));

        data.is_auto_spawn_allowed = xml::get_text_bool(
                xml::first_child(spawn_e, "auto_spawn"));

        data.can_be_summoned_by_mon = xml::get_text_bool(
                xml::first_child(spawn_e, "can_be_summoned_by_monster"));

        data.is_unique = xml::has_child(spawn_e, "unique");

        data.nr_left_allowed_to_spawn = xml::get_text_int(
                xml::first_child(spawn_e, "nr_allowed_to_spawn"));

        const std::string group_size_element_str = "group_size";

        for (auto e = xml::first_child(spawn_e, group_size_element_str);
             e;
             e = xml::next_sibling(e, group_size_element_str))
        {
                dump_group_size(e, data);
        }

        const std::string native_room_element_str = "native_room";

        for (auto e = xml::first_child(spawn_e, native_room_element_str);
             e;
             e = xml::next_sibling(e, native_room_element_str))
        {
                dump_native_room(e, data);
        }
}

static void read_actor_definitions_xml()
{
        xml::Doc doc;

        xml::load_file("res/data/monsters.xml", doc);

        auto top_e = xml::first_child(doc);

        auto mon_e = xml::first_child(top_e);

        for ( ; mon_e ; mon_e = xml::next_sibling(mon_e, "monster"))
        {
                const ActorId id = get_id(mon_e);

                ActorData& data = actor_data::data[(size_t)id];

                data.reset();

                data.id = id;

                dump_text(xml::first_child(mon_e, "text"), data);

                dump_gfx(xml::first_child(mon_e, "graphics"), data);

                dump_audio(xml::first_child(mon_e, "audio"), data);

                dump_attributes(xml::first_child(mon_e, "attributes"), data);

                auto items_e = xml::first_child(mon_e, "items");

                if (items_e)
                {
                        dump_items(items_e, data);
                }

                auto attacks_e = xml::first_child(mon_e, "attacks");

                if (attacks_e)
                {
                        dump_intr_attacks(attacks_e, data);
                }

                auto spells_e = xml::first_child(mon_e, "spells");

                if (spells_e)
                {
                        dump_spells(spells_e, data);
                }

                auto props_e = xml::first_child(mon_e, "properties");

                if (props_e)
                {
                        dump_properties(props_e, data);
                }

                auto ai_e = xml::first_child(mon_e, "ai");

                if (ai_e)
                {
                        dump_ai(ai_e, data);
                }

                dump_spawning(xml::first_child(mon_e, "spawning"), data);
        }
} // read_actor_definitions_xml

// -----------------------------------------------------------------------------
// ActorData
// -----------------------------------------------------------------------------
void ActorData::reset()
{
        id = ActorId::END;
        name_a = "";
        name_the = "";
        corpse_name_a = "";
        corpse_name_the = "";
        tile = TileId::empty;
        character = 'X';
        color = colors::yellow();

        // Default spawn group size is "alone"
        group_sizes.assign(
        {
                MonGroupSpawnRule(MonGroupSize::alone, 1)
        });

        hp = 0;
        item_sets.clear();
        intr_attacks.clear();
        spells.clear();
        spi = 0;
        speed_pct = (int)ActorSpeed::normal;

        for (size_t i = 0; i < (size_t)PropId::END; ++i)
        {
                natural_props[i] = false;
        }

        ability_values.reset();

        for (size_t i = 0; i < (size_t)AiId::END; ++i)
        {
                ai[i] = false;
        }

        ai[(size_t)AiId::moves_randomly_when_unaware] = true;

        nr_turns_aware = 0;
        ranged_cooldown_turns = 0;
        spawn_min_dlvl = -1;
        spawn_max_dlvl = -1;
        actor_size = ActorSize::humanoid;
        allow_generated_descr = true;
        nr_kills = 0;
        has_player_seen = false;
        can_open_doors = can_bash_doors = false;
        prevent_knockback = false;
        nr_left_allowed_to_spawn = -1;
        is_unique = false;
        is_auto_spawn_allowed = true;
        wary_msg = "";
        aware_msg_mon_seen = "";
        aware_msg_mon_hidden = "";
        use_cultist_aware_msg_mon_seen = false;
        use_cultist_aware_msg_mon_hidden = false;
        aware_sfx_mon_seen = SfxId::END;
        aware_sfx_mon_hidden = SfxId::END;
        spell_cast_msg = "";
        erratic_move_pct = 0;
        mon_shock_lvl = ShockLvl::none;
        is_humanoid = false;
        is_rat = false;
        is_canine = false;
        is_spider = false;
        is_undead = false;
        is_ghost = false;
        is_ghoul = false;
        is_snake = false;
        is_reptile = false;
        is_amphibian = false;
        can_be_summoned_by_mon = false;
        can_bleed = true;
        can_leave_corpse = true;
        prio_corpse_bash = false;
        native_rooms.clear();
        descr = "";
}

// -----------------------------------------------------------------------------
// actor_data
// -----------------------------------------------------------------------------
namespace actor_data
{

ActorData data[(size_t)ActorId::END];

void init()
{
        TRACE_FUNC_BEGIN;

        read_actor_definitions_xml();

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
