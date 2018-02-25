#include "actor_items.hpp"

#include <vector>

#include "actor.hpp"
#include "actor_player.hpp"
#include "item_data.hpp"
#include "item_factory.hpp"
#include "item_rod.hpp"
#include "map.hpp"
#include "player_spells.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static void make_for_player_occultist()
{
        auto& inv = map::player->inv();

        inv.put_in_slot(
                SlotId::wpn,
                item_factory::make(ItemId::hatchet),
                Verbosity::silent);

        inv.put_in_slot(
                SlotId::wpn_alt,
                item_factory::make(ItemId::pistol),
                Verbosity::silent);

        inv.put_in_backpack(item_factory::make(ItemId::pistol_mag));

        inv.put_in_slot(
                SlotId::body,
                item_factory::make(ItemId::armor_leather_jacket),
                Verbosity::silent);

        Item* spirit_pot = item_factory::make(ItemId::potion_spirit);

        spirit_pot->identify(Verbosity::silent);

        game::incr_player_xp(
                spirit_pot->data().xp_on_found,
                Verbosity::silent);

        spirit_pot->data().is_found = true;

        inv.put_in_backpack(spirit_pot);

        player_spells::learn_spell(SpellId::darkbolt, Verbosity::silent);
        player_spells::learn_spell(SpellId::searching, Verbosity::silent);

        const std::vector<ItemId> scrolls_identified =
        {
                ItemId::scroll_darkbolt,
                ItemId::scroll_searching
        };

        for (auto scroll_id : scrolls_identified)
        {
                std::unique_ptr<Item> temp_scroll(
                        item_factory::make(scroll_id));

                temp_scroll->identify(Verbosity::silent);

                game::incr_player_xp(
                        temp_scroll->data().xp_on_found,
                        Verbosity::silent);

                temp_scroll->data().is_found = true;
        }

        map::player->set_unarmed_wpn(
                static_cast<Wpn*>(
                        item_factory::make(ItemId::player_punch)));

        inv.put_in_backpack(item_factory::make(ItemId::dynamite, 1));
        inv.put_in_backpack(item_factory::make(ItemId::molotov, 1));
        inv.put_in_backpack(item_factory::make(ItemId::medical_bag));
        inv.put_in_backpack(item_factory::make(ItemId::lantern));
}

static void make_for_player_rogue()
{
        auto& inv = map::player->inv();

        auto* const dagger = item_factory::make(ItemId::dagger);

        static_cast<Wpn*>(dagger)->melee_base_dmg_.plus = 1;

        inv.put_in_slot(SlotId::wpn,
                        dagger,
                        Verbosity::silent);

        inv.put_in_slot(SlotId::wpn_alt,
                        item_factory::make(ItemId::pistol),
                        Verbosity::silent);

        for (int i = 0; i < 2; ++i)
        {
                inv.put_in_backpack(item_factory::make(ItemId::pistol_mag));
        }

        inv.put_in_slot(SlotId::body,
                        item_factory::make(ItemId::armor_leather_jacket),
                        Verbosity::silent);

        inv.put_in_backpack(item_factory::make(ItemId::iron_spike, 12));

        Item* rod_cloud_minds = item_factory::make(ItemId::rod_cloud_minds);

        rod_cloud_minds->identify(Verbosity::silent);

        game::incr_player_xp(rod_cloud_minds->data().xp_on_found,
                             Verbosity::silent);

        rod_cloud_minds->data().is_found = true;

        inv.put_in_backpack(rod_cloud_minds);

        map::player->set_unarmed_wpn(
                static_cast<Wpn*>(
                        item_factory::make(ItemId::player_punch)));

        inv.put_in_backpack(item_factory::make(ItemId::dynamite, 2));
        inv.put_in_backpack(item_factory::make(ItemId::molotov, 2));
        inv.put_in_backpack(item_factory::make(ItemId::medical_bag));
        inv.put_in_backpack(item_factory::make(ItemId::lantern));

        Item* const throwing_knives = item_factory::make(ItemId::thr_knife, 12);

        inv.put_in_backpack(throwing_knives);

        map::player->thrown_item_ = throwing_knives;
}

static void make_for_player_war_vet()
{
        auto& inv = map::player->inv();

        inv.put_in_slot(
                SlotId::wpn,
                item_factory::make(ItemId::machete),
                Verbosity::silent);

        for (int i = 0; i < 3; ++i)
        {
                inv.put_in_backpack(item_factory::make(ItemId::pistol_mag));
        }

        inv.put_in_slot(
                SlotId::wpn_alt,
                item_factory::make(ItemId::pistol),
                Verbosity::silent);

        inv.put_in_slot(
                SlotId::body,
                item_factory::make(ItemId::armor_flak_jacket),
                Verbosity::silent);

        map::player->set_unarmed_wpn(
                static_cast<Wpn*>(
                        item_factory::make(ItemId::player_punch)));

        inv.put_in_backpack(item_factory::make(ItemId::dynamite, 2));
        inv.put_in_backpack(item_factory::make(ItemId::molotov, 2));
        inv.put_in_backpack(item_factory::make(ItemId::smoke_grenade, 4));
        inv.put_in_backpack(item_factory::make(ItemId::flare, 2));
        inv.put_in_backpack(item_factory::make(ItemId::medical_bag));
        inv.put_in_backpack(item_factory::make(ItemId::lantern));
        inv.put_in_backpack(item_factory::make(ItemId::gas_mask));

        Item* const throwing_knives = item_factory::make(ItemId::thr_knife, 6);

        inv.put_in_backpack(throwing_knives);

        map::player->thrown_item_ = throwing_knives;
}

