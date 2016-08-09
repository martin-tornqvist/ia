#include "player_spells.hpp"

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
#include "map.hpp"
#include "saving.hpp"

namespace player_spells
{

namespace
{

enum class SpellSrc
{
    learned,
    item
};

struct SpellOpt
{
    SpellOpt() :
        spell       (nullptr),
        src         ((SpellSrc)0),
        src_item    (nullptr) {}

    SpellOpt(Spell* spell, SpellSrc src, Item* src_item) :
        spell       (spell),
        src         (src),
        src_item    (src_item) {}

    Spell* spell;
    SpellSrc src;
    Item* src_item;
};

std::vector<Spell*> learned_spells_;

//Spell skill levels
int spell_skill_pct_[(size_t)SpellId::END];

void draw(MenuBrowser& browser,
          const std::vector<SpellOpt>& spell_opts)
{
    const int nr_spells = spell_opts.size();

    render::clear_screen();

    render::draw_text_center("Invoke which power?",
                             Panel::screen,
                             P(screen_w / 2, 0),
                             clr_title);

    P p(0, 1);

    std::string key_str = "a) ";

    for (int i = 0; i < nr_spells; ++i)
    {
        const int       current_idx     = i;
        const bool      is_idx_marked   = browser.is_at_idx(current_idx);
        SpellOpt        spell_opt       = spell_opts[i];
        Spell* const    spell           = spell_opt.spell;
        std::string     name            = spell->name();
        const int       spi_label_x     = 26;
        const int       skill_label_x   = spi_label_x + 11;

        p.x = 0;

        const Clr clr = is_idx_marked ? clr_menu_highlight : clr_menu_drk;

        render::draw_text(key_str, Panel::screen, p, clr);

        ++key_str[0];

        p.x = key_str.size();

        render::draw_text(name, Panel::screen, p, clr);

        std::string fill_str = "";

        const size_t fill_size = spi_label_x - p.x - name.size();

        for (size_t ii = 0; ii < fill_size; ++ii)
        {
            fill_str.push_back('.');
        }

        Clr fill_clr = clr_gray;

        fill_clr.r /= 3;
        fill_clr.g /= 3;
        fill_clr.b /= 3;

        render::draw_text(fill_str,
                          Panel::screen,
                          P(p.x + name.size(), p.y),
                          fill_clr);

        p.x = spi_label_x;

        std::string str = "SP: ";

        render::draw_text(str,
                          Panel::screen,
                          p,
                          clr_gray_drk);

        p.x += str.size();

        //Casting from items always casts at base cost
        const bool is_base_cost_only = spell_opt.src == SpellSrc::item;

        const Range spi_cost = spell->spi_cost(is_base_cost_only, map::player);

        const std::string lower_str = to_str(spi_cost.min);
        const std::string upper_str = to_str(spi_cost.max);

        str = spi_cost.max == 1 ? "1" : (lower_str +  "-" + upper_str);

        render::draw_text(str,
                          Panel::screen,
                          p,
                          clr_white);

        const SpellId   id          = spell->id();
        const int       skill_pct   = spell_skill_pct_[(size_t)id];

        //Draw skill level if learned
        if (spell_opt.src == SpellSrc::learned)
        {
            p.x = skill_label_x;

            str = "Skill: ";

            render::draw_text(str,
                              Panel::screen,
                              p,
                              clr_gray_drk);

            str = to_str(skill_pct) + "%";

            p.x = descr_x0 - 1 - str.size();

            render::draw_text(str,
                              Panel::screen,
                              p,
                              clr_white);
        }

        if (is_idx_marked)
        {
            const auto descr = spell->descr();
            std::vector<StrAndClr> lines;

            if (!descr.empty())
            {
                for (const auto& line : descr)
                {
                    lines.push_back(StrAndClr(line, clr_white_high));
                }
            }

            switch (spell_opt.src)
            {
            //If spell source is "learned", add info about spell skills
            case SpellSrc::learned:
            {
                lines.push_back(StrAndClr("Higher skill levels reduces the number of Spirit Points "
                                          "required to cast the spell.",
                                          clr_white_high));

                if (skill_pct < 100)
                {
                    lines.push_back(StrAndClr("Skill level can be increased by casting from "
                                              "Manuscripts.",
                                              clr_white_high));
                }
            }
            break;

            //If spell source is an item, add info about this
            case SpellSrc::item:
            {
                const std::string item_name =
                    spell_opt.src_item->name(ItemRefType::plain, ItemRefInf::none);

                lines.push_back(StrAndClr("Spell granted by " + item_name + ".",
                                          clr_green));
            }
            break;
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

void spells_avail(std::vector<SpellOpt>& out)
{
    out.clear();

    for (Spell* const spell : learned_spells_)
    {
        out.push_back(SpellOpt(spell,
                               SpellSrc::learned,
                               nullptr));
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
                out.push_back(SpellOpt(spell,
                                       SpellSrc::item,
                                       item));
            }
        }
    }

    for (Item* item : inv.backpack_)
    {
        const std::vector<Spell*>& carrier_spells = item->carrier_spells();

        for (Spell* spell : carrier_spells)
        {
            out.push_back(SpellOpt(spell,
                                   SpellSrc::item,
                                   item));
        }
    }
}

void try_cast(const SpellOpt& spell_opt)
{
    ASSERT(spell_opt.spell);

    const auto& props = map::player->prop_handler();

    bool allow_cast = props.allow_cast_spell(Verbosity::verbose);

    if (allow_cast && spell_opt.src == SpellSrc::learned)
    {
        allow_cast = allow_cast && props.allow_speak(Verbosity::verbose);
    }

    if (allow_cast)
    {
        msg_log::clear();
        render::draw_map_state();

        Spell* const spell = spell_opt.spell;

        //Casting from items always casts at base cost
        const bool is_base_cost_only = spell_opt.src == SpellSrc::item;

        const Range spi_cost_range = spell->spi_cost(is_base_cost_only, map::player);

        if (spi_cost_range.max >= map::player->spi())
        {
            msg_log::add("Cast spell and risk depleting your spirit [y/n]?",
                         clr_white_high);

            render::draw_map_state();

            if (query::yes_or_no() == YesNoAnswer::no)
            {
                msg_log::clear();
                render::draw_map_state();
                return;
            }

            msg_log::clear();
        }

        msg_log::add("I cast " + spell->name() + "!");

        const bool is_warlock = player_bon::traits[(size_t)Trait::warlock];

        if (map::player->is_alive())
        {
            spell->cast(map::player, true, is_base_cost_only);

            if (is_warlock && rnd::one_in(2))
            {
                auto* const prop = new PropWarlockCharged(PropTurns::std);

                map::player->prop_handler().try_add(prop);
            }
        }
    }
}

} //namespace

void init()
{
    cleanup();
}

void cleanup()
{
    for (Spell* spell : learned_spells_)
    {
        delete spell;
    }

    learned_spells_.clear();

    for (size_t i = 0; i < (size_t)SpellId::END; ++i)
    {
        spell_skill_pct_[i] = 0;
    }
}

void save()
{
    saving::put_int(learned_spells_.size());

    for (Spell* s : learned_spells_)
    {
        saving::put_int((int)s->id());
    }

    for (size_t i = 0; i < (size_t)SpellId::END; ++i)
    {
        saving::put_int(spell_skill_pct_[i]);
    }
}

void load()
{
    const int nr_spells = saving::get_int();

    for (int i = 0; i < nr_spells; ++i)
    {
        const SpellId id = (SpellId)saving::get_int();

        learned_spells_.push_back(spell_handling::mk_spell_from_id(id));
    }

    for (size_t i = 0; i < (size_t)SpellId::END; ++i)
    {
        spell_skill_pct_[i] = saving::get_int();
    }
}

void player_select_spell_to_cast()
{
    std::vector<SpellOpt> spell_opts;
    spells_avail(spell_opts);

    if (spell_opts.empty())
    {
        msg_log::add("I do not know any spells to invoke.");
    }
    else //Has spells
    {
        MenuBrowser browser(spell_opts.size());

        render::draw_map_state();

        draw(browser, spell_opts);

        while (true)
        {
            const MenuAction action = menu_input::action(browser);

            switch (action)
            {
            case MenuAction::moved:
                draw(browser, spell_opts);
                break;

            case MenuAction::esc:
            case MenuAction::space:
                msg_log::clear();
                render::draw_map_state();
                return;

            case MenuAction::selected:
                try_cast(spell_opts[browser.y()]);
                return;

            default:
                break;
            }
        }
    }
}

bool is_spell_learned(const SpellId id)
{
    for (auto* s : learned_spells_)
    {
        if (s->id() == id)
        {
            return true;
        }
    }

    return false;
}

void learn_spell(const SpellId id, const Verbosity verbosity)
{
    Spell* const spell = spell_handling::mk_spell_from_id(id);

    const bool player_can_learn = spell->player_can_learn();

    ASSERT(player_can_learn);

    //Robustness for release mode
    if (!player_can_learn)
    {
        return;
    }

    const bool is_learned_before = is_spell_learned(id);

    ASSERT(!is_learned_before);

    //Robustness for release mode
    if (is_learned_before)
    {
        delete spell;

        return;
    }

    if (verbosity == Verbosity::verbose)
    {
        msg_log::add("I can now cast this incantation from memory.");
    }

    learned_spells_.push_back(spell);
}

void incr_spell_skill(const SpellId id, const Verbosity verbosity)
{
    DiceParam incr_dice;

    if (player_bon::bg() == Bg::occultist)
    {
        incr_dice = DiceParam(4, 6);
    }
    else //Not Occultist
    {
        incr_dice = DiceParam(1, 6);
    }

    const int incr = incr_dice.roll();

    int& v = spell_skill_pct_[(size_t)id];

    const int skill_before = v;

    v = std::min(100, v + incr);

    if (
        v > skill_before &&
        verbosity == Verbosity::verbose)
    {
        msg_log::add("I am now more proficient at casting this spell (" + to_str(v) + "%).");
    }
}

int spell_skill_pct(const SpellId id)
{
    return spell_skill_pct_[(size_t)id];
}

void set_spell_skill_pct(const SpellId id, const int val)
{
    spell_skill_pct_[(size_t)id] = val;
}

} //player_spells
