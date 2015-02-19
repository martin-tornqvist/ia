#include "item_factory.hpp"

#include "init.hpp"
#include "item_scroll.hpp"
#include "item_potion.hpp"
#include "item_jewelry.hpp"
#include "drop.hpp"
#include "item_device.hpp"
#include "utils.hpp"
#include "item_data.hpp"

using namespace std;

namespace Item_factory
{

Item* mk(const Item_id item_id, const int NR_ITEMS)
{
    Item*             r     = nullptr;
    Item_data_t* const  d     = Item_data::data[int(item_id)];
    Item_data_t*        ammo_d = nullptr;

    if (d->ranged.ammo_item_id != Item_id::END)
    {
        ammo_d = Item_data::data[int(d->ranged.ammo_item_id)];
    }

    switch (item_id)
    {
    case Item_id::trapezohedron:
        r = new Item(d);
        break;

    case Item_id::rock:
    case Item_id::thr_knife:
    case Item_id::iron_spike:
    case Item_id::dagger:
    case Item_id::hatchet:
    case Item_id::club:
    case Item_id::hammer:
    case Item_id::machete:
    case Item_id::axe:
    case Item_id::pitch_fork:
    case Item_id::sledge_hammer:
    case Item_id::player_kick:
    case Item_id::player_stomp:
    case Item_id::player_punch:
    case Item_id::zombie_axe:
    case Item_id::zombie_claw:
    case Item_id::zombie_claw_diseased:
    case Item_id::bloated_zombie_punch:
    case Item_id::bloated_zombie_spit:
    case Item_id::rat_bite:
    case Item_id::rat_bite_diseased:
    case Item_id::rat_thing_bite:
    case Item_id::brown_jenkin_bite:
    case Item_id::worm_mass_bite:
    case Item_id::green_spider_bite:
    case Item_id::white_spider_bite:
    case Item_id::red_spider_bite:
    case Item_id::shadow_spider_bite:
    case Item_id::leng_spider_bite:
    case Item_id::fire_hound_breath:
    case Item_id::frost_hound_breath:
    case Item_id::fire_hound_bite:
    case Item_id::frost_hound_bite:
    case Item_id::zuul_bite:
    case Item_id::giant_bat_bite:
    case Item_id::wolf_bite:
    case Item_id::ghost_claw:
    case Item_id::phantasm_sickle:
    case Item_id::wraith_claw:
    case Item_id::polyp_tentacle:
    case Item_id::greater_polyp_tentacle:
    case Item_id::ghoul_claw:
    case Item_id::shadow_claw:
    case Item_id::byakhee_claw:
    case Item_id::giant_mantis_claw:
    case Item_id::giant_locust_bite:
    case Item_id::mummy_maul:
    case Item_id::croc_head_mummy_spear:
    case Item_id::deep_one_javelin_att:
    case Item_id::deep_one_spear_att:
    case Item_id::ape_maul:
    case Item_id::ooze_black_spew_pus:
    case Item_id::ooze_clear_spew_pus:
    case Item_id::ooze_putrid_spew_pus:
    case Item_id::ooze_poison_spew_pus:
    case Item_id::colour_oOSpace_touch:
    case Item_id::chthonian_bite:
    case Item_id::hunting_horror_bite:
    case Item_id::dust_vortex_engulf:
    case Item_id::fire_vortex_engulf:
    case Item_id::frost_vortex_engulf:
    case Item_id::mold_spores:
    case Item_id::mi_go_sting:
        case Item_id::mi_go_commander_sting:
    case Item_id::the_high_priest_claw:
        r = new Wpn(d, ammo_d);
        break;

    case Item_id::pharaoh_staff:
        r = new Pharaoh_staff(d);
        break;

    case Item_id::dynamite:
        r = new Dynamite(d);
        break;

    case Item_id::flare:
        r = new Flare(d);
        break;

    case Item_id::molotov:
        r = new Molotov(d);
        break;

    case Item_id::smoke_grenade:
        r = new Smoke_grenade(d);
        break;

    case Item_id::sawed_off:
        r = new Sawed_off(d, ammo_d);
        break;

    case Item_id::pump_shotgun:
        r = new Pump_shotgun(d, ammo_d);
        break;

    case Item_id::shotgun_shell:
        r = new Ammo(d);
        break;

    case Item_id::machine_gun:
        r = new Machine_gun(d, ammo_d);
        break;

    case Item_id::drum_of_bullets:
    case Item_id::pistol_clip:
    case Item_id::incinerator_ammo:
    case Item_id::mi_go_gun_ammo:
        r = new Ammo_clip(d);
        break;

    case Item_id::pistol:
        r = new Pistol(d, ammo_d);
        break;

    case Item_id::flare_gun:
        r = new Flare_gun(d, ammo_d);
        break;

    case Item_id::incinerator:
        r = new Incinerator(d, ammo_d);
        break;

    case Item_id::spike_gun:
        r = new Spike_gun(d, ammo_d);
        break;

    case Item_id::mi_go_gun:
        r = new Mi_go_gun(d, ammo_d);
        break;

    case Item_id::armor_flack_jacket:
    case Item_id::armor_leather_jacket:
    case Item_id::armor_iron_suit:
        r = new Armor(d);
        break;

    case Item_id::armor_asb_suit:
        r = new Armor_asb_suit(d);
        break;

    case Item_id::armor_heavy_coat:
        r = new Armor_heavy_coat(d);
        break;

    case Item_id::armor_mi_go:
        r = new Armor_mi_go(d);
        break;

    case Item_id::gas_mask:
        r = new Gas_mask(d);
        break;

//    case Item_id::hideous_mask:
//        r = new Hideous_mask(d);
//        break;

    case Item_id::scroll_mayhem:
    case Item_id::scroll_telep:
    case Item_id::scroll_pest:
    case Item_id::scroll_slow_mon:
    case Item_id::scroll_terrify_mon:
    case Item_id::scroll_paral_mon:
    case Item_id::scroll_det_items:
    case Item_id::scroll_det_traps:
    case Item_id::scroll_det_mon:
    case Item_id::scroll_bless:
    case Item_id::scroll_darkbolt:
    case Item_id::scroll_aza_wrath:
    case Item_id::scroll_opening:
    case Item_id::scroll_sacr_life:
    case Item_id::scroll_sacr_spi:
    case Item_id::scroll_elem_res:
    case Item_id::scroll_summon_mon:
    case Item_id::scroll_light:
        r = new Scroll(d);
        break;

    case Item_id::potion_vitality:
        r = new Potion_vitality(d);
        break;

    case Item_id::potion_spirit:
        r = new Potion_spirit(d);
        break;

    case Item_id::potion_blindness:
        r = new Potion_blindness(d);
        break;

    case Item_id::potion_frenzy:
        r = new Potion_frenzy(d);
        break;

    case Item_id::potion_fortitude:
        r = new Potion_fortitude(d);
        break;

    case Item_id::potion_paralyze:
        r = new Potion_paral(d);
        break;

    case Item_id::potion_rElec:
        r = new Potion_rElec(d);
        break;

    case Item_id::potion_conf:
        r = new Potion_conf(d);
        break;

    case Item_id::potion_poison:
        r = new Potion_poison(d);
        break;

    case Item_id::potion_insight:
        r = new Potion_insight(d);
        break;

    case Item_id::potion_clairv:
        r = new Potion_clairv(d);
        break;

    case Item_id::potion_rFire:
        r = new Potion_rFire(d);
        break;

    case Item_id::potion_antidote:
        r = new Potion_antidote(d);
        break;

    case Item_id::potion_descent:
        r = new Potion_descent(d);
        break;

    case Item_id::device_blaster:
        r = new Device_blaster(d);
        break;

    case Item_id::device_shockwave:
        r = new Device_shockwave(d);
        break;

    case Item_id::device_rejuvenator:
        r = new Device_rejuvenator(d);
        break;

    case Item_id::device_translocator:
        r = new Device_translocator(d);
        break;

    case Item_id::device_sentry_drone:
        r = new Device_sentry_drone(d);
        break;

    case Item_id::electric_lantern:
        r = new Device_lantern(d);
        break;

    case Item_id::medical_bag:
        r = new Medical_bag(d);
        break;

    case Item_id::star_amulet:
    case Item_id::skull_amulet:
    case Item_id::spider_amulet:
    case Item_id::eye_amulet:
    case Item_id::moon_amulet:
    case Item_id::bat_amulet:
    case Item_id::scarab_amulet:
    case Item_id::dagger_amulet:
    case Item_id::golden_ring:
    case Item_id::silver_ring:
    case Item_id::carnelian_ring:
    case Item_id::garnet_ring:
    case Item_id::iron_ring:
    case Item_id::jade_ring:
    case Item_id::moonstone_ring:
    case Item_id::obsidian_ring:
    case Item_id::onyx_ring:
    case Item_id::topaz_ring:
    case Item_id::emerald_ring:
        r = new Jewelry(d);
        break;

    case Item_id::END:
        return nullptr;
    }

    if (!r->get_data().is_stackable && NR_ITEMS != 1)
    {
        TRACE << "Specified number of items (" + to_str(NR_ITEMS) + ") != 1 for "
              << "non-stackable item" << endl;
        assert(false);
    }
    else
    {
        r->nr_items_ = NR_ITEMS;
    }

    return r;
}

void set_item_randomized_properties(Item* item)
{
    const Item_data_t& d = item->get_data();

    if (d.id == Item_id::pharaoh_staff)
    {
        //TODO: This is a really hacky, temporary fix that shouldn't be here - refactor.
        item->melee_dmg_plus_ = 4;
    }
    else //Not Staff of the Pharohs
    {
        //If it is a pure melee weapon, it may get extra damage
        if (d.melee.is_melee_wpn && !d.ranged.is_ranged_wpn)
        {
            static_cast<Wpn*>(item)->set_random_melee_plus();
        }
    }

    //If firearm, spawn with random amount of ammo
    if (d.ranged.is_ranged_wpn && !d.ranged.has_infinite_ammo)
    {
        Wpn* const wpn = static_cast<Wpn*>(item);
        if (wpn->AMMO_CAP == 1)
        {
            wpn->nr_ammo_loaded = Rnd::coin_toss() ? 1 : 0;
        }
        else
        {
            if (d.ranged.is_machine_gun)
            {
                //Number of machine gun bullets loaded needs to be a multiple of the
                //number of projectiles fired in each burst
                const int CAP         = wpn->AMMO_CAP;
                const int CAP_SCALED  = CAP / NR_MG_PROJECTILES;
                const int MIN_SCALED  = CAP_SCALED / 4;
                wpn->nr_ammo_loaded     = Rnd::range(MIN_SCALED, CAP_SCALED) *
                                        NR_MG_PROJECTILES;
            }
            else //Not machinegun
            {
                wpn->nr_ammo_loaded = Rnd::range(wpn->AMMO_CAP / 4, wpn->AMMO_CAP);
            }
        }
    }

    if (d.is_stackable) {item->nr_items_ = Rnd::range(1, d.max_stack_at_spawn);}
}

Item* mk_item_on_floor(const Item_id item_id, const Pos& pos)
{
    Item* item = mk(item_id);
    set_item_randomized_properties(item);
    Item_drop::drop_item_on_map(pos, *item);
    return item;
}

Item* copy_item(Item* old_item)
{
    Item* new_item     = mk(old_item->get_data().id);
    new_item->nr_items_ = old_item->nr_items_;
    return new_item;
}

Item* mk_random_scroll_or_potion(const bool ALLOW_SCROLLS, const bool ALLOW_POTIONS)
{
    vector<Item_id> item_bucket;

    for (int i = 0; i < int(Item_id::END); ++i)
    {
        const Item_data_t* const d = Item_data::data[i];

        if (
            (d->type == Item_type::scroll && ALLOW_SCROLLS) ||
            (d->type == Item_type::potion && ALLOW_POTIONS))
        {
            item_bucket.push_back(static_cast<Item_id>(i));
        }
    }

    if (!item_bucket.empty())
    {
        const int ELEMENT = Rnd::range(0, item_bucket.size() - 1);
        return mk(item_bucket[ELEMENT]);
    }

    return nullptr;
}

} //Item_factory
