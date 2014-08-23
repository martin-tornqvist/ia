#include "Init.h"

#include <algorithm>
#include <vector>

#include "FeatureMob.h"
#include "GameTime.h"
#include "PlayerBon.h"
#include "Explosion.h"
#include "Utils.h"
#include "Map.h"
#include "FeatureRigid.h"
#include "Fov.h"
#include "Inventory.h"
#include "Item.h"
#include "Log.h"

using namespace std;

//------------------------------------------------------------------- SMOKE
void Smoke::onNewTurn() {
  auto* actor = Utils::getFirstActorAtPos(pos_);

  if(actor) {

    const bool IS_PLAYER = actor == Map::player;

    //Blinded by smoke?
    if(Rnd::oneIn(3)) {
      //TODO There needs to be some criteria here, so that e.g. a statue-monster or a
      //very alien monster can't get blinded by smoke (but do not use isHumanoid - rats,
      //wolves etc should definitely be blinded by smoke.
      bool playerWearsGasMask = false;
      auto* playerHeadItem    = Map::player->getInv().getSlot(SlotId::head)->item;
      if(playerHeadItem) {
        playerWearsGasMask = playerHeadItem->getData().id == ItemId::gasMask;
      }

      if(!IS_PLAYER || !playerWearsGasMask) {
        if(IS_PLAYER) {Log::addMsg("I am getting smoke in my eyes.");}
        actor->getPropHandler().tryApplyProp(
          new PropBlind(PropTurns::specific, Rnd::range(1, 3)));
      }
    }

    //Player choking?
    if(Rnd::oneIn(5)) {
      vector<PropId> propIds;
      actor->getPropHandler().getAllActivePropIds(propIds);
      if(find(begin(propIds), end(propIds), propRBreath) == end(propIds)) {
        const string sndMsg =
          (IS_PLAYER || !actor->isHumanoid()) ? "" : "I hear choking.";
        if(IS_PLAYER) {Log::addMsg("I am choking!", clrMsgBad);}
        const auto alerts = IS_PLAYER ? AlertsMonsters::yes : AlertsMonsters::no;
        SndEmit::emitSnd(Snd(sndMsg, SfxId::END, IgnoreMsgIfOriginSeen::yes, actor->pos,
                             actor, SndVol::low, alerts));
        actor->hit(1, DmgType::pure);
      }
    }
  }

  //If not permanent, count down turns left and possibly erase self
  if(nrTurnsLeft_ > -1) {
    if(--nrTurnsLeft_ <= 0) {GameTime::eraseMob(this, true);}
  }
}

string Smoke::getName(const Article article)  const {
  string ret = "";
  if(article == Article::the) {ret += "the ";}
  return ret + "smoke";
}

Clr Smoke::getClr() const {
  return clrGray;
}

//------------------------------------------------------------------- DYNAMITE
void LitDynamite::onNewTurn() {
  nrTurnsLeft_--;
  if(nrTurnsLeft_ <= 0) {
    const int D = PlayerBon::hasTrait(Trait::demExpert) ? 1 : 0;
    Explosion::runExplosionAt(pos_, ExplType::expl, ExplSrc::misc, D);
    GameTime::eraseMob(this, true);
  }
}

string LitDynamite::getName(const Article article)  const {
  string ret = article == Article::a ? "a " : "the ";
  return ret + "lit stick of dynamite";
}

Clr LitDynamite::getClr() const {
  return clrRedLgt;
}

//------------------------------------------------------------------- FLARE
void LitFlare::onNewTurn() {
  nrTurnsLeft_--;
  if(nrTurnsLeft_ <= 0) {GameTime::eraseMob(this, true);}
}

void LitFlare::addLight(bool light[MAP_W][MAP_H]) const {
  bool myLight[MAP_W][MAP_H];
  Utils::resetArray(myLight, false);
  const int R = FOV_STD_RADI_INT; //getLightRadius();
  Pos p0(max(0,         pos_.x - R),  max(0,          pos_.y - R));
  Pos p1(min(MAP_W - 1, pos_.x + R),  min(MAP_H - 1,  pos_.y + R));
  bool visionBlockers[MAP_W][MAP_H];
  for(int y = p0.y; y <= p1.y; ++y) {
    for(int x = p0.x; x <= p1.x; ++x) {
      visionBlockers[x][y] = !Map::cells[x][y].rigid->isVisionPassable();
    }
  }

  Fov::runFovOnArray(visionBlockers, pos_, myLight, false);
  for(int y = p0.y; y <= p1.y; ++y) {
    for(int x = p0.x; x <= p1.x; ++x) {
      if(myLight[x][y]) {light[x][y] = true;}
    }
  }
}

string LitFlare::getName(const Article article)  const {
  string ret = article == Article::a ? "a " : "the ";
  return ret + "lit flare";
}

Clr LitFlare::getClr() const {
  return clrYellow;
}
