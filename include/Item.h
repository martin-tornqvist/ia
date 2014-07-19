#ifndef ITEM_H
#define ITEM_H

#include <iostream>

#include "ItemData.h"
#include "Art.h"
#include "InventoryHandling.h"

class Property;

enum ItemActivateReturnType {
  itemActivate_keep, itemActivate_destroyed
};

class Item {
public:
  Item(ItemDataT* itemData) :
    nrItems(1), data_(itemData) {}

  Item& operator=(Item& other) {(void)other; return *this;}

  virtual ~Item() {}

  virtual void reset() {nrItems = 1;}

  const ItemDataT& getData()  const {return *data_;}
  virtual Clr getClr()  const {return data_->clr;}
  char getGlyph()             const {return data_->glyph;}
  TileId getTile()            const {return data_->tile;}

  virtual void identify(const bool IS_SILENT_IDENTIFY) {
    (void)IS_SILENT_IDENTIFY;
  }

  virtual void storeToSaveLines(std::vector<std::string>& lines)            {(void)lines;}
  virtual void setupFromSaveLines(std::vector<std::string>& lines)  {(void)lines;}

  int getWeight() const {return data_->itemWeight * nrItems;}

  std::string getWeightLabel() const {
    const int WEIGHT = getWeight();
    if(WEIGHT <= (itemWeight_extraLight + itemWeight_light) / 2)
      return "Very light";
    if(WEIGHT <= (itemWeight_light + itemWeight_medium) / 2)
      return "Light";
    if(WEIGHT <= (itemWeight_medium + itemWeight_heavy) / 2)
      return "Medium";
    return "Heavy";
  }

  virtual ConsumeItem activateDefault(Actor* const actor) {
    (void)actor;
    return ConsumeItem::no;
  }
  virtual std::string getDefaultActivationLabel() const  {return "";}
  virtual Clr getInterfaceClr() const         {return clrBrown;}

  virtual void newTurnInInventory() {}

  int nrItems;

  virtual void onWear() {}
  virtual void onTakeOff() {}

  //Properties to apply e.g. when wearing a ring of fire resistance
  std::vector<Prop*> propsEnabledOnCarrier;

  //Called by the ItemDrop class to make noise etc
  virtual void appplyDropEffects() {}

protected:
  void clearPropsEnabledOnCarrier() {
    for(Prop* prop : propsEnabledOnCarrier) {delete prop;}
    propsEnabledOnCarrier.resize(0);
  }

  ItemDataT* data_;
};

#endif
