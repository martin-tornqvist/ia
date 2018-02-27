#include "player_spells.hpp"

#include <vector>
#include <algorithm>

#include "init.hpp"
#include "item_scroll.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "browser.hpp"
#include "io.hpp"
#include "inventory.hpp"
#include "item_factory.hpp"
#include "player_bon.hpp"
#include "query.hpp"
#include "map.hpp"
#include "saving.hpp"
#include "property_handler.hpp"

namespace player_spells
{

namespace
{

std::vector<Spell*> learned_spells_;

SpellSkill spell_skills_[(size_t)SpellId::END];

std::vector<SpellOpt> spells_avail()
{
    std::vector<SpellOpt> ret;

    for (Spell* const spell : learned_spells_)
    {
        ret.push_back(SpellOpt(spell,
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
                ret.push_back(SpellOpt(spell,
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
            ret.push_back(SpellOpt(spell,
                                   SpellSrc::item,
                                   item));
        }
    }

    return ret;
}

void try_cast(const SpellOpt& spell_opt)
{
    ASSERT(spell_opt.spell);

    const auto& props = map::player->properties();

    bool allow_cast = props.allow_cast_intr_spell_absolute(Verbosity::verbose);

    if (allow_cast &&
        (spell_opt.src == SpellSrc::learned))
    {
        allow_cast = allow_cast && props.allow_speak(Verbosity::verbose);
    }

    if (!allow_cast)
    {
        return;
    }

    msg_log::clear();

    Spell* const spell = spell_opt.spell;

    const SpellSkill skill = map::player->spell_skill(spell->id());

    const Range spi_cost_range = spell->spi_cost(skill, map::player);

    if (spi_cost_range.max >= map::player->spi())
    {
        msg_log::add("Cast spell and risk depleting your spirit? [y/n]",
                     colors::light_white());

        if (query::yes_or_no() == BinaryAnswer::no)
        {
            msg_log::clear();

            return;
        }

        msg_log::clear();
    }

    msg_log::add("I cast " + spell->name() + "!");

    if (map::player->is_alive())
    {
        spell->cast(map::player, skill, IsIntrinsic::yes);
    }
}

} // namespace

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
        spell_skills_[i] = (SpellSkill)0;
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
        saving::put_int((int)spell_skills_[i]);
    }
}

void load()
{
    const int nr_spells = saving::get_int();

    for (int i = 0; i < nr_spells; ++i)
    {
        const SpellId id = (SpellId)saving::get_int();

        learned_spells_.push_back(spell_factory::make_spell_from_id(id));
    }

    for (size_t i = 0; i < (size_t)SpellId::END; ++i)
    {
        spell_skills_[i] = (SpellSkill)saving::get_int();
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
    if (is_spell_learned(id))
    {
        // Spell already known
        return;
    }

    Spell* const spell = spell_factory::make_spell_from_id(id);

    const bool player_can_learn = spell->player_can_learn();

    ASSERT(player_can_learn);

    // Robustness for release mode
    if (!player_can_learn)
    {
        return;
    }

    if (verbosity == Verbosity::verbose)
    {
        msg_log::add("I can now cast this incantation from memory.");
    }

    learned_spells_.push_back(spell);

    // HACK: If this is Animate Wepaons, also learn Subdue Weapons
    if (id == SpellId::anim_wpns)
    {
        learn_spell(SpellId::subdue_wpns, Verbosity::silent);
    }
}

void incr_spell_skill(const SpellId id)
{
    auto& skill = spell_skills_[(size_t)id];

    if (skill == SpellSkill::basic)
    {
        skill = SpellSkill::expert;
    }
    else if (skill == SpellSkill::expert)
    {
        skill = SpellSkill::master;
    }
}

SpellSkill spell_skill(const SpellId id)
{
    ASSERT(id != SpellId::END);

    if (id == SpellId::END)
    {
        return SpellSkill::basic;
    }

    return spell_skills_[(size_t)id];
}

void set_spell_skill(const SpellId id, const SpellSkill val)
{
    ASSERT(id != SpellId::END);

    if (id == SpellId::END)
    {
        return;
    }

    spell_skills_[(size_t)id] = val;
}

} // player_spells

// -----------------------------------------------------------------------------
// BrowseSpell
// -----------------------------------------------------------------------------
void BrowseSpell::on_start()
{
    spell_opts_ = player_spells::spells_avail();

    if (spell_opts_.empty())
    {
        //
        // Exit screen
        //
        states::pop();

        msg_log::add("I do not know any spells.");
        return;
    }

    browser_.reset(spell_opts_.size());
}

void BrowseSpell::draw()
{
    const int nr_spells = spell_opts_.size();

    io::draw_text_center("Use which power?",
                         Panel::screen,
                         P(screen_w / 2, 0),
                         colors::title());

    P p(0, 1);

    std::string key_str = "a) ";

    for (int i = 0; i < nr_spells; ++i)
    {
        const int current_idx = i;

        const bool is_idx_marked = browser_.is_at_idx(current_idx);

        SpellOpt spell_opt = spell_opts_[i];

        Spell* const spell = spell_opt.spell;

        std::string name = spell->name();

        const int spi_label_x = 26;

        const int skill_label_x = spi_label_x + 11;

        p.x = 0;

        const Color color =
            is_idx_marked ?
            colors::menu_highlight() :
            colors::menu_dark();

        io::draw_text(key_str,
                      Panel::screen,
                      p,
                      color);

        ++key_str[0];

        p.x = key_str.size();

        io::draw_text(name,
                      Panel::screen,
                      p,
                      color);

        std::string fill_str = "";

        const size_t fill_size = spi_label_x - p.x - name.size();

        for (size_t ii = 0; ii < fill_size; ++ii)
        {
            fill_str.push_back('.');
        }

        const Color fill_color = colors::gray().fraction(3.0);

        io::draw_text(fill_str,
                      Panel::screen,
                      P(p.x + name.size(), p.y),
                      fill_color);

        p.x = spi_label_x;

        std::string str = "SP: ";

        io::draw_text(str,
                          Panel::screen,
                          p,
                          colors::dark_gray());

        p.x += str.size();

        const SpellId id = spell->id();

        const auto skill = player_spells::spell_skill(id);

        const Range spi_cost = spell->spi_cost(skill, map::player);

        const std::string lower_str = std::to_string(spi_cost.min);
        const std::string upper_str = std::to_string(spi_cost.max);

        str =
            (spi_cost.max == 1) ?
            "1" :
            (lower_str +  "-" + upper_str);

        io::draw_text(str,
                          Panel::screen,
                          p,
                          colors::white());

        // Draw skill level if learned
        if ((spell_opt.src == SpellSrc::learned) &&
            spell->can_be_improved_with_skill())
        {
            p.x = skill_label_x;

            str = "Skill: ";

            io::draw_text(str,
                          Panel::screen,
                          p,
                          colors::dark_gray());

            p.x += str.size();

            switch (skill)
            {
            case SpellSkill::basic:
                str = "I";
                break;

            case SpellSkill::expert:
                str = "II";
                break;

            case SpellSkill::master:
                str = "III";
                break;
            }

            io::draw_text(str,
                          Panel::screen,
                          p,
                          colors::white());
        }

        if (is_idx_marked)
        {
            const auto skill = map::player->spell_skill(id);

            const auto descr = spell->descr(skill, IsIntrinsic::yes);

            std::vector<ColoredString> lines;

            if (!descr.empty())
            {
                for (const auto& line : descr)
                {
                    lines.push_back(ColoredString(line, colors::light_white()));
                }
            }

            switch (spell_opt.src)
            {
            // If spell source is an item, add info about this
            case SpellSrc::item:
            {
                const std::string item_name =
                    spell_opt.src_item->name(ItemRefType::plain,
                                             ItemRefInf::none);

                lines.push_back(ColoredString("Spell granted by " + item_name + ".",
                                          colors::green()));
            }
            break;

            case SpellSrc::manuscript:
            case SpellSrc::learned:
                break;
            }

            if (!lines.empty())
            {
                io::draw_descr_box(lines);
            }
        }

        ++p.y;
    }
}

void BrowseSpell::update()
{
    auto input = io::get(false);

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling_and_letters);

    switch (action)
    {
    case MenuAction::selected:
    {
        const auto spell_opt = spell_opts_[browser_.y()];

        //
        // Exit screen
        //
        states::pop();

        player_spells::try_cast(spell_opt);

        return;
    }
    break;

    case MenuAction::esc:
    case MenuAction::space:
    {
        //
        // Exit screen
        //
        states::pop();
        return;
    }
    break;

    default:
        break;
    }
}
