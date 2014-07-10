#include "Explosion.h"

#include "Renderer.h"
#include "Map.h"
#include "Log.h"
#include "MapParsing.h"
#include "SdlWrapper.h"
#include "LineCalc.h"
#include "ActorPlayer.h"
#include "Utils.h"
#include "SdlWrapper.h"
#include "PlayerBon.h"
#include "FeatureStatic.h"
#include "FeatureMob.h"

using namespace std;

namespace {

void draw(const vector< vector<Pos> >& posLists, bool blocked[MAP_W][MAP_H],
          const SDL_Color* const clrOverride) {
  Renderer::drawMapAndInterface();

  const SDL_Color& clrInner = clrOverride ? *clrOverride : clrYellow;
  const SDL_Color& clrOuter = clrOverride ? *clrOverride : clrRedLgt;

  const bool IS_TILES     = Config::isTilesMode();
  const int NR_ANIM_STEPS = IS_TILES ? 2 : 1;

  bool isAnyCellSeenByPlayer = false;

  for(int iAnim = 0; iAnim < NR_ANIM_STEPS; iAnim++) {

    const TileId tile = iAnim == 0 ? TileId::blast1 : TileId::blast2;

    const int NR_OUTER = posLists.size();
    for(int iOuter = 0; iOuter < NR_OUTER; iOuter++) {
      const SDL_Color& clr = iOuter == NR_OUTER - 1 ? clrOuter : clrInner;
      const vector<Pos>& inner = posLists.at(iOuter);
      for(const Pos& pos : inner) {
        if(
          Map::cells[pos.x][pos.y].isSeenByPlayer &&
          !blocked[pos.x][pos.y]) {
          isAnyCellSeenByPlayer = true;
          if(IS_TILES) {
            Renderer::drawTile(tile, Panel::map, pos, clr, clrBlack);
          } else {
            Renderer::drawGlyph('*', Panel::map, pos, clr, true, clrBlack);
          }
        }
      }
    }
    if(isAnyCellSeenByPlayer) {
      Renderer::updateScreen();
      SdlWrapper::sleep(Config::getDelayExplosion() / NR_ANIM_STEPS);
    }
  }
}

void getArea(const Pos& c, const int RADI, Rect& rectRef) {
  rectRef = Rect(Pos(max(c.x - RADI, 1),         max(c.y - RADI, 1)),
                 Pos(min(c.x + RADI, MAP_W - 2), min(c.y + RADI, MAP_H - 2)));
}

void getPositionsReached(const Rect& area, const Pos& origin,
                         bool blocked[MAP_W][MAP_H],
                         vector< vector<Pos> >& posListRef) {
  vector<Pos> line;
  for(int y = area.p0.y; y <= area.p1.y; ++y) {
    for(int x = area.p0.x; x <= area.p1.x; ++x) {
      const Pos pos(x, y);
      const int DIST = Utils::kingDist(pos, origin);
      bool isReached = true;
      if(DIST > 1) {
        LineCalc::calcNewLine(origin, pos, true, 999, false, line);
        for(Pos& posCheckBlock : line) {
          if(blocked[posCheckBlock.x][posCheckBlock.y]) {
            isReached = false;
            break;
          }
        }
      }
      if(isReached) {
        if(int(posListRef.size()) <= DIST) {posListRef.resize(DIST + 1);}
        posListRef.at(DIST).push_back(pos);
      }
    }
  }
}

} //namespace


namespace Explosion {

void runExplosionAt(const Pos& origin, const ExplType explType,
                    const ExplSrc explSrc, const int RADI_CHANGE,
                    const SfxId sfx, Prop* const prop,
                    const SDL_Color* const clrOverride) {
  Rect area;
  const int RADI = EXPLOSION_STD_RADI + RADI_CHANGE;
  getArea(origin, RADI, area);

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksProjectiles(), blocked);

  vector< vector<Pos> > posLists;
  getPositionsReached(area, origin, blocked, posLists);

  SndVol vol = explType == ExplType::expl ? SndVol::high : SndVol::low;

