#ifndef ITEM_DATA_H
#define ITEM_DATA_H

#include <vector>
#include <string>
#include "CommonData.h"

#include "AbilityValues.h"

#include "ItemDefinition.h"

using namespace std;

class Engine;
class Item;
struct ActorDefinition;

enum ItemRef_t {
  itemRef_plain,
  itemRef_a,
  itemRef_plural
};

class ItemData {
public:
  ItemData(Engine* engine) :
    eng(engine) {
    makeList();
  }
  ~ItemData() {
    for(unsigned int i = 1; i < endOfItemIds; i++)
      delete itemDefinitions[i];
  }

  string getItemRef(const Item& item, const ItemRef_t itemRefForm,
                    const bool SKIP_EXTRA_INFO = false) const;

  string getItemInterfaceRef(
    const Item& item, const bool ADD_A,
    const PrimaryAttackMode_t attackMode = primaryAttackMode_none) const;

  ItemDefinition* itemDefinitions[endOfItemIds];

  bool isWeaponStronger(const ItemDefinition& oldDef, const ItemDefinition& newDef, bool melee);
  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(vector<string>& lines);

private:
  void makeList();
  void setDmgFromFormula(ItemDefinition& d, const ActorDefinition& owningActor, const EntityStrength_t dmgStrength) const;
  void resetDef(ItemDefinition* const d, ItemDefArchetypes_t const archetype) const;

  void addFeatureFoundIn(ItemDefinition* const itemDef, const Feature_t featureId, const int CHANCE_TO_INCLUDE = 100) const;

  Engine* eng;
};

#endif
