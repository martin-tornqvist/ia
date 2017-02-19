#include "init.hpp"

#include "item_artifact.hpp"

#include "map.hpp"
#include "msg_log.hpp"
#include "game_time.hpp"
#include "io.hpp"
#include "actor_mon.hpp"
#include "text_format.hpp"
#include "actor_factory.hpp"
#include "feature_rigid.hpp"
#include "saving.hpp"
#include "game.hpp"


// -----------------------------------------------------------------------------
// Staff of the pharaohs
// -----------------------------------------------------------------------------
PharaohStaff::PharaohStaff(ItemDataT* const item_data) :
    Wpn(item_data)
{
    add_carrier_spell(new SpellPharaohStaff);
}


// -----------------------------------------------------------------------------
// Talisman of Reflection
// -----------------------------------------------------------------------------
ReflTalisman::ReflTalisman(ItemDataT* const item_data) :
    Item(item_data)
{

}

void ReflTalisman::on_pickup_hook()
{
    add_carrier_prop(
        new PropSpellReflect(PropTurns::indefinite),
        Verbosity::silent);
}

void ReflTalisman::on_removed_from_inv_hook()
{
    clear_carrier_props();
}

// -----------------------------------------------------------------------------
// Talisman of Resurrection
// -----------------------------------------------------------------------------
ResurrectTalisman::ResurrectTalisman(ItemDataT* const item_data) :
    Item(item_data)
{

}

// -----------------------------------------------------------------------------
// Horn of Malice
// -----------------------------------------------------------------------------
void HornOfMaliceHeard::run(Actor& actor) const
{
    if (!actor.is_player())
    {
        actor.prop_handler().try_add(
            new PropConflict(PropTurns::std));
    }
}

HornOfMalice::HornOfMalice(ItemDataT* const item_data) :
    Item(item_data),
    charges_(rnd::range(12, 18))
{

}

std::string HornOfMalice::name_inf() const
{
    return "{" + std::to_string(charges_) + "}";
}

void HornOfMalice::save()
{
    saving::put_int(charges_);
}

void HornOfMalice::load()
{
    charges_ = saving::get_int();
}

ConsumeItem HornOfMalice::activate(Actor* const actor)
{
    (void)actor;

    if (charges_ <= 0)
    {
        msg_log::add("It makes no sound.");

        return ConsumeItem::no;
    }

    auto effect = std::shared_ptr<SndHeardEffect>(
        new HornOfMaliceHeard);

    Snd snd("The Horn of Malice resounds!",
            SfxId::END, // TODO: Make a sound effect
            IgnoreMsgIfOriginSeen::no,
            map::player->pos,
            map::player,
            SndVol::high,
            AlertsMon::yes,
            MorePromptOnMsg::no,
            effect);

    snd_emit::run(snd);

    --charges_;

    game_time::tick();

    return ConsumeItem::no;
}

// -----------------------------------------------------------------------------
// Horn of Banishment
// -----------------------------------------------------------------------------
void HornOfBanishmentHeard::run(Actor& actor) const
{
    if (actor.has_prop(PropId::summoned))
    {
        if (map::player->can_see_actor(actor))
        {
            msg_log::add(actor.name_the() + mon_disappear_msg);
        }

        actor.set_destroyed();
    }
}

HornOfBanishment::HornOfBanishment(ItemDataT* const item_data) :
    Item(item_data),
    charges_(rnd::range(12, 18))
{

}

std::string HornOfBanishment::name_inf() const
{
    return "{" + std::to_string(charges_) + "}";
}

void HornOfBanishment::save()
{
    saving::put_int(charges_);
}

void HornOfBanishment::load()
{
    charges_ = saving::get_int();
}

