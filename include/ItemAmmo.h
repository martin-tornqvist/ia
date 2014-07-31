#ifndef ITEM_AMMO_CLIP_H
#define ITEM_AMMO_CLIP_H

#include "Item.h"

#include "Converters.h"

class ItemAmmo: public Item {
public:
  ItemAmmo(ItemDataT* const itemData) :
    Item(itemData) {}
  virtual ~ItemAmmo() {}
  virtual Clr getInterfaceClr() const {return clrWhite;}

private:
};

class ItemAmmoClip: public ItemAmmo {
public:
  ItemAmmoClip(ItemDataT* const itemData) :
    ItemAmmo(itemData) {
    setFullAmmo();
  }

  ~ItemAmmoClip() {}

  int ammo;

  void setFullAmmo() {ammo = data_->ranged.ammoContainedInClip;}

  void storeToSaveLines(std::vector<std::string>& lines) override {
    lines.push_back(toStr(ammo));
  }

  void setupFromSaveLines(std::vector<std::string>& lines) {
    ammo = toInt(lines.front());
    lines.erase(begin(lines));
  }

private:
};

#endif
