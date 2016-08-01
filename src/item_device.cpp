#include "item_device.hpp"

#include <algorithm>
#include <vector>

#include "init.hpp"
#include "actor_player.hpp"
#include "render.hpp"
#include "game_time.hpp"
#include "msg_log.hpp"
#include "knockback.hpp"
#include "inventory.hpp"
#include "map.hpp"
#include "audio.hpp"
#include "feature_rigid.hpp"
#include "actor_factory.hpp"
#include "save_handling.hpp"
#include "dungeon_master.hpp"

//---------------------------------------------------- DEVICE
Device::Device(ItemDataT* const item_data) :
    Item(item_data) {}

void Device::identify(const Verbosity verbosity)
{
    if (!data_->is_identified)
    {
        data_->is_identified = true;

        if (verbosity == Verbosity::verbose)
        {
            const std::string name_after = name(ItemRefType::a, ItemRefInf::none);

            msg_log::add("I have identified " + name_after + ".");

            msg_log::add("All its properties are now known to me.");

            dungeon_master::add_history_event("Comprehended " + name_after + ".");

            give_xp_for_identify();
        }
    }
}

//---------------------------------------------------- STRANGE DEVICE
StrangeDevice::StrangeDevice(ItemDataT* const item_data) :
    Device(item_data),
    condition_(rnd::coin_toss() ? Condition::fine : Condition::shoddy) {}

void StrangeDevice::save()
{
    save_handling::put_int(int(condition_));
}

void StrangeDevice::load()
{
    condition_ = Condition(save_handling::get_int());
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
    else //Not identified
    {
        return data_->base_descr;
    }
}

ConsumeItem StrangeDevice::activate(Actor* const actor)
{
    ASSERT(actor);

    if (data_->is_identified)
    {
        const std::string item_name     = name(ItemRefType::plain, ItemRefInf::none);
        const std::string item_name_a   = name(ItemRefType::a, ItemRefInf::none);

        msg_log::add("I activate " + item_name_a + "...");

        //Damage user? Fail to run effect? Condition degrade? Warning?
        const std::string hurt_msg  = "It hits me with a jolt of electricity!";

        bool is_effect_failed   = false;
        bool is_cond_degrade    = false;
        bool is_warning         = false;

        int bon = 0;

        if (actor->has_prop(PropId::blessed))
        {
            bon += 2;
        }

        if (actor->has_prop(PropId::cursed))
        {
            bon -= 2;
        }

        const int rnd = rnd::range(1, 8 + bon);

        switch (condition_)
        {
        case Condition::breaking:
        {
            if (rnd == 5 || rnd == 6)
            {
                msg_log::add(hurt_msg, clr_msg_bad);
                actor->hit(rnd::dice(2, 4), DmgType::electric);
            }

            is_effect_failed    = rnd == 3 || rnd == 4;
            is_cond_degrade     = rnd <= 2;
            is_warning          = rnd == 7 || rnd == 8;
        } break;

        case Condition::shoddy:
        {
            if (rnd == 4)
            {
                msg_log::add(hurt_msg, clr_msg_bad);
                actor->hit(rnd::dice(1, 4), DmgType::electric);
            }

            is_effect_failed    = rnd == 3;
            is_cond_degrade     = rnd <= 2;
            is_warning          = rnd == 5 || rnd == 6;
        } break;

        case Condition::fine:
        {
            is_cond_degrade     = rnd <= 2;
            is_warning          = rnd == 3 || rnd == 4;
        } break;
        }

        if (!map::player->is_alive())
        {
            return ConsumeItem::no;
        }

        ConsumeItem consumed_state = ConsumeItem::no;

        if (is_effect_failed)
        {
            msg_log::add("It suddenly stops.");
        }
        else
        {
            consumed_state = trigger_effect();
        }

        if (consumed_state == ConsumeItem::no)
        {
            if (is_cond_degrade)
            {
                if (condition_ == Condition::breaking)
                {
                    msg_log::add("The " + item_name + " breaks!");
                    consumed_state = ConsumeItem::yes;
                }
                else
                {
                    msg_log::add("The " + item_name + " makes a terrible grinding noise.");
                    msg_log::add("I seem to have damaged it.");
                    condition_ = Condition(int(condition_) - 1);
                }
            }

            if (is_warning)
            {
                msg_log::add("The " + item_name + " hums ominously.");
            }
        }

        game_time::tick();
        return consumed_state;
    }
    else //Not identified
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

//---------------------------------------------------- BLASTER
ConsumeItem DeviceBlaster::trigger_effect()
{
    std::vector<Actor*> tgt_bucket;
    map::player->seen_foes(tgt_bucket);

    if (tgt_bucket.empty())
    {
        msg_log::add("It seems to peruse area.");
    }
    else //Targets are available
    {
        Spell* const spell = spell_handling::mk_spell_from_id(SpellId::aza_wrath);
        spell->cast(map::player, false);
        delete spell;
    }

    return ConsumeItem::no;
}

//---------------------------------------------------- SHOCK WAVE
ConsumeItem DeviceShockwave::trigger_effect()
{
    msg_log::add("It triggers a shock wave around me.");

    const P& player_pos = map::player->pos;

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const P p(player_pos + P(dx, dy));
            Rigid* const rigid = map::cells[p.x][p.y].rigid;
            rigid->hit(DmgType::physical, DmgMethod::explosion);

            //game_time::update_light_map();
            map::player->update_fov();
            render::draw_map_state();
        }
    }

    for (Actor* actor : game_time::actors)
    {
        if (actor != map::player && actor->is_alive())
        {
            const P& other_pos = actor->pos;

            if (is_pos_adj(player_pos, other_pos, false))
            {
                actor->hit(rnd::dice(1, 8), DmgType::physical);

                if (actor->is_alive())
                {
                    knock_back::try_knock_back(*actor, player_pos, false, true);
                }
            }
        }
    }

    return ConsumeItem::no;
}

