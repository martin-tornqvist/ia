#ifndef ITEM_ARMOR_H
#define ITEM_ARMOR_H

#include "Engine.h"

#include "Item.h"

class Armor: public Item {
public:
  Armor(ItemDefinition* const itemDefinition, Engine* engine);

  ~Armor() {}

  int getDurability() const {return dur_;}

  string getArmorDataLine(const bool WITH_BRACKETS) const;

  int takeDurabilityHitAndGetReducedDamage(const int DMG_BEFORE,
      const DmgTypes_t dmgType);

  void itemSpecificAddSaveLines(vector<string>& lines) {
    lines.push_back(intToString(dur_));
  }

  void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
    dur_ = stringToInt(lines.front());
    lines.erase(lines.begin());
  }

  bool isDestroyed() {
    return getAbsorptionPoints(dmgType_physical) <= 0;
  }

  SDL_Color getInterfaceClr() const {return clrGray;}

  void setMaxDurability() {dur_ = 100;}

private:
  int getAbsorptionPoints(const DmgTypes_t dmgType) const;

  Engine* eng;
  int dur_;
};

#endif
