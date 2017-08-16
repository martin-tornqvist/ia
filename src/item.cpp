#include "init.hpp"

#include "item.hpp"

#include "map.hpp"
#include "game_time.hpp"
#include "map_parsing.hpp"
#include "properties.hpp"
#include "msg_log.hpp"
#include "explosion.hpp"
#include "io.hpp"
#include "query.hpp"
#include "item_factory.hpp"
#include "feature_mob.hpp"
#include "feature_rigid.hpp"
#include "item_data.hpp"
#include "saving.hpp"
#include "actor_factory.hpp"
#include "game.hpp"
#include "player_bon.hpp"
#include "text_format.hpp"

// -----------------------------------------------------------------------------
// Item
// -----------------------------------------------------------------------------
Item::Item(ItemDataT* item_data) :
    nr_items_       (1),
    melee_dmg_plus_ (0),
    data_           (item_data),
    actor_carrying_ (nullptr),
    carrier_props_  (),
    carrier_spells_ ()
{
    melee_dmg_plus_ = data_->melee.dmg.plus;
}

Item& Item::operator=(const Item& other)
{
    nr_items_ = other.nr_items_;

    melee_dmg_plus_ = other.melee_dmg_plus_;

    data_ = other.data_;

    actor_carrying_ = other.actor_carrying_;

    carrier_props_ = other.carrier_props_;

    carrier_spells_ = other.carrier_spells_;

    return *this;
}

Item::~Item()
{
    for (auto* prop : carrier_props_)
    {
        delete prop;
    }

    for (auto* spell : carrier_spells_)
    {
        delete spell;
    }
}

ItemId Item::id() const
{
    return data_->id;
}

const ItemDataT& Item::data() const
{
    return *data_;
}

Clr Item::clr() const
{
    return data_->clr;
}

char Item::glyph() const
{
    return data_->glyph;
}

TileId Item::tile() const
{
    return data_->tile;
}

std::vector<std::string> Item::descr() const
{
    return data_->base_descr;
}

Dice Item::dmg(const AttMode att_mode,
               const Actor* const attacker) const
{
    Dice dice;

    const Dice zero_dice(0, 0); // To compare against

    switch (att_mode)
    {
    case AttMode::melee:
    {
        dice = data_->melee.dmg;

        if (dice == zero_dice)
        {
            return dice;
        }

        dice.plus = melee_dmg_plus_;

        if (attacker == map::player)
        {
            if (player_bon::traits[(size_t)Trait::adept_melee_fighter])
            {
                ++dice.plus;
            }

            if (player_bon::traits[(size_t)Trait::expert_melee_fighter])
            {
                ++dice.plus;
            }

            if (player_bon::traits[(size_t)Trait::master_melee_fighter])
            {
                ++dice.plus;
            }
        }

        // Bonus damage from being frenzied?
        if (attacker && attacker->has_prop(PropId::frenzied))
        {
            ++dice.plus;
        }
    }
    break;

    case AttMode::ranged:
    {
        dice = data_->ranged.dmg;

        if (dice == zero_dice)
        {
            return dice;
        }
    }
    break;

    case AttMode::thrown:
    {
        const bool is_melee_wpn = data_->type == ItemType::melee_wpn;

        // Melee weapons do throw damage based on their melee damage
        if (is_melee_wpn)
        {
            dice = data_->melee.dmg;

            if (dice == zero_dice)
            {
                return dice;
            }

            dice.plus = melee_dmg_plus_;
        }
        else // Not a melee weapon
        {
            dice = data_->ranged.throw_dmg;

            if (dice == zero_dice)
            {
                return dice;
            }
        }
    }
    break;

    case AttMode::none:
    {
        ASSERT(false);
        break;
    }
    } // Attack mode switch

    // Apply item specific damage modifications
    specific_dmg_mod(dice, attacker);

    return dice;
}

int Item::weight() const
{
    return (int)data_->weight * nr_items_;
}

std::string Item::weight_str() const
{
    const int wgt = weight();

    if (wgt <= ((int)ItemWeight::extra_light + (int)ItemWeight::light) / 2)
    {
        return "very light";
    }

    if (wgt <= ((int)ItemWeight::light + (int)ItemWeight::medium) / 2)
    {
        return "light";
    }

    if (wgt <= ((int)ItemWeight::medium + (int)ItemWeight::heavy) / 2)
    {
        return "a bit heavy";
    }

    return "heavy";
}

ConsumeItem Item::activate(Actor* const actor)
{
    (void)actor;

    msg_log::add("I cannot apply that.");

    return ConsumeItem::no;
}

void Item::on_pickup(Actor& actor)
{
    ASSERT(!actor_carrying_);

    actor_carrying_ = &actor;

    if (actor_carrying_->is_player())
    {
        on_player_found();
    }

    on_pickup_hook();
}

void Item::on_equip(const Verbosity verbosity)
{
    ASSERT(actor_carrying_);

    on_equip_hook(verbosity);
}

void Item::on_unequip()
{
    ASSERT(actor_carrying_);

    on_unequip_hook();
}

void Item::on_removed_from_inv()
{
    on_removed_from_inv_hook();

    actor_carrying_ = nullptr;
}

void Item::on_player_found()
{
    if ((data_->value == ItemValue::major_treasure) &&
        data_->is_unique &&
        !data_->is_found)
    {
        const std::string item_name =
            name(ItemRefType::a,
                 ItemRefInf::none);

        msg_log::more_prompt();

        msg_log::add("I have found " + item_name + "!");

        game::incr_player_xp(5, Verbosity::verbose);

        data_->is_found = true;

        game::add_history_event("Found " + item_name + ".");
    }
}

