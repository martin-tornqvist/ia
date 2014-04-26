#ifndef ITEM_AMMO_CLIP_H
#define ITEM_AMMO_CLIP_H

#include "Item.h"

#include "Converters.h"

class ItemAmmo: public Item {
public:
  ItemAmmo(ItemData* const itemData) :
    Item(itemData) {}
  virtual ~ItemAmmo() {}
  virtual SDL_Color getInterfaceClr() const {return clrWhite;}

private:
};

class ItemAmmoClip: public ItemAmmo {
public:
  ItemAmmoClip(ItemData* const itemData) :
    ItemAmmo(itemData) {
    setFullAmmo();
  }

  ~ItemAmmoClip() {}

  int ammo;

  void setFullAmmo() {ammo = data_->ammoContainedInClip;}

  void storeToSaveLines(vector<string>& lines) override {
    lines.push_back(toStr(ammo));
  }

  void setupFromSaveLines(vector<string>& lines) {
    ammo = toInt(lines.front());
    lines.erase(lines.begin());
  }

private:
};

#endif
