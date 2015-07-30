#include "item_scroll.hpp"

#include <string>
#include <cassert>

#include "init.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "player_bon.hpp"
#include "msg_log.hpp"
#include "inventory.hpp"
#include "player_spells_handling.hpp"
#include "render.hpp"
#include "utils.hpp"
#include "item_factory.hpp"

using namespace std;

const string Scroll::real_name() const
{
    Spell* spell                = spell_handling::mk_spell_from_id(data_->spell_cast_from_scroll);
    const string scroll_name    = spell->name();
    delete spell;
    return scroll_name;
}

vector<string> Scroll::descr() const
{
    if (data_->is_identified)
    {
        const auto* const spell = mk_spell();
        const auto descr = spell->descr();
        delete spell;
        return descr;
    }
    else
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
        return read();
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
            const string scroll_name = name(Item_ref_type::a, Item_ref_inf::none);
            msg_log::add("It was " + scroll_name + ".");
            render::draw_map_and_interface();
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

Consume_item Scroll::read()
{
    render::draw_map_and_interface();

    if (!map::player->prop_handler().allow_see())
    {
        msg_log::add("I cannot read while blind.");
        return Consume_item::no;
    }

    auto* const spell = mk_spell();

    const string crumble_str = "It crumbles to dust.";

    if (data_->is_identified)
    {
        const string scroll_name = name(Item_ref_type::a, Item_ref_inf::none);

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

string Scroll::name_inf() const
{
    return (data_->is_tried && !data_->is_identified) ? "{Tried}" : "";
}

namespace scroll_handling
{

namespace
{

vector<string> false_names_;

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;

    //Init possible fake names
    false_names_.clear();
    false_names_.push_back("Cruensseasrjit");
    false_names_.push_back("Rudsceleratus");
    false_names_.push_back("Rudminuox");
    false_names_.push_back("Cruo-stragara_na");
    false_names_.push_back("Praya_navita");
    false_names_.push_back("Pretiacruento");
    false_names_.push_back("Pestis cruento");
    false_names_.push_back("Cruento pestis");
    false_names_.push_back("Domus-bhaava");
    false_names_.push_back("Acerbus-shatruex");
    false_names_.push_back("Pretaanluxis");
    false_names_.push_back("Praa_nsilenux");
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

    vector<string> cmb;
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

    TRACE << "Init scroll names" << endl;

    for (auto& d : item_data::data)
    {
        if (d.type == Item_type::scroll)
        {
            //False name
            const int NR_ELEMENTS = false_names_.size();
            const int ELEMENT     = rnd::range(0, NR_ELEMENTS - 1);

            const string& TITLE = false_names_[ELEMENT];

            d.base_name_un_id.names[int(Item_ref_type::plain)] =
                "Manuscript titled "    + TITLE;

            d.base_name_un_id.names[int(Item_ref_type::plural)] =
                "Manuscripts titled "   + TITLE;

            d.base_name_un_id.names[int(Item_ref_type::a)] =
                "a Manuscript titled "  + TITLE;

            false_names_.erase(false_names_.begin() + ELEMENT);

            //True name
            const Scroll* const scroll =
                static_cast<const Scroll*>(item_factory::mk(d.id, 1));

            const string REAL_TYPE_NAME = scroll->real_name();

            delete scroll;

            const string REAL_NAME        = "Manuscript of "    + REAL_TYPE_NAME;
            const string REAL_NAME_PLURAL = "Manuscripts of "   + REAL_TYPE_NAME;
            const string REAL_NAME_A      = "a Manuscript of "  + REAL_TYPE_NAME;

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
        if (item_data::data[i].type == Item_type::scroll)
        {
            auto& base_name_un_id = item_data::data[i].base_name_un_id;
            lines.push_back(base_name_un_id.names[int(Item_ref_type::plain)]);
            lines.push_back(base_name_un_id.names[int(Item_ref_type::plural)]);
            lines.push_back(base_name_un_id.names[int(Item_ref_type::a)]);
        }
    }
}

void setup_from_save_lines(vector<string>& lines)
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        if (item_data::data[i].type == Item_type::scroll)
        {
            auto& base_name_un_id = item_data::data[i].base_name_un_id;
            base_name_un_id.names[int(Item_ref_type::plain)]  = lines.front();
            lines.erase(begin(lines));
            base_name_un_id.names[int(Item_ref_type::plural)] = lines.front();
            lines.erase(begin(lines));
            base_name_un_id.names[int(Item_ref_type::a)]      = lines.front();
            lines.erase(begin(lines));
        }
    }
}

} //Scroll_handling