static void make_for_player_ghoul()
{
        map::player->set_unarmed_wpn(
                static_cast<Wpn*>(
                        item_factory::make(ItemId::player_ghoul_claw)));
}

static void make_for_player()
{
        const Bg bg = player_bon::bg();

        switch (bg)
        {
        case Bg::occultist:
        {
                make_for_player_occultist();
        }
        break;

        case Bg::rogue:
        {
                make_for_player_rogue();
        }
        break;

        case Bg::war_vet:
        {
                make_for_player_war_vet();
        }
        break;

        case Bg::ghoul:
        {
                make_for_player_ghoul();
        }
        break;

        case Bg::END:
                break;
        }
}

static void make_random_item_to_backpack(Actor& actor,
                                         std::vector<ItemId>& item_id_bucket)
{
        if (!item_id_bucket.empty())
        {
                const ItemId item_id = rnd::element(item_id_bucket);

                Item* item = item_factory::make(item_id);

                actor.inv().put_in_backpack(item);
        }
}

static void make_item_set_minor_treasure(Actor& actor)
{
        std::vector<ItemId> item_bucket;

        for (int i = 0; i < (int)ItemId::END; ++i)
        {
                const ItemData& d = item_data::data[i];

                if (d.value == ItemValue::minor_treasure)
                {
                        item_bucket.push_back((ItemId)i);
                }
        }

        make_random_item_to_backpack(actor, item_bucket);
}

static void make_item_set_rare_treasure(Actor& actor)
{
        std::vector<ItemId> item_bucket;

        for (int i = 0; i < (int)ItemId::END; ++i)
        {
                const ItemData& d = item_data::data[i];

                if (d.value == ItemValue::rare_treasure)
                {
                        item_bucket.push_back((ItemId)i);
                }
        }

        make_random_item_to_backpack(actor, item_bucket);
}

static void make_item_set_supreme_treasure(Actor& actor)
{
        std::vector<ItemId> item_bucket;

        for (int i = 0; i < (int)ItemId::END; ++i)
        {
                const ItemData& d = item_data::data[i];

                if (d.value == ItemValue::supreme_treasure)
                {
                        item_bucket.push_back((ItemId)i);
                }
        }

        make_random_item_to_backpack(actor, item_bucket);
}

static void make_item_set_firearm(Actor& actor)
{
        Inventory& inv = actor.inv();

        // If we are on an early dungeon level, lean heavily towards pistols
        const bool is_low_dlvl = map::dlvl < 4;

        std::vector<int> weights =
        {
                (is_low_dlvl ? 20 : 6), // Pistol
                3,                      // Pump shotgun
                3,                      // Sawed-off shotgun
                1                       // Machine Gun
        };

        const int choice = rnd::weighted_choice(weights);

        switch (choice)
        {
        case 0:
        {
                // Pistol
                Item* item = item_factory::make(ItemId::pistol);

                Wpn* wpn = static_cast<Wpn*>(item);

                const int ammo_cap = wpn->data().ranged.max_ammo;

                wpn->ammo_loaded_ = rnd::range(ammo_cap / 2, ammo_cap);

                inv.put_in_slot(SlotId::wpn, item, Verbosity::silent);

                if (rnd::coin_toss())
                {
                        inv.put_in_backpack(
                                item_factory::make(ItemId::pistol_mag));
                }
        }
        break;

        case 1:
        {
                // Pump shotgun
                Item* item = item_factory::make(ItemId::pump_shotgun);

                Wpn* wpn = static_cast<Wpn*>(item);

                const int ammo_cap = wpn->data().ranged.max_ammo;

                wpn->ammo_loaded_ = rnd::range(ammo_cap / 2, ammo_cap);

                inv.put_in_slot(SlotId::wpn, item, Verbosity::silent);

                item = item_factory::make(ItemId::shotgun_shell);

                item->nr_items_ = rnd::range(1, 6);

                inv.put_in_backpack(item);
        }
        break;

        case 2:
        {
                // Sawed-off shotgun
                inv.put_in_slot(SlotId::wpn,
                                item_factory::make(ItemId::sawed_off),
                                Verbosity::silent);

                Item* item = item_factory::make(ItemId::shotgun_shell);

                item->nr_items_ = rnd::range(1, 6);

                inv.put_in_backpack(item);
        }
        break;

        case 3:
        {
                // Tommy Gun

                // Number of bullets loaded needs to be a multiple of the number
                // of projectiles fired in each burst
                Item* item = item_factory::make(ItemId::machine_gun);

                Wpn* const wpn = static_cast<Wpn*>(item);

                const int cap_scaled =
                        wpn->data().ranged.max_ammo /
                        nr_mg_projectiles;

                const int min_scaled = cap_scaled / 2;

                wpn->ammo_loaded_ =
                        rnd::range(min_scaled, cap_scaled) * nr_mg_projectiles;

                inv.put_in_slot(SlotId::wpn, item, Verbosity::silent);
        }
        break;
        }
}

