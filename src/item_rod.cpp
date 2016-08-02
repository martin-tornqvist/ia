#include "item_rod.hpp"

#include "init.hpp"
#include "save_handling.hpp"
#include "item_data.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "game_time.hpp"
#include "dungeon_master.hpp"
#include "item_factory.hpp"
#include "fov.hpp"
#include "map_parsing.hpp"
#include "actor_mon.hpp"
#include "render.hpp"
#include "feature_rigid.hpp"
#include "save_handling.hpp"

void Rod::save()
{
    save_handling::put_int(nr_charge_turns_left_);
}

void Rod::load()
{
    nr_charge_turns_left_ = save_handling::get_int();
}

ConsumeItem Rod::activate(Actor* const actor)
{
    (void)actor;

    if (nr_charge_turns_left_ > 0)
    {
        const std::string rod_name = name(ItemRefType::plain, ItemRefInf::none);

        msg_log::add("The " + rod_name + " is still charging.");

        return ConsumeItem::no;
    }

    data_->is_tried = true;

    //TODO: Sfx

    const std::string rod_name_a = name(ItemRefType::a, ItemRefInf::none);

    msg_log::add("I activate " + rod_name_a + "...");

    activate_impl();

    if (data_->is_identified)
    {
        map::player->incr_shock(ShockLvl::heavy, ShockSrc::use_strange_item);
    }
    else //Not identified
    {
        msg_log::add("Nothing happens.");
    }

    //NOTE: Various rods cannot have different recharge time, since the player
    //      could trivially "metagame identify" rods based on the this value
    //      when they aren't identified in-game. So here we set a fixed value,
    //      same for all rods.

    //TODO: This number could be a good candidate for a trait or background to
    //      provide a bonus for!
    nr_charge_turns_left_ = 250;

    if (map::player->is_alive())
    {
        game_time::tick();
    }

    return ConsumeItem::no;
}

void Rod::on_std_turn_in_inv(const InvType inv_type)
{
    (void)inv_type;

    if (nr_charge_turns_left_ > 0)
    {
        --nr_charge_turns_left_;

        if (nr_charge_turns_left_ == 0)
        {
            const std::string rod_name = name(ItemRefType::plain, ItemRefInf::none);

            msg_log::add("The " + rod_name + " has finished charging.");
        }
    }
}

std::vector<std::string> Rod::descr() const
{
    if (data_->is_identified)
    {
        return
        {
            descr_identified()
        };
    }
    else //Not identified
    {
        return data_->base_descr;
    }
}

void Rod::identify(const Verbosity verbosity)
{
    if (!data_->is_identified)
    {
        data_->is_identified = true;

        if (verbosity == Verbosity::verbose)
        {
            const std::string name_after = name(ItemRefType::a, ItemRefInf::none);

            msg_log::add("I have identified " + name_after + ".");

            dungeon_master::add_history_event("Identified " + name_after + ".");

            give_xp_for_identify();
        }
    }
}

std::string Rod::name_inf() const
{
    const std::string charge_str = "Charging:" + to_str(nr_charge_turns_left_);

    if (data_->is_identified)
    {
        return nr_charge_turns_left_ > 0 ? ("{" + charge_str + "}") : "";
    }
    else //Not identified
    {
        if (nr_charge_turns_left_ > 0)
        {
            return std::string("{") + (data_->is_tried ? "Tried, " : "") + charge_str + "}";
        }
        else
        {
            return data_->is_tried ? "{Tried}" : "";
        }
    }
}

void RodPurgeInvis::activate_impl()
{
    bool blocked[map_w][map_h];

    map_parse::run(cell_check::BlocksLos(),
                   blocked,
                   MapParseMode::overwrite,
                   fov::get_fov_rect(map::player->pos));

    LosResult fov[map_w][map_h];

    fov::run(map::player->pos, blocked, fov);

    bool is_effect_noticed = false;

    for (Actor* const actor : game_time::actors)
    {
        if (!actor->is_player())
        {
            //Reveal invisible monsters
            const P& p(actor->pos);

            const LosResult& los = fov[p.x][p.y];

            if (!los.is_blocked_hard && !los.is_blocked_by_drk && actor->has_prop(PropId::invis))
            {
                actor->prop_handler().end_prop(PropId::invis);

                if (map::player->can_see_actor(*actor))
                {
                    is_effect_noticed = true;
                }
            }

            //Reveal sneaking monsters
            Mon* const mon = static_cast<Mon*>(actor);

            if (mon->is_sneaking_)
            {
                mon->is_sneaking_ = false;

                if (map::player->can_see_actor(*actor))
                {
                    mon->is_sneaking_ = false;

                    render::draw_map_state();

                    const std::string mon_name = mon->name_a();

                    msg_log::add(mon_name + " is revealed!");

                    is_effect_noticed = true;
                }
            }
        }
    }

    if (is_effect_noticed)
    {
        identify(Verbosity::verbose);
    }
}

void RodCuring::activate_impl()
{
    Player& player = *map::player;

    std::vector<PropId> props_can_heal =
    {
        PropId::blind,
        PropId::poisoned,
        PropId::infected,
        PropId::diseased,
        PropId::weakened,
    };

    bool is_something_healed = false;

    for (PropId prop_id : props_can_heal)
    {
        if (player.prop_handler().end_prop(prop_id))
        {
            is_something_healed = true;
        }
    }

    if (player.restore_hp(3, false /* Not allowed above max */))
    {
        is_something_healed = true;
    }

    if (!is_something_healed)
    {
        msg_log::add("I feel fine.");
    }

    identify(Verbosity::verbose);
}

