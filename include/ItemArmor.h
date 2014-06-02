#ifndef ITEM_ARMOR_H
#define ITEM_ARMOR_H

#include "Item.h"

class Armor: public Item {
public:
  Armor(ItemDataT* const itemData);

  ~Armor() {}

  int getDurability() const {return dur_;}

  std::string getArmorDataLine(const bool WITH_BRACKETS) const;

  int takeDurHitAndGetReducedDmg(const int DMG_BEFORE);

  void storeToSaveLines(std::vector<std::string>& lines) override {
    lines.push_back(toStr(dur_));
  }

  void setupFromSaveLines(std::vector<std::string>& lines) override {
    dur_ = toInt(lines.front());
    lines.erase(lines.begin());
  }

  inline bool isDestroyed() {return getAbsorptionPoints() <= 0;}

  SDL_Color getInterfaceClr() const {return clrGray;}

  void setMaxDurability() {dur_ = 100;}

  virtual void onWear()     override {}
  virtual void onTakeOff()  override {}

protected:
  int getAbsorptionPoints() const;

  int dur_;
};

class ArmorAsbSuit: public Armor {
public:
  ArmorAsbSuit(ItemDataT* const itemData) :
    Armor(itemData) {}
  ~ArmorAsbSuit() {}

  void onWear()     override;
  void onTakeOff()  override;
};

class ArmorHeavyCoat: public Armor {
public:
  ArmorHeavyCoat(ItemDataT* const itemData) :
    Armor(itemData) {}
  ~ArmorHeavyCoat() {}

  void onWear()     override;
  void onTakeOff()  override;
};

#endif
