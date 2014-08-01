#ifndef ITEM_H
#define ITEM_H

#include <iostream>

#include "ItemData.h"
#include "Art.h"
#include "InventoryHandling.h"

enum ItemActivateReturnType {
  itemActivate_keep, itemActivate_destroyed
};

class Item {
public:
  Item(ItemDataT* itemData) : nrItems(1), data_(itemData) {}

  Item& operator=(Item& other) {(void)other; return *this;}

  virtual ~Item() {}

  virtual void reset() {nrItems = 1;}

  const ItemDataT&  getData()   const {return *data_;}
  virtual Clr       getClr()    const {return data_->clr;}
  char              getGlyph()  const {return data_->glyph;}
  TileId            getTile()   const {return data_->tile;}

  virtual void identify(const bool IS_SILENT_IDENTIFY) {(void)IS_SILENT_IDENTIFY;}

  virtual void storeToSaveLines(std::vector<std::string>& lines)    {(void)lines;}
  virtual void setupFromSaveLines(std::vector<std::string>& lines)  {(void)lines;}

  int getWeight() const {return data_->itemWeight * nrItems;}

  std::string getWeightLabel() const;

  virtual ConsumeItem activateDefault(Actor* const actor) {
    (void)actor;
    return ConsumeItem::no;
  }
  virtual std::string getDefaultActivationLabel() const {return "";}
  virtual Clr getInterfaceClr()                   const {return clrBrown;}

  virtual void newTurnInInventory() {}

  int nrItems;

  virtual void onWear() {}
  virtual void onTakeOff() {}

  //Properties to apply e.g. when wearing something like a ring of fire resistance
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

class Headwear: public Item {
public:
  Headwear(ItemDataT* itemData) : Item(itemData) {}

  Clr getInterfaceClr() const override {return clrBrown;}
};

class HideousMask: public Headwear {
public:
  HideousMask(ItemDataT* itemData) : Headwear(itemData) {}

  void  newTurnInInventory() override;
};

class GasMask: public Headwear {
public:
  GasMask(ItemDataT* itemData) : Headwear(itemData) {}

  void onWear()     override;
  void onTakeOff()  override;
};

#endif
