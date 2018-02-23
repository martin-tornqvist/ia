#include "actor_items.hpp"

#include <vector>

#include "actor.hpp"
#include "actor_player.hpp"
#include "item_data.hpp"
#include "item_factory.hpp"
#include "item_rod.hpp"
#include "map.hpp"

// TODO: Learning spells should not be done here
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

        // TODO: Learning spells should not be done here
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

static void make_for_monster(Actor& actor)
{
        for (auto& intr_attack : actor.data().intr_attacks)
        {
                auto* item = item_factory::make(intr_attack.item_id);

                // Override damage with the damage in the intrinsic attack data
                // (we always override both melee and ranged damage - it doesn't
                // matter since only one damage type will be used, and the other
                // will have no effect)
                const auto dice = Dice(1, intr_attack.dmg);

                item->melee_base_dmg_ = dice;
                item->ranged_base_dmg_ = dice;

                actor.inv().put_in_intrinsics(item);
        }
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
