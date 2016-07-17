#include "item_potion.hpp"

#include "init.hpp"
#include "properties.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "actor_mon.hpp"
#include "player_spells_handling.hpp"
#include "item_scroll.hpp"
#include "game_time.hpp"
#include "audio.hpp"
#include "render.hpp"
#include "inventory.hpp"
#include "map_parsing.hpp"
#include "feature_rigid.hpp"
#include "item_factory.hpp"
#include "save_handling.hpp"
#include "dungeon_master.hpp"

Consume_item Potion::activate(Actor* const actor)
{
    ASSERT(actor);

    if (actor->prop_handler().allow_eat(Verbosity::verbose))
    {
        if (actor->is_player())
        {
            data_->is_tried = true;

            audio::play(Sfx_id::potion_quaff);

            if (data_->is_identified)
            {
                const std::string potion_name = name(Item_ref_type::a, Item_ref_inf::none);
                msg_log::add("I drink " + potion_name + "...");
            }
            else //Not identified
            {
                const std::string potion_name = name(Item_ref_type::plain, Item_ref_inf::none);
                msg_log::add("I drink an unknown " + potion_name + "...");
            }

            map::player->incr_shock(Shock_lvl::heavy, Shock_src::use_strange_item);
        }

        quaff_impl(*actor);

        if (map::player->is_alive())
        {
            game_time::tick();
        }

        return Consume_item::yes;
    }

    return Consume_item::no;
}

void Potion::identify(const Verbosity verbosity)
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

std::vector<std::string> Potion::descr() const
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

void Potion::on_collide(const P& pos, Actor* const actor)
{
    if (!map::cells[pos.x][pos.y].rigid->is_bottomless() || actor)
    {
        //Render and print message
        const bool player_see_cell = map::cells[pos.x][pos.y].is_seen_by_player;

        if (player_see_cell)
        {
            //TODO: Use standard animation
            render::draw_glyph('*', Panel::map, pos, data_->clr);

            if (actor)
            {
                if (actor->is_alive())
                {
                    msg_log::add("The potion shatters on " + actor->name_the() + ".");
                }
            }
            else //No actor here
            {
                Feature* const f = map::cells[pos.x][pos.y].rigid;
                msg_log::add("The potion shatters on " + f->name(Article::the) + ".");
            }
        }

        if (actor)
        {
            //If the blow from the bottle didn't kill the actor, apply what's inside
            if (actor->is_alive())
            {
                collide_hook(pos, actor);

                if (actor->is_alive() && !data_->is_identified && player_see_cell)
                {
                    //This did not identify the potion
                    msg_log::add("It had no apparent effect...");
                }
            }
        }
    }
}

std::string Potion::name_inf() const
{
    return (data_->is_tried && !data_->is_identified) ? "{Tried}" : "";
}