std::string Item::name(const ItemRefType ref_type,
                       const ItemRefInf inf,
                       const ItemRefAttInf att_inf) const
{
    ItemRefType ref_type_used = ref_type;

    // If requested ref type is "plural" and this is a single item, use ref type
    // "a" instead.
    if ((ref_type == ItemRefType::plural) &&
        (!data_->is_stackable || (nr_items_ == 1)))
    {
        ref_type_used = ItemRefType::a;
    }

    std::string nr_str = "";

    if (ref_type_used == ItemRefType::plural)
    {
        nr_str = std::to_string(nr_items_);
    }

    std::string dmg_str = "";

    std::string hit_str;

    ItemRefAttInf att_inf_used = att_inf;

    // If caller requested attack info depending on main attack mode,
    // setup the attack info used to a specific type
    if (att_inf == ItemRefAttInf::wpn_main_att_mode)
    {
        switch (data_->main_att_mode)
        {
        case AttMode::melee:
            att_inf_used = ItemRefAttInf::melee;
            break;

        case AttMode::ranged:
            att_inf_used = ItemRefAttInf::ranged;
            break;

        case AttMode::thrown:
            att_inf_used = ItemRefAttInf::thrown;
            break;

        case AttMode::none:
            att_inf_used = ItemRefAttInf::none;
            break;
        }
    }

    dmg_str = this->dmg_str(att_inf, ItemRefDmgValue::average_and_melee_plus);

    switch (att_inf_used)
    {
    case ItemRefAttInf::melee:
    {
        const int hit_int = data_->melee.hit_chance_mod;

        hit_str =
            ((hit_int >= 0) ? "+" : "") +
            std::to_string(hit_int) + "%";
    }
    break;

    case ItemRefAttInf::ranged:
    {
        const int hit_int = data_->ranged.hit_chance_mod;

        hit_str =
            ((hit_int >= 0) ? "+" : "") +
            std::to_string(hit_int) + "%";
    }
    break;

    case ItemRefAttInf::thrown:
    {
        const int hit_int = data_->ranged.throw_hit_chance_mod;

        hit_str =
            ((hit_int >= 0) ? "+" : "") +
            std::to_string(hit_int) + "%";
    }
    break;

    case ItemRefAttInf::none:
        break;

    case ItemRefAttInf::wpn_main_att_mode:
        TRACE << "Bad attack info type: " << (int)att_inf_used << std::endl;

        ASSERT(false);
        break;
    }

    std::string inf_str = "";

    if (inf == ItemRefInf::yes)
    {
        inf_str = name_inf();
    }

    const auto& names_used =
        data_->is_identified ?
        data_->base_name :
        data_->base_name_un_id;

    const std::string base_name = names_used.names[(size_t)ref_type_used];

    const std::string ret =
        nr_str + (nr_str.empty() ? "" : " ") + base_name +
        (dmg_str.empty() ? "" : " ") + dmg_str +
        (hit_str.empty() ? "" : " ") + hit_str +
        (inf_str.empty() ? "" : " ") + inf_str;

    ASSERT(!ret.empty());

    return ret;
}

std::string Item::dmg_str(const ItemRefAttInf att_inf,
                          const ItemRefDmgValue dmg_value) const
{
    if (!data_->allow_display_dmg)
    {
        return "";
    }

    std::string dmg_str = "";

    ItemRefAttInf att_inf_used = att_inf;

    // If caller requested attack info depending on main attack mode,
    // setup the attack info used to a specific type
    if (att_inf == ItemRefAttInf::wpn_main_att_mode)
    {
        switch (data_->main_att_mode)
        {
        case AttMode::melee:
            att_inf_used = ItemRefAttInf::melee;
            break;

        case AttMode::ranged:
            att_inf_used = ItemRefAttInf::ranged;
            break;

        case AttMode::thrown:
            att_inf_used = ItemRefAttInf::thrown;
            break;

        case AttMode::none:
            att_inf_used = ItemRefAttInf::none;
            break;
        }
    }

    const Dice zero_dice(0, 0); // To compare against

    switch (att_inf_used)
    {
    case ItemRefAttInf::melee:
    {
        if (data_->melee.dmg != zero_dice)
        {
            const Dice dmg_dice = dmg(AttMode::melee, map::player);

            const std::string str_avg = dmg_dice.str_avg();

            switch (dmg_value)
            {
            case ItemRefDmgValue::average:
            {
                dmg_str = str_avg;
            }
            break;

            case ItemRefDmgValue::average_and_melee_plus:
            {
                dmg_str = str_avg;

                // Get damage if not used by an actor (no skill bonus, etc)
                const Dice dmg_dice_raw = dmg(AttMode::melee, nullptr);

                const std::string str_plus = dmg_dice_raw.str_plus();

                if (!str_plus.empty())
                {
                    dmg_str += " {" + str_plus + "}";
                }
            }
            break;

            case ItemRefDmgValue::dice:
            {
                dmg_str = dmg_dice.str();
            }
            break;
            }
        }
    }
    break;

    case ItemRefAttInf::ranged:
    {
        if (data_->ranged.dmg != zero_dice)
        {
            Dice dmg_dice = dmg(AttMode::ranged, map::player);

            if (data_->ranged.is_machine_gun)
            {
                dmg_dice.rolls *= nr_mg_projectiles;

                dmg_dice.plus *= nr_mg_projectiles;
            }

            dmg_str =
                ((dmg_value == ItemRefDmgValue::average) ||
                 (dmg_value == ItemRefDmgValue::average_and_melee_plus)) ?
                dmg_dice.str_avg() :
                dmg_dice.str();
        }
    }
    break;

    case ItemRefAttInf::thrown:
    {
        // Print damage if non-zero throwing damage, or melee weapon with non
        // zero melee damage (melee weapons use melee damage when thrown)
        if ((data_->ranged.throw_dmg != zero_dice) ||
            ((data_->main_att_mode == AttMode::melee) &&
             (data_->melee.dmg != zero_dice)))
        {
            //
            // NOTE: "dmg" will return melee damage if this is a melee weapon
            //
            const Dice dmg_dice = dmg(AttMode::thrown, map::player);

            dmg_str =
                ((dmg_value == ItemRefDmgValue::average) ||
                 (dmg_value == ItemRefDmgValue::average_and_melee_plus)) ?
                dmg_dice.str_avg() :
                dmg_dice.str();
        }
    }
    break;

    case ItemRefAttInf::none:
        break;

    case ItemRefAttInf::wpn_main_att_mode:
        TRACE << "Bad attack info type: " << (int)att_inf_used << std::endl;

        ASSERT(false);
        break;
    }

    return dmg_str;
}

