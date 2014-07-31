#include "ItemArmor.h"

#include <string>
#include <assert.h>

#include "Init.h"
#include "Log.h"
#include "Utils.h"
#include "PlayerBon.h"

using namespace std;

Armor::Armor(ItemDataT* const itemData) :
  Item(itemData), dur_(Rnd::range(80, 100)) {}

string Armor::getArmorDataLine(const bool WITH_BRACKETS) const {

  const int AP = getAbsorptionPoints();

  if(AP <= 0) {assert(false && "Armor AP less than 1"); return "";}

  const string absorptionPointsStr = toStr(AP);

  if(WITH_BRACKETS) {
    return "[" + absorptionPointsStr + "]";
  } else {
    return absorptionPointsStr;
  }
}

int Armor::takeDurHitAndGetReducedDmg(const int DMG_BEFORE) {
  TRACE_FUNC_BEGIN;

  //Absorption points (AP) = damage soaked up instead of hitting the player
  //DDF = Damage (to) Durability Factor
  //(how much damage the durability takes per attack damage point)

  const int AP_BEFORE = getAbsorptionPoints();

  const double DDF_BASE         = data_->armor.dmgToDurabilityFactor;
  //TODO Add check for if wearer is player
  const double DDF_WAR_VET_MOD  = PlayerBon::getBg() == Bg::warVet ?
                                  0.5 : 1.0;
  const double DDF_K            = 1.5;

  const double DMG_BEFORE_DB = double(DMG_BEFORE);

  dur_ -= int(DMG_BEFORE_DB * DDF_BASE * DDF_WAR_VET_MOD * DDF_K);

  dur_ = max(0, dur_);

  const int AP_AFTER = getAbsorptionPoints();

  if(AP_AFTER < AP_BEFORE && AP_AFTER != 0) {
    const string armorName =
      ItemData::getItemRef(*this, ItemRefType::plain, true);
    Log::addMsg("My " + armorName + " is damaged!", clrMsgWarning);
  }

  TRACE << "Armor: Damage before: " + toStr(DMG_BEFORE) << endl;

  const int DMG_AFTER = max(1, DMG_BEFORE - AP_BEFORE);

  TRACE << "Armor: Damage after: " + toStr(DMG_AFTER) << endl;

  TRACE_FUNC_END;
  return DMG_AFTER;
}

int Armor::getAbsorptionPoints() const {
  const int AP_MAX = data_->armor.absorptionPoints;

  if(dur_ > 60) {return AP_MAX;}
  if(dur_ > 40) {return max(0, AP_MAX - 1);}
  if(dur_ > 25) {return max(0, AP_MAX - 2);}
  if(dur_ > 15) {return max(0, AP_MAX - 3);}

  return 0;
}

void ArmorAsbSuit::onWear() {
  propsEnabledOnCarrier.push_back(new PropRFire(PropTurns::indefinite));
  propsEnabledOnCarrier.push_back(new PropRAcid(PropTurns::indefinite));
  propsEnabledOnCarrier.push_back(new PropRElec(PropTurns::indefinite));
}

void ArmorAsbSuit::onTakeOff() {
  clearPropsEnabledOnCarrier();
}

void ArmorHeavyCoat::onWear() {
  propsEnabledOnCarrier.push_back(new PropRCold(PropTurns::indefinite));
}

void ArmorHeavyCoat::onTakeOff() {
  clearPropsEnabledOnCarrier();
}

