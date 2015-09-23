#ifndef ITEM_SCROLL_H
#define ITEM_SCROLL_H

#include "item.hpp"

class Actor;
class Spell;

class Scroll: public Item
{
public:
    Scroll(Item_data_t* const item_data) :
        Item(item_data) {}

    ~Scroll() {}

    Clr interface_clr() const override
    {
        return clr_magenta;
    }

    Consume_item activate(Actor* const actor) override;

    const std::string real_name() const;

    std::vector<std::string> descr() const override;

    void identify(const Verbosity verbosity) override;

    Spell* mk_spell() const;

protected:
    void try_learn();

    std::string name_inf() const override;
};

namespace scroll_handling
{

void init();

void save();
void load();

} //Scroll_handling

#endif
