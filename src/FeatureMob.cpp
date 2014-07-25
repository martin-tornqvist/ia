#include "Init.h"

#include <algorithm>

#include "FeatureMob.h"
#include "GameTime.h"
#include "PlayerBon.h"
#include "Explosion.h"
#include "Utils.h"
#include "Map.h"
#include "FeatureRigid.h"
#include "Fov.h"

using namespace std;

//------------------------------------------------------------------- SMOKE
void Smoke::onNewTurn() {
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
    const int D = PlayerBon::hasTrait(Trait::demolitionExpert) ? 1 : 0;
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