void Potion_vitality::quaff_impl(Actor& actor)
{
    std::vector<Prop_id> props_can_heal =
    {
        Prop_id::blind,
        Prop_id::poisoned,
        Prop_id::infected,
        Prop_id::diseased,
        Prop_id::weakened,
        Prop_id::wound
    };

    for (Prop_id prop_id : props_can_heal)
    {
        actor.prop_handler().end_prop(prop_id);
    }

    //HP is always restored at least up to maximum hp, but can go beyond
    const int hp          = actor.hp();
    const int hp_max      = actor.hp_max(true);
    const int hp_restored = std::max(20, hp_max - hp);

    actor.restore_hp(hp_restored, true);

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_vitality::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_spirit::quaff_impl(Actor& actor)
{
    //SPI is always restored at least up to maximum spi, but can go beyond
    const int spi           = actor.spi();
    const int spi_max       = actor.spi_max();
    const int spi_restored  = std::max(10, spi_max - spi);

    actor.restore_spi(spi_restored, true);

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_spirit::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_blindness::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_blind(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_blindness::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_paral::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_paralyzed(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_paral::collide_hook(const P& pos, Actor* const actor)
{

    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_disease::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_diseased(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_conf::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_confused(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_conf::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_fortitude::quaff_impl(Actor& actor)
{
    Prop_handler& prop_handler = actor.prop_handler();

    Prop_rFear* const rFear = new Prop_rFear(Prop_turns::std);

    const int nr_turns_left = rFear->nr_turns_left();

    Prop_rConf* const   rConf   = new Prop_rConf(Prop_turns::specific,  nr_turns_left);
    Prop_rSleep* const  rSleep  = new Prop_rSleep(Prop_turns::specific, nr_turns_left);

    prop_handler.try_add(rFear);
    prop_handler.try_add(rConf);
    prop_handler.try_add(rSleep);

    prop_handler.end_prop(Prop_id::frenzied);

    //Remove a random insanity symptom if this is the player
    if (actor.is_player())
    {
        const std::vector<const Ins_sympt*> sympts = insanity::active_sympts();

        if (!sympts.empty())
        {
            const size_t        idx = rnd::range(0, sympts.size() - 1);
            const Ins_sympt_id  id  = sympts[idx]->id();

            insanity::end_sympt(id);
        }

        map::player->restore_shock(999, false);

        msg_log::add("I feel more at ease.");
    }

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_fortitude::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_poison::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_poisoned(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_poison::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_rFire::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_rFire(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_rFire::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_curing::quaff_impl(Actor& actor)
{
    std::vector<Prop_id> props_can_heal =
    {
        Prop_id::blind,
        Prop_id::poisoned,
        Prop_id::infected,
        Prop_id::diseased,
        Prop_id::weakened,
    };

    bool is_noticable = false;

    for (Prop_id prop_id : props_can_heal)
    {
        if (actor.prop_handler().end_prop(prop_id))
        {
            is_noticable = true;
        }
    }

    if (actor.restore_hp(3, false /*Not allowed above max*/))
    {
        is_noticable = true;
    }

    if (!is_noticable && actor.is_player())
    {
        msg_log::add("I feel fine.");

        is_noticable = true;
    }

    if (is_noticable && map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_curing::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_rElec::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_rElec(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_rElec::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_rAcid::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_rAcid(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_rAcid::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_insight::quaff_impl(Actor& actor)
{
    (void)actor;

    auto& inv = map::player->inv();

    std::vector<Item*> identify_bucket;

    auto& slots = inv.slots_;

    for (Inv_slot& slot : slots)
    {
        Item* const item = slot.item;

        if (item)
        {
            const Item_data_t& d = item->data();

            if (!d.is_identified)
            {
                identify_bucket.push_back(item);
            }
        }
    }

    std::vector<Item*>& general = inv.backpack_;

    for (Item* item : general)
    {
        if (item->id() != Item_id::potion_insight)
        {
            const Item_data_t& d = item->data();

            if (!d.is_identified)
            {
                identify_bucket.push_back(item);
            }
        }
    }

    const size_t NR_elementS = identify_bucket.size();

    if (NR_elementS > 0)
    {
        const int     idx                   = rnd::range(0, NR_elementS - 1);
        Item* const   item                  = identify_bucket[idx];
        const std::string  item_name_before = item->name(Item_ref_type::a, Item_ref_inf::none);

        msg_log::add("I gain intuitions about " + item_name_before + "...",
                     clr_white, false, More_prompt_on_msg::yes);

        item->identify(Verbosity::verbose);

        const std::string item_name_after = item->name(Item_ref_type::a, Item_ref_inf::none);
    }

    identify(Verbosity::verbose);
}

void Potion_clairv::quaff_impl(Actor& actor)
{
    if (actor.is_player())
    {
        msg_log::add("I see far and wide!");

        std::vector<P> anim_cells;
        anim_cells.clear();

        bool blocked[map_w][map_h];
        map_parse::run(cell_check::Blocks_los(), blocked);

        for (int x = 0; x < map_w; ++x)
        {
            for (int y = 0; y < map_h; ++y)
            {
                Cell& cell = map::cells[x][y];

                if (!blocked[x][y] && !cell.is_dark)
                {
                    cell.is_explored = true;
                    cell.is_seen_by_player = true;
                    anim_cells.push_back(P(x, y));
                }
            }
        }

        render::draw_map_state(Update_screen::no);

        map::player->update_fov();

        render::draw_blast_at_cells(anim_cells, clr_white);

        render::draw_map_state(Update_screen::yes);
    }

    identify(Verbosity::verbose);
}

void Potion_descent::quaff_impl(Actor& actor)
{
    (void)actor;

    if (map::dlvl < dlvl_last - 1)
    {
        map::player->prop_handler().try_add(new Prop_descend(Prop_turns::std));
    }
    else
    {
        msg_log::add("I feel a faint sinking sensation, but it quickly disappears...");
    }

    identify(Verbosity::verbose);
}

void Potion_invis::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_invisible(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_invis::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_see_invis::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add(new Prop_see_invis(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_see_invis::collide_hook(const P& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

namespace potion_handling
{

namespace
{

std::vector<Potion_look> potion_looks_;

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;

    //Init possible potion colors and fake names
    potion_looks_.clear();
    potion_looks_.push_back(Potion_look {"Golden",   "a Golden",   clr_yellow});
    potion_looks_.push_back(Potion_look {"Yellow",   "a Yellow",   clr_yellow});
    potion_looks_.push_back(Potion_look {"Dark",     "a Dark",     clr_gray});
    potion_looks_.push_back(Potion_look {"Black",    "a Black",    clr_gray});
    potion_looks_.push_back(Potion_look {"Oily",     "an Oily",    clr_gray});
    potion_looks_.push_back(Potion_look {"Smoky",    "a Smoky",    clr_white});
    potion_looks_.push_back(Potion_look {"Slimy",    "a Slimy",    clr_green});
    potion_looks_.push_back(Potion_look {"Green",    "a Green",    clr_green_lgt});
    potion_looks_.push_back(Potion_look {"Fiery",    "a Fiery",    clr_red_lgt});
    potion_looks_.push_back(Potion_look {"Murky",    "a Murky",    clr_brown_drk});
    potion_looks_.push_back(Potion_look {"Muddy",    "a Muddy",    clr_brown});
    potion_looks_.push_back(Potion_look {"Violet",   "a Violet",   clr_violet});
    potion_looks_.push_back(Potion_look {"Orange",   "an Orange",  clr_orange});
    potion_looks_.push_back(Potion_look {"Watery",   "a Watery",   clr_blue_lgt});
    potion_looks_.push_back(Potion_look {"Metallic", "a Metallic", clr_gray});
    potion_looks_.push_back(Potion_look {"Clear",    "a Clear",    clr_white_high});
    potion_looks_.push_back(Potion_look {"Misty",    "a Misty",    clr_white_high});
    potion_looks_.push_back(Potion_look {"Bloody",   "a Bloody",   clr_red});
    potion_looks_.push_back(Potion_look {"Magenta",  "a Magenta",  clr_magenta});
    potion_looks_.push_back(Potion_look {"Clotted",  "a Clotted",  clr_green});
    potion_looks_.push_back(Potion_look {"Moldy",    "a Moldy",    clr_brown});
    potion_looks_.push_back(Potion_look {"Frothy",   "a Frothy",   clr_white});

    for (auto& d : item_data::data)
    {
        if (d.type == Item_type::potion)
        {
            //Color and false name
            const size_t idx = rnd::range(0, potion_looks_.size() - 1);

            Potion_look& look = potion_looks_[idx];

            d.base_name_un_id.names[int(Item_ref_type::plain)]   = look.name_plain + " Potion";
            d.base_name_un_id.names[int(Item_ref_type::plural)]  = look.name_plain + " Potions";
            d.base_name_un_id.names[int(Item_ref_type::a)]       = look.name_a     + " Potion";
            d.clr = look.clr;

            potion_looks_.erase(potion_looks_.begin() + idx);

            //True name
            const Potion* const potion =
                static_cast<const Potion*>(item_factory::mk(d.id, 1));

            const std::string real_type_name = potion->real_name();

            delete potion;

            const std::string real_name        = "Potion of "    + real_type_name;
            const std::string real_name_plural = "Potions of "   + real_type_name;
            const std::string real_name_a      = "a Potion of "  + real_type_name;

            d.base_name.names[int(Item_ref_type::plain)]  = real_name;
            d.base_name.names[int(Item_ref_type::plural)] = real_name_plural;
            d.base_name.names[int(Item_ref_type::a)]      = real_name_a;
        }
    }

    TRACE_FUNC_END;
}

void save()
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        Item_data_t& d = item_data::data[i];

        if (d.type == Item_type::potion)
        {
            save_handling::put_str(d.base_name_un_id.names[int(Item_ref_type::plain)]);
            save_handling::put_str(d.base_name_un_id.names[int(Item_ref_type::plural)]);
            save_handling::put_str(d.base_name_un_id.names[int(Item_ref_type::a)]);

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

        if (d.type == Item_type::potion)
        {
            d.base_name_un_id.names[int(Item_ref_type::plain)]  = save_handling::get_str();
            d.base_name_un_id.names[int(Item_ref_type::plural)] = save_handling::get_str();
            d.base_name_un_id.names[int(Item_ref_type::a)]      = save_handling::get_str();

            d.clr.r = save_handling::get_int();
            d.clr.g = save_handling::get_int();
            d.clr.b = save_handling::get_int();
        }
    }
}

} //potion_handling
