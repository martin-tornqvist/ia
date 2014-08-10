#include "Item.h"

#include "Map.h"
#include "GameTime.h"
#include "Utils.h"
#include "MapParsing.h"
#include "Properties.h"
#include "Log.h"

using namespace std;

//------------------------------------------------------------------- ITEM
string Item::getWeightLabel() const {
  const int WEIGHT = getWeight();
  if(WEIGHT <= (itemWeight_extraLight + itemWeight_light) / 2) {
    return "Very light";
  }
  if(WEIGHT <= (itemWeight_light + itemWeight_medium) / 2) {
    return "Light";
  }
  if(WEIGHT <= (itemWeight_medium + itemWeight_heavy) / 2) {
    return "Medium";
  }
  return "Heavy";
}

ConsumeItem Item::activateDefault(Actor* const actor) {
  (void)actor;
  Log::addMsg("I cannot apply that.");
  return ConsumeItem::no;
}

//------------------------------------------------------------------- HIDEOUS MASK
void HideousMask::newTurnInInventory() {
  vector<Actor*> adjActors;
  const Pos p(Map::player->pos);
  for(auto* const actor : GameTime::actors_) {
    if(
      actor->deadState == ActorDeadState::alive &&
      Utils::isPosAdj(p, actor->pos, false)) {
      adjActors.push_back(actor);
    }
  }
  if(!adjActors.empty()) {
    bool visionBlockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), visionBlockers);
    for(auto* const actor : adjActors) {
      if(Rnd::oneIn(4) && actor->isSeeingActor(*Map::player, visionBlockers)) {
        actor->getPropHandler().tryApplyProp(new PropTerrified(PropTurns::std));
      }
    }
  }
}

//------------------------------------------------------------------- GAS MASK
void GasMask::onTakeOff() {
  clearPropsEnabledOnCarrier();
}

void GasMask::onWear() {
  propsEnabledOnCarrier.push_back(new PropRBreath(PropTurns::indefinite));
}
