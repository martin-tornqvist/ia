#include "ItemArmor.h"

#include "Engine.h"
#include "Log.h"

Armor::Armor(ItemDefinition* const itemDefinition, Engine* engine) :
  Item(itemDefinition), eng(engine), dur_(eng->dice.getInRange(60, 100)) {
}

string Armor::getArmorDataLine(const bool WITH_BRACKETS) const {
  const string apLabelOverRide = def_->armorData.overRideAbsorptionPointLabel;
  const int AP = getAbsorptionPoints(dmgType_physical);

  if(apLabelOverRide == "" && AP <= 0) {
    return "";
  }

  const string absorptionPointsStr =
    apLabelOverRide == "" ? intToString(AP) : apLabelOverRide;

  if(WITH_BRACKETS) {
    return "[" + absorptionPointsStr + "]";
  } else {
    return absorptionPointsStr;
  }
}

int Armor::takeDurabilityHitAndGetReducedDamage(const int DMG_BEFORE,
    const DmgTypes_t dmgType) {
  tracer << "Armor::takeDurabilityHitAndGetReducedDamage()..." << endl;
  //Absorption points (AP) = damage soaked up instead of hitting the player
  //DDF = Damage (to) Durability Factor
  //(how much damage the durability takes per attack damage point)

  const int DMG_TYPE_AP = getAbsorptionPoints(dmgType);

  const int PHYSICAL_AP_BEFORE = getAbsorptionPoints(dmgType_physical);

  const double DDF_INTRINSIC = def_->armorData.damageToDurabilityFactors[dmgType];
  const double DDF_RANDOM = double(eng->dice.percentile()) / 100.0;
  const double DDF_ADJUST = 3.0;

  const double DMG_BEFORE_DB = double(DMG_BEFORE);
  dur_ -= int(
            DMG_BEFORE_DB * DDF_INTRINSIC * DDF_RANDOM * DDF_ADJUST);

  dur_ = max(0, dur_);

  const int PHYSICAL_AP_AFTER = getAbsorptionPoints(dmgType_physical);

  if(PHYSICAL_AP_AFTER < PHYSICAL_AP_BEFORE && PHYSICAL_AP_AFTER != 0) {
    const string armorName = eng->itemData->getItemRef(*this, itemRef_plain, true);
    eng->log->addMessage("My " + armorName + " is damaged!");
  }

  tracer << "Armor: Damage before: " + intToString(DMG_BEFORE) << endl;

  const int DAMAGE_REDUCTION = max(1, DMG_BEFORE - DMG_TYPE_AP);

  tracer << "Armor: Damage reduction: " + intToString(DAMAGE_REDUCTION) << endl;

  tracer << "Armor::takeDurabilityHitAndGetReducedDamage() [DONE]" << endl;
  return DAMAGE_REDUCTION;
}

int Armor::getAbsorptionPoints(const DmgTypes_t dmgType) const {
  const int AP_MAX = def_->armorData.absorptionPoints[dmgType];

  if(dur_ > 60) {return AP_MAX;}
  if(dur_ > 40) {return max(0, AP_MAX - 1);}
  if(dur_ > 25) {return max(0, AP_MAX - 2);}
  if(dur_ > 15) {return max(0, AP_MAX - 3);}

  return 0;
}