bool Item::is_in_effective_range_lmt(const P& p0, const P& p1) const
{
    return
        king_dist(p0, p1) <=
        data_->ranged.effective_range;
}

void Item::add_carrier_prop(Prop* const prop, const Verbosity verbosity)
{
    ASSERT(actor_carrying_);
    ASSERT(prop);

    actor_carrying_->prop_handler()
        .add_prop_from_equipped_item(this,
                                     prop,
                                     verbosity);
}

void Item::clear_carrier_props()
{
    ASSERT(actor_carrying_);

    actor_carrying_->prop_handler().remove_props_for_item(this);
}

void Item::add_carrier_spell(Spell* const spell)
{
    ASSERT(spell);

    carrier_spells_.push_back(spell);
}

void Item::clear_carrier_spells()
{
    for (Spell* const spell : carrier_spells_)
    {
        delete spell;
    }

    carrier_spells_.clear();
}

void Item::give_xp_for_identify(const Verbosity verbosity)
{
    game::incr_player_xp(data_->xp_on_identify,
                         verbosity);
}

// -----------------------------------------------------------------------------
// Armor
// -----------------------------------------------------------------------------
Armor::Armor(ItemDataT* const item_data) :
    Item    (item_data),
    dur_    (rnd::range(80, 100)) {}

void Armor::save()
{
    saving::put_int(dur_);
}

void Armor::load()
{
    dur_ = saving::get_int();
}

int Armor::armor_points() const
{
    // NOTE: AP must be able to reach zero, otherwise the armor will never count
    //       as destroyed.

    const int ap_max = data_->armor.armor_points;

    if (dur_ > 60)
    {
        return ap_max;
    }

    if (dur_ > 40)
    {
        return std::max(0, ap_max - 1);
    }

    if (dur_ > 25)
    {
        return std::max(0, ap_max - 2);
    }

    if (dur_ > 15)
    {
        return std::max(0, ap_max - 3);
    }

    return 0;
}

void Armor::hit(const int dmg)
{
    const int ap_before = armor_points();

    // Damage factor
    const double dmg_db = double(dmg);

    // Armor durability factor
    const double df = data_->armor.dmg_to_durability_factor;

    // Scaling factor
    const double k = 2.0;

    // Armor lasts twice as long for War Vets
    double war_vet_k = 1.0;

    ASSERT(actor_carrying_);

    if ((actor_carrying_ == map::player) &&
        (player_bon::bg() == Bg::war_vet))
    {
        war_vet_k = 0.5;
    }

    dur_ -= (int)(dmg_db * df * k * war_vet_k);

    dur_ = std::max(0, dur_);

    const int ap_after = armor_points();

    if ((ap_after < ap_before) &&
        (ap_after != 0))
    {
        const std::string armor_name = name(ItemRefType::plain);

        msg_log::add("My " + armor_name + " is damaged!", clr_msg_note);
    }
}

std::string Armor::name_inf() const
{
    const int ap = armor_points();

    const std::string ap_str = std::to_string(std::max(1, ap));

    return "[" + ap_str + "]";
}

void ArmorAsbSuit::on_equip_hook(const Verbosity verbosity)
{
    (void)verbosity;

    add_carrier_prop(new PropRFire(PropTurns::indefinite), Verbosity::silent);
    add_carrier_prop(new PropRAcid(PropTurns::indefinite), Verbosity::silent);
    add_carrier_prop(new PropRElec(PropTurns::indefinite), Verbosity::silent);
}

void ArmorAsbSuit::on_unequip_hook()
{
    clear_carrier_props();
}

void ArmorMiGo::on_equip_hook(const Verbosity verbosity)
{
    if (verbosity == Verbosity::verbose)
    {
        msg_log::add("The armor joins with my skin!",
                     clr_text,
                     false,
                     MorePromptOnMsg::yes);

        map::player->incr_shock(ShockLvl::terrifying,
                                ShockSrc::use_strange_item);
    }
}

// -----------------------------------------------------------------------------
// Weapon
// -----------------------------------------------------------------------------
Wpn::Wpn(ItemDataT* const item_data) :
    Item                (item_data),
    nr_ammo_loaded_     (0),
    ammo_data_          (nullptr)
{
    const auto ammo_item_id = data_->ranged.ammo_item_id;

    if (ammo_item_id != ItemId::END)
    {
        ammo_data_ = &item_data::data[(size_t)ammo_item_id];
        nr_ammo_loaded_ = data_->ranged.max_ammo;
    }
}

void Wpn::save()
{
    saving::put_int(melee_dmg_plus_);
    saving::put_int(nr_ammo_loaded_);
}

void Wpn::load()
{
    melee_dmg_plus_ = saving::get_int();
    nr_ammo_loaded_ = saving::get_int();
}

