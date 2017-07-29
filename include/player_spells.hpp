#ifndef PLAYER_SPELLS_HPP
#define PLAYER_SPELLS_HPP

#include <vector>

#include "spells.hpp"
#include "state.hpp"
#include "browser.hpp"

class Spell;

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

namespace player_spells
{

void init();
void cleanup();

void save();
void load();

void learn_spell(const SpellId id, const Verbosity verbosity);

void incr_spell_skill(const SpellId id);

SpellSkill spell_skill(const SpellId id);

void set_spell_skill(const SpellId id, const SpellSkill val);

bool is_spell_learned(const SpellId id);

} // player_spells

class BrowseSpell: public State
{
public:
    BrowseSpell() :
        State       (),
        browser_    (),
        spell_opts_ () {}

    void on_start() override;

    void draw() override;

    void update() override;

    StateId id() override;

private:
    MenuBrowser browser_;

    std::vector<SpellOpt> spell_opts_;
};

#endif // PLAYER_SPELLS_HPP
