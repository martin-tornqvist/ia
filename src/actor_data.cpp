#include "actor_data.hpp"

#include <string>
#include <vector>
#include <math.h>
#include <unordered_map>

#include "saving.hpp"

#include "tinyxml2.h"

void ActorDataT::reset()
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
        MonGroupSpawnRule(MonGroupSize::alone, 1),
    });

    hp = 0;
    dmg_melee = 0;
    dmg_ranged = 0;
    ranged_cooldown_turns = 0;
    spi = 0;
    speed_pct = (int)ActorSpeed::normal;

    for (size_t i = 0; i < (size_t)PropId::END; ++i)
    {
        natural_props[i] = false;
    }

    ability_vals.reset();

    for (size_t i = 0; i < (size_t)AiId::END; ++i)
    {
        ai[i] = false;
    }

    ai[(size_t)AiId::moves_randomly_when_unaware] = true;

    nr_turns_aware = 0;
    spawn_min_dlvl = -1;
    spawn_max_dlvl = -1;
    actor_size = ActorSize::humanoid;
    is_humanoid = false;
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
    aware_sfx_mon_seen = SfxId::END;
    aware_sfx_mon_hidden = SfxId::END;
    spell_cast_msg = "";
    erratic_move_pct = 0;
    mon_shock_lvl = ShockLvl::none;
    is_rat = false;
    is_canine = false;
    is_spider = false;
    is_undead = false;
    is_ghost = false;
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

