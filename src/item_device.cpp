#include "item_device.hpp"

#include <algorithm>
#include <vector>

#include "actor_player.hpp"
#include "render.hpp"
#include "game_time.hpp"
#include "msg_log.hpp"
#include "knockback.hpp"
#include "inventory.hpp"
#include "map.hpp"
#include "audio.hpp"
#include "utils.hpp"
#include "feature_rigid.hpp"
#include "actor_factory.hpp"

using namespace std;

//---------------------------------------------------- DEVICE
Device::Device(Item_data_t* const item_data) :
    Item(item_data) {}

void Device::identify(const bool IS_SILENT_IDENTIFY)
{
    (void)IS_SILENT_IDENTIFY;
    data_->is_identified = true;
}

//---------------------------------------------------- STRANGE DEVICE
Strange_device::Strange_device(Item_data_t* const item_data) :
    Device(item_data),
    condition_(rnd::coin_toss() ? Condition::fine : Condition::shoddy) {}

void Strange_device::store_to_save_lines(vector<string>& lines)
{
    lines.push_back(to_str(int(condition_)));
}

void Strange_device::setup_from_save_lines(vector<string>& lines)
{
    condition_ = Condition(to_int(lines.front()));
    lines.erase(begin(lines));
}

vector<string> Strange_device::get_descr() const
{
    if (data_->is_identified)
    {
        auto descr = get_descr_identified();

        string cond_str = "It seems ";

        switch (condition_)
        {
        case Condition::fine:     cond_str += "to be in fine condition.";    break;

        case Condition::shoddy:   cond_str += "to be in shoddy condition.";  break;

        case Condition::breaking: cond_str += "almost broken.";              break;
        }

        descr.push_back(cond_str);

        return descr;
    }
    else //Not identified
    {
        return data_->base_descr;
    }
}

