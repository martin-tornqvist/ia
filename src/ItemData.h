#ifndef ITEM_DATA_H
#define ITEM_DATA_H

#include "SDL/SDL.h"
#include <vector>
#include <string>
#include "ConstTypes.h"

#include "AbilityValues.h"

#include "ItemDefinition.h"

using namespace std;

class Engine;
class Item;
struct ActorDefinition;

class ItemData {
public:
    ItemData(Engine* engine) :
        eng(engine) {
        makeList();
    }
    ~ItemData() {
        for(unsigned int i = 1; i < endOfItemDevNames; i++)
            delete itemDefinitions[i];
    }

    ItemDefinition* itemDefinitions[endOfItemDevNames];

    string itemInterfaceName(Item* const item, const bool PUT_A_OR_AN_IN_FRONT) const;

    bool isWeaponStronger(const ItemDefinition& oldDef, const ItemDefinition& newDef, bool melee);

    void addSaveLines(vector<string>& lines) const;
    void setParametersFromSaveLines(vector<string>& lines);

private:
    void makeList();

    void setDmgFromFormula(ItemDefinition& d, const ActorDefinition& owningActor, const EntityStrength_t dmgStrength) const;

    void resetDef(ItemDefinition* const d, ItemDefArchetypes_t const archetype) const;

    Engine* eng;
};

#endif
