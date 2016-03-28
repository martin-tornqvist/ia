#include "item_scroll.hpp"

#include <string>

#include "init.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "player_bon.hpp"
#include "msg_log.hpp"
#include "inventory.hpp"
#include "player_spells_handling.hpp"
#include "render.hpp"
#include "item_factory.hpp"
#include "save_handling.hpp"
#include "dungeon_master.hpp"

const std::string Scroll::real_name() const
{
    Spell* spell = spell_handling::mk_spell_from_id(data_->spell_cast_from_scroll);

    const std::string scroll_name = spell->name();

    delete spell;

    return scroll_name;
}

std::vector<std::string> Scroll::descr() const
{
    if (data_->is_identified)
    {
        const auto* const spell = mk_spell();
        const auto descr = spell->descr();
        delete spell;
        return descr;
    }
    else //Not identified
    {
        return data_->base_descr;
    }
}

Consume_item Scroll::activate(Actor* const actor)
{
    auto& prop_handler = actor->prop_handler();

    if (
        prop_handler.allow_cast_spell(Verbosity::verbose)   &&
        prop_handler.allow_read(Verbosity::verbose)         &&
        prop_handler.allow_speak(Verbosity::verbose))
    {
        render::draw_map_and_interface();

        if (!map::player->prop_handler().allow_see())
        {
            msg_log::add("I cannot read while blind.");
            return Consume_item::no;
        }

        const P& player_pos(map::player->pos);

        const Cell& cell = map::cells[player_pos.x][player_pos.y];

        if (cell.is_dark && !cell.is_lit)
        {
            msg_log::add("It's too dark to read here.");
            return Consume_item::no;
        }

        auto* const spell = mk_spell();

        const std::string crumble_str = "The Manuscript crumbles to dust.";

        if (data_->is_identified)
        {
            const std::string scroll_name = name(Item_ref_type::a, Item_ref_inf::none);

            msg_log::add("I read " + scroll_name + "...");

            spell->cast(map::player, false);

            msg_log::add(crumble_str);

            try_learn();
        }
        else //Not already identified
        {
            msg_log::add("I recite the forbidden incantations on the manuscript...");

            data_->is_tried = true;

            const auto is_noticed = spell->cast(map::player, false);

            msg_log::add(crumble_str);

            if (is_noticed == Spell_effect_noticed::yes)
            {
                identify(Verbosity::verbose);
            }
        }

        delete spell;

        return Consume_item::yes;
    }

    return Consume_item::no;
}

Spell* Scroll::mk_spell() const
{
    return spell_handling::mk_spell_from_id(data_->spell_cast_from_scroll);
}

void Scroll::identify(const Verbosity verbosity)
{
    if (!data_->is_identified)
    {
        data_->is_identified = true;

        if (verbosity == Verbosity::verbose)
        {
            const std::string name_after = name(Item_ref_type::a, Item_ref_inf::none);

            msg_log::add("I have identified " + name_after + ".");

            render::draw_map_and_interface();

            dungeon_master::add_history_event("Identified " + name_after + ".");
        }
    }
}

void Scroll::try_learn()
{
    if (player_bon::bg() == Bg::occultist)
    {
        Spell* const spell = mk_spell();

        if (
            spell->is_avail_for_player() &&
            !player_spells_handling::is_spell_learned(spell->id()))
        {
            msg_log::add("I learn to cast this incantation by heart!");
            player_spells_handling::learn_spell_if_not_known(spell);
        }
        else
        {
            delete spell;
        }
    }
}

std::string Scroll::name_inf() const
{
    return (data_->is_tried && !data_->is_identified) ? "{Tried}" : "";
}

namespace scroll_handling
{

namespace
{

std::vector<std::string> false_names_;

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;

