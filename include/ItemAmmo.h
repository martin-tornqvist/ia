#ifndef ITEM_AMMO_CLIP_H
#define ITEM_AMMO_CLIP_H

#include "Item.h"

#include "Converters.h"

class ItemAmmo: public Item {
public:
  ItemAmmo(ItemData* const itemData, Engine& engine) :
    Item(itemData, engine) {}
  virtual ~ItemAmmo() {}
  virtual SDL_Color getInterfaceClr() const {return clrWhite;}

private:
};

class ItemAmmoClip: public ItemAmmo {
public:
  ItemAmmoClip(ItemData* const itemData, Engine& engine) :
    ItemAmmo(itemData, engine) {
    setFullAmmo();
  }

  ~ItemAmmoClip() {}

  int ammo;

  void setFullAmmo() {ammo = data_->ammoContainedInClip;}

  void addSaveLines_(vector<string>& lines) {
    lines.push_back(toString(data_->ammoContainedInClip));
  }

  void setParamsFromSaveLines_(vector<string>& lines) {
    data_->ammoContainedInClip = toInt(lines.front());
    lines.erase(lines.begin());
  }

private:
};

#endif
