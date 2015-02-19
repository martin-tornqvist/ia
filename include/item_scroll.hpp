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

    Clr get_interface_clr() const override {return clr_magenta;}

    Consume_item activate(Actor* const actor) override;

    Consume_item read();

    const std::string get_real_name() const;

    std::vector<std::string> get_descr() const override;

    void identify(const bool IS_SILENT_IDENTIFY) override;

    Spell* mk_spell() const;

protected:
    void try_learn();

    std::string get_name_inf() const override;
};

namespace Scroll_handling
{

void init();

void store_to_save_lines(std::vector<std::string>& lines);
void setup_from_save_lines(std::vector<std::string>& lines);

} //Scroll_handling

#endif
