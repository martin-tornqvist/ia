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

// -----------------------------------------------------------------------------
// Item
// -----------------------------------------------------------------------------
Item::Item(ItemDataT* item_data) :
    nr_items_       (1),
    melee_dmg_plus_ (0),
    data_           (item_data),
    actor_carrying_ (nullptr),
    carrier_props_  (),
    carrier_spells_ () {}

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
    if (actor_carrying_)
    {
        on_removed_from_inv();
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

DiceParam Item::dmg(const AttMode att_mode, const Actor* const actor) const
{
    DiceParam out;

    switch (att_mode)
    {
    case AttMode::melee:
    {
        out = data_->melee.dmg;
        out.plus = melee_dmg_plus_;

        if (actor == map::player)
        {
            if (player_bon::traits[(size_t)Trait::adept_melee_fighter])
            {
                ++out.plus;
            }

            if (player_bon::traits[(size_t)Trait::expert_melee_fighter])
            {
                ++out.plus;
            }
        }

        //Bonus damage from being frenzied?
        if (actor && actor->has_prop(PropId::frenzied))
        {
            ++out.plus;
        }
    }
    break;

    case AttMode::ranged:
    {
        out = data_->ranged.dmg;

        if (actor == map::player)
        {
            if ( player_bon::traits[(size_t)Trait::adept_marksman])
            {
                ++out.plus;
            }

            if ( player_bon::traits[(size_t)Trait::expert_marksman])
            {
                ++out.plus;
            }
        }
    }
    break;

    case AttMode::thrown:
    {
        const bool is_melee_wpn = data_->type == ItemType::melee_wpn;

        //Melee weapons do throw damage based on their melee damage
        if (is_melee_wpn)
        {
            out = data_->melee.dmg;
            out.plus = melee_dmg_plus_;
        }
        else //Not a melee weapon
        {
            out = data_->ranged.throw_dmg;
        }

        if (actor == map::player)
        {
            if ( player_bon::traits[(size_t)Trait::adept_marksman])
            {
                ++out.plus;
            }

            if ( player_bon::traits[(size_t)Trait::expert_marksman])
            {
                ++out.plus;
            }
        }
    }
    break;

    case AttMode::none:
    {
        ASSERT(false);
        break;
    }
    }

    return out;
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

    on_pickup_hook();
}

void Item::on_equip(const Verbosity verbosity)
{
    ASSERT(actor_carrying_);

    on_equip_hook(verbosity);
}

UnequipAllowed Item::on_unequip()
{
    ASSERT(actor_carrying_);

    return on_unequip_hook();
}

void Item::on_removed_from_inv()
{
    actor_carrying_ = nullptr;

    on_removed_from_inv_hook();
}

std::string Item::name(const ItemRefType ref_type,
                       const ItemRefInf inf,
                       const ItemRefAttInf att_inf) const
{
    ItemRefType ref_type_used = ref_type;

    // If requested ref type is "plural" and this is a single item, use ref type
    // "a" instead.
    if (ref_type == ItemRefType::plural &&
        (!data_->is_stackable || nr_items_ == 1))
    {
        ref_type_used = ItemRefType::a;
    }

    std::string nr_str = "";

    if (ref_type_used == ItemRefType::plural)
    {
        nr_str = to_str(nr_items_) + " ";
    }

    std::string att_str = "";

    ItemRefAttInf att_inf_used = att_inf;

    if (att_inf == ItemRefAttInf::wpn_context)
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

    if (att_inf_used == ItemRefAttInf::melee)
    {
        const DiceParam dmg_dice = dmg(AttMode::melee, map::player);

        const std::string rolls_str = to_str(dmg_dice.rolls);
        const std::string sides_str = to_str(dmg_dice.sides);

        const int plus = dmg_dice.plus;

        const std::string plus_str =
            plus == 0 ? "" :
            plus > 0 ?
            ("+" + to_str(plus)) :
            ("-" + to_str(plus));

        const int item_skill = data_->melee.hit_chance_mod;

        const int melee_skill =
            map::player->ability(AbilityId::melee, true);

        const int skill_tot =
            std::max(0, std::min(100, item_skill + melee_skill));

        const std::string skill_str = to_str(skill_tot) + "%";

        att_str =
            " " +
            rolls_str +
            "d" +
            sides_str +
            plus_str +
            " " +
            skill_str;
    }

    const int ranged_skill = map::player->ability(AbilityId::ranged, true);

    if (att_inf_used == ItemRefAttInf::ranged)
    {
        std::string dmg_str = data_->ranged.dmg_info_override;

        if (dmg_str.empty())
        {
            const DiceParam dmg_dice = dmg(AttMode::ranged, map::player);

            const int mul =
                data_->ranged.is_machine_gun ?
                nr_mg_projectiles : 1;

            const std::string rolls_str = to_str(dmg_dice.rolls * mul);
            const std::string sides_str = to_str(dmg_dice.sides);
            const int plus = dmg_dice.plus * mul;

            const std::string plus_str = plus == 0 ? "" :
                plus > 0 ?
                ("+" + to_str(plus)) :
                ("-" + to_str(plus));

            dmg_str = rolls_str + "d" + sides_str + plus_str;
        }

        const int item_skill = data_->ranged.hit_chance_mod;

        const int skill_tot =
            std::max(0, std::min(100, item_skill + ranged_skill));

        const std::string skill_str = to_str(skill_tot) + "%";

        att_str =
            " " +
            dmg_str +
            " " +
            skill_str;
    }

    if (att_inf_used == ItemRefAttInf::thrown)
    {
        const DiceParam dmg_dice = dmg(AttMode::thrown, map::player);

        const std::string rolls_str = to_str(dmg_dice.rolls);
        const std::string sides_str = to_str(dmg_dice.sides);
        const int plus = dmg_dice.plus;

        const std::string plus_str =
            plus == 0 ? "" :
            plus  > 0 ?
            ("+" + to_str(plus)) :
            ("-" + to_str(plus));

        const int item_skill = data_->ranged.throw_hit_chance_mod;

        const int skill_tot =
            std::max(0, std::min(100, item_skill + ranged_skill));

        const std::string skill_str = to_str(skill_tot) + "%";

        att_str =
            " " +
            rolls_str +
            "d" +
            sides_str +
            plus_str +
            " " +
            skill_str;
    }

    std::string inf_str = "";

    if (inf == ItemRefInf::yes)
    {
        inf_str = name_inf();

        if (!inf_str.empty())
        {
            inf_str.insert(0, " ");
        }
    }

    const auto& names_used =
        data_->is_identified ?
        data_->base_name : data_->base_name_un_id;

    const std::string base_name = names_used.names[(size_t)ref_type_used];

    const std::string ret = nr_str + base_name + att_str + inf_str;

    ASSERT(!ret.empty());

    return ret;
}

bool Item::is_in_effective_range_lmt(const P& p0, const P& p1) const
{
    return king_dist(p0, p1) <= data_->ranged.effective_range;
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

std::string Armor::armor_points_str(const bool with_brackets) const
{
    const int ap = armor_points();
    const std::string ap_str = to_str(std::max(1, ap));

    return with_brackets ? ("[" + ap_str + "]") : ap_str;
}

int Armor::take_dur_hit_and_get_reduced_dmg(const int dmg_before)
{
    TRACE_FUNC_BEGIN;

    //AP:  Armor points
    //     Damage soaked up instead of hitting the player
    //DFF: Damage (to) Durability Factor
    //     A factor of how much damage the armor durability takes per attack damage point

    const int ap_before = armor_points();

    //TODO: Add check for if wearer is player!

    //Damage factor
    const double dmg_before_db = double(dmg_before);

    //Adjustment factor
    const double k = 2.0;

    //Armor durability factor
    const double armor_ddf = data_->armor.dmg_to_durability_factor;

    //Armor lasts twice as long for War Vets
    const double war_vet_ddf = (player_bon::bg() == Bg::war_vet) ? 0.5 : 1.0;

    dur_ -= int(dmg_before_db * k * armor_ddf * war_vet_ddf);

    dur_ = std::max(0, dur_);

    const int ap_after = armor_points();

    if (ap_after < ap_before && ap_after != 0)
    {
        const std::string armor_name = name(ItemRefType::plain);
        msg_log::add("My " + armor_name + " is damaged!", clr_msg_note);
    }

    TRACE << "Damage before: " + to_str(dmg_before) << std::endl;

    const int dmg_after = std::max(1, dmg_before - ap_before);

    TRACE << "Damage after: " + to_str(dmg_after) << std::endl;

    TRACE_FUNC_END;
    return dmg_after;
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

void ArmorAsbSuit::on_equip_hook(const Verbosity verbosity)
{
    (void)verbosity;

    add_carrier_prop(new PropRFire(PropTurns::indefinite), Verbosity::silent);
    add_carrier_prop(new PropRAcid(PropTurns::indefinite), Verbosity::silent);
    add_carrier_prop(new PropRElec(PropTurns::indefinite), Verbosity::silent);
    add_carrier_prop(new PropRBreath(PropTurns::indefinite), Verbosity::silent);
}

UnequipAllowed ArmorAsbSuit::on_unequip_hook()
{
    clear_carrier_props();

    return UnequipAllowed::yes;
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

UnequipAllowed ArmorMiGo::on_unequip_hook()
{
    msg_log::add("I attempt to tear off the armor, it rips my skin!",
                 clr_msg_bad,
                 false,
                 MorePromptOnMsg::yes);

    map::player->hit(rnd::range(1, 3), DmgType::pure);

    if (rnd::coin_toss())
    {
        //NOTE: There is no need to print a message here, a message is always
        //      printed when taking off armor.
        return UnequipAllowed::yes;
    }
    else //Armor is stuck
    {
        msg_log::add("I fail to tear it off.",
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);

        return UnequipAllowed::no;
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
        is_low_dlvl ? 1  : 10   //Total:    543 or 590
    };

    melee_dmg_plus_ = rnd::weighted_choice(weights);
}

std::string Wpn::name_inf() const
{
    if (data_->ranged.is_ranged_wpn && !data_->ranged.has_infinite_ammo)
    {
        return to_str(nr_ammo_loaded_) + "/" + to_str(data_->ranged.max_ammo);
    }

    return "";
}

// -----------------------------------------------------------------------------
// Player ghoul claw
// -----------------------------------------------------------------------------
void PlayerGhoulClaw::on_melee_hit(Actor& actor_hit)
{
    //TODO: If some "constructed" monster is added (something not made of flesh, e.g. a golem),
    //then a Ghoul player would be able to feed from it, which would be a problem. In that case,
    //there should probably be a field in the actor data called something like either
    //"is_flesh_body", or "is_construct".


    //Ghoul feeding from Ravenous trait?
    //NOTE: Player should never feed on monsters such as Ghosts or Shadows. Checking that the
    //monster is not Ethereal and that it can bleed should be a pretty good rule for this.
    //We should NOT check if the monster can leave a corpse however, since some monsters such
    //as Worms don't leave a corpse, and you should be able to feed on those.
    const ActorDataT& d = actor_hit.data();

    const bool is_ethereal = actor_hit.has_prop(PropId::ethereal);

    const bool is_hp_missing = map::player->hp() < map::player->hp_max(true);
    const bool is_wounded = map::player->prop_handler().prop(PropId::wound);
    const bool is_feed_needed = is_hp_missing || is_wounded;

    if (!is_ethereal &&
        d.can_bleed &&
        player_bon::traits[(size_t)Trait::ravenous] &&
        is_feed_needed &&
        rnd::one_in(4))
    {
        Snd snd("",
                SfxId::bite,
                IgnoreMsgIfOriginSeen::yes,
                actor_hit.pos,
                map::player,
                SndVol::low,
                AlertsMon::yes,
                MorePromptOnMsg::no);

        snd_emit::run(snd);

        map::player->on_feed();
    }

    if (actor_hit.state() == ActorState::alive)
    {
        //Poison victim from Ghoul Toxic trait?
        if (player_bon::traits[(size_t)Trait::toxic] &&
            rnd::one_in(4))
        {
            Prop* const poison = new PropPoisoned(PropTurns::std);

            actor_hit.prop_handler().try_add(poison);
        }

        //Terrify victim from Ghoul Indomitable Fury trait?
        if (player_bon::traits[(size_t)Trait::indomitable_fury] &&
            map::player->has_prop(PropId::frenzied))
        {
            Prop* const fear = new PropTerrified(PropTurns::std);

            actor_hit.prop_handler().try_add(fear);
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
        const int nr_worms = rnd::range(1, 2);

        std::vector<ActorId> ids(nr_worms, ActorId::worm_mass);

        actor_factory::summon(actor_killed.pos,
                              ids,
                              MakeMonAware::yes,
                              map::player,
                              nullptr,
                              Verbosity::silent);
    }
}

// -----------------------------------------------------------------------------
// Staff of the pharaohs
// -----------------------------------------------------------------------------
PharaohStaff::PharaohStaff(ItemDataT* const item_data) :
    Wpn(item_data)
{
    item_data->allow_spawn = false;

    add_carrier_spell(new SpellPharaohStaff);
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
void RavenPeck::on_melee_hit(Actor& actor_hit)
{
    if (!actor_hit.is_alive())
    {
        return;
    }

    //Gas mask and Asbesthos suit protects against blindness
    Item* const head_item = actor_hit.inv().item_in_slot(SlotId::head);
    Item* const body_item = actor_hit.inv().item_in_slot(SlotId::body);

    if ((head_item && head_item->id() == ItemId::gas_mask) ||
        (body_item && body_item->id() == ItemId::armor_asb_suit))
    {
        return;
    }

    Prop* const prop = new PropBlind(PropTurns::specific, 3);

    actor_hit.prop_handler().try_add(prop);
}

// -----------------------------------------------------------------------------
// Dust vortex enguld
// -----------------------------------------------------------------------------
void DustVortexEngulf::on_melee_hit(Actor& actor_hit)
{
    if (!actor_hit.is_alive())
    {
        return;
    }

    //Gas mask and Asbesthos suit protects against blindness
    Item* const head_item = actor_hit.inv().item_in_slot(SlotId::head);
    Item* const body_item = actor_hit.inv().item_in_slot(SlotId::body);

    if ((head_item && head_item->id() == ItemId::gas_mask) ||
        (body_item && body_item->id() == ItemId::armor_asb_suit))
    {
        return;
    }

    Prop* const prop = new PropBlind(PropTurns::std);

    actor_hit.prop_handler().try_add(prop);
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

    //Gas mask and Asbesthos suit protects against blindness
    Item* const head_item = actor_hit.inv().item_in_slot(SlotId::head);
    Item* const body_item = actor_hit.inv().item_in_slot(SlotId::body);

    if ((head_item && head_item->id() == ItemId::gas_mask) ||
        (body_item && body_item->id() == ItemId::armor_asb_suit))
    {
        return;
    }

    Prop* const prop = new PropBlind(PropTurns::specific, 7);

    actor_hit.prop_handler().try_add(prop);
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
    nr_supplies_            (50),
    nr_turns_left_action_   (-1),
    current_action_             (MedBagAction::END) {}

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

    auto& inv = actor_carrying_->inv();

    //Check for existing medical bag in inventory
    for (Item* const other : inv.backpack_)
    {
        if (other != this && other->id() == id())
        {
            //Add my turns left to the other medical bag, then destroy self
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

    std::vector<Actor*> seen_foes;

    map::player->seen_foes(seen_foes);

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

    //Action can be done
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

    start_msg += " (" + to_str(nr_turns_left_action_) + " turns)...";

    msg_log::add(start_msg);

    game_time::tick();

    return ConsumeItem::no;
}

MedBagAction MedicalBag::choose_action() const
{
    //Infection?
    if (map::player->has_prop(PropId::infected))
    {
        return MedBagAction::sanitize_infection;
    }

    //Wound?
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
    else //Time still remaining on the current action
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
        Prop* const wound_prop = map::player->prop_handler().prop(PropId::wound);

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
//                    actor->prop_handler().try_add(
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

    add_carrier_prop(new PropRBreath(PropTurns::indefinite),
                     Verbosity::silent);
}

UnequipAllowed GasMask::on_unequip_hook()
{
    clear_carrier_props();

    return UnequipAllowed::yes;
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

        msg_log::add("Light " + name + " [y/n]?");

        auto result = query::yes_or_no();

        if (result == YesNoAnswer::no)
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
    const bool is_dem_exp = player_bon::traits[(size_t)Trait::dem_expert];

    const PassTime pass_time =
        (is_dem_exp && rnd::coin_toss()) ?
        PassTime::no :
        PassTime::yes;

    const std::string swift_str =
        (pass_time == PassTime::no) ?
        "swiftly " : "";

    msg_log::add("I " + swift_str + "light a dynamite stick.");

    game_time::tick(pass_time);
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
    else //Fuse has run out
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
    const bool is_dem_exp = player_bon::traits[(size_t)Trait::dem_expert];

    const PassTime pass_time = (is_dem_exp && rnd::coin_toss()) ?
                                PassTime::no :
                                PassTime::yes;

    const std::string swift_str = (pass_time == PassTime::no) ? "swiftly " : "";

    msg_log::add("I " + swift_str + "light a Molotov Cocktail.");

    game_time::tick(pass_time);
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

        snd_emit::run(snd);

        explosion::run(player_pos,
                       ExplType::apply_prop,
                       ExplSrc::misc,
                       EmitExplSnd::no,
                       0,
                       new PropBurning(PropTurns::std));

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

    snd_emit::run(snd);

    explosion::run(p,
                   ExplType::apply_prop,
                   ExplSrc::player_use_moltv_intended,
                   EmitExplSnd::no,
                   d,
                   new PropBurning(PropTurns::std));
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

    snd_emit::run(snd);

    explosion::run(player_pos,
                   ExplType::apply_prop,
                   ExplSrc::misc,
                   EmitExplSnd::no,
                   0,
                   new PropBurning(PropTurns::std));

    delete this;
}

// -----------------------------------------------------------------------------
// Flare
// -----------------------------------------------------------------------------
void Flare::on_player_ignite() const
{
    const bool is_dem_exp = player_bon::traits[(size_t)Trait::dem_expert];

    const PassTime pass_time = (is_dem_exp && rnd::coin_toss()) ?
                                PassTime::no :
                                PassTime::yes;

    const std::string swift_str = (pass_time == PassTime::no) ? "swiftly " : "";

    msg_log::add("I " + swift_str + "light a Flare.");

    game_time::tick(pass_time);
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
    const bool is_dem_exp = player_bon::traits[(size_t)Trait::dem_expert];

    const PassTime pass_time =
        (is_dem_exp && rnd::coin_toss()) ?
        PassTime::no :
        PassTime::yes;

    const std::string swift_str =
        (pass_time == PassTime::no) ?
        "swiftly " : "";

    msg_log::add("I " + swift_str + "ignite a smoke grenade.");

    game_time::tick(pass_time);
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