Clr Wpn::clr() const
{
    if (data_->ranged.is_ranged_wpn && !data_->ranged.has_infinite_ammo)
    {
        if (nr_ammo_loaded_ == 0)
        {
            Clr ret = data_->clr;
            ret.r /= 2;
            ret.g /= 2;
            ret.b /= 2;
            return ret;
        }
    }

    return data_->clr;
}

void Wpn::set_random_melee_plus()
{
    const bool is_low_dlvl = map::dlvl < 6;

    // Element corresponds to plus damage value (+0, +1, +2, etc)
    const std::vector<int> weights =
    {
        100,                    //          100
        220,                    //          320
        120,                    //          450
        70,                     //          520
        is_low_dlvl ? 20 : 40,  //          540 or 560
        is_low_dlvl ? 2  : 20,  //          542 or 580
        is_low_dlvl ? 1  : 10   // Total:   543 or 590
    };

    melee_dmg_plus_ = rnd::weighted_choice(weights);
}

std::string Wpn::name_inf() const
{
    if (data_->ranged.is_ranged_wpn &&
        !data_->ranged.has_infinite_ammo)
    {
        return
            std::to_string(nr_ammo_loaded_) +
            "/" +
            std::to_string(data_->ranged.max_ammo);
    }

    return "";
}

// -----------------------------------------------------------------------------
// Spiked Mace
// -----------------------------------------------------------------------------
void SpikedMace::on_melee_hit(Actor& actor_hit, const int dmg)
{
    (void)dmg;

    if (!actor_hit.is_alive())
    {
        return;
    }

    const int stun_pct = 25;

    if (rnd::percent(stun_pct))
    {
        Prop* const prop = new PropParalyzed(PropTurns::specific, 2);

        actor_hit.prop_handler().apply(prop);
    }
}

// -----------------------------------------------------------------------------
// Player ghoul claw
// -----------------------------------------------------------------------------
void PlayerGhoulClaw::on_melee_hit(Actor& actor_hit, const int dmg)
{
    (void)dmg;

    //
    // TODO: If some "constructed" monster is added (something not made of
    //       flesh, e.g. a golem), then a Ghoul player would be able to feed
    //       from it, which would be a problem. In that case, there should
    //       probably be a field in the actor data called something like either
    //       "is_flesh_body", or "is_construct".
    //

    // Ghoul feeding from Ravenous trait?

    //
    // NOTE: Player should never feed on monsters such as Ghosts or Shadows.
    //       Checking that the monster is not Ethereal and that it can bleed
    //       should be a pretty good rule for this.
    //       We should NOT check if the monster can leave a corpse however,
    //       since some monsters such as Worms don't leave a corpse, and you
    //       SHOULD be able to feed on those.
    const ActorDataT& d = actor_hit.data();

    const bool is_ethereal = actor_hit.has_prop(PropId::ethereal);

    const bool is_hp_missing = map::player->hp() < map::player->hp_max(true);
    const bool is_wounded = map::player->prop_handler().prop(PropId::wound);
    const bool is_feed_needed = is_hp_missing || is_wounded;

    if (!is_ethereal &&
        d.can_bleed &&
        player_bon::traits[(size_t)Trait::ravenous] &&
        is_feed_needed &&
        rnd::one_in(6))
    {
        Snd snd("",
                SfxId::bite,
                IgnoreMsgIfOriginSeen::yes,
                actor_hit.pos,
                map::player,
                SndVol::low,
                AlertsMon::yes,
                MorePromptOnMsg::no);

        snd.run();

        map::player->on_feed();
    }

    if (actor_hit.state() == ActorState::alive)
    {
        // Poison victim from Ghoul Toxic trait?
        if (player_bon::traits[(size_t)Trait::toxic] &&
            rnd::one_in(4))
        {
            Prop* const poison = new PropPoisoned(PropTurns::std);

            actor_hit.prop_handler().apply(poison);
        }

        // Terrify victim from Ghoul Indomitable Fury trait?
        if (player_bon::traits[(size_t)Trait::indomitable_fury] &&
            map::player->has_prop(PropId::frenzied))
        {
            Prop* const fear = new PropTerrified(PropTurns::std);

            actor_hit.prop_handler().apply(fear);
        }
    }
}

void PlayerGhoulClaw::on_melee_kill(Actor& actor_killed)
{
    // TODO: See TODO note in melee hit hook for Ghoul claw concerning
    // "constructed monsters".

    const ActorDataT& d = actor_killed.data();

    const bool is_ethereal = actor_killed.has_prop(PropId::ethereal);

    if (player_bon::traits[(size_t)Trait::foul] &&
        !is_ethereal &&
        d.can_leave_corpse &&
        rnd::one_in(3))
    {
        const size_t nr_worms = rnd::range(1, 2);

        actor_factory::spawn(actor_killed.pos,
                              {nr_worms, ActorId::worm_mass},
                              MakeMonAware::yes,
                              map::player);
    }
}

// -----------------------------------------------------------------------------
// Zombie Dust
// -----------------------------------------------------------------------------
void ZombieDust::on_ranged_hit(Actor& actor_hit)
{
    if (actor_hit.state() == ActorState::alive &&
        !actor_hit.data().is_undead)
    {
        actor_hit.prop_handler().apply(
            new PropParalyzed(PropTurns::std));
    }
}

// -----------------------------------------------------------------------------
// Sawed off shotgun
// -----------------------------------------------------------------------------
SawedOff::SawedOff(ItemDataT* const item_data) :
    Wpn(item_data) {}

// -----------------------------------------------------------------------------
// Pump shotgun
// -----------------------------------------------------------------------------
PumpShotgun::PumpShotgun(ItemDataT* const item_data) :
    Wpn(item_data) {}

// -----------------------------------------------------------------------------
// Flare gun
// -----------------------------------------------------------------------------
FlareGun::FlareGun(ItemDataT* const item_data) :
    Wpn(item_data) {}

