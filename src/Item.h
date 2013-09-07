#ifndef ITEM_H
#define ITEM_H

#include <iostream>

#include "ItemData.h"
#include "Art.h"

class Property;

enum ItemActivateReturn_t {
  itemActivate_keep, itemActivate_destroyed
};

class Item {
public:
  Item(ItemData* itemData, Engine* engine) :
    nrItems(1), data_(itemData), eng(engine) {}

  virtual ~Item() {}

  virtual void reset() {nrItems = 1;}

  const ItemData& getData() const       {return *data_;}
  virtual SDL_Color getColor() const    {return data_->color;}
  char getGlyph() const                 {return data_->glyph;}
  Tile_t getTile() const                {return data_->tile;}

  virtual void identify(const bool IS_SILENT_IDENTIFY) {
    (void)IS_SILENT_IDENTIFY;
  }

  virtual void itemSpecificAddSaveLines(vector<string>& lines) {
    (void)lines;
  }
  virtual void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
    (void)lines;
  }

  int getWeight() const {return data_->itemWeight * nrItems;}

  string getWeightLabel() const {
    const int WEIGHT = getWeight();
    if(WEIGHT <= (itemWeight_extraLight + itemWeight_light) / 2)
      return "Very light";
    if(WEIGHT <= (itemWeight_light + itemWeight_medium) / 2)
      return "Light";
    if(WEIGHT <= (itemWeight_medium + itemWeight_heavy) / 2)
      return "Medium";
    return "Heavy";
  }

  virtual bool activateDefault(Actor* const actor) {
    (void)actor;
    return false;
  }
  virtual string getDefaultActivationLabel() const  {return "";}
  virtual SDL_Color getInterfaceClr() const         {return clrBrown;}

  virtual void newTurnInInventory() {}

  int nrItems;

  virtual void onWear() {}
  virtual void onTakeOff() {}

  //Properties to apply e.g. when wearing a ring of fire resistance
  vector<Prop*> propsEnabledOnCarrier;

protected:
  void clearPropsEnabledOnCarrier() {
    for(unsigned int i = 0; i < propsEnabledOnCarrier.size(); i++) {
      delete propsEnabledOnCarrier.at(i);
    }
    propsEnabledOnCarrier.resize(0);
  }

  ItemData* data_;

  Engine* eng;

  //Called by the ItemDrop class to make noise etc
  friend class ItemDrop;
  virtual void appplyDropEffects() {}
};

#endif
