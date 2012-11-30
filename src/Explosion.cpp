#include "Explosion.h"

#include "Engine.h"

#include "FeatureData.h"
#include "FeatureSmoke.h"
#include "Render.h"
#include "Map.h"
#include "Fov.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Postmortem.h"

void ExplosionMaker::renderExplosion(const BasicData* data, bool reach[MAP_X_CELLS][MAP_Y_CELLS]) {
  eng->renderer->drawMapAndInterface();

  for(int x = max(1, data->x0 + 1); x <= min(MAP_X_CELLS - 2, data->x1 - 1); x++) {
    for(int y = max(1, data->y0 + 1); y <= min(MAP_Y_CELLS - 2, data->y1 - 1); y++) {
      if(eng->map->playerVision[x][y]) {
        if(reach[x][y]) {
          eng->renderer->addOverlay(coord(x, y), '*', clrYellow);
        }
      }
    }
  }

  eng->renderer->drawMapAndInterface();
  eng->sleep(eng->config->DELAY_EXPLOSION / 2);

  for(int x = max(1, data->x0); x <= min(MAP_X_CELLS - 2, data->x1); x++) {
    for(int y = max(1, data->y0); y <= min(MAP_Y_CELLS - 2, data->y1); y++) {
      if(eng->map->playerVision[x][y]) {
        if(reach[x][y]) {
          if(x == data->x0 || x == data->x1 || y == data->y0 || y == data->y1) {
            eng->renderer->addOverlay(coord(x, y), '*', clrRedLight);
          }
        }
      }
    }
  }
  eng->renderer->drawMapAndInterface();
}

void ExplosionMaker::renderExplosionWithColorOverride(const BasicData* data, const sf::Color clr, bool reach[MAP_X_CELLS][MAP_Y_CELLS]) {
  eng->renderer->drawMapAndInterface();

  for(int x = max(1, data->x0); x <= min(MAP_X_CELLS - 2, data->x1); x++) {
    for(int y = max(1, data->y0); y <= min(MAP_Y_CELLS - 2, data->y1); y++) {
      if(eng->map->playerVision[x][y]) {
        if(reach[x][y]) {
          eng->renderer->addOverlay(coord(x, y), '*', clr);
        }
      }
    }
  }
  eng->renderer->drawMapAndInterface();
}