// -----------------------------------------------------------------------------
// Pistol
// -----------------------------------------------------------------------------
Pistol::Pistol(ItemDataT* const item_data) :
    Wpn(item_data) {}

// -----------------------------------------------------------------------------
// Machine gun
// -----------------------------------------------------------------------------
MachineGun::MachineGun(ItemDataT* const item_data) :
    Wpn(item_data) {}

// -----------------------------------------------------------------------------
// Mi-go electric gun
// -----------------------------------------------------------------------------
MiGoGun::MiGoGun(ItemDataT* const item_data) :
    Wpn(item_data) {}

void MiGoGun::specific_dmg_mod(Dice& dice,
                               const Actor* const actor) const
{
    if ((actor == map::player) &&
        player_bon::traits[(size_t)Trait::elec_incl])
    {
        ++dice.plus;
    }
}

// -----------------------------------------------------------------------------
// Spike gun
// -----------------------------------------------------------------------------
SpikeGun::SpikeGun(ItemDataT* const item_data) :
    Wpn(item_data) {}

// -----------------------------------------------------------------------------
// Incinerator
// -----------------------------------------------------------------------------
Incinerator::Incinerator(ItemDataT* const item_data) :
    Wpn(item_data) {}

void Incinerator::on_projectile_blocked(
    const P& pos, Actor* actor_hit)
{
    (void)actor_hit;
    explosion::run(pos, ExplType::expl);
}

// -----------------------------------------------------------------------------
// Raven peck
// -----------------------------------------------------------------------------
void RavenPeck::on_melee_hit(Actor& actor_hit, const int dmg)
{
    (void)dmg;

    if (!actor_hit.is_alive())
    {
        return;
    }

    // Gas mask and Asbestos suit protects against blindness
    Item* const head_item = actor_hit.inv().item_in_slot(SlotId::head);
    Item* const body_item = actor_hit.inv().item_in_slot(SlotId::body);

    if ((head_item && head_item->id() == ItemId::gas_mask) ||
        (body_item && body_item->id() == ItemId::armor_asb_suit))
    {
        return;
    }

    Prop* const prop = new PropBlind(PropTurns::specific, 2);

    actor_hit.prop_handler().apply(prop);
}

// -----------------------------------------------------------------------------
// Vampire Bat Bite
// -----------------------------------------------------------------------------
void VampireBatBite::on_melee_hit(Actor& actor_hit, const int dmg)
{
    if (!actor_hit.is_alive())
    {
        return;
    }

    actor_carrying_->restore_hp(dmg,
                                false,
                                Verbosity::verbose);
}

// -----------------------------------------------------------------------------
// Mind Leech Sting
// -----------------------------------------------------------------------------
void MindLeechSting::on_melee_hit(Actor& actor_hit, const int dmg)
{
    (void)dmg;

    if (!actor_hit.is_alive() ||
        !actor_hit.is_player())
    {
        return;
    }

    auto* const mon = actor_carrying_;

    if (map::player->ins() >= 50 ||
        map::player->has_prop(PropId::confused) ||
        map::player->has_prop(PropId::frenzied))
    {
        const bool player_see_mon = map::player->can_see_actor(*mon);

        if (player_see_mon)
        {
            const std::string mon_name_the =
                text_format::first_to_upper(mon->name_the());

            msg_log::add(mon_name_the + " looks shocked!");
        }

        mon->hit(rnd::dice(3, 5), DmgType::pure);

        if (mon->is_alive())
        {
            mon->prop_handler().apply(new PropConfused(PropTurns::std));

            mon->prop_handler().apply(new PropTerrified(PropTurns::std));
        }
    }
    else // Player mind can be eaten
    {
        map::player->incr_shock(ShockLvl::mind_shattering,
                                ShockSrc::misc);

        // Make the monster pause, so things don't get too crazy
        mon->prop_handler().apply(
            new PropWaiting(PropTurns::specific, 2));
    }
}

// -----------------------------------------------------------------------------
// Spirit Leech Sting
// -----------------------------------------------------------------------------
void SpiritLeechSting::on_melee_hit(Actor& actor_hit, const int dmg)
{
    (void)dmg;

    if (!actor_hit.is_alive() ||
        !actor_hit.is_player())
    {
        return;
    }

    map::player->change_max_spi(-1, Verbosity::verbose);

    auto* const mon = actor_carrying_;

    mon->change_max_spi(1, Verbosity::silent);

    mon->restore_spi(1, false, Verbosity::silent);

    // Make the monster pause, so things don't get too crazy
    mon->prop_handler().apply(
        new PropWaiting(PropTurns::specific, 2));
}

// -----------------------------------------------------------------------------
// Life Leech Sting
// -----------------------------------------------------------------------------
void LifeLeechSting::on_melee_hit(Actor& actor_hit, const int dmg)
{
    (void)dmg;

    if (!actor_hit.is_alive() ||
        !actor_hit.is_player())
    {
        return;
    }

    map::player->change_max_hp(-1, Verbosity::verbose);

    auto* const mon = actor_carrying_;

    mon->change_max_hp(1, Verbosity::silent);

    mon->restore_hp(1, false, Verbosity::silent);

    // Make the monster pause, so things don't get too crazy
    mon->prop_handler().apply(
        new PropWaiting(PropTurns::specific, 2));
}

// -----------------------------------------------------------------------------
// Dust vortex enguld
// -----------------------------------------------------------------------------
void DustVortexEngulf::on_melee_hit(Actor& actor_hit, const int dmg)
{
    (void)dmg;

    if (!actor_hit.is_alive())
    {
        return;
    }

    // Gas mask and Asbestos suit protects against blindness
    Item* const head_item = actor_hit.inv().item_in_slot(SlotId::head);
    Item* const body_item = actor_hit.inv().item_in_slot(SlotId::body);

    if ((head_item && head_item->id() == ItemId::gas_mask) ||
        (body_item && body_item->id() == ItemId::armor_asb_suit))
    {
        return;
    }

    Prop* const prop = new PropBlind(PropTurns::std);

    actor_hit.prop_handler().apply(prop);
}

