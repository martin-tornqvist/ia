#include "player_spells_handling.hpp"

#include <vector>
#include <algorithm>

#include "init.hpp"
#include "item_scroll.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "menu_input.hpp"
#include "render.hpp"
#include "inventory.hpp"
#include "item_factory.hpp"
#include "player_bon.hpp"
#include "query.hpp"
#include "utils.hpp"
#include "map.hpp"

namespace player_spells_handling
{

namespace
{

struct Spell_opt
{
    Spell_opt() :
        spell       (nullptr),
        src_item    (nullptr) {}

    Spell_opt(Spell* spell_, Item* src_item_) :
        spell       (spell_),
        src_item    (src_item_) {}

    Spell*  spell;
    Item*   src_item;
};

std::vector<Spell*>  known_spells_;
Spell_opt        prev_cast_;

void draw(Menu_browser& browser, const std::vector<Spell_opt>& spell_opts)
{
    const int NR_SPELLS = spell_opts.size();

    render::clear_screen();

    render::draw_text_centered("Invoke power", Panel::screen, Pos(SCREEN_W / 2, 0), clr_orange);

    Pos p(0, 1);

    std::string key_str = "a) ";

    for (int i = 0; i < NR_SPELLS; ++i)
    {
        const int       CUR_IDX         = i;
        Scroll          scroll(nullptr);
        Clr             scroll_clr      = scroll.interface_clr();
        const bool      IS_IDX_MARKED   = browser.is_at_idx(CUR_IDX);
        Spell_opt       spell_opt       = spell_opts[i];
        Spell* const    spell           = spell_opt.spell;
        std::string     name            = spell->name();
        const int       SPI_X           = 25;
        const int       SHOCK_X         = SPI_X + 10;

        p.x = 0;

        Clr clr = IS_IDX_MARKED ? clr_white_high : clr_menu_drk;

        render::draw_text(key_str, Panel::screen, p, clr);

        ++key_str[0];

        clr = IS_IDX_MARKED ? clr_white : scroll_clr;

        p.x = key_str.size();

        render::draw_text(name, Panel::screen, p, clr);

        std::string fill_str = "";

        const size_t FILL_SIZE = SPI_X - p.x - name.size();

        for (size_t ii = 0; ii < FILL_SIZE; ii++)
        {
            fill_str.push_back('.');
        }

        Clr fill_clr = clr_gray;

        fill_clr.r /= 3;
        fill_clr.g /= 3;
        fill_clr.b /= 3;

        render::draw_text(fill_str, Panel::screen, Pos(p.x + name.size(), p.y), fill_clr);

        p.x = SPI_X;

        std::string  info_str = "SPI: ";

        const Range spi_cost = spell->spi_cost(false, map::player);
        const std::string lower_str = to_str(spi_cost.min);
        const std::string upper_str = to_str(spi_cost.max);

        info_str += spi_cost.max == 1 ? "1" : (lower_str +  "-" + upper_str);

        render::draw_text(info_str, Panel::screen, p, clr_white);

        p.x = SHOCK_X;

        const Intr_spell_shock shock_type = spell->shock_type_intr_cast();

        switch (shock_type)
        {
        case Intr_spell_shock::mild:
            info_str = "Mild";
            break;

        case Intr_spell_shock::disturbing:
            info_str = "Disturbing";
            break;

        case Intr_spell_shock::severe:
            info_str = "Severe";
            break;
        }

        render::draw_text(info_str, Panel::screen, p, clr_white);

        if (IS_IDX_MARKED)
        {
            const auto descr = spell->descr();
            std::vector<Str_and_clr> lines;

            if (!descr.empty())
            {
                for (const auto& line : descr)
                {
                    lines.push_back({line, clr_white_high});
                }
            }

            //If spell source is an item, add info about this
            if (spell_opt.src_item)
            {
                const std::string item_name =
                    spell_opt.src_item->name(Item_ref_type::plain, Item_ref_inf::none);

                lines.push_back({"Spell granted by " + item_name + ".", clr_green});
            }

            if (!lines.empty())
            {
                render::draw_descr_box(lines);
            }
        }

        ++p.y;
    }

    render::update_screen();
}

void spells_avail(std::vector<Spell_opt>& out)
{
    out.clear();

    for (Spell* const spell : known_spells_)
    {
        out.push_back(Spell_opt(spell, nullptr));
    }

    Inventory& inv = map::player->inv();

    for (auto slot : inv.slots_)
    {
        Item* item = slot.item;

        if (item)
        {
            const std::vector<Spell*>& carrier_spells = item->carrier_spells();

            for (Spell* spell : carrier_spells)
            {
                out.push_back(Spell_opt(spell, item));
            }
        }
    }

    for (Item* item : inv.backpack_)
    {
        const std::vector<Spell*>& carrier_spells = item->carrier_spells();

        for (Spell* spell : carrier_spells)
        {
            out.push_back(Spell_opt(spell, item));
        }
    }
}

void try_cast(const Spell_opt& spell_opt)
{
    assert(spell_opt.spell);

    if (
        map::player->prop_handler().allow_cast_spell(Verbosity::verbose) &&
        map::player->prop_handler().allow_speak(Verbosity::verbose))
    {
        msg_log::clear();
        render::draw_map_and_interface();

        Spell* const spell = spell_opt.spell;

        const Range spi_cost_range = spell->spi_cost(false, map::player);

        if (spi_cost_range.max >= map::player->spi())
        {
            msg_log::add("Cast spell and risk depleting your spirit [y/n]?",
                         clr_white_high);

            render::draw_map_and_interface();

            if (query::yes_or_no() == Yes_no_answer::no)
            {
                msg_log::clear();
                render::draw_map_and_interface();
                return;
            }

            msg_log::clear();
        }

        const bool IS_BLOOD_SORC = player_bon::traits[int(Trait::blood_sorcerer)];
        const bool IS_WARLOCK    = player_bon::traits[int(Trait::warlock)];

        const int BLOOD_SORC_HP_DRAINED = 2;

        if (IS_BLOOD_SORC)
        {
            if (map::player->hp() <= BLOOD_SORC_HP_DRAINED)
            {
                msg_log::add("I do not have enough life force to cast this spell.");
                render::draw_map_and_interface();
                return;
            }
        }

        msg_log::add("I cast " + spell->name() + "!");

        if (IS_BLOOD_SORC)
        {
            map::player->hit(BLOOD_SORC_HP_DRAINED, Dmg_type::pure);
        }

        if (map::player->is_alive())
        {
            spell->cast(map::player, true);
            prev_cast_ = spell_opt;

            if (IS_WARLOCK && rnd::one_in(2))
            {
                auto* const prop = new Prop_warlock_charged(Prop_turns::std);
                map::player->prop_handler().try_add_prop(prop);
            }
        }
    }
}

} //Player_spells_handling

void init()
{
    cleanup();
}

void cleanup()
{
    for (Spell* spell : known_spells_) {delete spell;}

    known_spells_.clear();
    prev_cast_ = Spell_opt();
}

void store_to_save_lines(std::vector<std::string>& lines)
{
    lines.push_back(to_str(known_spells_.size()));

    for (Spell* s : known_spells_)
    {
        lines.push_back(to_str(int(s->id())));
    }
}

void setup_from_save_lines(std::vector<std::string>& lines)
{
    const int NR_SPELLS = to_int(lines.front());

    lines.erase(begin(lines));

    for (int i = 0; i < NR_SPELLS; ++i)
    {
        const int ID = to_int(lines.front());
        lines.erase(begin(lines));
        known_spells_.push_back(spell_handling::mk_spell_from_id(Spell_id(ID)));
    }
}

void player_select_spell_to_cast()
{
    std::vector<Spell_opt> spell_opts;
    spells_avail(spell_opts);

    if (spell_opts.empty())
    {
        msg_log::add("I do not know any spells to invoke.");
    }
    else
    {
        auto spell_opt_sort = [](const Spell_opt & opt1, const Spell_opt & opt2)
        {
            return opt1.spell->name() < opt2.spell->name();
        };

        sort(spell_opts.begin(), spell_opts.end(), spell_opt_sort);

        Menu_browser browser(spell_opts.size());

        render::draw_map_and_interface();

        draw(browser, spell_opts);

        while (true)
        {
            const Menu_action action = menu_input::action(browser);

            switch (action)
            {
            case Menu_action::moved:
                draw(browser, spell_opts);
                break;

            case Menu_action::esc:
            case Menu_action::space:
                msg_log::clear();
                render::draw_map_and_interface();
                return;

            case Menu_action::selected:
                try_cast(spell_opts[browser.y()]);
                return;

            default: {}
                break;
            }
        }
    }
}

void try_cast_prev_spell()
{
    TRACE_FUNC_BEGIN;

    bool is_prev_spell_ok = false;

    if (prev_cast_.spell)
    {
        //Checking if previous spell is still available (it could for example have been
        //granted by an item that was dropped)
        std::vector<Spell_opt> spell_opts;
        spells_avail(spell_opts);

        auto spell_opt_cmp = [&](const Spell_opt & opt) {return opt.spell == prev_cast_.spell;};

        is_prev_spell_ok = find_if(begin(spell_opts), end(spell_opts), spell_opt_cmp) !=
                           end(spell_opts);
    }

    if (is_prev_spell_ok)
    {
        TRACE << "Previous spell is available, casting" << std::endl;
        try_cast(prev_cast_);
    }
    else
    {
        TRACE << "No previous spell set, player picks spell instead" << std::endl;
        player_select_spell_to_cast();
    }

    TRACE_FUNC_END;
}

bool is_spell_learned(const Spell_id id)
{
    for (auto* s : known_spells_) {if (s->id() == id) {return true;}}

    return false;
}

void learn_spell_if_not_known(const Spell_id id)
{
    learn_spell_if_not_known(spell_handling::mk_spell_from_id(id));
}

void learn_spell_if_not_known(Spell* const spell)
{
    bool is_already_learned = false;

    for (Spell* spell_cmpr : known_spells_)
    {
        if (spell_cmpr->id() == spell->id())
        {
            is_already_learned = true;
            break;
        }
    }

    if (is_already_learned)
    {
        delete spell;
    }
    else
    {
        known_spells_.push_back(spell);
    }
}

} //Player_spells_handling
