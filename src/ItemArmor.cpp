#include "ItemArmor.h"

#include "Engine.h"
#include "Log.h"

Armor::Armor(ItemData* const itemData, Engine* engine) :
  Item(itemData, engine), dur_(engine->dice.range(60, 100)) {}

string Armor::getArmorDataLine(const bool WITH_BRACKETS) const {

  const int AP = getAbsorptionPoints();

  if(AP <= 0) {return "";} //Should not happen

  const string absorptionPointsStr = intToString(AP);

  if(WITH_BRACKETS) {
    return "[" + absorptionPointsStr + "]";
  } else {
    return absorptionPointsStr;
  }
}

int Armor::takeDurabilityHitAndGetReducedDamage(const int DMG_BEFORE) {
  trace << "Armor::takeDurabilityHitAndGetReducedDamage()..." << endl;
  //Absorption points (AP) = damage soaked up instead of hitting the player
  //DDF = Damage (to) Durability Factor
  //(how much damage the durability takes per attack damage point)

  const int AP_BEFORE = getAbsorptionPoints();

  const double DDF_BASE     = data_->armorData.damageToDurabilityFactor;
  const double RND_FRACTION = double(eng->dice.percentile()) / 100.0;
  const double DDF_ADJUST   = 3.0;

  const double DMG_BEFORE_DB = double(DMG_BEFORE);
  dur_ -= int(DMG_BEFORE_DB * DDF_BASE * RND_FRACTION * DDF_ADJUST);

  dur_ = max(0, dur_);

  const int AP_AFTER = getAbsorptionPoints();

  if(AP_AFTER < AP_BEFORE && AP_AFTER != 0) {
    const string armorName =
      eng->itemDataHandler->getItemRef(*this, itemRef_plain, true);
    eng->log->addMsg("My " + armorName + " is damaged!");
  }

  trace << "Armor: Damage before: " + intToString(DMG_BEFORE) << endl;

  const int DMG_AFTER = max(1, DMG_BEFORE - AP_BEFORE);

  trace << "Armor: Damage after: " + intToString(DMG_AFTER) << endl;

  trace << "Armor::takeDurabilityHitAndGetReducedDamage() [DONE]" << endl;
  return DMG_AFTER;
}

int Armor::getAbsorptionPoints() const {
  const int AP_MAX = data_->armorData.absorptionPoints;

  if(dur_ > 60) {return AP_MAX;}
  if(dur_ > 40) {return max(0, AP_MAX - 1);}
  if(dur_ > 25) {return max(0, AP_MAX - 2);}
  if(dur_ > 15) {return max(0, AP_MAX - 3);}

  return 0;
}

void ArmorAsbestosSuit::onWear() {
  propsEnabledOnCarrier.push_back(new PropRFire(eng, propTurnsIndefinite));
  propsEnabledOnCarrier.push_back(new PropRAcid(eng, propTurnsIndefinite));
  propsEnabledOnCarrier.push_back(new PropRElectric(eng, propTurnsIndefinite));
}

void ArmorAsbestosSuit::onTakeOff() {
  clearPropsEnabledOnCarrier();
}

void ArmorHeavyCoat::onWear() {
  propsEnabledOnCarrier.push_back(new PropRCold(eng, propTurnsIndefinite));
}

void ArmorHeavyCoat::onTakeOff() {
  clearPropsEnabledOnCarrier();
}