static void make_item_set_spike_gun(Actor& actor)
{
        Inventory& inv = actor.inv();

        {
                Item* item = item_factory::make(ItemId::spike_gun);

                Wpn* wpn = static_cast<Wpn*>(item);

                const int ammo_cap = wpn->data().ranged.max_ammo;

                wpn->ammo_loaded_ = rnd::range(ammo_cap / 2, ammo_cap);

                inv.put_in_slot(SlotId::wpn, item, Verbosity::silent);
        }

        {
                Item* item = item_factory::make(ItemId::iron_spike);

                item->nr_items_ = rnd::range(1, 6);

                inv.put_in_backpack(item);
        }
}

static void make_item_set_priest_dagger(Actor& actor)
{
        Item* item = item_factory::make(ItemId::dagger);

        const std::vector<int> weights =
        {
                6,
                3,
                1
        };

        item->melee_base_dmg_.plus = rnd::weighted_choice(weights) + 1;

        actor.inv().put_in_slot(SlotId::wpn, item, Verbosity::silent);
}

static void make_item_set_mi_go_gun(Actor& actor)
{
        actor.inv().put_in_slot(
                SlotId::wpn,
                item_factory::make(ItemId::mi_go_gun),
                Verbosity::silent);
}

static void make_item_set_mi_go_armor(Actor& actor)
{
        actor.inv().put_in_slot(
                SlotId::body,
                item_factory::make(ItemId::armor_mi_go),
                Verbosity::silent);
}

static void make_item_set_high_priest_guard_war_vet(Actor& actor)
{
        actor.inv().put_in_slot(
                SlotId::wpn,
                item_factory::make(ItemId::machine_gun),
                Verbosity::silent);
}

static void make_item_set_high_priest_guard_rogue(Actor& actor)
{
        Item* const item = item_factory::make(ItemId::machete);

        item->melee_base_dmg_.plus = 1;

        actor.inv().put_in_slot(
                SlotId::wpn,
                item,
                Verbosity::silent);
}

static void make_item_sets(Actor& actor)
{
        for (const ActorItemSetData& item_set : actor.data().item_sets)
        {
                if (!rnd::percent(item_set.pct_chance_to_spawn))
                {
                        continue;
                }

                const int nr = item_set.nr_spawned_range.roll();

                for (int i = 0; i < nr; ++i)
                {
                        switch (item_set.item_set_id)
                        {
                        case ItemSetId::minor_treasure:
                                make_item_set_minor_treasure(actor);
                                break;

                        case ItemSetId::rare_treasure:
                                make_item_set_rare_treasure(actor);
                                break;

                        case ItemSetId::supreme_treasure:
                                make_item_set_supreme_treasure(actor);
                                break;

                        case ItemSetId::firearm:
                                make_item_set_firearm(actor);
                                break;

                        case ItemSetId::spike_gun:
                                make_item_set_spike_gun(actor);
                                break;

                        case ItemSetId::priest_dagger:
                                make_item_set_priest_dagger(actor);
                                break;

                        case ItemSetId::mi_go_gun:
                                make_item_set_mi_go_gun(actor);
                                break;

                        case ItemSetId::mi_go_armor:
                                make_item_set_mi_go_armor(actor);
                                break;

                        case ItemSetId::high_priest_guard_war_vet:
                                make_item_set_high_priest_guard_war_vet(actor);
                                break;

                        case ItemSetId::high_priest_guard_rogue:
                                make_item_set_high_priest_guard_rogue(actor);
                                break;
                        }
                }
        }
}

static void make_intr_attacks(Actor& actor)
{
        for (auto& intr_attack : actor.data().intr_attacks)
        {
                auto* item = item_factory::make(intr_attack.item_id);

                // Override damage with the damage in the intrinsic attack data
                // (we always override both melee and ranged damage - this
                // doesn't matter, since only one damage type will be used and
                // the other will have no effect)
                const auto dice = Dice(1, intr_attack.dmg);

                item->melee_base_dmg_ = dice;
                item->ranged_base_dmg_ = dice;

                actor.inv().put_in_intrinsics(item);
        }
}

static void make_for_monster(Actor& actor)
{
        make_item_sets(actor);

        make_intr_attacks(actor);
}

// -----------------------------------------------------------------------------
// actor_items
// -----------------------------------------------------------------------------
namespace actor_items
{

void make_for_actor(Actor& actor)
{
        if (actor.is_player())
        {
                make_for_player();
        }
        else // Not player
        {
                make_for_monster(actor);
        }
}

} // actor_items
