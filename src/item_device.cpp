#include "item_device.hpp"

#include <algorithm>
#include <vector>

#include "init.hpp"
#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "io.hpp"
#include "game_time.hpp"
#include "msg_log.hpp"
#include "knockback.hpp"
#include "inventory.hpp"
#include "map.hpp"
#include "audio.hpp"
#include "feature_rigid.hpp"
#include "actor_factory.hpp"
#include "saving.hpp"
#include "game.hpp"
#include "text_format.hpp"
#include "map_parsing.hpp"
#include "property.hpp"
#include "property_data.hpp"

// -----------------------------------------------------------------------------
// Device
// -----------------------------------------------------------------------------
Device::Device(ItemDataT* const item_data) :
    Item(item_data) {}

void Device::identify(const Verbosity verbosity)
{
    if (!data_->is_identified)
    {
        data_->is_identified = true;

        if (verbosity == Verbosity::verbose)
        {
            const std::string name_after =
                name(ItemRefType::a, ItemRefInf::none);

            msg_log::add("I have identified " + name_after + ".");

            msg_log::add("All its properties are now known to me.");

            game::add_history_event("Comprehended " + name_after + ".");

            game::incr_player_xp(15);
        }
    }
}

// -----------------------------------------------------------------------------
// Strange device
// -----------------------------------------------------------------------------
StrangeDevice::StrangeDevice(ItemDataT* const item_data) :
    Device(item_data),
    condition_(rnd::coin_toss() ? Condition::fine : Condition::shoddy) {}

void StrangeDevice::save()
{
    saving::put_int(int(condition_));
}

void StrangeDevice::load()
{
    condition_ = Condition(saving::get_int());
}

std::vector<std::string> StrangeDevice::descr() const
{
    if (data_->is_identified)
    {
        const std::string descr = descr_identified();

        std::vector<std::string> out = {descr};

        std::string cond_str = "It seems ";

        switch (condition_)
        {
        case Condition::fine:
            cond_str += "to be in fine condition.";
            break;

        case Condition::shoddy:
            cond_str += "to be in shoddy condition.";
            break;

        case Condition::breaking:
            cond_str += "almost broken.";
            break;
        }

        out.push_back(cond_str);

        return out;
    }
    else // Not identified
    {
        return data_->base_descr;
    }
}

ConsumeItem StrangeDevice::activate(Actor* const actor)
{
    ASSERT(actor);

    if (data_->is_identified)
    {
        const std::string item_name =
            name(ItemRefType::plain, ItemRefInf::none);

        const std::string item_name_a =
            name(ItemRefType::a, ItemRefInf::none);

        msg_log::add("I activate " + item_name_a + "...");

        // Damage user? Fail to run effect? Condition degrade? Warning?
        const std::string hurt_msg  = "It hits me with a jolt of electricity!";

        bool is_effect_failed = false;

        bool is_cond_degrade = false;

        bool is_warning = false;

        int max = 8;

        if (actor->is_player() &&
            player_bon::traits[(size_t)Trait::elec_incl])
        {
            max += 2;
        }

        const int rnd = rnd::range(1, max);

        switch (condition_)
        {
        case Condition::breaking:
        {
            is_cond_degrade = (rnd <= 2);

            is_effect_failed =
                (rnd == 3) ||
                (rnd == 4);

            if ((rnd == 5) ||
                (rnd == 6))
            {
                msg_log::add(hurt_msg, colors::msg_bad());

                actor->hit(rnd::range(1, 3), DmgType::electric);
            }

            is_warning =
                (rnd == 7) ||
                (rnd == 8);
        }
        break;

        case Condition::shoddy:
        {
            is_cond_degrade = (rnd <= 2);

            is_effect_failed = (rnd == 3);

            if (rnd == 4)
            {
                msg_log::add(hurt_msg, colors::msg_bad());

                actor->hit(1, DmgType::electric);
            }

            is_warning =
                (rnd == 5) ||
                (rnd == 6);
        }
        break;

        case Condition::fine:
        {
            is_cond_degrade = (rnd <= 2);

            is_warning =
                (rnd == 3) ||
                (rnd == 4);
        }
        break;
        }

        if (!map::player->is_alive())
        {
            return ConsumeItem::no;
        }

        ConsumeItem consumed = ConsumeItem::no;

        if (is_effect_failed)
        {
            msg_log::add("It suddenly stops.");
        }
        else
        {
            consumed = run_effect();
        }

        if (consumed == ConsumeItem::no)
        {
            if (is_cond_degrade)
            {
                if (condition_ == Condition::breaking)
                {
                    msg_log::add("The " + item_name + " breaks!");
                    consumed = ConsumeItem::yes;
                }
                else
                {
                    msg_log::add("The " +
                                 item_name +
                                 " makes a terrible grinding noise.");

                    msg_log::add("I seem to have damaged it.");

                    condition_ = Condition(int(condition_) - 1);
                }
            }

            if (is_warning)
            {
                msg_log::add("The " + item_name + " hums ominously.");
            }
        }

        map::player->incr_shock(ShockLvl::terrifying,
                                ShockSrc::use_strange_item);

        game_time::tick();

        return consumed;
    }
    else // Not identified
    {
        msg_log::add("This device is completely alien to me, ");

        msg_log::add("I could never understand it through normal means.");

        return ConsumeItem::no;
    }
}

std::string StrangeDevice::name_inf() const
{
    if (data_->is_identified)
    {
        switch (condition_)
        {
        case Condition::breaking:
            return "{breaking}";

        case Condition::shoddy:
            return "{shoddy}";

        case Condition::fine:
            return "{fine}";
        }
    }

    return "";
}