void RodOpening::activate_impl()
{
    bool is_any_opened = false;

    for (int y = 1; y < map_h - 1; ++y)
    {
        for (int x = 1; x < map_w - 1; ++x)
        {
            const auto& cell = map::cells[x][y];

            if (cell.is_seen_by_player)
            {
                DidOpen did_open = cell.rigid->open(nullptr);

                if (did_open == DidOpen::yes)
                {
                    is_any_opened = true;
                }
            }
        }
    }

    if (is_any_opened)
    {
        map::player->update_fov();
        render::draw_map_state();

        identify(Verbosity::verbose);
    }
}

void RodBless::activate_impl()
{
    const int nr_turns = rnd::range(8, 12);

    Prop* const prop = new PropBlessed(PropTurns::specific, nr_turns);

    map::player->prop_handler().try_add(prop);

    identify(Verbosity::verbose);
}

namespace rod_handling
{

namespace
{

std::vector<RodLook> rod_looks_;

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;

    //Init possible rod colors and fake names
    rod_looks_.clear();
    rod_looks_.push_back({"Iron",       "an Iron",      clr_gray});
    rod_looks_.push_back({"Zinc",       "a Zinc",       clr_white_high});
    rod_looks_.push_back({"Chromium",   "a Chromium",   clr_white_high});
    rod_looks_.push_back({"Tin",        "a Tin",        clr_white_high});
    rod_looks_.push_back({"Silver",     "a Silver",     clr_white_high});
    rod_looks_.push_back({"Golden",     "a Golden",     clr_yellow});
    rod_looks_.push_back({"Nickel",     "a Nickel",     clr_white_high});
    rod_looks_.push_back({"Copper",     "a Copper",     clr_brown});
    rod_looks_.push_back({"Lead",       "a Lead",       clr_gray});
    rod_looks_.push_back({"Tungsten",   "a Tungsten",   clr_white});
    rod_looks_.push_back({"Platinum",   "a Platinum",   clr_white_high});
    rod_looks_.push_back({"Lithium",    "a Lithium",    clr_white});
    rod_looks_.push_back({"Zirconium",  "a Zirconium",  clr_white});
    rod_looks_.push_back({"Gallium",    "a Gallium",    clr_white_high});
    rod_looks_.push_back({"Cobalt",     "a Cobalt",     clr_blue_lgt});
    rod_looks_.push_back({"Titanium",   "a Titanium",   clr_white_high});
    rod_looks_.push_back({"Magnesium",  "a Magnesium",  clr_white});

    for (auto& d : item_data::data)
    {
        if (d.type == ItemType::rod)
        {
            //Color and false name
            const size_t idx = rnd::range(0, rod_looks_.size() - 1);

            RodLook& look = rod_looks_[idx];

            d.base_name_un_id.names[int(ItemRefType::plain)]   = look.name_plain + " Rod";
            d.base_name_un_id.names[int(ItemRefType::plural)]  = look.name_plain + " Rods";
            d.base_name_un_id.names[int(ItemRefType::a)]       = look.name_a     + " Rod";
            d.clr = look.clr;

            rod_looks_.erase(rod_looks_.begin() + idx);

            //True name
            const Rod* const rod = static_cast<const Rod*>(item_factory::mk(d.id, 1));

            const std::string real_type_name = rod->real_name();

            delete rod;

            const std::string real_name        = "Rod of "    + real_type_name;
            const std::string real_name_plural = "Rods of "   + real_type_name;
            const std::string real_name_a      = "a Rod of "  + real_type_name;

            d.base_name.names[int(ItemRefType::plain)]  = real_name;
            d.base_name.names[int(ItemRefType::plural)] = real_name_plural;
            d.base_name.names[int(ItemRefType::a)]      = real_name_a;
        }
    }

    TRACE_FUNC_END;
}

void save()
{
    for (int i = 0; i < int(ItemId::END); ++i)
    {
        ItemDataT& d = item_data::data[i];

        if (d.type == ItemType::rod)
        {
            save_handling::put_str(d.base_name_un_id.names[size_t(ItemRefType::plain)]);
            save_handling::put_str(d.base_name_un_id.names[size_t(ItemRefType::plural)]);
            save_handling::put_str(d.base_name_un_id.names[size_t(ItemRefType::a)]);

            save_handling::put_int(d.clr.r);
            save_handling::put_int(d.clr.g);
            save_handling::put_int(d.clr.b);
        }
    }
}

void load()
{
    for (int i = 0; i < int(ItemId::END); ++i)
    {
        ItemDataT& d = item_data::data[i];

        if (d.type == ItemType::rod)
        {
            d.base_name_un_id.names[size_t(ItemRefType::plain)]  = save_handling::get_str();
            d.base_name_un_id.names[size_t(ItemRefType::plural)] = save_handling::get_str();
            d.base_name_un_id.names[size_t(ItemRefType::a)]      = save_handling::get_str();

            d.clr.r = save_handling::get_int();
            d.clr.g = save_handling::get_int();
            d.clr.b = save_handling::get_int();
        }
    }
}

} //rod_handling