// -----------------------------------------------------------------------------
// Spitting cobra spit
// -----------------------------------------------------------------------------
void SpittingCobraSpit::on_ranged_hit(Actor& actor_hit)
{
    if (!actor_hit.is_alive())
    {
        return;
    }

    // Gas mask and Asbestos suit protects against blindness
    Item* const head_item = actor_hit.inv().item_in_slot(SlotId::head);
    Item* const body_item = actor_hit.inv().item_in_slot(SlotId::body);

    if ((head_item && head_item->id() == ItemId::gas_mask) ||
        (body_item && body_item->id() == ItemId::armor_asb_suit))
    {
        return;
    }

    Prop* const prop = new PropBlind(PropTurns::specific, 7);

    actor_hit.prop_handler().apply(prop);
}

// -----------------------------------------------------------------------------
// Ammo mag
// -----------------------------------------------------------------------------
AmmoMag::AmmoMag(ItemDataT* const item_data) : Ammo(item_data)
{
    set_full_ammo();
}

void AmmoMag::save()
{
    saving::put_int(ammo_);
}

void AmmoMag::load()
{
    ammo_ = saving::get_int();
}

void AmmoMag::set_full_ammo()
{
    ammo_ = data_->ranged.max_ammo;
}

// -----------------------------------------------------------------------------
// Medical bag
// -----------------------------------------------------------------------------
MedicalBag::MedicalBag(ItemDataT* const item_data) :
    Item                    (item_data),
    nr_supplies_            (24),
    nr_turns_left_action_   (-1),
    current_action_         (MedBagAction::END) {}

void MedicalBag::save()
{
    saving::put_int(nr_supplies_);
}

void MedicalBag::load()
{
    nr_supplies_ = saving::get_int();
}

void MedicalBag::on_pickup_hook()
{
    ASSERT(actor_carrying_);

    if (!actor_carrying_->is_player())
    {
        return;
    }

    auto& inv = actor_carrying_->inv();

    // Check for existing medical bag in inventory
    for (Item* const other : inv.backpack_)
    {
        if (other != this && other->id() == id())
        {
            // Add my turns left to the other medical bag, then destroy self
            static_cast<MedicalBag*>(other)->nr_supplies_ += nr_supplies_;

            inv.remove_item_in_backpack_with_ptr(this, true);

            return;
        }
    }
}

ConsumeItem MedicalBag::activate(Actor* const actor)
{
    (void)actor;

    if (player_bon::bg() == Bg::ghoul)
    {
        msg_log::add("It is of no use to me.");

        current_action_ = MedBagAction::END;

        return ConsumeItem::no;
    }

    if (map::player->has_prop(PropId::poisoned))
    {
        msg_log::add("Not while poisoned.");

        current_action_ = MedBagAction::END;

        return ConsumeItem::no;
    }

    const auto seen_foes = map::player->seen_foes();

    if (!seen_foes.empty())
    {
        msg_log::add("Not while an enemy is near.");

        current_action_ = MedBagAction::END;

        return ConsumeItem::no;
    }

    current_action_ = choose_action();

    if (current_action_ == MedBagAction::END)
    {
        msg_log::clear();

        msg_log::add("I have no wounds to treat.");

        return ConsumeItem::no;
    }

    const int nr_supplies_needed = tot_suppl_for_action(current_action_);

    const bool is_enough_supplies = nr_supplies_ >= nr_supplies_needed;

    if (!is_enough_supplies)
    {
        msg_log::add("I do not have enough medical supplies.");

        current_action_ = MedBagAction::END;

        return ConsumeItem::no;
    }

    // Action can be done
    map::player->active_medical_bag = this;

    nr_turns_left_action_ = tot_turns_for_action(current_action_);

    std::string start_msg = "";

    switch (current_action_)
    {
    case MedBagAction::treat_wound:
        start_msg = "I start treating a wound";
        break;

    case MedBagAction::sanitize_infection:
        start_msg = "I start to sanitize an infection";
        break;

    case MedBagAction::END:
        ASSERT(false);
        break;
    }

    start_msg += " (" + std::to_string(nr_turns_left_action_) + " turns)...";

    msg_log::add(start_msg);

    game_time::tick();

    return ConsumeItem::no;
}

MedBagAction MedicalBag::choose_action() const
{
    // Infection?
    if (map::player->has_prop(PropId::infected))
    {
        return MedBagAction::sanitize_infection;
    }

    // Wound?
    if (map::player->has_prop(PropId::wound))
    {
        return MedBagAction::treat_wound;
    }

    return MedBagAction::END;
}

void MedicalBag::continue_action()
{
    ASSERT(current_action_ != MedBagAction::END);

    --nr_turns_left_action_;

    if (nr_turns_left_action_ <= 0)
    {
        finish_current_action();
    }
    else // Time still remaining on the current action
    {
        game_time::tick();
    }
}

void MedicalBag::finish_current_action()
{
    map::player->active_medical_bag = nullptr;

    switch (current_action_)
    {
    case MedBagAction::treat_wound:
    {
        Prop* const wound_prop =
            map::player->prop_handler().prop(PropId::wound);

        ASSERT(wound_prop);

        PropWound* const wound = static_cast<PropWound*>(wound_prop);

        wound->heal_one_wound();
    }
    break;

    case MedBagAction::sanitize_infection:
    {
        map::player->prop_handler().end_prop(PropId::infected);
    }
    break;

    case MedBagAction::END:
        ASSERT(false);
        break;
    }

    nr_supplies_ -= tot_suppl_for_action(current_action_);

    current_action_ = MedBagAction::END;

    if (nr_supplies_ <= 0)
    {
        map::player->inv().remove_item_in_backpack_with_ptr(this, true);

        game::add_history_event("Ran out of medical supplies.");
    }
}