    //Init possible fake names
    false_names_.clear();
    false_names_.push_back("Cruensseasrjit");
    false_names_.push_back("Rudsceleratus");
    false_names_.push_back("Rudminuox");
    false_names_.push_back("Cruo stragara-na");
    false_names_.push_back("Praya navita");
    false_names_.push_back("Pretia Cruento");
    false_names_.push_back("Pestis Cruento");
    false_names_.push_back("Cruento Pestis");
    false_names_.push_back("Domus-bhaava");
    false_names_.push_back("Acerbus-shatruex");
    false_names_.push_back("Pretaanluxis");
    false_names_.push_back("Praansilenux");
    false_names_.push_back("Quodpipax");
    false_names_.push_back("Lokemundux");
    false_names_.push_back("Profanuxes");
    false_names_.push_back("Shaantitus");
    false_names_.push_back("Geropayati");
    false_names_.push_back("Vilomaxus");
    false_names_.push_back("Bhuudesco");
    false_names_.push_back("Durbentia");
    false_names_.push_back("Bhuuesco");
    false_names_.push_back("Maravita");
    false_names_.push_back("Infirmux");

    std::vector<std::string> cmb;
    cmb.clear();
    cmb.push_back("Cruo");
    cmb.push_back("Cruonit");
    cmb.push_back("Cruentu");
    cmb.push_back("Marana");
    cmb.push_back("Domus");
    cmb.push_back("Malax");
    cmb.push_back("Caecux");
    cmb.push_back("Eximha");
    cmb.push_back("Vorox");
    cmb.push_back("Bibox");
    cmb.push_back("Pallex");
    cmb.push_back("Profanx");
    cmb.push_back("Invisuu");
    cmb.push_back("Invisux");
    cmb.push_back("Odiosuu");
    cmb.push_back("Odiosux");
    cmb.push_back("Vigra");
    cmb.push_back("Crudux");
    cmb.push_back("Desco");
    cmb.push_back("Esco");
    cmb.push_back("Gero");
    cmb.push_back("Klaatu");
    cmb.push_back("Barada");
    cmb.push_back("Nikto");

    const size_t NR_CMB_PARTS = cmb.size();

    for (size_t i = 0; i < NR_CMB_PARTS; ++i)
    {
        for (size_t ii = 0; ii < NR_CMB_PARTS; ii++)
        {
            if (i != ii)
            {
                false_names_.push_back(cmb[i] + " " + cmb[ii]);
            }
        }
    }

    for (auto& d : item_data::data)
    {
        if (d.type == Item_type::scroll)
        {
            //False name
            const size_t IDX = rnd::range(0, false_names_.size() - 1);

            const std::string& TITLE = false_names_[IDX];

            d.base_name_un_id.names[int(Item_ref_type::plain)] =
                "Manuscript titled "    + TITLE;

            d.base_name_un_id.names[int(Item_ref_type::plural)] =
                "Manuscripts titled "   + TITLE;

            d.base_name_un_id.names[int(Item_ref_type::a)] =
                "a Manuscript titled "  + TITLE;

            false_names_.erase(false_names_.begin() + IDX);

            //True name
            const Scroll* const scroll = static_cast<const Scroll*>(item_factory::mk(d.id, 1));

            const std::string REAL_TYPE_NAME = scroll->real_name();

            delete scroll;

            const std::string REAL_NAME        = "Manuscript of "    + REAL_TYPE_NAME;
            const std::string REAL_NAME_PLURAL = "Manuscripts of "   + REAL_TYPE_NAME;
            const std::string REAL_NAME_A      = "a Manuscript of "  + REAL_TYPE_NAME;

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
        if (item_data::data[i].type == Item_type::scroll)
        {
            auto& base_name_un_id = item_data::data[i].base_name_un_id;

            save_handling::put_str(base_name_un_id.names[int(Item_ref_type::plain)]);
            save_handling::put_str(base_name_un_id.names[int(Item_ref_type::plural)]);
            save_handling::put_str(base_name_un_id.names[int(Item_ref_type::a)]);
        }
    }
}

void load()
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        if (item_data::data[i].type == Item_type::scroll)
        {
            auto& base_name_un_id = item_data::data[i].base_name_un_id;

            base_name_un_id.names[int(Item_ref_type::plain)]  = save_handling::get_str();
            base_name_un_id.names[int(Item_ref_type::plural)] = save_handling::get_str();
            base_name_un_id.names[int(Item_ref_type::a)]      = save_handling::get_str();
        }
    }
}

} //Scroll_handling