ConsumeItem HornOfBanishment::activate(Actor* const actor)
{
    (void)actor;

    if (charges_ <= 0)
    {
        msg_log::add("It makes no sound.");

        return ConsumeItem::no;
    }

    auto effect = std::shared_ptr<SndHeardEffect>(
        new HornOfBanishmentHeard);

    Snd snd("The Horn of Banishment resounds!",
            SfxId::END, // TODO: Make a sound effect
            IgnoreMsgIfOriginSeen::no,
            map::player->pos,
            map::player,
            SndVol::high,
            AlertsMon::yes,
            MorePromptOnMsg::no,
            effect);

    snd_emit::run(snd);

    --charges_;

    game_time::tick();

    return ConsumeItem::no;
}

// -----------------------------------------------------------------------------
// Arcane Clockwork
// -----------------------------------------------------------------------------
Clockwork::Clockwork(ItemDataT* const item_data) :
    Item(item_data),
    charges_(rnd::range(12, 18))
{

}

std::string Clockwork::name_inf() const
{
    return "{" + std::to_string(charges_) + "}";
}

void Clockwork::save()
{
    saving::put_int(charges_);
}

void Clockwork::load()
{
    charges_ = saving::get_int();
}

ConsumeItem Clockwork::activate(Actor* const actor)
{
    (void)actor;

    if (charges_ <= 0)
    {
        msg_log::add("Nothing happens.");

        return ConsumeItem::no;
    }

    auto& props = map::player->prop_handler();

    if (props.has_prop(PropId::clockwork_hasted))
    {
        msg_log::add("It will not move.");

        return ConsumeItem::no;
    }

    msg_log::add("I wind up the clockwork.");

    map::player->incr_shock(ShockLvl::terrifying,
                            ShockSrc::use_strange_item);

    if (!map::player->is_alive())
    {
        return ConsumeItem::no;
    }

    props.try_add(new PropClockworkHasted(PropTurns::std));

    --charges_;

    game_time::tick();

    return ConsumeItem::no;
}

void Clockwork::on_pickup_hook()
{
    add_carrier_prop(
        new PropRSlow(PropTurns::indefinite),
        Verbosity::verbose);
}

void Clockwork::on_removed_from_inv_hook()
{
    clear_carrier_props();
}

// -----------------------------------------------------------------------------
// Spirit Dagger
// -----------------------------------------------------------------------------
SpiritDagger::SpiritDagger(ItemDataT* const item_data) :
    Wpn(item_data)
{

}

void SpiritDagger::on_melee_hit(Actor& actor_hit)
{
    // Only drain spirit if the victim did not die from the normal attack,
    // otherwise the actor might be killed twice
    if (actor_hit.is_alive())
    {
        actor_hit.hit_spi(1, Verbosity::verbose);
    }

    // We give the player spirit regardless of the above
    map::player->restore_spi(1, true, Verbosity::verbose);
}

// -----------------------------------------------------------------------------
// Sorcery Orb
// -----------------------------------------------------------------------------
OrbOfSorcery::OrbOfSorcery(ItemDataT* const item_data) :
    Item(item_data)
{

}

// -----------------------------------------------------------------------------
// Sorcery Orb
// -----------------------------------------------------------------------------
OrbOfLife::OrbOfLife(ItemDataT* const item_data) :
    Item(item_data)
{

}

void OrbOfLife::on_std_turn_in_inv(const InvType inv_type)
{
    (void)inv_type;

    ASSERT(actor_carrying_);

    const int turns_per_hp = 4;

    if (game_time::turn_nr() % turns_per_hp == 0)
    {
        actor_carrying_->restore_hp(1, false, Verbosity::silent);
    }
}

void OrbOfLife::on_pickup_hook()
{
    map::player->change_max_hp(4, Verbosity::verbose);

    add_carrier_prop(
        new PropRPoison(PropTurns::indefinite),
        Verbosity::verbose);

    add_carrier_prop(
        new PropRDisease(PropTurns::indefinite),
        Verbosity::verbose);
}

void OrbOfLife::on_removed_from_inv_hook()
{
    map::player->change_max_hp(-4, Verbosity::verbose);

    clear_carrier_props();
}
