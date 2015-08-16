#include "item_potion.hpp"

#include <climits>

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
#include "map_travel.hpp"
#include "map_parsing.hpp"
#include "utils.hpp"
#include "feature_rigid.hpp"
#include "item_factory.hpp"

Consume_item Potion::activate(Actor* const actor)
{
    assert(actor);

    if (actor->prop_handler().allow_eat(Verbosity::verbose))
    {
        quaff(*actor);
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
            const string potion_name = name(Item_ref_type::a, Item_ref_inf::none);
            msg_log::add("It was " + potion_name + ".");
            map::player->incr_shock(Shock_lvl::heavy, Shock_src::use_strange_item);
        }
    }
}

vector<string> Potion::descr() const
{
    if (data_->is_identified)
    {
        return descr_identified();
    }
    else
    {
        return data_->base_descr;
    }
}

void Potion::on_collide(const Pos& pos, Actor* const actor)
{
    if (!map::cells[pos.x][pos.y].rigid->is_bottomless() || actor)
    {
        //Render and print message
        const bool PLAYER_SEE_CELL = map::cells[pos.x][pos.y].is_seen_by_player;

        if (PLAYER_SEE_CELL)
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

                if (actor->is_alive() && !data_->is_identified && PLAYER_SEE_CELL)
                {
                    //This did not identify the potion
                    msg_log::add("It had no apparent effect...");
                }
            }
        }
    }
}

void Potion::quaff(Actor& actor)
{
    if (actor.is_player())
    {
        data_->is_tried = true;

        audio::play(Sfx_id::potion_quaff);

        if (data_->is_identified)
        {
            const string potion_name = name(Item_ref_type::a, Item_ref_inf::none);
            msg_log::add("I drink " + potion_name + "...");
        }
        else //Unidentified
        {
            const string potion_name = name(Item_ref_type::plain, Item_ref_inf::none);
            msg_log::add("I drink an unknown " + potion_name + "...");
        }

        map::player->incr_shock(Shock_lvl::heavy, Shock_src::use_strange_item);
    }

    quaff_impl(actor);

    if (map::player->is_alive())
    {
        game_time::tick();
    }
}

string Potion::name_inf() const
{
    return (data_->is_tried && !data_->is_identified) ? "{Tried}" : "";
}