namespace actor_data
{

ActorDataT data[(size_t)ActorId::END];

namespace
{

// **********************************************************************
// TODO: TEMPORARY CODE
// **********************************************************************
std::unordered_map<std::string, ActorId> str_to_actor_id_map = {
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
    {"phantasm", ActorId::phantasm},
    {"wraith", ActorId::wraith},
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
    {"leng_elder", ActorId::leng_elder},
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

std::unordered_map<ActorId, std::string> actor_id_to_str_map = {
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
    {ActorId::phantasm, "phantasm"},
    {ActorId::wraith, "wraith"},
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
    {ActorId::leng_elder, "leng_elder"},
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

// TODO: Some of the xml handling functionality here should be moved to a
// common "xml" namespace
void read_actor_definitions_xml()
{
    tinyxml2::XMLDocument doc;

    const std::string xml_path = "res/data/monsters.xml";

    TRACE << "Loading actor definitions file: " << xml_path << std::endl;

    const auto load_result = doc.LoadFile(xml_path.c_str());

    if (load_result != tinyxml2::XML_SUCCESS)
    {
        TRACE_ERROR_RELEASE
            << "Could not load actor definitions file at: "
            << xml_path
            << std::endl;

        PANIC

        return;
    }

    TRACE << "Reading actor definitions" << std::endl;

    auto get_text_str = [](tinyxml2::XMLElement* const e)
    {
        std::string str = "";

        const char* c_str = e->GetText();

        if (c_str)
        {
            str = c_str;
        }

        return str;
    };

    auto get_text_bool = [](tinyxml2::XMLElement* const e)
    {
        bool value = false;

        const auto result = e->QueryBoolText(&value);

        if (result != tinyxml2::XML_SUCCESS)
        {
            TRACE_ERROR_RELEASE
                << "While parsing boolean value from xml element "
                << "\"" << e->Value() << "\", "
                << "tinyxml2 reported error code: "
                << result << std::endl;

            PANIC
        }

        return value;
    };

    auto get_text_int = [](tinyxml2::XMLElement* const e)
    {
        int value = false;

        const auto result = e->QueryIntText(&value);

        if (result != tinyxml2::XML_SUCCESS)
        {
            TRACE_ERROR_RELEASE
                << "While parsing integer value from xml element "
                << "\"" << e->Value() << "\", "
                << "tinyxml2 reported error code: "
                << result << std::endl;

            PANIC
        }

        return value;
    };

    auto mon_e = doc.FirstChildElement()->FirstChildElement();

    for ( ; mon_e ; mon_e = mon_e->NextSiblingElement())
    {
        ASSERT(std::string(mon_e->Name()) == "monster");

        const std::string id_str = mon_e->Attribute("id");

        const auto id_search = str_to_actor_id_map.find(id_str);

        ASSERT(id_search != end(str_to_actor_id_map));

        const auto id = id_search->second;

        ActorDataT& d = data[(size_t)id];

        d.reset();

        d.id = id;

        // ---------------------------------------------------------------------
        // Text
        // ---------------------------------------------------------------------
        auto text_e = mon_e->FirstChildElement("text");

        d.name_a = get_text_str(
            text_e->FirstChildElement("name_a"));

        d.name_the = get_text_str(
            text_e->FirstChildElement("name_the"));

        d.corpse_name_a = get_text_str(
            text_e->FirstChildElement("corpse_name_a"));

        d.corpse_name_the = get_text_str(
            text_e->FirstChildElement("corpse_name_the"));

        d.descr = get_text_str(
            text_e->FirstChildElement("description"));

        d.allow_generated_descr = get_text_bool(
            text_e->FirstChildElement("allow_generated_description"));

        d.wary_msg = get_text_str(
            text_e->FirstChildElement("wary_message"));

        d.aware_msg_mon_seen = get_text_str(
            text_e->FirstChildElement("aware_message_seen"));

        d.aware_msg_mon_hidden = get_text_str(
            text_e->FirstChildElement("aware_message_hidden"));

        d.spell_cast_msg = get_text_str(
            text_e->FirstChildElement("spell_message"));

        // ---------------------------------------------------------------------
        // Graphics
        // ---------------------------------------------------------------------
        auto gfx_e = mon_e->FirstChildElement("graphics");

        d.tile = str_to_tile_id_map.at(
            get_text_str(gfx_e->FirstChildElement("tile")));

        const std::string char_str = get_text_str(
            gfx_e->FirstChildElement("character"));

        ASSERT(char_str.length() == 1);

        d.character = char_str[0];

        d.color = colors::name_to_color(
            get_text_str(gfx_e->FirstChildElement("color")));

        // ---------------------------------------------------------------------
        // Audio
        // ---------------------------------------------------------------------
        auto audio_e = mon_e->FirstChildElement("audio");

        d.aware_sfx_mon_seen = str_to_sfx_id_map.at(
            get_text_str(audio_e->FirstChildElement("aware_sfx_seen")));

        d.aware_sfx_mon_hidden = str_to_sfx_id_map.at(
            get_text_str(audio_e->FirstChildElement("aware_sfx_hidden")));

        // ---------------------------------------------------------------------
        // Attributes
        // ---------------------------------------------------------------------
        auto attrib_e = mon_e->FirstChildElement("attributes");

        d.hp = get_text_int(
            attrib_e->FirstChildElement("hit_points"));

        d.spi = get_text_int(
            attrib_e->FirstChildElement("spirit"));

        d.dmg_melee = get_text_int(
            attrib_e->FirstChildElement("melee_damage"));

        d.dmg_ranged = get_text_int(
            attrib_e->FirstChildElement("ranged_damage"));

        d.speed_pct = get_text_int(
            attrib_e->FirstChildElement("speed_percent"));

        d.mon_shock_lvl = str_to_shock_lvl_map.at(
            get_text_str(attrib_e->FirstChildElement("shock_level")));

        d.ability_vals.set_val(
            AbilityId::melee,
            get_text_int(attrib_e->FirstChildElement("melee")));

        d.ability_vals.set_val(
            AbilityId::ranged,
            get_text_int(attrib_e->FirstChildElement("ranged")));

        d.ability_vals.set_val(
            AbilityId::dodging,
            get_text_int(attrib_e->FirstChildElement("dodging")));

        d.ability_vals.set_val(
            AbilityId::stealth,
            get_text_int(attrib_e->FirstChildElement("stealth")));

        d.ability_vals.set_val(
            AbilityId::searching,
            get_text_int(attrib_e->FirstChildElement("searching")));

        d.can_open_doors = get_text_bool(
            attrib_e->FirstChildElement("can_open_doors"));

        d.can_bash_doors = get_text_bool(
            attrib_e->FirstChildElement("can_bash_doors"));

        d.actor_size = str_to_actor_size_map.at(
            get_text_str(attrib_e->FirstChildElement("size")));

        d.prevent_knockback = get_text_bool(
            attrib_e->FirstChildElement("always_prevent_knockback"));

        d.is_rat = get_text_bool(
            attrib_e->FirstChildElement("is_rat"));

        d.is_canine = get_text_bool(
            attrib_e->FirstChildElement("is_canine"));

        d.is_spider = get_text_bool(
            attrib_e->FirstChildElement("is_spider"));

        d.is_undead = get_text_bool(
            attrib_e->FirstChildElement("is_undead"));

        d.is_ghost = get_text_bool(
            attrib_e->FirstChildElement("is_ghost"));

        d.is_snake = get_text_bool(
            attrib_e->FirstChildElement("is_snake"));

        d.is_reptile = get_text_bool(
            attrib_e->FirstChildElement("is_reptile"));

        d.is_amphibian = get_text_bool(
            attrib_e->FirstChildElement("is_amphibian"));

        d.can_bleed = get_text_bool(
            attrib_e->FirstChildElement("can_bleed"));

        d.can_leave_corpse = get_text_bool(
            attrib_e->FirstChildElement("can_leave_corpse"));

        d.prio_corpse_bash = get_text_bool(
            attrib_e->FirstChildElement("prioritize_destroying_corpse"));

        // ---------------------------------------------------------------------
        // Properties
        // ---------------------------------------------------------------------
        auto props_e = mon_e->FirstChildElement("properties");

        if (props_e)
        {
            for (auto e = props_e->FirstChildElement();
                 e != nullptr;
                 e = e->NextSiblingElement())
            {
                const auto prop_id = str_to_prop_id_map.at(get_text_str(e));

                d.natural_props[(size_t)prop_id] = true;
            }
        }

        // ---------------------------------------------------------------------
        // AI
        // ---------------------------------------------------------------------
        auto ai_e = mon_e->FirstChildElement("ai");

        if (ai_e)
        {
            d.erratic_move_pct = get_text_int(
                ai_e->FirstChildElement("erratic_move_percent"));

            d.nr_turns_aware = get_text_int(
                ai_e->FirstChildElement("turns_aware"));

            d.ranged_cooldown_turns = get_text_int(
                ai_e->FirstChildElement("ranged_cooldown_turns"));

            for (size_t i = 0; i < (size_t)AiId::END; ++i)
            {
                const std::string ai_id_str = ai_id_to_str_map.at((AiId)i);

                d.ai[i] = get_text_bool(
                    ai_e->FirstChildElement(ai_id_str.c_str()));
            }
        }

        // ---------------------------------------------------------------------
        // Spawning
        // ---------------------------------------------------------------------
        auto spawning_e = mon_e->FirstChildElement("spawning");

        d.spawn_min_dlvl = get_text_int(
            spawning_e->FirstChildElement("min_dungeon_level"));

        d.spawn_max_dlvl = get_text_int(
            spawning_e->FirstChildElement("max_dungeon_level"));

        d.is_auto_spawn_allowed = get_text_bool(
            spawning_e->FirstChildElement("auto_spawn"));

        d.can_be_summoned_by_mon = get_text_bool(
            spawning_e->FirstChildElement("can_be_summoned_by_monster"));

        d.is_unique = get_text_bool(
            spawning_e->FirstChildElement("is_unique"));

        d.nr_left_allowed_to_spawn = get_text_int(
            spawning_e->FirstChildElement("nr_left_allowed_to_spawn"));

        const char* group_size_element_str = "group_size";

        for (auto e = spawning_e->FirstChildElement(group_size_element_str);
             e;
             e = e->NextSiblingElement(group_size_element_str))
        {
            const auto group_size = str_to_group_size_map.at(get_text_str(e));

            int weight = 0;

            const auto result = e->QueryAttribute("weight", &weight);

            if (result != tinyxml2::XML_SUCCESS)
            {
                weight = 1;
            }

            d.group_sizes.push_back({group_size, weight});
        }

        const char* native_room_element_str = "native_room";

        for (auto e = spawning_e->FirstChildElement(native_room_element_str);
             e;
             e = e->NextSiblingElement(native_room_element_str))
        {
            const auto room_type = str_to_room_type_map.at(get_text_str(e));

            d.native_rooms.push_back(room_type);
        }
    }
} // read_actor_definitions_xml

// TODO: Some of the xml handling functionality here should be moved to a
// common "xml" namespace
/*
void dump_actor_definitions_to_xml()

    tinyxml2::XMLDocument xml_doc;

    auto xml_root = xml_doc.NewElement("monsters");

    xml_doc.InsertFirstChild(xml_root);

    for (auto& d : data)
    {
        const auto id_search = actor_id_to_str_map.find(d.id);

        const std::string id_str = id_search->second;

        const std::string comment_line =
            "************************************************************";

        xml_root->InsertEndChild(
            xml_doc.NewComment(comment_line.c_str()));

        xml_root->InsertEndChild(
            xml_doc.NewComment((" " + id_str + " ").c_str()));

        xml_root->InsertEndChild(
            xml_doc.NewComment(comment_line.c_str()));

        auto mon_e = xml_doc.NewElement("monster");
        mon_e->SetAttribute("id", id_str.c_str());
        xml_root->InsertEndChild(mon_e);

        auto add_e_str = [&](const std::string& name,
                             const std::string& text,
                             tinyxml2::XMLElement* parent)
        {
            auto e = xml_doc.NewElement(name.c_str());
            e->SetText(text.c_str());
            parent->InsertEndChild(e);
        };

        auto add_e_int = [&](const std::string& name,
                             const int value,
                             tinyxml2::XMLElement* parent)
        {
            auto e = xml_doc.NewElement(name.c_str());
            e->SetText(value);
            parent->InsertEndChild(e);
        };

        auto add_e_bool = [&](const std::string& name,
                              const bool value,
                              tinyxml2::XMLElement* parent)
        {
            auto e = xml_doc.NewElement(name.c_str());
            e->SetText(value);
            parent->InsertEndChild(e);
        };

        auto text_e = xml_doc.NewElement("text");
        mon_e->InsertEndChild(text_e);

        auto gfx_e = xml_doc.NewElement("graphics");
        mon_e->InsertEndChild(gfx_e);

        auto audio_e = xml_doc.NewElement("audio");
        mon_e->InsertEndChild(audio_e);

        auto attrib_e = xml_doc.NewElement("attributes");
        mon_e->InsertEndChild(attrib_e);

        auto props_e = xml_doc.NewElement("properties");
        mon_e->InsertEndChild(props_e);

        auto ai_e = xml_doc.NewElement("ai");
        mon_e->InsertEndChild(ai_e);

        auto spawn_e = xml_doc.NewElement("spawning");
        mon_e->InsertEndChild(spawn_e);

        // ---------------------------------------------------------------------
        // Text
        // ---------------------------------------------------------------------
        add_e_str("name_a", d.name_a, text_e);
        add_e_str("name_the", d.name_the, text_e);
        add_e_str("corpse_name_a", d.corpse_name_a, text_e);
        add_e_str("corpse_name_the", d.corpse_name_the, text_e);
        add_e_str("description", d.descr, text_e);
        add_e_bool("allow_generated_description",
                   d.allow_generated_descr,
                   text_e);
        add_e_str("wary_message", d.wary_msg, text_e);
        add_e_str("aware_message_seen", d.aware_msg_mon_seen, text_e);
        add_e_str("aware_message_hidden", d.aware_msg_mon_hidden, text_e);
        add_e_str("spell_message", d.spell_cast_msg, text_e);

        // ---------------------------------------------------------------------
        // Graphics
        // ---------------------------------------------------------------------
        add_e_str("tile", tile_id_to_str_map.at(d.tile), gfx_e);
        add_e_str("character", std::string(1, d.character), gfx_e);
        add_e_str("color", colors::color_to_name(d.color), gfx_e);

        // ---------------------------------------------------------------------
        // Audio
        // ---------------------------------------------------------------------
        if (d.aware_sfx_mon_seen != SfxId::END)
        {
            add_e_str("aware_sfx_seen",
                      sfx_id_to_str_map.at(d.aware_sfx_mon_seen),
                      audio_e);
        }

        if (d.aware_sfx_mon_hidden != SfxId::END)
        {
            add_e_str("aware_sfx_hidden",
                      sfx_id_to_str_map.at(d.aware_sfx_mon_hidden),
                      audio_e);
        }

        // ---------------------------------------------------------------------
        // Attributes
        // ---------------------------------------------------------------------
        add_e_int("hit_points", d.hp, attrib_e);
        add_e_int("spirit", d.spi, attrib_e);
        add_e_int("melee_damage", d.dmg_melee, attrib_e);
        add_e_int("ranged_damage", d.dmg_ranged, attrib_e);
        add_e_int("speed_percent", d.speed_pct, attrib_e);
        add_e_str("shock_level",
                  shock_lvl_to_str_map.at(d.mon_shock_lvl),
                  attrib_e);

        for (size_t i = 0; i < (size_t)AbilityId::END; ++i)
        {
            const auto ability_id = (AbilityId)i;

            add_e_int(ability_id_to_str_map.at(ability_id),
                      d.ability_vals.raw_val(ability_id),
                      attrib_e);
        }

        add_e_bool("can_open_doors", d.can_open_doors, attrib_e);
        add_e_bool("can_bash_doors", d.can_bash_doors, attrib_e);
        add_e_str("size", actor_size_to_str_map.at(d.actor_size), attrib_e);
        add_e_bool("always_prevent_knockback", d.prevent_knockback, attrib_e);
        add_e_bool("is_humanoid", d.is_humanoid, attrib_e);
        add_e_bool("is_rat", d.is_rat, attrib_e);
        add_e_bool("is_canine", d.is_canine, attrib_e);
        add_e_bool("is_spider", d.is_spider, attrib_e);
        add_e_bool("is_undead", d.is_undead, attrib_e);
        add_e_bool("is_ghost", d.is_ghost, attrib_e);
        add_e_bool("is_snake", d.is_snake, attrib_e);
        add_e_bool("is_reptile", d.is_reptile, attrib_e);
        add_e_bool("is_amphibian", d.is_amphibian, attrib_e);
        add_e_bool("can_bleed", d.can_bleed, attrib_e);
        add_e_bool("can_leave_corpse", d.can_leave_corpse, attrib_e);
        add_e_bool("prioritize_destroying_corpse",
                   d.prio_corpse_bash,
                   attrib_e);

        // ---------------------------------------------------------------------
        // Properties
        // ---------------------------------------------------------------------
        for (size_t i = 0; i < (size_t)PropId::END; ++i)
        {
            if (d.natural_props[i])
            {
                const auto prop_id = (PropId)i;

                add_e_str("property", prop_id_to_str_map.at(prop_id), props_e);
            }
        }

        // ---------------------------------------------------------------------
        // AI
        // ---------------------------------------------------------------------
        add_e_int("erratic_move_percent", (int)d.erratic_move_pct, ai_e);
        add_e_int("turns_aware", d.nr_turns_aware, ai_e);
        add_e_int("ranged_cooldown_turns", d.ranged_cooldown_turns, ai_e);

        for (size_t i = 0; i < (size_t)AiId::END; ++i)
        {
            const auto ai_id = (AiId)i;

            add_e_bool(ai_id_to_str_map.at(ai_id), d.ai[i], ai_e);
        }

        // ---------------------------------------------------------------------
        // Spawning
        // ---------------------------------------------------------------------
        add_e_int("min_dungeon_level", d.spawn_min_dlvl, spawn_e);
        add_e_int("max_dungeon_level", d.spawn_max_dlvl, spawn_e);
        add_e_bool("auto_spawn", d.is_auto_spawn_allowed, spawn_e);
        add_e_bool("can_be_summoned_by_monster",
                   d.can_be_summoned_by_mon,
                   spawn_e);
        add_e_bool("is_unique", d.is_unique, spawn_e);
        add_e_int("nr_left_allowed_to_spawn",
                  d.nr_left_allowed_to_spawn,
                  spawn_e);
        for (const auto& group : d.group_sizes)
        {
            auto e = xml_doc.NewElement("group_size");

            e->SetText(
                group_size_to_str_map.at(group.group_size).c_str());

            if (d.group_sizes.size() > 1)
            {
                e->SetAttribute("weight", group.weight);
            }

            spawn_e->InsertEndChild(e);
        }

        for (size_t i = 0; i < d.native_rooms.size(); ++i)
        {
            const auto room_type = d.native_rooms[i];

            add_e_str("native_room",
                      room_type_to_str_map.at(room_type),
                      spawn_e);
        }
    }

    xml_doc.SaveFile("monsters.xml");
} // dump_actor_definitions_to_xml
*/

} // namespace

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
