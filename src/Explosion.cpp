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

namespace {
void draw(const vector< vector<Pos> >& posLists, Engine& eng,
          const bool SHOULD_OVERRIDE_CLR, const SDL_Color& clrOverride) {

  eng.renderer->drawMapAndInterface();

  const SDL_Color& clrInner = SHOULD_OVERRIDE_CLR ? clrOverride : clrYellow;
  const SDL_Color& clrOuter = SHOULD_OVERRIDE_CLR ? clrOverride : clrRedLgt;

  const bool IS_TILES     = eng.config->isTilesMode;
  const int NR_ANIM_STEPS = IS_TILES ? 2 : 1;

  for(int iAnim = 0; iAnim < NR_ANIM_STEPS; iAnim++) {

    const Tile_t tile = iAnim == 0 ? tile_blast1 : tile_blast2;

    const int NR_OUTER = posLists.size();
    for(int iOuter = 0; iOuter < NR_OUTER; iOuter++) {
      const SDL_Color& clr = iOuter == NR_OUTER - 1 ? clrOuter : clrInner;
      const vector<Pos>& inner = posLists.at(iOuter);
      for(const Pos & pos : inner) {
        if(eng.map->cells[pos.x][pos.y].isSeenByPlayer) {
          if(IS_TILES) {
            eng.renderer->drawTile(tile, panel_map, pos, clr, clrBlack);
          } else {
            eng.renderer->drawGlyph('*', panel_map, pos, clr, true, clrBlack);
          }
        }
      }
    }
    eng.renderer->updateScreen();
    eng.sdlWrapper->sleep(eng.config->delayExplosion / NR_ANIM_STEPS);
  }
}

void getArea(const Pos& c, const int RADI, Rect& rectRef) {
  rectRef = Rect(Pos(max(c.x - RADI, 1), min(c.y - RADI, MAP_W - 2)),
                 Pos(max(c.x + RADI, 1), min(c.y + RADI, MAP_H - 2)));
}

void getPositionsReached(const Rect& area, const Pos& origin, const int RADI,
                         Engine& eng, vector< vector<Pos> >& posListRef) {
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksProjectiles(eng), blockers);

  posListRef.resize(RADI + 1);

  vector<Pos> line;
  for(int y = area.x0y0.y; y <= area.x1y1.y; y++) {
    for(int x = area.x0y0.x; x <= area.x1y1.x; x++) {
      const Pos pos(x, y);
      const int DIST_TO_CENTER = eng.basicUtils->chebyshevDist(pos, origin);
      bool isReached = true;
      if(DIST_TO_CENTER > 1) {
        eng.lineCalc->calcNewLine(origin, pos, true, 999, false, line);
        for(Pos & posCheckBlock : line) {
          if(blockers[posCheckBlock.x][posCheckBlock.y]) {
            isReached = false;
            break;
          }
        }
      }
      if(isReached) {
        posListRef.at(DIST_TO_CENTER).push_back(pos);
      }
    }
  }
}
} //Namespace

namespace Explosion {
void runExplosionAt(const Pos& origin, Engine& eng, const int RADI_CHANGE,
                    const Sfx_t sfx, const bool SHOULD_DO_EXPLOSION_DMG,
                    Prop* const prop, const bool SHOULD_OVERRIDE_CLR,
                    const SDL_Color& clrOverride) {
  Rect area;
  const int RADI = 2 + RADI_CHANGE; //2 is default radius
  getArea(origin, RADI, area);

  vector< vector<Pos> > posLists;
  getPositionsReached(area, origin, RADI, eng, posLists);

  Sound snd("I hear an explosion!", sfx, true, origin,
            SHOULD_DO_EXPLOSION_DMG, true);
  eng.soundEmitter->emitSound(snd);

  draw(posLists, eng, SHOULD_OVERRIDE_CLR, clrOverride);

  //Do damage, apply effect
  const int DMG_ROLLS = 5;
  const int DMG_SIDES = 6;
  const int DMG_PLUS  = 10;

  Actor* actorArray[MAP_W][MAP_H];
  eng.basicUtils->makeActorArray(actorArray);

  const int NR_OUTER = posLists.size();
  for(int curRadi = 0; curRadi < NR_OUTER; curRadi++) {
    const vector<Pos>& inner = posLists.at(curRadi);

    for(const Pos & pos : inner) {

      Actor* actor = actorArray[pos.x][pos.y];

      if(SHOULD_DO_EXPLOSION_DMG) {
        //Damage environment
        if(curRadi <= 1) {eng.map->switchToDestroyedFeatAt(pos);}
        const int DMG = eng.dice(DMG_ROLLS - curRadi, DMG_SIDES) + DMG_PLUS;

        //Damage actor
        if(actor != NULL) {
          if(actor->deadState == actorDeadState_alive) {
            if(actor == eng.player) {
              eng.log->addMsg("I am hit by an explosion!", clrMsgBad);
            }
            actor->hit(DMG, dmgType_physical, true);
          }
        }
        if(eng.dice.fraction(6, 10)) {
          eng.featureFactory->spawnFeatureAt(
            feature_smoke, pos, new SmokeSpawnData(eng.dice.range(2, 4)));
        }
      }

      //Apply property
      if(prop != NULL && actor != NULL) {
        if(actor->deadState == actorDeadState_alive) {
          PropHandler& propHlr = actor->getPropHandler();
          Prop* propCpy =
            propHlr.makePropFromId(prop->getId(), propTurnsSpecified,
                                   prop->turnsLeft_);
          propHlr.tryApplyProp(propCpy);

        }
      }
    }
  }

  eng.player->updateFov();
  eng.renderer->drawMapAndInterface();

  if(prop != NULL) { delete prop;}
}

void runSmokeExplosionAt(const Pos& origin, Engine& eng) {
  Rect area;
  const int RADI = 2; //TODO Parameter
  getArea(origin, RADI, area);

  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksProjectiles(eng), blockers);

  vector< vector<Pos> > posLists;
  getPositionsReached(area, origin, RADI, eng, posLists);

  //TODO Sound message?
  Sound snd("", endOfSfx, true, origin, false, true);
  eng.soundEmitter->emitSound(snd);

  for(const vector<Pos>& inner : posLists) {
    for(const Pos & pos : inner) {
      eng.featureFactory->spawnFeatureAt(
        feature_smoke, pos, new SmokeSpawnData(eng.dice.range(17, 22)));
    }
  }

  eng.player->updateFov();
  eng.renderer->drawMapAndInterface();
}
} //Explosion
