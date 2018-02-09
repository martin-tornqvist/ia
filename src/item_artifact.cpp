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
#include "actor_player.hpp"
#include "property.hpp"
#include "property_data.hpp"

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
    auto prop = new PropSpellReflect();

    prop->set_indefinite();

    add_carrier_prop(prop, Verbosity::silent);
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
// Talisman of Teleporation Control
// -----------------------------------------------------------------------------
TeleCtrlTalisman::TeleCtrlTalisman(ItemDataT* const item_data) :
    Item(item_data)
{

}

void TeleCtrlTalisman::on_pickup_hook()
{
    auto prop = new PropTeleControl();

    prop->set_indefinite();

    add_carrier_prop(prop, Verbosity::silent);
}

void TeleCtrlTalisman::on_removed_from_inv_hook()
{
    clear_carrier_props();
}

// -----------------------------------------------------------------------------
// Horn of Malice
// -----------------------------------------------------------------------------
void HornOfMaliceHeard::run(Actor& actor) const
{
    if (!actor.is_player())
    {
        actor.apply_prop(
            new PropConflict());
    }
}

HornOfMalice::HornOfMalice(ItemDataT* const item_data) :
    Item(item_data),
    charges_(rnd::range(4, 6))
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
            const std::string name_the =
                text_format::first_to_upper(
                    actor.name_the());

            msg_log::add(name_the + mon_disappear_msg);
        }

        actor.set_state(ActorState::destroyed);
    }
}

HornOfBanishment::HornOfBanishment(ItemDataT* const item_data) :
    Item(item_data),
    charges_(rnd::range(4, 6))
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
    charges_(rnd::range(4, 6))
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

    props.apply(new PropClockworkHasted());

    --charges_;

    game_time::tick();

    return ConsumeItem::no;
}

void Clockwork::on_pickup_hook()
{
    auto prop = new PropRSlow();

    prop->set_indefinite();

    add_carrier_prop(prop, Verbosity::verbose);
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

void SpiritDagger::on_melee_hit(Actor& actor_hit, const int dmg)
{
    (void)dmg;

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
OrbOfLife::OrbOfLife(ItemDataT* const item_data) :
    Item(item_data)
{

}

void OrbOfLife::on_pickup_hook()
{
    map::player->change_max_hp(4, Verbosity::verbose);

    auto prop_r_poison = new PropRPoison();

    prop_r_poison->set_indefinite();

    add_carrier_prop(prop_r_poison, Verbosity::verbose);

    auto prop_r_disease = new PropRDisease();

    prop_r_disease->set_indefinite();

    add_carrier_prop(prop_r_disease, Verbosity::verbose);
}

void OrbOfLife::on_removed_from_inv_hook()
{
    map::player->change_max_hp(-4, Verbosity::verbose);

    clear_carrier_props();
}
