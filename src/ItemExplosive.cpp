#include "ItemExplosive.h"

#include <vector>

#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "GameTime.h"
#include "PlayerBon.h"
#include "Utils.h"
#include "Map.h"
#include "Input.h"
#include "Marker.h"
#include "Look.h"
#include "Throwing.h"
#include "ItemFactory.h"
#include "Explosion.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"

using namespace std;

//------------------------------------------------------------------- EXPLOSIVE
ConsumeItem Explosive::activateDefault(Actor* const actor) {
  (void)actor;
  //Make a copy to use as the held ignited explosive.
  auto* cpy = static_cast<Explosive*>(ItemFactory::mk(getData().id, 1));

  cpy->fuseTurns_               = getStdFuseTurns();
  Map::player->activeExplosive  = cpy;
  Map::player->updateClr();
  cpy->onPlayerIgnite();
  return ConsumeItem::yes;
}

//------------------------------------------------------------------- DYNAMITE
void Dynamite::onPlayerIgnite() const {
  const bool IS_SWIFT   = PlayerBon::hasTrait(Trait::demExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Log::addMsg("I " + swiftStr + "light a dynamite stick.");
  Renderer::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void Dynamite::onStdTurnPlayerHoldIgnited() {
  fuseTurns_--;
  if(fuseTurns_ > 0) {
    string fuseMsg = "***F";
    for(int i = 0; i < fuseTurns_; ++i) {fuseMsg += "Z";}
    fuseMsg += "***";
    Log::addMsg(fuseMsg, clrYellow);
  } else {
    Log::addMsg("The dynamite explodes in my hands!");
    Map::player->activeExplosive = nullptr;
    Explosion::runExplosionAt(Map::player->pos, ExplType::expl);
    Map::player->updateClr();
    fuseTurns_ = -1;
    delete this;
  }
}

void Dynamite::onThrownIgnitedLanding(const Pos& p) {
  GameTime::addMob(new LitDynamite(p, fuseTurns_));
}

void Dynamite::onPlayerParalyzed() {
  Log::addMsg("The lit Dynamite stick falls from my hands!");
  Map::player->activeExplosive = nullptr;
  Map::player->updateClr();
  const Pos& p = Map::player->pos;
  auto* const f = Map::cells[p.x][p.y].rigid;
  if(!f->isBottomless()) {GameTime::addMob(new LitDynamite(p, fuseTurns_));}
  delete this;
}

//------------------------------------------------------------------- MOLOTOV
void Molotov::onPlayerIgnite() const {
  const bool IS_SWIFT   = PlayerBon::hasTrait(Trait::demExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Log::addMsg("I " + swiftStr + "light a Molotov Cocktail.");
  Renderer::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void Molotov::onStdTurnPlayerHoldIgnited() {
  fuseTurns_--;

  if(fuseTurns_ <= 0) {
    Log::addMsg("The Molotov Cocktail explodes in my hands!");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    Explosion::runExplosionAt(Map::player->pos, ExplType::applyProp, ExplSrc::misc, 0,
                              SfxId::explosionMolotov, new PropBurning(PropTurns::std));
    delete this;
  }
}

void Molotov::onThrownIgnitedLanding(const Pos& p) {
  const int D = PlayerBon::hasTrait(Trait::demExpert) ? 1 : 0;
  Explosion::runExplosionAt(p, ExplType::applyProp, ExplSrc::playerUseMoltvIntended, D,
                            SfxId::explosionMolotov, new PropBurning(PropTurns::std));
}


void Molotov::onPlayerParalyzed() {
  Log::addMsg("The lit Molotov Cocktail falls from my hands!");
  Map::player->activeExplosive = nullptr;
  Map::player->updateClr();
  Explosion::runExplosionAt(Map::player->pos, ExplType::applyProp, ExplSrc::misc, 0,
                            SfxId::explosionMolotov, new PropBurning(PropTurns::std));
  delete this;
}

//------------------------------------------------------------------- FLARE
void Flare::onPlayerIgnite() const {
  const bool IS_SWIFT   = PlayerBon::hasTrait(Trait::demExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Log::addMsg("I " + swiftStr + "light a Flare.");
  GameTime::updateLightMap();
  Map::player->updateFov();
  Renderer::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void Flare::onStdTurnPlayerHoldIgnited() {
  fuseTurns_--;
  if(fuseTurns_ <= 0) {
    Log::addMsg("The flare is extinguished.");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    delete this;
  }
}

void Flare::onThrownIgnitedLanding(const Pos& p) {
  GameTime::addMob(new LitFlare(p, fuseTurns_));
  GameTime::updateLightMap();
  Map::player->updateFov();
  Renderer::drawMapAndInterface();
}

void Flare::onPlayerParalyzed() {
  Log::addMsg("The lit Flare falls from my hands.");
  Map::player->activeExplosive = nullptr;
  Map::player->updateClr();
  const Pos&  p = Map::player->pos;
  auto* const f = Map::cells[p.x][p.y].rigid;
  if(!f->isBottomless()) {GameTime::addMob(new LitFlare(p, fuseTurns_));}
  GameTime::updateLightMap();
  Map::player->updateFov();
  Renderer::drawMapAndInterface();
  delete this;
}

//------------------------------------------------------------------- SMOKE GRENADE
void SmokeGrenade::onPlayerIgnite() const {
  const bool IS_SWIFT   = PlayerBon::hasTrait(Trait::demExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Log::addMsg("I " + swiftStr + "ignite a smoke grenade.");
  Renderer::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void SmokeGrenade::onStdTurnPlayerHoldIgnited() {
  if(fuseTurns_ < getStdFuseTurns()) {
    Explosion::runSmokeExplosionAt(Map::player->pos);
  }
  fuseTurns_--;
  if(fuseTurns_ <= 0) {
    Log::addMsg("The smoke grenade is extinguished.");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    delete this;
  }
}

void SmokeGrenade::onThrownIgnitedLanding(const Pos& p) {
  Explosion::runSmokeExplosionAt(p);
  Map::player->updateFov();
  Renderer::drawMapAndInterface();
}

void SmokeGrenade::onPlayerParalyzed() {
  Log::addMsg("The ignited smoke grenade falls from my hands.");
  Map::player->activeExplosive = nullptr;
  Map::player->updateClr();
  const Pos&  p = Map::player->pos;
  auto* const f = Map::cells[p.x][p.y].rigid;
  if(!f->isBottomless()) {Explosion::runSmokeExplosionAt(Map::player->pos);}
  Map::player->updateFov();
  Renderer::drawMapAndInterface();
  delete this;
}