void MedicalBag::interrupted()
{
    msg_log::add("My healing is disrupted.");

    current_action_ = MedBagAction::END;

    nr_turns_left_action_ = -1;

    map::player->active_medical_bag = nullptr;
}

int MedicalBag::tot_suppl_for_action(const MedBagAction action) const
{
    const bool is_healer = player_bon::traits[(size_t)Trait::healer];
    const int div = is_healer ? 2 : 1;

    switch (action)
    {
    case MedBagAction::treat_wound:
        return 8 / div;

    case MedBagAction::sanitize_infection:
        return 2 / div;

    case MedBagAction::END:
        break;
    }

    ASSERT(false);

    return 0;
}

int MedicalBag::tot_turns_for_action(const MedBagAction action) const
{
    const bool is_healer = player_bon::traits[(size_t)Trait::healer];
    const int div = is_healer ? 2 : 1;

    switch (action)
    {
    case MedBagAction::treat_wound:
        return 80 / div;

    case MedBagAction::sanitize_infection:
        return 20 / div;

    case MedBagAction::END:
        break;
    }

    ASSERT(false);

    return 0;
}

// -----------------------------------------------------------------------------
// Hideous mask
// -----------------------------------------------------------------------------
//HideousMask::HideousMask(ItemDataT* item_data) : Headwear(item_data)
//{
//    item_data->allow_spawn = false;
//}
//
//void HideousMask::on_std_turn_in_inv(const InvType inv_type)
//{
//    if (inv_type == InvType::slots)
//    {
//        std::vector<Actor*> adj_actors;
//        const P p(map::player->pos);
//        for (auto* const actor : game_time::actors)
//        {
//            if (actor->is_alive() && is_pos_adj(p, actor->pos, false))
//            {
//                adj_actors.push_back(actor);
//            }
//        }
//        if (!adj_actors.empty())
//        {
//            bool blocked_los[map_w][map_h];
//            map_parse::run(cell_check::BlocksLos(), blocked_los);
//            for (auto* const actor : adj_actors)
//            {
//                if (rnd::one_in(4) && actor->can_see_actor(*map::player, blocked_los))
//                {
//                    actor->prop_handler().apply(
//                        new PropTerrified(PropTurns::std));
//                }
//            }
//        }
//    }
//}

// -----------------------------------------------------------------------------
// Gas mask
// -----------------------------------------------------------------------------
void GasMask::on_equip_hook(const Verbosity verbosity)
{
    (void)verbosity;
}

void GasMask::on_unequip_hook()
{
    clear_carrier_props();
}

void GasMask::decr_turns_left(Inventory& carrier_inv)
{
    --nr_turns_left_;

    if (nr_turns_left_ <= 0)
    {
        const std::string item_name =
            name(ItemRefType::plain, ItemRefInf::none);

        msg_log::add("My " + item_name + " expires.",
                     clr_msg_note,
                     true,
                     MorePromptOnMsg::yes);

        carrier_inv.decr_item(this);
    }
}

// -----------------------------------------------------------------------------
// Explosive
// -----------------------------------------------------------------------------
ConsumeItem Explosive::activate(Actor* const actor)
{
    (void)actor;

    if (map::player->has_prop(PropId::burning))
    {
        msg_log::add("Not while burning.");

        return ConsumeItem::no;
    }

    const Explosive* const held_explosive = map::player->active_explosive;

    if (held_explosive)
    {
        const std::string name_held =
            held_explosive->name(ItemRefType::a, ItemRefInf::none);

        msg_log::add("I am already holding " + name_held + ".");

        return ConsumeItem::no;
    }

    if (config::is_light_explosive_prompt())
    {
        const std::string name = this->name(ItemRefType::a);

        msg_log::add("Light " + name + "? [y/n]");

        auto result = query::yes_or_no();

        if (result == BinaryAnswer::no)
        {
            msg_log::clear();

            return ConsumeItem::no;
        }

        msg_log::clear();
    }

    // Make a copy to use as the held ignited explosive.
    auto* cpy = static_cast<Explosive*>(item_factory::mk(data().id, 1));

    cpy->fuse_turns_ = std_fuse_turns();

    map::player->active_explosive = cpy;

    cpy->on_player_ignite();

    return ConsumeItem::yes;
}

// -----------------------------------------------------------------------------
// Dynamite
// -----------------------------------------------------------------------------
void Dynamite::on_player_ignite() const
{
    msg_log::add("I light a dynamite stick.");

    const int speed_pct_diff =
        player_bon::traits[(size_t)Trait::dem_expert] ?
        100 :
        0;

    game_time::tick(speed_pct_diff);
}

void Dynamite::on_std_turn_player_hold_ignited()
{
    --fuse_turns_;

    if (fuse_turns_ > 0)
    {
        std::string fuse_msg = "***F";

        for (int i = 0; i < fuse_turns_; ++i)
        {
            fuse_msg += "Z";
        }

        fuse_msg += "***";

        msg_log::add(fuse_msg, clr_yellow);
    }
    else // Fuse has run out
    {
        msg_log::add("The dynamite explodes in my hand!");

        map::player->active_explosive = nullptr;

        explosion::run(map::player->pos, ExplType::expl);

        fuse_turns_ = -1;

        delete this;
    }
}

void Dynamite::on_thrown_ignited_landing(const P& p)
{
    game_time::add_mob(new LitDynamite(p, fuse_turns_));
}

