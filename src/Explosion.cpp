#include "Explosion.h"

#include "Engine.h"

#include "FeatureSmoke.h"
#include "Renderer.h"
#include "Map.h"
#include "Log.h"
#include "MapParsing.h"
#include "SdlWrapper.h"
#include "LineCalc.h"
#include "ActorPlayer.h"
#include "Utils.h"
#include "SdlWrapper.h"

namespace {

void draw(const vector< vector<Pos> >& posLists, bool blockers[MAP_W][MAP_H],
          const bool SHOULD_OVERRIDE_CLR, const SDL_Color& clrOverride,
          Engine& eng) {
  Renderer::drawMapAndInterface();

  const SDL_Color& clrInner = SHOULD_OVERRIDE_CLR ? clrOverride : clrYellow;
  const SDL_Color& clrOuter = SHOULD_OVERRIDE_CLR ? clrOverride : clrRedLgt;

  const bool IS_TILES     = Config::isTilesMode();
  const int NR_ANIM_STEPS = IS_TILES ? 2 : 1;

  bool isAnyCellSeenByPlayer = false;

  for(int iAnim = 0; iAnim < NR_ANIM_STEPS; iAnim++) {

    const TileId tile = iAnim == 0 ? tile_blast1 : tile_blast2;

    const int NR_OUTER = posLists.size();
    for(int iOuter = 0; iOuter < NR_OUTER; iOuter++) {
      const SDL_Color& clr = iOuter == NR_OUTER - 1 ? clrOuter : clrInner;
      const vector<Pos>& inner = posLists.at(iOuter);
      for(const Pos & pos : inner) {
        if(
          eng.map->cells[pos.x][pos.y].isSeenByPlayer &&
          blockers[pos.x][pos.y] == false) {
          isAnyCellSeenByPlayer = true;
          if(IS_TILES) {
            Renderer::drawTile(tile, panel_map, pos, clr, clrBlack);
          } else {
            Renderer::drawGlyph('*', panel_map, pos, clr, true, clrBlack);
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
                         bool blockers[MAP_W][MAP_H], Engine& eng,
                         vector< vector<Pos> >& posListRef) {
  vector<Pos> line;
  for(int y = area.x0y0.y; y <= area.x1y1.y; y++) {
    for(int x = area.x0y0.x; x <= area.x1y1.x; x++) {
      const Pos pos(x, y);
      const int DIST = Utils::chebyshevDist(pos, origin);
      bool isReached = true;
      if(DIST > 1) {
        eng.lineCalc->calcNewLine(origin, pos, true, 999, false, line);
        for(Pos & posCheckBlock : line) {
          if(blockers[posCheckBlock.x][posCheckBlock.y]) {
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

void runExplosionAt(const Pos& origin, Engine& eng, const int RADI_CHANGE,
                    const SfxId sfx, const bool SHOULD_DO_EXPLOSION_DMG,
                    Prop* const prop, const bool SHOULD_OVERRIDE_CLR,
                    const SDL_Color& clrOverride) {
  Rect area;
  const int RADI = EXPLOSION_STD_RADI + RADI_CHANGE;
  getArea(origin, RADI, area);

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksProjectiles(eng), blockers);

  vector< vector<Pos> > posLists;
  getPositionsReached(area, origin, blockers, eng, posLists);

  SndVol vol = SHOULD_DO_EXPLOSION_DMG ? SndVol::high : SndVol::low;

  Snd snd("I hear an explosion!", sfx, IgnoreMsgIfOriginSeen::yes, origin,
          NULL, vol, AlertsMonsters::yes);
  SndEmit::emitSnd(snd, eng);

  draw(posLists, blockers, SHOULD_OVERRIDE_CLR, clrOverride, eng);

  //Do damage, apply effect
  const int DMG_ROLLS = 5;
  const int DMG_SIDES = 6;
  const int DMG_PLUS  = 10;

  Actor* livingActors[MAP_W][MAP_H];
  vector<Actor*> corpses[MAP_W][MAP_H];

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      livingActors[x][y] = NULL;
      corpses[x][y].resize(0);
    }
  }

  for(Actor * actor : eng.gameTime->actors_) {
    const Pos& pos = actor->pos;
    if(actor->deadState == ActorDeadState::alive) {
      livingActors[pos.x][pos.y] = actor;
    } else if(actor->deadState == ActorDeadState::corpse) {
      corpses[pos.x][pos.y].push_back(actor);
    }
  }

  const int NR_OUTER = posLists.size();
  for(int curRadi = 0; curRadi < NR_OUTER; curRadi++) {
    const vector<Pos>& inner = posLists.at(curRadi);

    for(const Pos & pos : inner) {

      Actor* livingActor          = livingActors[pos.x][pos.y];
      vector<Actor*> corpsesHere  = corpses[pos.x][pos.y];

      if(SHOULD_DO_EXPLOSION_DMG) {
        //Damage environment
        if(curRadi <= 1) {eng.map->switchToDestroyedFeatAt(pos);}
        const int DMG = Rnd::dice(DMG_ROLLS - curRadi, DMG_SIDES) + DMG_PLUS;

        //Damage living actor
        if(livingActor != NULL) {
          if(livingActor == eng.player) {
            eng.log->addMsg("I am hit by an explosion!", clrMsgBad);
          }
          livingActor->hit(DMG, dmgType_physical, true);
        }
        //Damage dead actors
        for(Actor * corpse : corpsesHere) {
          corpse->hit(DMG, dmgType_physical, true);
        }

        if(Rnd::fraction(6, 10)) {
          eng.featureFactory->spawnFeatureAt(
            feature_smoke, pos, new SmokeSpawnData(Rnd::range(2, 4)));
        }
      }

      //Apply property
      if(prop != NULL) {
        if(livingActor != NULL) {
          PropHandler& propHlr = livingActor->getPropHandler();
          Prop* propCpy = propHlr.makeProp(prop->getId(), propTurnsSpecific,
                                           prop->turnsLeft_);
          propHlr.tryApplyProp(propCpy);
        }
        //If property is burning, also apply it to corpses
        if(prop->getId() == propBurning) {
          for(Actor * corpse : corpsesHere) {
            PropHandler& propHlr = corpse->getPropHandler();
            Prop* propCpy = propHlr.makeProp(prop->getId(), propTurnsSpecific,
                                             prop->turnsLeft_);
            propHlr.tryApplyProp(propCpy);
          }
        }
      }
    }
  }

  eng.player->updateFov();
  Renderer::drawMapAndInterface();

  if(prop != NULL) {delete prop;}
}

void runSmokeExplosionAt(const Pos& origin, Engine& eng) {
  Rect area;
  const int RADI = EXPLOSION_STD_RADI;
  getArea(origin, RADI, area);

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksProjectiles(eng), blockers);

  vector< vector<Pos> > posLists;
  getPositionsReached(area, origin, blockers, eng, posLists);

  //TODO Sound message?
  Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, origin, NULL,
          SndVol::low, AlertsMonsters::yes);
  SndEmit::emitSnd(snd, eng);

  for(const vector<Pos>& inner : posLists) {
    for(const Pos & pos : inner) {
      if(blockers[pos.x][pos.y] == false) {
        eng.featureFactory->spawnFeatureAt(
          feature_smoke, pos, new SmokeSpawnData(Rnd::range(17, 22)));
      }
    }
  }

  eng.player->updateFov();
  Renderer::drawMapAndInterface();
}

} //Explosion