  Snd snd("I hear an explosion!", sfx, IgnoreMsgIfOriginSeen::yes, origin,
          nullptr, vol, AlertsMonsters::yes);
  SndEmit::emitSnd(snd);

  draw(posLists, blocked, clrOverride);

  //Do damage, apply effect

  Actor* livingActors[MAP_W][MAP_H];
  vector<Actor*> corpses[MAP_W][MAP_H];

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      livingActors[x][y] = nullptr;
      corpses[x][y].resize(0);
    }
  }

  for(Actor* actor : GameTime::actors_) {
    const Pos& pos = actor->pos;
    if(actor->deadState == ActorDeadState::alive) {
      livingActors[pos.x][pos.y] = actor;
    } else if(actor->deadState == ActorDeadState::corpse) {
      corpses[pos.x][pos.y].push_back(actor);
    }
  }

  const bool IS_DEM_EXP = PlayerBon::hasTrait(Trait::demolitionExpert);

  const int NR_OUTER = posLists.size();
  for(int curRadi = 0; curRadi < NR_OUTER; curRadi++) {
    const vector<Pos>& positionsAtCurRadi = posLists.at(curRadi);

    for(const Pos& pos : positionsAtCurRadi) {

      Actor* livingActor          = livingActors[pos.x][pos.y];
      vector<Actor*> corpsesHere  = corpses[pos.x][pos.y];

      if(explType == ExplType::expl) {
        //Damage environment
        Cell& cell = Map::cells[pos.x][pos.y];
        cell.featureStatic->hit(DmgType::physical, DmgMethod::explosion, nullptr);

        const int ROLLS = EXPL_DMG_ROLLS - curRadi;
        const int DMG   = Rnd::dice(ROLLS, EXPL_DMG_SIDES) + EXPL_DMG_PLUS;

        //Damage living actor
        if(livingActor) {
          if(livingActor == Map::player) {
            Log::addMsg("I am hit by an explosion!", clrMsgBad);
          }
          livingActor->hit(DMG, DmgType::physical, true);
        }
        //Damage dead actors
        for(Actor* corpse : corpsesHere) {corpse->hit(DMG, DmgType::physical, true);}

        //Add smoke
        if(Rnd::fraction(6, 10)) {GameTime::addMob(new Smoke(pos, Rnd::range(2, 4)));}
      }

      //Apply property
      if(prop) {
        if(
          livingActor &&
          (livingActor != Map::player || !IS_DEM_EXP ||
           explSrc != ExplSrc::playerUseMoltvIntended)) {
          PropHandler& propHlr = livingActor->getPropHandler();
          Prop* propCpy = propHlr.mkProp(prop->getId(), propTurnsSpecific,
                                         prop->turnsLeft_);
          propHlr.tryApplyProp(propCpy);
        }
        //If property is burning, also apply it to corpses
        if(prop->getId() == propBurning) {
          for(Actor* corpse : corpsesHere) {
            PropHandler& propHlr = corpse->getPropHandler();
            Prop* propCpy = propHlr.mkProp(prop->getId(), propTurnsSpecific,
                                           prop->turnsLeft_);
            propHlr.tryApplyProp(propCpy);
          }
        }
      }
    }
  }

  Map::player->updateFov();
  Renderer::drawMapAndInterface();

  if(prop) {delete prop;}
}

void runSmokeExplosionAt(const Pos& origin) {
  Rect area;
  const int RADI = EXPLOSION_STD_RADI;
  getArea(origin, RADI, area);

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksProjectiles(), blocked);

  vector< vector<Pos> > posLists;
  getPositionsReached(area, origin, blocked, posLists);

  //TODO Sound message?
  Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, origin, nullptr,
          SndVol::low, AlertsMonsters::yes);
  SndEmit::emitSnd(snd);

  for(const vector<Pos>& inner : posLists) {
    for(const Pos& pos : inner) {
      if(!blocked[pos.x][pos.y]) {
        GameTime::addMob(new Smoke(pos, Rnd::range(17, 22)));
      }
    }
  }

  Map::player->updateFov();
  Renderer::drawMapAndInterface();
}

} //Explosion

