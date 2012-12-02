#include "ItemArmor.h"

#include "Engine.h"
#include "Log.h"

Armor::Armor(ItemDefinition* const itemDefinition, Engine* engine) :
  Item(itemDefinition), eng(engine), durability(eng->dice.getInRange(60, 100)) {
}

string Armor::getArmorDataLine() const {
  const string apLabelOverRide = def_->armorData.overRideAbsorptionPointLabel;
  const int ABS_POINTS = getAbsorptionPoints(damageType_physical);

  if(apLabelOverRide == "" && ABS_POINTS <= 0) {
    return "";
  }

  const string absorptionPoints = apLabelOverRide == "" ? intToString(ABS_POINTS) : apLabelOverRide;
  return "[" + absorptionPoints + "]";
}

int Armor::takeDurabilityHitAndGetReducedDamage(const int DAMAGE_BEFORE, const DamageTypes_t damageType) {
  tracer << "Armor::takeDurabilityHitAndGetReducedDamage()..." << endl;
  //Absorption points = damage soaked up by armor instead of hitting the player
  //DDF = Damage (to) Durability Factor, how much damage the durability takes per attack damage point

  const int ABSORPTION_POINTS_BEFORE = getAbsorptionPoints(damageType);

  const double DDF_INTRINSIC = def_->armorData.damageToDurabilityFactors[damageType];
  const double DDF_RANDOM = static_cast<float>(eng->dice(1, 100)) / 100.0;
  const double DDF_ADJUST = 5.0;

  durability = max(0, durability - static_cast<int>(static_cast<double>(DAMAGE_BEFORE) * DDF_INTRINSIC * DDF_RANDOM * DDF_ADJUST));

  const int ABSORPTION_POINTS_CURRENT = getAbsorptionPoints(damageType_physical);
  if(ABSORPTION_POINTS_CURRENT < ABSORPTION_POINTS_BEFORE && ABSORPTION_POINTS_CURRENT != 0) {
    const string armorName = eng->itemData->getItemRef(this, itemRef_plain, true);
    eng->log->addMessage("My " + armorName + " is damaged!");
  }

  tracer << "Armor: Damage before: " + intToString(DAMAGE_BEFORE) << endl;

  const int DAMAGE_REDUCTION = max(1, DAMAGE_BEFORE - ABSORPTION_POINTS_BEFORE);

  tracer << "Armor: Damage reduction: " + intToString(DAMAGE_REDUCTION) << endl;

  tracer << "Armor::takeDurabilityHitAndGetReducedDamage() [DONE]" << endl;
  return DAMAGE_REDUCTION;
}

int Armor::getAbsorptionPoints(const DamageTypes_t damageType) const {
  const int ABSORPTION_POINTS_MAX = def_->armorData.absorptionPoints[damageType];

  if(durability > 60) {
    return ABSORPTION_POINTS_MAX;
  }

  if(durability > 40) {
    return max(0, ABSORPTION_POINTS_MAX - 1);
  }

  if(durability > 25) {
    return max(0, ABSORPTION_POINTS_MAX - 2);
  }

  if(durability > 15) {
    return max(0, ABSORPTION_POINTS_MAX - 3);
  }

  return 0;
}