void Potion_vitality::quaff_impl(Actor& actor)
{
    actor.prop_handler().end_props_by_magic_healing();

    //HP is always restored at least up to maximum HP, but can go beyond
    const int HP          = actor.hp();
    const int HP_MAX      = actor.hp_max(true);
    const int HP_RESTORED = std::max(20, HP_MAX - HP);

    actor.restore_hp(HP_RESTORED);

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_vitality::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_spirit::quaff_impl(Actor& actor)
{
    //SPI is always restored at least up to maximum SPI, but can go beyond
    const int SPI           = actor.spi();
    const int SPI_MAX       = actor.spi_max();
    const int SPI_RESTORED  = std::max(10, SPI_MAX - SPI);

    actor.restore_spi(SPI_RESTORED, true);

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_spirit::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_blindness::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_blind(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_blindness::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_paral::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_paralyzed(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_paral::collide_hook(const Pos& pos, Actor* const actor)
{

    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_disease::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_diseased(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_conf::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_confused(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_conf::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_frenzy::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_frenzied(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_frenzy::collide_hook(const Pos& pos, Actor* const actor)
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

    Prop_rFear* const   rFear   = new Prop_rFear(Prop_turns::std);

    const int NR_TURNS_LEFT = rFear->nr_turns_left();

    Prop_rConf* const   rConf   = new Prop_rConf(Prop_turns::specific,  NR_TURNS_LEFT);
    Prop_rSleep* const  rSleep  = new Prop_rSleep(Prop_turns::specific, NR_TURNS_LEFT);

    prop_handler.try_add_prop(rFear);
    prop_handler.try_add_prop(rConf);
    prop_handler.try_add_prop(rSleep);

    if (actor.is_player())
    {
        bool is_phobias_cured = false;

        for (int i = 0; i < int(Phobia::END); ++i)
        {
            if (map::player->phobias[i])
            {
                map::player->phobias[i] = false;
                is_phobias_cured = true;
            }
        }

        if (is_phobias_cured)
        {
            msg_log::add("All my phobias are cured!");
        }

        bool is_obsessions_cured = false;

        for (int i = 0; i < int(Obsession::END); ++i)
        {
            if (map::player->obsessions[i])
            {
                map::player->obsessions[i] = false;
                is_obsessions_cured = true;
            }
        }

        if (is_obsessions_cured)
        {
            msg_log::add("All my obsessions are cured!");
        }

        map::player->restore_shock(999, false);
        msg_log::add("I feel more at ease.");
    }

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_fortitude::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_poison::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_poisoned(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_poison::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_rFire::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_rFire(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_rFire::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_antidote::quaff_impl(Actor& actor)
{
    const bool WAS_POISONED = actor.prop_handler().end_prop(Prop_id::poisoned);

    if (WAS_POISONED && map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_antidote::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_rElec::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_rElec(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_rElec::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_rAcid::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_rAcid(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_rAcid::collide_hook(const Pos& pos, Actor* const actor)
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

    vector<Item*> identify_bucket;

    auto& slots = inv.slots_;

    for (Inv_slot& slot : slots)
    {
        Item* const item = slot.item;

        if (item)
        {
            const Item_data_t& d = item->data();

            if (!d.is_identified) {identify_bucket.push_back(item);}
        }
    }

    vector<Item*>& general = inv.backpack_;

    for (Item* item : general)
    {
        if (item->id() != Item_id::potion_insight)
        {
            const Item_data_t& d = item->data();

            if (!d.is_identified) {identify_bucket.push_back(item);}
        }
    }

    const size_t NR_ELEMENTS = identify_bucket.size();

    if (NR_ELEMENTS > 0)
    {
        const int     IDX             = rnd::range(0, NR_ELEMENTS - 1);
        Item* const   item            = identify_bucket[IDX];
        const string  item_name_before  = item->name(Item_ref_type::a, Item_ref_inf::none);

        msg_log::add("I gain intuitions about " + item_name_before + "...", clr_white, false,
                     More_prompt_on_msg::yes);

        item->identify(Verbosity::silent);

        const string item_name_after = item->name(Item_ref_type::a, Item_ref_inf::none);

        render::draw_map_and_interface(true);

        if (item_name_before != item_name_after)
        {
            msg_log::add("It is identified as " + item_name_after + "!");
        }
        else //Item name is same as before
        {
            //Typically, items that change names when identified have a "nonsense" name
            //first (e.g. "A Green Potion"), that change into something more descriptive
            //(e.g. "A Potion of Fire Resistance"). In those cases, the message can be
            //something like "Aha, [old name] is a [new name]!". But when the name is the
            //same, the message needs to be different, (e.g. "An Iron Ring"). Then we
            //print a message like "Aha, I understand it now".

            //TODO: This is hacky and fragile, but it works for now.

            msg_log::add("All its properties are known to me.");
        }
    }

    identify(Verbosity::verbose);
}

void Potion_clairv::quaff_impl(Actor& actor)
{
    if (actor.is_player())
    {
        msg_log::add("I see far and wide!");

        vector<Pos> anim_cells;
        anim_cells.clear();

        bool blocked[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_los(), blocked);

        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                Cell& cell = map::cells[x][y];

                if (!blocked[x][y] && !cell.is_dark)
                {
                    cell.is_explored = true;
                    cell.is_seen_by_player = true;
                    anim_cells.push_back(Pos(x, y));
                }
            }
        }

        render::draw_map_and_interface(false);

        map::player->update_fov();

        render::draw_blast_at_cells(anim_cells, clr_white);
    }

    identify(Verbosity::verbose);
}

void Potion_descent::quaff_impl(Actor& actor)
{
    (void)actor;

    if (map::dlvl < DLVL_LAST - 1)
    {
        render::draw_map_and_interface();
        msg_log::add("I sink downwards!", clr_white, false, More_prompt_on_msg::yes);
        map_travel::go_to_nxt();
    }
    else
    {
        msg_log::add("I feel a faint sinking sensation.");
    }

    identify(Verbosity::verbose);
}

void Potion_invis::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_invisible(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_invis::collide_hook(const Pos& pos, Actor* const actor)
{
    (void)pos;

    if (actor)
    {
        quaff_impl(*actor);
    }
}

void Potion_seeing::quaff_impl(Actor& actor)
{
    actor.prop_handler().try_add_prop(new Prop_seeing(Prop_turns::std));

    if (map::player->can_see_actor(actor))
    {
        identify(Verbosity::verbose);
    }
}

void Potion_seeing::collide_hook(const Pos& pos, Actor* const actor)
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

vector<Potion_look> potion_looks_;

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

    TRACE << "Init potion names" << endl;

    for (auto& d : item_data::data)
    {
        if (d.type == Item_type::potion)
        {
            //Color and false name
            const int ELEMENT = rnd::range(0, potion_looks_.size() - 1);

            Potion_look& look = potion_looks_[ELEMENT];

            d.base_name_un_id.names[int(Item_ref_type::plain)]   = look.name_plain + " potion";
            d.base_name_un_id.names[int(Item_ref_type::plural)]  = look.name_plain + " potions";
            d.base_name_un_id.names[int(Item_ref_type::a)]       = look.name_a     + " potion";
            d.clr = look.clr;

            potion_looks_.erase(potion_looks_.begin() + ELEMENT);

            //True name
            const Potion* const potion =
                static_cast<const Potion*>(item_factory::mk(d.id, 1));

            const string REAL_TYPE_NAME = potion->real_name();

            delete potion;

            const string REAL_NAME        = "Potion of "    + REAL_TYPE_NAME;
            const string REAL_NAME_PLURAL = "Potions of "   + REAL_TYPE_NAME;
            const string REAL_NAME_A      = "a potion of "  + REAL_TYPE_NAME;

            d.base_name.names[int(Item_ref_type::plain)]  = REAL_NAME;
            d.base_name.names[int(Item_ref_type::plural)] = REAL_NAME_PLURAL;
            d.base_name.names[int(Item_ref_type::a)]      = REAL_NAME_A;
        }
    }

    TRACE_FUNC_END;
}

void store_to_save_lines(vector<string>& lines)
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        Item_data_t& d = item_data::data[i];

        if (d.type == Item_type::potion)
        {
            lines.push_back(d.base_name_un_id.names[int(Item_ref_type::plain)]);
            lines.push_back(d.base_name_un_id.names[int(Item_ref_type::plural)]);
            lines.push_back(d.base_name_un_id.names[int(Item_ref_type::a)]);
            lines.push_back(to_str(d.clr.r));
            lines.push_back(to_str(d.clr.g));
            lines.push_back(to_str(d.clr.b));
        }
    }
}

void setup_from_save_lines(vector<string>& lines)
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        Item_data_t& d = item_data::data[i];

        if (d.type == Item_type::potion)
        {
            d.base_name_un_id.names[int(Item_ref_type::plain)]  = lines.front();
            lines.erase(begin(lines));
            d.base_name_un_id.names[int(Item_ref_type::plural)] = lines.front();
            lines.erase(begin(lines));
            d.base_name_un_id.names[int(Item_ref_type::a)]      = lines.front();
            lines.erase(begin(lines));
            d.clr.r = to_int(lines.front());
            lines.erase(begin(lines));
            d.clr.g = to_int(lines.front());
            lines.erase(begin(lines));
            d.clr.b = to_int(lines.front());
            lines.erase(begin(lines));
        }
    }
}

} //Potion_handling
