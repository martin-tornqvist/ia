#ifndef ITEM_SCROLL_HPP
#define ITEM_SCROLL_HPP

#include "item.hpp"

class Actor;
class Spell;

class Scroll: public Item
{
public:
    Scroll(ItemDataT* const item_data) :
        Item(item_data) {}

    ~Scroll() {}

    Color interface_color() const override
    {
        return colors::magenta();
    }

    ConsumeItem activate(Actor* const actor) override;

    const std::string real_name() const;

    std::vector<std::string> descr() const override;

    void identify(const Verbosity verbosity) override;

    Spell* mk_spell() const;

protected:
    std::string name_inf() const override;
};

namespace scroll_handling
{

void init();

void save();
void load();

} // scroll_handling

#endif
