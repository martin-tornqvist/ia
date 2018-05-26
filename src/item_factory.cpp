#include "item_factory.hpp"

#include "init.hpp"
#include "item_scroll.hpp"
#include "item_potion.hpp"
#include "item_rod.hpp"
#include "item_artifact.hpp"
#include "drop.hpp"
#include "item_device.hpp"
#include "item_data.hpp"
#include "game_time.hpp"

namespace item_factory
{

Item* make(const ItemId item_id, const int nr_items)
{
    Item* r = nullptr;

    ItemData* d = &item_data::data[(size_t)item_id];

    switch (item_id)
    {
    case ItemId::trapez:
        r = new Item(d);
        break;

    case ItemId::sawed_off:
    case ItemId::pump_shotgun:
    case ItemId::machine_gun:
    case ItemId::pistol:
    case ItemId::flare_gun:
    case ItemId::spike_gun:
    case ItemId::rock:
    case ItemId::thr_knife:
    case ItemId::iron_spike:
    case ItemId::dagger:
    case ItemId::hatchet:
    case ItemId::club:
    case ItemId::hammer:
    case ItemId::machete:
    case ItemId::axe:
    case ItemId::pitch_fork:
    case ItemId::sledge_hammer:
    case ItemId::trap_dart:
    case ItemId::trap_dart_poison:
    case ItemId::trap_spear:
    case ItemId::trap_spear_poison:
    case ItemId::player_kick:
    case ItemId::player_stomp:
    case ItemId::player_punch:
    case ItemId::intr_bite:
    case ItemId::intr_claw:
    case ItemId::intr_punch:
    case ItemId::intr_zombie_axe:
    case ItemId::intr_acid_spit:
    case ItemId::intr_fire_breath:
    case ItemId::intr_energy_breath:
    case ItemId::intr_strangle:
    case ItemId::intr_ghost_touch:
    case ItemId::intr_sting:
    case ItemId::intr_spear_thrust:
    case ItemId::intr_net_throw:
    case ItemId::intr_maul:
    case ItemId::intr_pus_spew:
    case ItemId::intr_acid_touch:
    case ItemId::intr_fire_engulf:
    case ItemId::intr_energy_engulf:
    case ItemId::intr_spores:
    case ItemId::intr_web_bola:
        r = new Wpn(d);
        break;

    case ItemId::incinerator:
        r = new Incinerator(d);
        break;

    case ItemId::mi_go_gun:
        r = new MiGoGun(d);
        break;

    case ItemId::spiked_mace:
        r = new SpikedMace(d);
        break;

    case ItemId::shotgun_shell:
        r = new Ammo(d);
        break;

    case ItemId::drum_of_bullets:
    case ItemId::pistol_mag:
    case ItemId::incinerator_ammo:
        r = new AmmoMag(d);
        break;

    case ItemId::zombie_dust:
        r = new ZombieDust(d);
        break;

    case ItemId::dynamite:
        r = new Dynamite(d);
        break;

    case ItemId::flare:
        r = new Flare(d);
        break;

    case ItemId::molotov:
        r = new Molotov(d);
        break;

    case ItemId::smoke_grenade:
        r = new SmokeGrenade(d);
        break;

    case ItemId::player_ghoul_claw:
        r = new PlayerGhoulClaw(d);
        break;

    case ItemId::intr_raven_peck:
        r = new RavenPeck(d);
        break;

    case ItemId::intr_vampiric_bite:
        r = new VampiricBite(d);
        break;

    case ItemId::intr_mind_leech_sting:
        r = new MindLeechSting(d);
        break;

    case ItemId::intr_spirit_leech_sting:
        r = new SpiritLeechSting(d);
        break;

    case ItemId::intr_life_leech_sting:
        r = new LifeLeechSting(d);
        break;

    case ItemId::intr_dust_engulf:
        r = new DustEngulf(d);
        break;

    case ItemId::intr_snake_venom_spit:
        r = new SnakeVenomSpit(d);
        break;

    case ItemId::armor_flak_jacket:
    case ItemId::armor_leather_jacket:
    case ItemId::armor_iron_suit:
        r = new Armor(d);
        break;

    case ItemId::armor_asb_suit:
        r = new ArmorAsbSuit(d);
        break;

    case ItemId::armor_mi_go:
        r = new ArmorMiGo(d);
        break;

    case ItemId::gas_mask:
        r = new GasMask(d);
        break;

    case ItemId::scroll_mayhem:
    case ItemId::scroll_telep:
    case ItemId::scroll_pest:
    case ItemId::scroll_enfeeble:
    case ItemId::scroll_searching:
    case ItemId::scroll_bless:
    case ItemId::scroll_darkbolt:
    case ItemId::scroll_aza_wrath:
    case ItemId::scroll_opening:
    case ItemId::scroll_res:
    case ItemId::scroll_summon_mon:
    case ItemId::scroll_light:
    case ItemId::scroll_anim_wpns:
    case ItemId::scroll_transmut:
    case ItemId::scroll_heal:
    case ItemId::scroll_see_invis:
    case ItemId::scroll_spell_shield:
        r = new Scroll(d);
        break;

    case ItemId::potion_vitality:
        r = new PotionVitality(d);
        break;

    case ItemId::potion_spirit:
        r = new PotionSpirit(d);
        break;

    case ItemId::potion_blindness:
        r = new PotionBlindness(d);
        break;

    case ItemId::potion_fortitude:
        r = new PotionFortitude(d);
        break;

    case ItemId::potion_paralyze:
        r = new PotionParal(d);
        break;

    case ItemId::potion_r_elec:
        r = new PotionRElec(d);
        break;

    case ItemId::potion_conf:
        r = new PotionConf(d);
        break;

    case ItemId::potion_poison:
        r = new PotionPoison(d);
        break;

    case ItemId::potion_insight:
        r = new PotionInsight(d);
        break;

    case ItemId::potion_r_fire:
        r = new PotionRFire(d);
        break;

    case ItemId::potion_curing:
        r = new PotionCuring(d);
        break;

    case ItemId::potion_descent:
        r = new PotionDescent(d);
        break;

    case ItemId::potion_invis:
        r = new PotionInvis(d);
        break;

    case ItemId::device_blaster:
        r = new DeviceBlaster(d);
        break;

    case ItemId::device_rejuvenator:
        r = new DeviceRejuvenator(d);
        break;

    case ItemId::device_translocator:
        r = new DeviceTranslocator(d);
        break;

    case ItemId::device_sentry_drone:
        r = new DeviceSentryDrone(d);
        break;

    case ItemId::device_deafening:
        r = new DeviceDeafening(d);
        break;

    case ItemId::lantern:
        r = new DeviceLantern(d);
        break;

    case ItemId::rod_curing:
        r = new RodCuring(d);
        break;

    case ItemId::rod_opening:
        r = new RodOpening(d);
        break;

    case ItemId::rod_bless:
        r = new RodBless(d);
        break;

    case ItemId::rod_cloud_minds:
        r = new RodCloudMinds(d);
        break;

    case ItemId::rod_shockwave:
        r = new RodShockwave(d);
        break;

    case ItemId::medical_bag:
        r = new MedicalBag(d);
        break;

    case ItemId::pharaoh_staff:
        r = new PharaohStaff(d);
        break;

    case ItemId::refl_talisman:
        r = new ReflTalisman(d);
        break;

    case ItemId::resurrect_talisman:
        r = new ResurrectTalisman(d);
        break;

    case ItemId::tele_ctrl_talisman:
        r = new TeleCtrlTalisman(d);
        break;

    case ItemId::horn_of_malice:
        r = new HornOfMalice(d);
        break;

    case ItemId::horn_of_banishment:
        r = new HornOfBanishment(d);
        break;

    case ItemId::clockwork:
        r = new Clockwork(d);
        break;

    case ItemId::spirit_dagger:
        r = new SpiritDagger(d);
        break;

    case ItemId::orb_of_life:
        r = new OrbOfLife(d);
        break;

    case ItemId::END:
            break;
    }

    if (!r)
    {
            return nullptr;
    }

    // Sanity check number of items (non-stackable items should never be set to
    // anything other than one item)
    if (!r->data().is_stackable &&
        (nr_items != 1))
    {
        TRACE << "Specified number of items ("
              << nr_items
              << ") != 1 for "
              << "non-stackable item: "
              << int(d->id) << ", "
              << r->name(ItemRefType::plain)
              << std::endl;

        ASSERT(false);
    }

    r->nr_items_ = nr_items;

    if (d->is_unique)
    {
        d->allow_spawn = false;
    }

    return r;
}

void set_item_randomized_properties(Item* item)
{
    const ItemData& d = item->data();

    ASSERT(d.type != ItemType::melee_wpn_intr &&
           d.type != ItemType::ranged_wpn_intr);

    // If it is a pure melee weapon, and "plus" damage is not already specified
    // randomize the extra damage
    if (d.melee.is_melee_wpn &&
        !d.ranged.is_ranged_wpn &&
        (item->melee_base_dmg_.plus == 0))
    {
        static_cast<Wpn*>(item)->set_random_melee_plus();
    }

    // If firearm, spawn with random amount of ammo
    if (d.ranged.is_ranged_wpn &&
        !d.ranged.has_infinite_ammo)
    {
        Wpn* const wpn = static_cast<Wpn*>(item);

        if (wpn->data().ranged.max_ammo == 1)
        {
            wpn->ammo_loaded_ = rnd::coin_toss() ? 1 : 0;
        }
        else // Weapon ammo capacity > 1
        {
            const int ammo_cap = wpn->data().ranged.max_ammo;

            if (d.ranged.is_machine_gun)
            {
                // Number of machine gun bullets loaded needs to be a multiple
                // of the number of projectiles fired in each burst

                const int cap_scaled = ammo_cap / nr_mg_projectiles;
                const int min_scaled = cap_scaled / 4;

                wpn->ammo_loaded_ =
                    rnd::range(min_scaled, cap_scaled) *
                    nr_mg_projectiles;
            }
            else // Not machinegun
            {
                wpn->ammo_loaded_ = rnd::range(ammo_cap / 4, ammo_cap);
            }
        }
    }

    if (d.is_stackable)
    {
        item->nr_items_ = rnd::range(1, d.max_stack_at_spawn);
    }

    // Vary number of Medical supplies
    if (d.id == ItemId::medical_bag)
    {
        MedicalBag* const medbag = static_cast<MedicalBag*>(item);

        const int nr_supplies_max = medbag->nr_supplies_;

        const int nr_supplies_min = nr_supplies_max - (nr_supplies_max / 3);

        medbag->nr_supplies_ = rnd::range(nr_supplies_min, nr_supplies_max);
    }

    // Vary Lantern duration
    if (d.id == ItemId::lantern)
    {
        DeviceLantern* const lantern = static_cast<DeviceLantern*>(item);

        const int duration_max = lantern->nr_turns_left_;

        const int duration_min = duration_max / 2;

        lantern->nr_turns_left_ = rnd::range(duration_min, duration_max);
    }
}

Item* make_item_on_floor(const ItemId item_id, const P& pos)
{
    Item* item = make(item_id);

    set_item_randomized_properties(item);

    item_drop::drop_item_on_map(pos, *item);

    return item;
}

Item* copy_item(const Item& item_to_copy)
{
    Item* new_item = make(item_to_copy.id());

    *new_item = item_to_copy;

    return new_item;
}

} // item_factory
