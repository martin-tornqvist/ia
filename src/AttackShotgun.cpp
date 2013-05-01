#include "Attack.h"

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Postmortem.h"

void Attack::shotgun(const coord& origin, const coord& target, Weapon* const weapon) {
  AttackData data;
  getAttackData(data, target, origin, weapon, false);
  printRangedInitiateMessages(data);

  const ActorSizes_t intendedAimLevel = data.aimLevel;

  //Add blocking features to array
  bool featureBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeShootBlockerFeaturesArray(featureBlockers);

  //An array of actors
  Actor* actorArray[MAP_X_CELLS][MAP_Y_CELLS];
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      actorArray[x][y] = NULL;
    }
  }
  const unsigned int ACTOR_LOOP_SIZE = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < ACTOR_LOOP_SIZE; i++) {
    Actor* curActor = eng->gameTime->getActorAt(i);
    if(curActor->deadState == actorDeadState_alive) {
      actorArray[curActor->pos.x][curActor->pos.y] = curActor;
    }
  }

  vector<coord> path = eng->mapTests->getLine(origin.x, origin.y, target.x, target.y, false, 9999);

  const int CHANCE_FOR_STRAY_HIT = 50;
  const int STRAY_DMG_DIV = 2;

  int nrActorsHit = 0;

  int monsterKilledInElement = -1;

  for(unsigned int i = 1; i < path.size(); i++) {
    //If travelled further than two steps after a killed monster, stop projectile.
    if(monsterKilledInElement != -1) {
      if(i > static_cast<unsigned int>(monsterKilledInElement + 1)) {
        break;
      }
    }

    const coord curPos(path.at(i));

    bool allowStrayHit = true;

    if(actorArray[curPos.x][curPos.y] != NULL) {

      //Only attempt hit if aiming at a level that would hit the actor
      const ActorSizes_t sizeOfActor = actorArray[curPos.x][curPos.y]->getDef()->actorSize;
      if(sizeOfActor >= actorSize_humanoid || curPos == target) {

        //If this is the intended target cell, attempt direct hit
        if(curPos == target) {
          getAttackData(data, target, curPos, weapon, false);
          if(eng->basicUtils->chebyshevDistance(origin, curPos) <= weapon->effectiveRangeLimit) {
            if(data.attackResult >= successSmall) {
              if(eng->map->playerVision[curPos.x][curPos.y]) {
                eng->renderer->drawMapAndInterface(false);
                eng->renderer->coverCellInMap(curPos.x, curPos.y);
                if(eng->config->USE_TILE_SET) {
                  eng->renderer->drawTileInMap(tile_blastAnimation2, curPos, clrRedLight);
                } else {
                  eng->renderer->drawCharacter('*', renderArea_mainScreen, curPos, clrRedLight);
                }
                eng->renderer->updateScreen();
                eng->sleep(eng->config->DELAY_SHOTGUN);
              }

              //Messages
              printProjectileAtActorMessages(data, projectileHitType_cleanHit);

              //Damage
              data.currentDefender->hit(data.dmg, weapon->getDef().rangedDamageType);

              nrActorsHit++;

              allowStrayHit = false;

              eng->renderer->drawMapAndInterface();

              //Special shotgun behavior:
              //If current defender was killed, and player aimed at humanoid level, or at floor level
              //but beyond the current position, the shot will continue one cell.
              const bool IS_TARGET_KILLED = data.currentDefender->deadState != actorDeadState_alive;
              if(IS_TARGET_KILLED && monsterKilledInElement == -1) {
                monsterKilledInElement = i;
              }
              if(nrActorsHit >= 2 || IS_TARGET_KILLED == false || (intendedAimLevel == actorSize_floor && curPos == target)) {
                break;
              }
            }
          }
        }

        //Stray hit?
        if(allowStrayHit) {
          getAttackData(data, target, curPos, weapon, false);
          if(eng->dice.percentile() < CHANCE_FOR_STRAY_HIT) {
            if(eng->map->playerVision[curPos.x][curPos.y]) {
              eng->renderer->drawMapAndInterface(false);
              eng->renderer->coverCellInMap(curPos);
              if(eng->config->USE_TILE_SET) {
                eng->renderer->drawTileInMap(tile_blastAnimation2, curPos, clrRedLight);
              } else {
                eng->renderer->drawCharacter('*', renderArea_mainScreen, curPos, clrRedLight);
              }
              eng->renderer->updateScreen();
              eng->sleep(eng->config->DELAY_SHOTGUN);
            }

            data.attackResult = successSmall;

            //Messages
            printProjectileAtActorMessages(data, projectileHitType_strayHit);

            //Damage
            data.currentDefender->hit(data.dmg / STRAY_DMG_DIV, weapon->getDef().rangedDamageType);

            nrActorsHit++;

            eng->renderer->drawMapAndInterface();

            //Special shotgun behavior:
            //If current defender was killed, and player aimed at humanoid level, or at floor level
            //but beyond the current position, the shot will continue one cell.
            const bool IS_TARGET_KILLED = data.currentDefender->deadState != actorDeadState_alive;
            if(IS_TARGET_KILLED && monsterKilledInElement == -1) {
              monsterKilledInElement = i;
            }
            if(nrActorsHit >= 2 || IS_TARGET_KILLED == false || (intendedAimLevel == actorSize_floor && curPos == target)) {
              break;
            }
          }
        }
      }
    }

    //Wall hit?
    getAttackData(data, target, curPos, weapon, false);
    if(featureBlockers[curPos.x][curPos.y]) {
      if(eng->map->playerVision[curPos.x][curPos.y]) {
        eng->renderer->drawMapAndInterface(false);
        eng->renderer->coverCellInMap(curPos);
        if(eng->config->USE_TILE_SET) {
          eng->renderer->drawTileInMap(tile_blastAnimation2, curPos, clrYellow);
        } else {
          eng->renderer->drawCharacter('*', renderArea_mainScreen, curPos, clrYellow);
        }
        eng->renderer->updateScreen();
        eng->sleep(eng->config->DELAY_SHOTGUN);
        eng->renderer->drawMapAndInterface();
      }
      break;
    }

    //Floor hit?
    getAttackData(data, target, curPos, weapon, false);
    if(intendedAimLevel == actorSize_floor && curPos == target) {
      if(eng->map->playerVision[curPos.x][curPos.y]) {
        eng->renderer->drawMapAndInterface(false);
        eng->renderer->coverCellInMap(curPos);
        if(eng->config->USE_TILE_SET) {
          eng->renderer->drawTileInMap(tile_blastAnimation2, curPos, clrYellow);
        } else {
          eng->renderer->drawCharacter('*', renderArea_mainScreen, curPos, clrYellow);
        }
        eng->renderer->updateScreen();
        eng->sleep(eng->config->DELAY_SHOTGUN);
        eng->renderer->drawMapAndInterface();
      }
      break;
    }
  }
}

