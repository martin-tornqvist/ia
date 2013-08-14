#ifndef ITEM_AMMO_CLIP_H
#define ITEM_AMMO_CLIP_H

#include "Item.h"

#include "Converters.h"

class ItemAmmo: public Item {
public:
  ItemAmmo(ItemDef* const itemDefinition) : Item(itemDefinition) {
  }

  virtual ~ItemAmmo() {
  }

  virtual SDL_Color getInterfaceClr() const {return clrWhite;}

private:
};

class ItemAmmoClip: public ItemAmmo {
public:
  ItemAmmoClip(ItemDef* const itemDefinition) : ItemAmmo(itemDefinition) {
    setFullAmmo();
  }

  ~ItemAmmoClip() {}

  int ammo;

  void setFullAmmo() {
    ammo = def_->ammoContainedInClip;
  }

  void itemSpecificAddSaveLines(vector<string>& lines) {
    lines.push_back(intToString(def_->ammoContainedInClip));
  }

  void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
    def_->ammoContainedInClip = stringToInt(lines.front());
    lines.erase(lines.begin());
  }

private:
};

#endif
