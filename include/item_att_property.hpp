#ifndef ITEM_ATT_PROPERTY_HPP
#define ITEM_ATT_PROPERTY_HPP

class Prop;

struct ItemAttProp
{
    ItemAttProp() :
        prop(nullptr),
        pct_chance_to_apply(100) {}

    ItemAttProp(Prop* const prop) :
        prop(prop),
        pct_chance_to_apply(100) {}

    Prop* prop;

    int pct_chance_to_apply;
};

#endif // ITEM_ATT_PROPERTY_HPP
