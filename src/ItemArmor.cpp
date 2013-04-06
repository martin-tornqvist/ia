#include "ItemArmor.h"

#include "Engine.h"
#include "Log.h"

Armor::Armor(ItemDefinition* const itemDefinition, Engine* engine) :
  Item(itemDefinition), eng(engine), durability(eng->dice.getInRange(60, 100)) {
}

string Armor::getArmorDataLine(const bool WITH_BRACKETS) const {
  const string apLabelOverRide = def_->armorData.overRideAbsorptionPointLabel;
  const int AP = getAbsorptionPoints(damageType_physical);

  if(apLabelOverRide == "" && AP <= 0) {
    return "";
  }

  const string absorptionPointsStr = apLabelOverRide == "" ? intToString(AP) : apLabelOverRide;
  if(WITH_BRACKETS) {
    return "[" + absorptionPointsStr + "]";
  } else {
    return absorptionPointsStr;
  }
}

int Armor::takeDurabilityHitAndGetReducedDamage(const int DAMAGE_BEFORE, const DamageTypes_t damageType) {
  tracer << "Armor::takeDurabilityHitAndGetReducedDamage()..." << endl;
  //Absorption points = damage soaked up by armor instead of hitting the player
  //DDF = Damage (to) Durability Factor, how much damage the durability takes per attack damage point

  const int DMG_TYPE_AP = getAbsorptionPoints(damageType);

  const int PHYSICAL_AP_BEFORE = getAbsorptionPoints(damageType_physical);

  const double DDF_INTRINSIC = def_->armorData.damageToDurabilityFactors[damageType];
  const double DDF_RANDOM = static_cast<float>(eng->dice.getInRange(1, 100)) / 100.0;
  const double DDF_ADJUST = 3.0;

  durability = max(0, durability - static_cast<int>(static_cast<double>(DAMAGE_BEFORE) * DDF_INTRINSIC * DDF_RANDOM * DDF_ADJUST));

  const int PHYSICAL_AP_AFTER = getAbsorptionPoints(damageType_physical);

  if(PHYSICAL_AP_AFTER < PHYSICAL_AP_BEFORE && PHYSICAL_AP_AFTER != 0) {
    const string armorName = eng->itemData->getItemRef(this, itemRef_plain, true);
    eng->log->addMessage("My " + armorName + " is damaged!");
  }

  tracer << "Armor: Damage before: " + intToString(DAMAGE_BEFORE) << endl;

  const int DAMAGE_REDUCTION = max(1, DAMAGE_BEFORE - DMG_TYPE_AP);

  tracer << "Armor: Damage reduction: " + intToString(DAMAGE_REDUCTION) << endl;

  tracer << "Armor::takeDurabilityHitAndGetReducedDamage() [DONE]" << endl;
  return DAMAGE_REDUCTION;
}

int Armor::getAbsorptionPoints(const DamageTypes_t damageType) const {
  const int AP_MAX = def_->armorData.absorptionPoints[damageType];

  if(durability > 60) {
    return AP_MAX;
  }

  if(durability > 40) {
    return max(0, AP_MAX - 1);
  }

  if(durability > 25) {
    return max(0, AP_MAX - 2);
  }

  if(durability > 15) {
    return max(0, AP_MAX - 3);
  }

  return 0;
}