void Dynamite::on_player_paralyzed()
{
    msg_log::add("The lit Dynamite stick falls from my hand!");

    map::player->active_explosive = nullptr;

    const P& p = map::player->pos;

    auto* const f = map::cells[p.x][p.y].rigid;

    if (!f->is_bottomless())
    {
        game_time::add_mob(new LitDynamite(p, fuse_turns_));
    }

    delete this;
}

// -----------------------------------------------------------------------------
// Molotov
// -----------------------------------------------------------------------------
void Molotov::on_player_ignite() const
{
    msg_log::add("I light a Molotov Cocktail.");

    const int speed_pct_diff =
        player_bon::traits[(size_t)Trait::dem_expert] ?
        100 :
        0;

    game_time::tick(speed_pct_diff);
}

void Molotov::on_std_turn_player_hold_ignited()
{
    --fuse_turns_;

    if (fuse_turns_ <= 0)
    {
        msg_log::add("The Molotov Cocktail explodes in my hand!");

        map::player->active_explosive = nullptr;

        const P player_pos = map::player->pos;

        Snd snd("I hear an explosion!",
                SfxId::explosion_molotov,
                IgnoreMsgIfOriginSeen::yes,
                player_pos,
                nullptr,
                SndVol::high,
                AlertsMon::yes);

        snd.run();

        explosion::run(player_pos,
                       ExplType::apply_prop,
                       ExplSrc::misc,
                       EmitExplSnd::no,
                       0,
                       ExplExclCenter::no,
                       {new PropBurning(PropTurns::std)});

        delete this;
    }
}

void Molotov::on_thrown_ignited_landing(const P& p)
{
    const int d = player_bon::traits[(size_t)Trait::dem_expert] ? 1 : 0;

    Snd snd("I hear an explosion!",
            SfxId::explosion_molotov,
            IgnoreMsgIfOriginSeen::yes,
            p,
            nullptr,
            SndVol::high,
            AlertsMon::yes);

    snd.run();

    explosion::run(p,
                   ExplType::apply_prop,
                   ExplSrc::player_use_moltv_intended,
                   EmitExplSnd::no,
                   d,
                   ExplExclCenter::no,
                   {new PropBurning(PropTurns::std)});
}


void Molotov::on_player_paralyzed()
{
    msg_log::add("The lit Molotov Cocktail falls from my hand!");

    map::player->active_explosive = nullptr;

    const P player_pos = map::player->pos;

    Snd snd("I hear an explosion!",
            SfxId::explosion_molotov,
            IgnoreMsgIfOriginSeen::yes,
            player_pos,
            nullptr,
            SndVol::high,
            AlertsMon::yes);

    snd.run();

    explosion::run(player_pos,
                   ExplType::apply_prop,
                   ExplSrc::misc,
                   EmitExplSnd::no,
                   0,
                   ExplExclCenter::no,
                   {new PropBurning(PropTurns::std)});

    delete this;
}

// -----------------------------------------------------------------------------
// Flare
// -----------------------------------------------------------------------------
void Flare::on_player_ignite() const
{
    msg_log::add("I light a Flare.");

    const int speed_pct_diff =
        player_bon::traits[(size_t)Trait::dem_expert] ?
        100 :
        0;

    game_time::tick(speed_pct_diff);
}

void Flare::on_std_turn_player_hold_ignited()
{
    --fuse_turns_;

    if (fuse_turns_ <= 0)
    {
        msg_log::add("The flare is extinguished.");

        map::player->active_explosive = nullptr;

        delete this;
    }
}

void Flare::on_thrown_ignited_landing(const P& p)
{
    game_time::add_mob(new LitFlare(p, fuse_turns_));
}

void Flare::on_player_paralyzed()
{
    msg_log::add("The lit Flare falls from my hand!");

    map::player->active_explosive = nullptr;

    const P& p = map::player->pos;
    auto* const f = map::cells[p.x][p.y].rigid;

    if (!f->is_bottomless())
    {
        game_time::add_mob(new LitFlare(p, fuse_turns_));
    }

    delete this;
}

// -----------------------------------------------------------------------------
// Smoke grenade
// -----------------------------------------------------------------------------
void SmokeGrenade::on_player_ignite() const
{
    msg_log::add("I ignite a smoke grenade.");

    const int speed_pct_diff =
        player_bon::traits[(size_t)Trait::dem_expert] ?
        100 :
        0;

    game_time::tick(speed_pct_diff);
}

void SmokeGrenade::on_std_turn_player_hold_ignited()
{
    if (fuse_turns_ < std_fuse_turns() && rnd::coin_toss())
    {
        const int d =
            player_bon::traits[(size_t)Trait::dem_expert] ?
            1 : 0;

        explosion::run_smoke_explosion_at(map::player->pos, d);
    }

    --fuse_turns_;

    if (fuse_turns_ <= 0)
    {
        msg_log::add("The smoke grenade is extinguished.");

        map::player->active_explosive = nullptr;

        delete this;
    }
}

void SmokeGrenade::on_thrown_ignited_landing(const P& p)
{
    const int d =
        player_bon::traits[(size_t)Trait::dem_expert] ?
        1 : 0;

    explosion::run_smoke_explosion_at(p, d);
}

void SmokeGrenade::on_player_paralyzed()
{
    msg_log::add("The ignited smoke grenade falls from my hand!");

    map::player->active_explosive = nullptr;

    const P& p = map::player->pos;
    auto* const f = map::cells[p.x][p.y].rigid;

    if (!f->is_bottomless())
    {
        explosion::run_smoke_explosion_at(map::player->pos);
    }

    delete this;
}

Clr SmokeGrenade::ignited_projectile_clr() const
{
    return data().clr;
}