void ExplosionMaker::runExplosion(const coord& origin, const bool DO_EXPLOSION_DMG, StatusEffect* const effect,
                                  const bool OVERRIDE_EXPLOSION_RENDERING, const sf::Color colorOverride) {
  BasicData data(origin, width, height);

  //Set up explosion reach array
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeShootBlockerFeaturesArray(blockers);
  bool reach[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetBoolArray(reach, false);

  for(int x = max(1, data.x0); x <= min(MAP_X_CELLS - 2, data.x1); x++) {
    for(int y = max(1, data.y0); y <= min(MAP_Y_CELLS - 2, data.y1); y++) {
      reach[x][y] = eng->fov->checkOneCell(blockers, coord(x, y), origin, false) && !blockers[x][y];
    }
  }
  reach[origin.x][origin.y] = true;

  //Assuming all explosions alerts monsters to player.
  if(DO_EXPLOSION_DMG) {
    eng->soundEmitter->emitSound(Sound("I hear an explosion!", true, origin, true, true));
    eng->audio->playSound(audio_explosion);
  }

  //Render
  if(eng->config->USE_TILE_SET) {
    bool forbiddenRenderCells[MAP_X_CELLS][MAP_Y_CELLS];
    eng->basicUtils->resetBoolArray(forbiddenRenderCells, true);
    for(int y = 1; y < MAP_Y_CELLS - 2; y++) {
      for(int x = 1; x < MAP_X_CELLS - 2; x++) {
        forbiddenRenderCells[x][y] = reach[x][y] == false || eng->map->playerVision[x][y] == false;
      }
    }

    if(OVERRIDE_EXPLOSION_RENDERING) {
      eng->renderer->drawBlastAnimationAtField(origin, (data.x1 - data.x0) / 2, forbiddenRenderCells, colorOverride, colorOverride,
          eng->config->DELAY_EXPLOSION);
    } else {
      eng->renderer->drawBlastAnimationAtField(origin, (data.x1 - data.x0) / 2, forbiddenRenderCells, clrYellow, clrRedLight,
          eng->config->DELAY_EXPLOSION);
    }
  } else {
    if(OVERRIDE_EXPLOSION_RENDERING) {
      renderExplosionWithColorOverride(&data, colorOverride, reach);
    } else {
      renderExplosion(&data, reach);
    }
  }

  //Delay before applying damage and effects
  eng->sleep(eng->config->DELAY_EXPLOSION);

  //Do damage, apply effect
  const int EXPLOSION_DMG_ROLLS = 5;
  const int EXPLOSION_DMG_SIDES = 6;
  const int EXPLOSION_DMG_PLUS = 10;
  Actor* currentActor;
  for(int x = max(1, data.x0); x <= min(MAP_X_CELLS - 2, data.x1); x++) {
    for(int y = max(1, data.y0); y <= min(MAP_Y_CELLS - 2, data.y1); y++) {

      if(DO_EXPLOSION_DMG) {
        if(eng->mapTests->isCellsNeighbours(coord(x, y), origin, false)) {
          eng->map->switchToDestroyedFeatAt(coord(x, y));

          if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
            eng->map->switchToDestroyedFeatAt(coord(x, y));
          }
        }
      }

      if(reach[x][y] == true) {
        const int CHEBY_DIST = eng->basicUtils->chebyshevDistance(origin.x, origin.y, x, y);
        const int EXPLOSION_DMG_AT_DIST = eng->dice(EXPLOSION_DMG_ROLLS - CHEBY_DIST, EXPLOSION_DMG_SIDES) + EXPLOSION_DMG_PLUS;

        //Damage actor, or apply effect?
        const unsigned int SIZE_OF_ACTOR_LOOP = eng->gameTime->getLoopSize();
        for(unsigned int i = 0; i < SIZE_OF_ACTOR_LOOP; i++) {
          currentActor = eng->gameTime->getActorAt(i);
          if(currentActor->pos.x == x && currentActor->pos.y == y) {

            if(DO_EXPLOSION_DMG == true) {
              if(currentActor == eng->player) {
                eng->log->addMessage("I am hit by an explosion!", clrMessageBad);
              }
              const bool DIED = currentActor->hit(EXPLOSION_DMG_AT_DIST, damageType_physical);
              if(DIED == true) {
                if(currentActor == eng->player) {
                  if(currentActor->deadState == actorDeadState_corpse) {
                    eng->postmortem->setCauseOfDeath("Hit by an explosion");
                  } else {
                    eng->postmortem->setCauseOfDeath("Blown to pieces by an explosion");
                  }
                }
              }
            }

            if(effect != NULL) {
              if(currentActor->deadState == actorDeadState_alive) {
                //Making a copy of the effect, because the handler may destroy the parameter effect.
                StatusEffect* effectCpy = effect->copy();
                currentActor->getStatusEffectsHandler()->attemptAddEffect(effectCpy);
              }
            }

          }
        }

        if(DO_EXPLOSION_DMG == true) {
          if(eng->dice(1, 100) < 55) {
            eng->featureFactory->spawnFeatureAt(feature_smoke, coord(x, y), new SmokeSpawnData(1 + eng->dice(1, 3)));
          }
        }
      }
    }
  }

  //Set graphics back to normal
  eng->renderer->clearOverlay();
  eng->player->FOVupdate();
  eng->renderer->drawMapAndInterface();

  if(effect != NULL) {
    delete effect;
  }
}

void ExplosionMaker::runSmokeExplosion(const coord& origin, const bool SMALL_RADIUS) {
  const int RADIUS = SMALL_RADIUS == true ? 3 : width;
  BasicData data(origin, RADIUS, RADIUS);

  //Set up explosion reach array
  bool reach[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetBoolArray(reach, false);

  //There are two scans for blocking objects made, pretty unoptimised, but it doesn't matter.

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeShootBlockerFeaturesArray(blockers);

  for(int x = max(1, data.x0); x <= min(MAP_X_CELLS - 2, data.x1); x++) {
    for(int y = max(1, data.y0); y <= min(MAP_Y_CELLS - 2, data.y1); y++) {
      //As opposed to the explosion reach, the smoke explosion must not reach into walls and other solid objects
      reach[x][y] = !blockers[x][y] && eng->fov->checkOneCell(blockers, coord(x, y), origin, false);
    }
  }
  reach[origin.x][origin.y] = true;

  for(int x = max(1, data.x0); x <= min(MAP_X_CELLS - 2, data.x1); x++) {
    for(int y = max(1, data.y0); y <= min(MAP_Y_CELLS - 2, data.y1); y++) {
      if(reach[x][y] == true) {
        eng->featureFactory->spawnFeatureAt(feature_smoke, coord(x, y), new SmokeSpawnData(16 + eng->dice(1, 6)));
      }
    }
  }

  //Draw map
  eng->player->FOVupdate();
  eng->renderer->drawMapAndInterface();

  //Delay
  eng->sleep(eng->config->DELAY_EXPLOSION);
}