//---------------------------------------------------- REJUVENATOR
ConsumeItem DeviceRejuvenator::trigger_effect()
{
    msg_log::add("It repairs my body.");

    std::vector<PropId> props_can_heal =
    {
        PropId::blind,
        PropId::poisoned,
        PropId::infected,
        PropId::diseased,
        PropId::weakened,
        PropId::wound
    };

    for (PropId prop_id : props_can_heal)
    {
        map::player->prop_handler().end_prop(prop_id);
    }

    map::player->restore_hp(999);

    return ConsumeItem::no;
}

//---------------------------------------------------- TRANSLOCATOR
ConsumeItem DeviceTranslocator::trigger_effect()
{
    Player* const player = map::player;
    std::vector<Actor*> seen_foes;
    player->seen_foes(seen_foes);

    if (seen_foes.empty())
    {
        msg_log::add("It seems to peruse area.");
    }
    else //Seen targets are available
    {
        for (Actor* actor : seen_foes)
        {
            msg_log::add(actor->name_the() + " is teleported.");
            render::draw_blast_at_cells(std::vector<P> {actor->pos}, clr_yellow);
            actor->teleport();
        }
    }

    return ConsumeItem::no;
}

//---------------------------------------------------- SENTRY DRONE
ConsumeItem DeviceSentryDrone::trigger_effect()
{
    msg_log::add("The Sentry Drone awakens!");
    actor_factory::summon(map::player->pos,
                          {ActorId::sentry_drone},
                          MakeMonAware::yes,
                          map::player);
    return ConsumeItem::yes;
}

//---------------------------------------------------- ELECTRIC LANTERN
DeviceLantern::DeviceLantern(ItemDataT* const item_data) :
    Device                  (item_data),
    nr_turns_left_          (500),
    nr_flicker_turns_left_  (-1),
    working_state_          (LanternWorkingState::working),
    is_activated_           (false) {}

std::string DeviceLantern::name_inf() const
{
    std::string inf = "{" + to_str(nr_turns_left_);

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
    game_time::tick();
    return ConsumeItem::no;
}

void DeviceLantern::save()
{
    save_handling::put_int(nr_turns_left_);
    save_handling::put_int(nr_flicker_turns_left_);
    save_handling::put_int(int(working_state_));
    save_handling::put_bool(is_activated_);
}

void DeviceLantern::load()
{
    nr_turns_left_          = save_handling::get_int();
    nr_flicker_turns_left_  = save_handling::get_int();
    working_state_          = LanternWorkingState(save_handling::get_int());
    is_activated_           = save_handling::get_bool();
}

void DeviceLantern::on_pickup_hook()
{
    ASSERT(actor_carrying_);

    //Check for existing electric lantern in inventory
    Inventory& inv = actor_carrying_->inv();

    for (Item* const other : inv.backpack_)
    {
        if (other != this && other->id() == id())
        {
            //Add my turns left to the other lantern, then destroy self (it's better to keep
            //the existing lantern, to that lit state etc is preserved)
            static_cast<DeviceLantern*>(other)->nr_turns_left_ += nr_turns_left_;
            inv.remove_item_in_backpack_with_ptr(this, true);
            return;
        }
    }
}

void DeviceLantern::toggle()
{
    const std::string toggle_str = is_activated_ ? "I turn off" : "I turn on";

    msg_log::add(toggle_str + " an Electric Lantern.");

    is_activated_ = !is_activated_;

    audio::play(SfxId::lantern);

    game_time::update_light_map();

    map::player->update_fov();

    render::draw_map_state();
}

void DeviceLantern::on_std_turn_in_inv(const InvType inv_type)
{
    (void)inv_type;

    if (is_activated_)
    {
        if (working_state_ == LanternWorkingState::working)
        {
            --nr_turns_left_;
        }

        if (nr_turns_left_ <= 0)
        {
            msg_log::add("My Electric Lantern has run out.",
                         clr_msg_note,
                         true,
                         MorePromptOnMsg::yes);

            dungeon_master::add_history_event("My Electric Lantern ran out.");

            //NOTE: The this deletes the object
            map::player->inv().remove_item_in_backpack_with_ptr(this, true);

            game_time::update_light_map();
            map::player->update_fov();
            render::draw_map_state();

            return;
        }

        //This point reached means the lantern is not destroyed

        if (nr_flicker_turns_left_ > 0)
        {
            //Already flickering, count down instead
            --nr_flicker_turns_left_;

            if (nr_flicker_turns_left_ <= 0)
            {
                working_state_ = LanternWorkingState::working;

                game_time::update_light_map();
                map::player->update_fov();
                render::draw_map_state();
            }
        }
        else //Not flickering
        {
            if (rnd::one_in(40))
            {
                msg_log::add("My Electric Lantern flickers...");
                working_state_          = LanternWorkingState::flicker;
                nr_flicker_turns_left_  = rnd::range(4, 8);

                game_time::update_light_map();
                map::player->update_fov();
                render::draw_map_state();
            }
            else //Not flickering
            {
                working_state_ = LanternWorkingState::working;
            }
        }
    }
}

LgtSize DeviceLantern::lgt_size() const
{
    if (is_activated_)
    {
        switch (working_state_)
        {
        case LanternWorkingState::working:
            return LgtSize::fov;

        case LanternWorkingState::flicker:
            return LgtSize::small;
        }
    }

    return LgtSize::none;
}
