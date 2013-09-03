#ifndef ITEM_ARMOR_H
#define ITEM_ARMOR_H

#include "Engine.h"

#include "Item.h"

class Armor: public Item {
public:
  Armor(ItemData* const itemData, Engine* engine);

  ~Armor() {}

  int getDurability() const {return dur_;}

  string getArmorDataLine(const bool WITH_BRACKETS) const;

  int takeDurabilityHitAndGetReducedDamage(const int DMG_BEFORE);

  void itemSpecificAddSaveLines(vector<string>& lines) {
    lines.push_back(intToString(dur_));
  }

  void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
    dur_ = stringToInt(lines.front());
    lines.erase(lines.begin());
  }

  inline bool isDestroyed() {return getAbsorptionPoints() <= 0;}

  SDL_Color getInterfaceClr() const {return clrGray;}

  void setMaxDurability() {dur_ = 100;}

  virtual void onWear() {}
  virtual void onTakeOff() {}

protected:
  int getAbsorptionPoints() const;

  int dur_;
};

class ArmorAsbestosSuit: public Armor {
public:
  ArmorAsbestosSuit(ItemData* const itemData, Engine* engine) :
    Armor(itemData, engine) {}
  ~ArmorAsbestosSuit() {}

  void onWear();
  void onTakeOff();
};

#endif