Consume_item Strange_device::activate(Actor* const actor)
{
    (void)actor;

    if (data_->is_identified)
    {
        const string item_name   = get_name(Item_ref_type::plain, Item_ref_inf::none);
        const string item_name_a  = get_name(Item_ref_type::a, Item_ref_inf::none);

        msg_log::add("I activate " + item_name_a + "...");

        //Damage user? Fail to run effect? Condition degrade? Warning?
        const string hurt_msg  = "It hits me with a jolt of electricity!";
        bool is_effect_failed   = false;
        bool is_cond_degrade    = false;
        bool is_warning        = false;
        int bon = 0;
        bool props[size_t(Prop_id::END)];
        actor->get_prop_handler().get_prop_ids(props);

        if (props[int(Prop_id::blessed)])
        {
            bon += 2;
        }

        if (props[int(Prop_id::cursed)])
        {
            bon -= 2;
        }

        const int RND = rnd::range(1, 8 + bon);

        switch (condition_)
        {
        case Condition::breaking:
        {
            if (RND == 5 || RND == 6)
            {
                msg_log::add(hurt_msg, clr_msg_bad);
                actor->hit(rnd::dice(2, 4), Dmg_type::electric);
            }

            is_effect_failed  = RND == 3 || RND == 4;
            is_cond_degrade   = RND <= 2;
            is_warning       = RND == 7 || RND == 8;
        } break;

        case Condition::shoddy:
        {
            if (RND == 4)
            {
                msg_log::add(hurt_msg, clr_msg_bad);
                actor->hit(rnd::dice(1, 4), Dmg_type::electric);
            }

            is_effect_failed  = RND == 3;
            is_cond_degrade   = RND <= 2;
            is_warning       = RND == 5 || RND == 6;
        } break;

        case Condition::fine:
        {
            is_cond_degrade   = RND <= 2;
            is_warning       = RND == 3 || RND == 4;
        } break;
        }

        if (!map::player->is_alive())
        {
            return Consume_item::no;
        }

        Consume_item consumed_state = Consume_item::no;

        if (is_effect_failed)
        {
            msg_log::add("It suddenly stops.");
        }
        else
        {
            consumed_state = trigger_effect();
        }

        if (consumed_state == Consume_item::no)
        {
            if (is_cond_degrade)
            {
                if (condition_ == Condition::breaking)
                {
                    msg_log::add("The " + item_name + " breaks!");
                    consumed_state = Consume_item::yes;
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
        return Consume_item::no;
    }
}

std::string Strange_device::get_name_inf() const
{
    if (data_->is_identified)
    {
        switch (condition_)
        {
        case Condition::breaking: return "{breaking}";

        case Condition::shoddy:   return "{shoddy}";

        case Condition::fine:     return "{fine}";
        }
    }

    return "";
}

//---------------------------------------------------- BLASTER
Consume_item Device_blaster::trigger_effect()
{
    vector<Actor*> target_bucket;
    map::player->get_seen_foes(target_bucket);

    if (target_bucket.empty())
    {
        msg_log::add("It seems to peruse area.");
    }
    else //Targets are available
    {
        Spell* const spell = spell_handling::mk_spell_from_id(Spell_id::aza_wrath);
        spell->cast(map::player, false);
        delete spell;
    }

    return Consume_item::no;
}

//---------------------------------------------------- SHOCK WAVE
Consume_item Device_shockwave::trigger_effect()
{
    msg_log::add("It triggers a shock wave around me.");

    const Pos& player_pos = map::player->pos;

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const Pos p(player_pos + Pos(dx, dy));
            Rigid* const rigid = map::cells[p.x][p.y].rigid;
            rigid->hit(Dmg_type::physical, Dmg_method::explosion);

            //game_time::update_light_map();
            map::player->update_fov();
            render::draw_map_and_interface();
        }
    }

    for (Actor* actor : game_time::actors_)
    {
        if (actor != map::player && actor->is_alive())
        {
            const Pos& other_pos = actor->pos;

            if (utils::is_pos_adj(player_pos, other_pos, false))
            {
                actor->hit(rnd::dice(1, 8), Dmg_type::physical);

                if (actor->is_alive())
                {
                    knock_back::try_knock_back(*actor, player_pos, false, true);
                }
            }
        }
    }

    return Consume_item::no;
}

//---------------------------------------------------- REJUVENATOR
Consume_item Device_rejuvenator::trigger_effect()
{
    msg_log::add("It repairs my body.");
    map::player->get_prop_handler().end_applied_props_by_magic_healing();
    map::player->restore_hp(999, false);
    return Consume_item::no;
}

//---------------------------------------------------- TRANSLOCATOR
Consume_item Device_translocator::trigger_effect()
{
    Player* const player = map::player;
    vector<Actor*> seen_foes;
    player->get_seen_foes(seen_foes);

    if (seen_foes.empty())
    {
        msg_log::add("It seems to peruse area.");
    }
    else //Seen targets are available
    {
        for (Actor* actor : seen_foes)
        {
            msg_log::add(actor->get_name_the() + " is teleported.");
            render::draw_blast_at_cells(vector<Pos> {actor->pos}, clr_yellow);
            actor->teleport();
        }
    }

    return Consume_item::no;
}

//---------------------------------------------------- SENTRY DRONE
Consume_item Device_sentry_drone::trigger_effect()
{
    msg_log::add("The Sentry Drone awakens!");
    actor_factory::summon(map::player->pos, {Actor_id::sentry_drone}, true, map::player);
    return Consume_item::yes;
}

//---------------------------------------------------- ELECTRIC LANTERN
Device_lantern::Device_lantern(Item_data_t* const item_data) :
    Device(item_data),
    nr_turns_left_(500),
    nr_flicker_turns_left_(-1),
    working_state_(Lantern_working_state::working),
    is_activated_(false) {}

std::string Device_lantern::get_name_inf() const
{
    string inf = "{" + to_str(nr_turns_left_);

    if (is_activated_) {inf += ", Lit";}

    return inf + "}";
}

Consume_item Device_lantern::activate(Actor* const actor)
{
    (void)actor;
    toggle();
    game_time::tick();
    return Consume_item::no;
}

void Device_lantern::store_to_save_lines(vector<string>& lines)
{
    lines.push_back(to_str(nr_turns_left_));
    lines.push_back(to_str(nr_flicker_turns_left_));
    lines.push_back(to_str(int(working_state_)));
    lines.push_back(is_activated_ ? "1" : "0");
}

void Device_lantern::setup_from_save_lines(vector<string>& lines)
{
    nr_turns_left_          = to_int(lines.front());
    lines.erase(begin(lines));
    nr_flicker_turns_left_   = to_int(lines.front());
    lines.erase(begin(lines));
    working_state_         = Lantern_working_state(to_int(lines.front()));
    lines.erase(begin(lines));
    is_activated_          = lines.front() == "1";
    lines.erase(begin(lines));
}

void Device_lantern::on_pickup_to_backpack(Inventory& inv)
{
    //Check for existing electric lantern in inventory
    for (Item* const other : inv.general_)
    {
        if (other != this && other->get_id() == get_id())
        {
            //Add my turns left to the other lantern, then destroy self (it's better to keep
            //the existing lantern, to that lit state etc is preserved)
            static_cast<Device_lantern*>(other)->nr_turns_left_ += nr_turns_left_;
            inv.remove_item_in_backpack_with_ptr(this, true);
            return;
        }
    }
}

void Device_lantern::toggle()
{
    const string toggle_str = is_activated_ ? "I turn off" : "I turn on";
    msg_log::add(toggle_str + " an Electric Lantern.");

    is_activated_ = !is_activated_;

    audio::play(Sfx_id::electric_lantern);
    game_time::update_light_map();
    map::player->update_fov();
    render::draw_map_and_interface();
}

void Device_lantern::on_std_turn_in_inv(const Inv_type inv_type)
{
    (void)inv_type;

    if (is_activated_)
    {
        if (working_state_ == Lantern_working_state::working)
        {
            --nr_turns_left_;
        }

        if (nr_turns_left_ <= 0)
        {
            msg_log::add("My Electric Lantern breaks!", clr_msg_note, true, true);

            //NOTE: The this deletes the object
            map::player->get_inv().remove_item_in_backpack_with_ptr(this, true);

            game_time::update_light_map();
            map::player->update_fov();
            render::draw_map_and_interface();

            return;
        }
        else if (nr_turns_left_ <= 3)
        {
            msg_log::add("My Electric Lantern is breaking.", clr_msg_note, true, true);
        }

        //This point reached means the lantern is not destroyed

        if (nr_flicker_turns_left_ > 0)
        {
            //Already flickering, count down instead
            nr_flicker_turns_left_--;

            if (nr_flicker_turns_left_ <= 0)
            {
                working_state_ = Lantern_working_state::working;

                game_time::update_light_map();
                map::player->update_fov();
                render::draw_map_and_interface();
            }
        }
        else //Not flickering
        {
            if (rnd::one_in(40))
            {
                msg_log::add("My Electric Lantern flickers...");
                working_state_         = Lantern_working_state::flicker;
                nr_flicker_turns_left_   = rnd::range(4, 8);

                game_time::update_light_map();
                map::player->update_fov();
                render::draw_map_and_interface();
            }
            else
            {
                working_state_ = Lantern_working_state::working;
            }
        }
    }
}

Lgt_size Device_lantern::get_lgt_size() const
{
    if (is_activated_)
    {
        switch (working_state_)
        {
        case Lantern_working_state::working: return Lgt_size::fov;

        case Lantern_working_state::flicker: return Lgt_size::small;
        }
    }

    return Lgt_size::none;
}
