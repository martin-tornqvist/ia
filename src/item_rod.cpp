#include "item_rod.hpp"

#include "init.hpp"
#include "save_handling.hpp"
#include "item_data.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "game_time.hpp"
#include "dungeon_master.hpp"
#include "utils.hpp"
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

Consume_item Rod::activate(Actor* const actor)
{
    (void)actor;

    if (nr_charge_turns_left_ > 0)
    {
        const std::string rod_name = name(Item_ref_type::plain, Item_ref_inf::none);

        msg_log::add("The " + rod_name + " is still charging.");

        return Consume_item::no;
    }

    data_->is_tried = true;

    //TODO: Sfx

    const std::string rod_name_a = name(Item_ref_type::a, Item_ref_inf::none);

    msg_log::add("I activate " + rod_name_a + "...");

    activate_impl();

    if (data_->is_identified)
    {
        map::player->incr_shock(Shock_lvl::heavy, Shock_src::use_strange_item);
    }
    else //Not identified
    {
        msg_log::add("Nothing happens.");
    }

    //NOTE: Various rods cannot have different recharge time, since the player could trivially
    //"metagame identify" rods based on the this value when they aren't identified in-game. So here
    //we set a fixed value, same for all rods.

    //TODO: This number could be a good candidate for a trait or background to provide a bonus for.
    nr_charge_turns_left_ = 250;

    if (map::player->is_alive())
    {
        game_time::tick();
    }

    return Consume_item::no;
}

void Rod::on_std_turn_in_inv(const Inv_type inv_type)
{
    (void)inv_type;

    if (nr_charge_turns_left_ > 0)
    {
        --nr_charge_turns_left_;

        if (nr_charge_turns_left_ == 0)
        {
            const std::string rod_name = name(Item_ref_type::plain, Item_ref_inf::none);

            msg_log::add("The " + rod_name + " has finished charging.",
                         clr_msg_note,
                         false,
                         More_prompt_on_msg::yes);
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
            const std::string name_after = name(Item_ref_type::a, Item_ref_inf::none);

            msg_log::add("I have identified " + name_after + ".");

            dungeon_master::add_history_event("Identified " + name_after + ".");
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

void Rod_purge_invis::activate_impl()
{
    bool blocked[MAP_W][MAP_H];

    map_parse::run(cell_check::Blocks_los(),
                   blocked,
                   Map_parse_mode::overwrite,
                   fov::get_fov_rect(map::player->pos));

    Los_result fov[MAP_W][MAP_H];

    fov::run(map::player->pos, blocked, fov);

    bool is_effect_noticed = false;

    for (Actor* const actor : game_time::actors)
    {
        if (!actor->is_player())
        {
            //Reveal invisible monsters
            const P& p(actor->pos);

            const Los_result& los = fov[p.x][p.y];

            if (!los.is_blocked_hard && !los.is_blocked_by_drk && actor->has_prop(Prop_id::invis))
            {
                actor->prop_handler().end_prop(Prop_id::invis);

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

                    render::draw_map_and_interface();

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

void Rod_curing::activate_impl()
{
    Player& player = *map::player;

    std::vector<Prop_id> props_can_heal =
    {
        Prop_id::blind,
        Prop_id::poisoned,
        Prop_id::infected,
        Prop_id::diseased,
        Prop_id::weakened,
    };

    bool is_something_healed = false;

    for (Prop_id prop_id : props_can_heal)
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

void Rod_opening::activate_impl()
{
    bool is_any_opened = false;

    for (int y = 1; y < MAP_H - 1; ++y)
    {
        for (int x = 1; x < MAP_W - 1; ++x)
        {
            const auto& cell = map::cells[x][y];

            if (cell.is_seen_by_player)
            {
                Did_open did_open = cell.rigid->open(nullptr);

                if (did_open == Did_open::yes)
                {
                    is_any_opened = true;
                }
            }
        }
    }

    if (is_any_opened)
    {
        map::player->update_fov();
        render::draw_map_and_interface();

        identify(Verbosity::verbose);
    }
}

void Rod_bless::activate_impl()
{
    const int NR_TURNS = rnd::range(8, 12);

    Prop* const prop = new Prop_blessed(Prop_turns::specific, NR_TURNS);

    map::player->prop_handler().try_add_prop(prop);

    identify(Verbosity::verbose);
}

namespace rod_handling
{

namespace
{

std::vector<Rod_look> rod_looks_;

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
        if (d.type == Item_type::rod)
        {
            //Color and false name
            const size_t IDX = rnd::range(0, rod_looks_.size() - 1);

            Rod_look& look = rod_looks_[IDX];

            d.base_name_un_id.names[int(Item_ref_type::plain)]   = look.name_plain + " Rod";
            d.base_name_un_id.names[int(Item_ref_type::plural)]  = look.name_plain + " Rods";
            d.base_name_un_id.names[int(Item_ref_type::a)]       = look.name_a     + " Rod";
            d.clr = look.clr;

            rod_looks_.erase(rod_looks_.begin() + IDX);

            //True name
            const Rod* const rod = static_cast<const Rod*>(item_factory::mk(d.id, 1));

            const std::string REAL_TYPE_NAME = rod->real_name();

            delete rod;

            const std::string REAL_NAME        = "Rod of "    + REAL_TYPE_NAME;
            const std::string REAL_NAME_PLURAL = "Rods of "   + REAL_TYPE_NAME;
            const std::string REAL_NAME_A      = "a Rod of "  + REAL_TYPE_NAME;

            d.base_name.names[int(Item_ref_type::plain)]  = REAL_NAME;
            d.base_name.names[int(Item_ref_type::plural)] = REAL_NAME_PLURAL;
            d.base_name.names[int(Item_ref_type::a)]      = REAL_NAME_A;
        }
    }

    TRACE_FUNC_END;
}

void save()
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        Item_data_t& d = item_data::data[i];

        if (d.type == Item_type::rod)
        {
            save_handling::put_str(d.base_name_un_id.names[size_t(Item_ref_type::plain)]);
            save_handling::put_str(d.base_name_un_id.names[size_t(Item_ref_type::plural)]);
            save_handling::put_str(d.base_name_un_id.names[size_t(Item_ref_type::a)]);

            save_handling::put_int(d.clr.r);
            save_handling::put_int(d.clr.g);
            save_handling::put_int(d.clr.b);
        }
    }
}

void load()
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        Item_data_t& d = item_data::data[i];

        if (d.type == Item_type::rod)
        {
            d.base_name_un_id.names[size_t(Item_ref_type::plain)]  = save_handling::get_str();
            d.base_name_un_id.names[size_t(Item_ref_type::plural)] = save_handling::get_str();
            d.base_name_un_id.names[size_t(Item_ref_type::a)]      = save_handling::get_str();

            d.clr.r = save_handling::get_int();
            d.clr.g = save_handling::get_int();
            d.clr.b = save_handling::get_int();
        }
    }
}

} //rod_handling