// -----------------------------------------------------------------------------
// Blaster
// -----------------------------------------------------------------------------
ConsumeItem DeviceBlaster::run_effect()
{
    const auto tgt_bucket = map::player->seen_foes();

    if (tgt_bucket.empty())
    {
        msg_log::add("It seems to peruse area.");
    }
    else // Targets are available
    {
        Spell* const spell =
            spell_handling::mk_spell_from_id(SpellId::aza_wrath);

        spell->cast(map::player,
                    SpellSkill::basic,
                    IsIntrinsic::no);

        delete spell;
    }

    return ConsumeItem::no;
}

// -----------------------------------------------------------------------------
// Rejuvenator
// -----------------------------------------------------------------------------
ConsumeItem DeviceRejuvenator::run_effect()
{
    msg_log::add("It repairs my body.");

    std::vector<PropId> props_can_heal =
    {
        PropId::blind,
        PropId::poisoned,
        PropId::infected,
        PropId::diseased,
        PropId::weakened,
        PropId::hp_sap,
        PropId::wound
    };

    for (PropId prop_id : props_can_heal)
    {
        map::player->prop_handler().end_prop(prop_id);
    }

    map::player->restore_hp(999);

    map::player->incr_shock(ShockLvl::mind_shattering,
                            ShockSrc::use_strange_item);

    return ConsumeItem::no;
}

// -----------------------------------------------------------------------------
// Translocator
// -----------------------------------------------------------------------------
ConsumeItem DeviceTranslocator::run_effect()
{
    Player* const player = map::player;

    const auto seen_foes = player->seen_foes();

    if (seen_foes.empty())
    {
        msg_log::add("It seems to peruse area.");
    }
    else // Seen targets are available
    {
        for (Actor* actor : seen_foes)
        {
            msg_log::add(text_format::first_to_upper(actor->name_the()) +
                         " is teleported.");

            io::draw_blast_at_cells(std::vector<P> {actor->pos},
                                    colors::yellow());

            actor->teleport();
        }
    }

    return ConsumeItem::no;
}

// -----------------------------------------------------------------------------
// Sentry drone
// -----------------------------------------------------------------------------
ConsumeItem DeviceSentryDrone::run_effect()
{
    msg_log::add("The Sentry Drone awakens!");

    actor_factory::spawn(map::player->pos, {ActorId::sentry_drone})
        .make_aware_of_player()
        .set_leader(map::player);

    return ConsumeItem::yes;
}

// -----------------------------------------------------------------------------
// Deafening
// -----------------------------------------------------------------------------
ConsumeItem DeviceDeafening::run_effect()
{
    msg_log::add("The device emits a piercing resonance.");

    for (Actor* const actor : game_time::actors)
    {
        if (actor->is_player())
        {
            continue;
        }

        actor->apply_prop(
            new PropDeaf());
    }

    return ConsumeItem::no;
}

// -----------------------------------------------------------------------------
// Electric lantern
// -----------------------------------------------------------------------------
DeviceLantern::DeviceLantern(ItemDataT* const item_data) :
    Device                  (item_data),
    nr_turns_left_          (100),
    is_activated_           (false) {}

std::string DeviceLantern::name_inf() const
{
    std::string inf = "{" + std::to_string(nr_turns_left_);

    if (is_activated_)
    {
        inf += ", Lit";
    }

    return inf + "}";
}

ConsumeItem DeviceLantern::activate(Actor* const actor)
{
    (void)actor;

    toggle();

    map::update_vision();

    game_time::tick();

    return ConsumeItem::no;
}

void DeviceLantern::save()
{
    saving::put_int(nr_turns_left_);
    saving::put_bool(is_activated_);
}

void DeviceLantern::load()
{
    nr_turns_left_ = saving::get_int();
    is_activated_ = saving::get_bool();
}

void DeviceLantern::on_pickup_hook()
{
    ASSERT(actor_carrying_);

    // Check for existing electric lantern in inventory
    Inventory& inv = actor_carrying_->inv();

    for (Item* const other : inv.backpack_)
    {
        if (other != this && other->id() == id())
        {
            auto* other_lantern = static_cast<DeviceLantern*>(other);

            other_lantern->nr_turns_left_ += nr_turns_left_;

            inv.remove_item_in_backpack_with_ptr(this, true);

            return;
        }
    }
}

void DeviceLantern::toggle()
{
    const std::string toggle_str =
        is_activated_ ?
        "I turn off" :
        "I turn on";

    msg_log::add(toggle_str + " an Electric Lantern.");

    is_activated_ = !is_activated_;

    // Discourage flipping on and off frequently
    if (is_activated_ && (nr_turns_left_ >= 4))
    {
        nr_turns_left_ -= 2;
    }

    audio::play(SfxId::lantern);
}

void DeviceLantern::on_std_turn_in_inv(const InvType inv_type)
{
    (void)inv_type;

    if (!is_activated_)
    {
        return;
    }

    if (!(player_bon::has_trait(Trait::elec_incl) &&
          ((game_time::turn_nr() % 2) == 0)))
    {
        --nr_turns_left_;
    }

    if (nr_turns_left_ <= 0)
    {
        msg_log::add("My Electric Lantern has expired.",
                     colors::msg_note(),
                     true,
                     MorePromptOnMsg::yes);

        game::add_history_event("My Electric Lantern expired.");

        // NOTE: The this deletes the object
        map::player->inv().remove_item_in_backpack_with_ptr(this, true);
    }
}

LgtSize DeviceLantern::lgt_size() const
{
        return
            is_activated_ ?
            LgtSize::fov :
            LgtSize::none;
}
