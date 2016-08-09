#include "item_scroll.hpp"

#include <string>

#include "init.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "player_bon.hpp"
#include "msg_log.hpp"
#include "inventory.hpp"
#include "player_spells.hpp"
#include "render.hpp"
#include "item_factory.hpp"
#include "saving.hpp"
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

ConsumeItem Scroll::activate(Actor* const actor)
{
    auto& prop_handler = actor->prop_handler();

    if (
        prop_handler.allow_cast_spell(Verbosity::verbose)   &&
        prop_handler.allow_read(Verbosity::verbose)         &&
        prop_handler.allow_speak(Verbosity::verbose))
    {
        render::draw_map_state();

        if (!map::player->prop_handler().allow_see())
        {
            msg_log::add("I cannot read while blind.");
            return ConsumeItem::no;
        }

        const P& player_pos(map::player->pos);

        const Cell& cell = map::cells[player_pos.x][player_pos.y];

        if (cell.is_dark && !cell.is_lit)
        {
            msg_log::add("It's too dark to read here.");
            return ConsumeItem::no;
        }

        auto* const spell = mk_spell();

        const std::string crumble_str = "The Manuscript crumbles to dust.";

        const bool is_identified_before = data_->is_identified;

        if (is_identified_before)
        {
            const std::string scroll_name = name(ItemRefType::a, ItemRefInf::none);

            msg_log::add("I read " + scroll_name + "...");
        }
        else //Not already identified
        {
            msg_log::add("I recite the forbidden incantations on the manuscript...");

            data_->is_tried = true;
        }

        const auto is_noticed = spell->cast(map::player, false, true);

        msg_log::add(crumble_str);

        if (
            !is_identified_before &&
            is_noticed == SpellEffectNoticed::yes)
        {
            identify(Verbosity::verbose);
        }

        //Learn spell, increase skill level
        if (
            data_->is_identified &&
            spell->player_can_learn())
        {
            const SpellId id = spell->id();

            const bool is_learned_before = player_spells::is_spell_learned(id);

            if (!is_learned_before)
            {
                player_spells::learn_spell(id, Verbosity::verbose);
            }

            const auto skill_incr_verbosity =
                is_learned_before ?
                Verbosity::verbose : Verbosity::silent;

            player_spells::incr_spell_skill(id, skill_incr_verbosity);
        }

        delete spell;

        return ConsumeItem::yes;
    }

    return ConsumeItem::no;
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
            const std::string name_after = name(ItemRefType::a, ItemRefInf::none);

            msg_log::add("I have identified " + name_after + ".");

            render::draw_map_state();

            dungeon_master::add_history_event("Identified " + name_after + ".");

            give_xp_for_identify();
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

    const size_t nr_cmb_parts = cmb.size();

    for (size_t i = 0; i < nr_cmb_parts; ++i)
    {
        for (size_t ii = 0; ii < nr_cmb_parts; ii++)
        {
            if (i != ii)
            {
                false_names_.push_back(cmb[i] + " " + cmb[ii]);
            }
        }
    }

    for (auto& d : item_data::data)
    {
        if (d.type == ItemType::scroll)
        {
            //False name
            const size_t idx = rnd::range(0, false_names_.size() - 1);

            const std::string& TITLE = false_names_[idx];

            d.base_name_un_id.names[int(ItemRefType::plain)] =
                "Manuscript titled "    + TITLE;

            d.base_name_un_id.names[int(ItemRefType::plural)] =
                "Manuscripts titled "   + TITLE;

            d.base_name_un_id.names[int(ItemRefType::a)] =
                "a Manuscript titled "  + TITLE;

            false_names_.erase(false_names_.begin() + idx);

            //True name
            const Scroll* const scroll = static_cast<const Scroll*>(item_factory::mk(d.id, 1));

            const std::string real_type_name = scroll->real_name();

            delete scroll;

            const std::string real_name        = "Manuscript of "    + real_type_name;
            const std::string real_name_plural = "Manuscripts of "   + real_type_name;
            const std::string real_name_a      = "a Manuscript of "  + real_type_name;

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
        if (item_data::data[i].type == ItemType::scroll)
        {
            auto& base_name_un_id = item_data::data[i].base_name_un_id;

            saving::put_str(base_name_un_id.names[int(ItemRefType::plain)]);
            saving::put_str(base_name_un_id.names[int(ItemRefType::plural)]);
            saving::put_str(base_name_un_id.names[int(ItemRefType::a)]);
        }
    }
}

void load()
{
    for (int i = 0; i < int(ItemId::END); ++i)
    {
        if (item_data::data[i].type == ItemType::scroll)
        {
            auto& base_name_un_id = item_data::data[i].base_name_un_id;

            base_name_un_id.names[int(ItemRefType::plain)]  = saving::get_str();
            base_name_un_id.names[int(ItemRefType::plural)] = saving::get_str();
            base_name_un_id.names[int(ItemRefType::a)]      = saving::get_str();
        }
    }
}

} //scroll_handling
