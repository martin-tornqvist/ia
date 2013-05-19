#include "Thrower.h"

#include "Engine.h"

#include "Item.h"
#include "ItemPotion.h"
#include "ActorData.h"
#include "ActorPlayer.h"
#include "Render.h"
#include "Map.h"
#include "Log.h"
#include "FeatureFactory.h"
#include "Explosion.h"
#include "ItemDrop.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "Attack.h"

void Thrower::playerThrowLitExplosive(const coord& aimCell) {
  const int DYNAMITE_FUSE = eng->player->dynamiteFuseTurns;
  const int FLARE_FUSE = eng->player->flareFuseTurns;

  eng->player->explosiveThrown();

  vector<coord> path =
    eng->mapTests->getLine(eng->player->pos, aimCell, true, THROWING_RANGE_LIMIT);

  //Render
  const char glyph = eng->itemData->itemDefinitions[item_dynamite]->glyph;
  SDL_Color clr = DYNAMITE_FUSE != -1 ? clrRedLight : clrYellow;
  for(unsigned int i = 1; i < path.size() - 1; i++) {
    eng->renderer->drawMapAndInterface(false);
    if(eng->map->playerVision[path[i].x][path[i].y]) {
      eng->renderer->drawCharacter(
        glyph, renderArea_mainScreen, path[i].x, path[i].y, clr);
      eng->renderer->updateScreen();
      eng->sleep(eng->config->DELAY_PROJECTILE_DRAW);
    }
  }

  Feature* const featureAtDest =
    eng->map->featuresStatic[path.back().x][path.back().y];
  const bool IS_DEST_FEAT_BOTTOMLESS = featureAtDest->isBottomless();

  if(DYNAMITE_FUSE != -1) {
    eng->log->addMessage("I throw a lit dynamite stick.");
    if(IS_DEST_FEAT_BOTTOMLESS == false) {
      eng->featureFactory->spawnFeatureAt(
        feature_litDynamite, path.back(), new DynamiteSpawnData(DYNAMITE_FUSE));
    }
  } else if(FLARE_FUSE != -1) {
    eng->log->addMessage("I throw a lit flare.");
    if(IS_DEST_FEAT_BOTTOMLESS == false) {
      eng->featureFactory->spawnFeatureAt(
        feature_litFlare, path.back(), new DynamiteSpawnData(FLARE_FUSE));
    }
    eng->gameTime->updateLightMap();
    eng->player->updateFov();
    eng->renderer->drawMapAndInterface();
  } else {
    eng->log->addMessage("I throw a lit Molotov Cocktail.");
    if(IS_DEST_FEAT_BOTTOMLESS == false) {
//      eng->audio->playSound(audio_molotovExplosion);
      eng->explosionMaker->runExplosion(
        path.back(), false, new StatusBurning(eng));
    }
  }

  eng->gameTime->endTurnOfCurrentActor();
}

void Thrower::throwItem(Actor& actorThrowing, const coord& targetCell,
                        Item& itemThrown) {
//  Inventory* inventory = actorThrowing->getInventory();
//  Item* const itemStack = inventory->getItemInSlot(slot_missiles);

  const ItemDefinition& itemDefStack = itemThrown.getDef();

  MissileAttackData* data = new MissileAttackData(
    actorThrowing, itemThrown, targetCell, actorThrowing.pos, eng);

  const ActorSizes_t aimLevel = data->intendedAimLevel;

  vector<coord> path = eng->mapTests->getLine(
                         actorThrowing.pos, targetCell,
                         false, THROWING_RANGE_LIMIT);

//    Item* itemThrown = eng->itemFactory->copyItem(itemStack);
//    itemThrown->numberOfItems = 1;
  const ItemDefinition& itemThrownDef = itemThrown.getDef();

//    inventory->decreaseItemInSlot(slot_missiles);

  const string itemName_a =
    eng->itemData->getItemRef(itemThrown, itemRef_a, true);
  if(&actorThrowing == eng->player) {
    eng->log->clearLog();
    eng->log->addMessage("I throw " + itemName_a + ".");
  } else {
    if(eng->map->playerVision[path.front().x][path.front().y] == true) {
      eng->log->addMessage(
        actorThrowing.getNameThe() + " throws " + itemName_a + ".");
    }
  }
  eng->renderer->drawMapAndInterface(true);

  int blockedInElement = -1;

  const char glyph = itemThrown.getGlyph();
  const SDL_Color clr = itemThrown.getColor();

  int chanceToDestroyItem = 0;

  coord curPos(-1, -1);

  for(unsigned int i = 1; i < path.size(); i++) {
    eng->renderer->drawMapAndInterface(false);

    curPos.set(path.at(i));

    Actor* const actorHere = eng->mapTests->getActorAtPos(curPos);
    if(actorHere != NULL) {
      if(
        curPos == targetCell ||
        actorHere->getDef()->actorSize >= actorSize_humanoid) {

        delete data;
        data = new MissileAttackData(actorThrowing, itemThrown, targetCell,
                                     curPos, eng, aimLevel);

        if(data->attackResult >= successSmall) {
          if(eng->map->playerVision[curPos.x][curPos.y]) {
            eng->renderer->drawCharacter('*', renderArea_mainScreen,
                                         curPos, clrRedLight);
            eng->renderer->updateScreen();
            eng->sleep(eng->config->DELAY_PROJECTILE_DRAW * 4);
          }
          const SDL_Color hitMessageClr =
            actorHere == eng->player ? clrMessageBad : clrMessageGood;
          eng->log->addMessage(
            actorHere->getNameThe() + " is hit.", hitMessageClr);
          actorHere->hit(data->dmg, damageType_physical);

          //If the thing that hit an actor is a potion, let it make stuff happen...
          if(itemThrownDef.isQuaffable) {
            dynamic_cast<Potion*>(&itemThrown)->collide(
              curPos, actorHere, itemThrownDef, eng);
            delete &itemThrown;
            delete data;
            eng->gameTime->endTurnOfCurrentActor();
            return;
          }

          blockedInElement = i;
          chanceToDestroyItem = 25;
          break;
        }
      }
    }

    if(eng->map->playerVision[curPos.x][curPos.y]) {
      eng->renderer->drawCharacter(glyph, renderArea_mainScreen, curPos, clr);
      eng->renderer->updateScreen();
      eng->sleep(eng->config->DELAY_PROJECTILE_DRAW);
    }

    if(curPos == targetCell) {
      blockedInElement = i;
      break;
    }

    const Feature* featureHere = eng->map->featuresStatic[curPos.x][curPos.y];
    if(featureHere->isShootPassable() == false) {
      blockedInElement = itemThrownDef.isQuaffable ? i : i - 1;
      break;
    }
  }

  //If potion, collide it on the landscape
  if(itemThrownDef.isQuaffable) {
    if(blockedInElement >= 0) {
      dynamic_cast<Potion*>(&itemThrown)->collide(
        path.at(blockedInElement), NULL, itemThrownDef, eng);
      delete &itemThrown;
      delete data;
      eng->gameTime->endTurnOfCurrentActor();
      return;
    }
  }

  if(eng->dice.percentile() < chanceToDestroyItem) {
    delete &itemThrown;
  } else {
    const int DROP_ELEMENT = blockedInElement == -1 ?
                             path.size() - 1 : blockedInElement;
    const coord dropPos = path.at(DROP_ELEMENT);
    const MaterialType_t materialAtDropPos =
      eng->map->featuresStatic[dropPos.x][dropPos.y]->getMaterialType();
    if(materialAtDropPos == materialType_hard) {
      const bool IS_ALERTING_MONSTERS = &actorThrowing == eng->player;
      Sound sound(itemThrownDef.landOnHardSurfaceSoundMessage,
                  true, dropPos, false, IS_ALERTING_MONSTERS);
      eng->soundEmitter->emitSound(sound);
    }
    eng->itemDrop->dropItemOnMap(dropPos, itemThrown);
  }

  delete data;
  eng->renderer->drawMapAndInterface();
  eng->gameTime->endTurnOfCurrentActor();
}

//vector<coord> Thrower::getFlightPath(const coord& origin, const coord& aimPos,
//                                     const bool INCLUDE_BLOCKED_CELL,
//                                     const bool CONTINUE_UNTUL_BLOCKED) {
//  ThrownObject obj;
//  obj.flightPath.resize(0);
//
//  double deltaX = (double(aimPos.x) - double(origin.x));
//  double deltaY = (double(aimPos.y) - double(origin.y));
//
//  obj.xPos = origin.x;
//  obj.yPos = o;
//  obj.xPos_prec = double(originX) + 0.5;
//  obj.yPos_prec = double(originY) + 0.5;
//  obj.hypot = sqrt(deltaX * deltaX + deltaY * deltaY);
//  obj.xIncr = (deltaX / obj.hypot);
//  obj.yIncr = (deltaY / obj.hypot);
//  obj.aimX = aimX;
//  obj.aimY = aimY;
//
//  //Get maximum cell flightlength
//  const int abilityCellPotenetial = 12; //Placeholder value***
//  const int aimedCellPotential =
//    CONTINUE_UNTUL_BLOCKED == true ? 9999 : eng->basicUtils->manhattanDistance(originX, originY, aimX, aimY);
//  obj.cellPotential = min(abilityCellPotenetial, aimedCellPotential);
//
//  //Add origin cell as first cell in flightpath
//  coord c;
//  c.x = obj.xPos;
//  c.y = obj.yPos;
//  obj.flightPath.push_back(c);
//
//  bool done = false;
//  while(done == false) {
//    setLineUntilSolidAndDeductCells(obj, INCLUDE_BLOCKED_CELL, CONTINUE_UNTUL_BLOCKED);
//
//    if(obj.cellPotential <= 0) {
//      done = true;
//    }
//  }
//
//  return obj.flightPath;
//}
//
//void Thrower::setLineUntilSolidAndDeductCells(ThrownObject& obj, const bool INCLUDE_BLOCKED_CELL, const bool CONTINUE_UNTUL_BLOCKED) {
//  coord c;
//
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  eng->mapTests->makeShootBlockerFeaturesArray(blockers);
//
//  for(double i = 0; i <= 9999; i += 0.004) {
//    if(obj.cellPotential <= 0) {
//      i = 999999;
//    }
//
//    if(i < 999999) {
//      obj.xPos_prec += obj.xIncr * 0.004;
//      obj.yPos_prec += obj.yIncr * 0.004;
//
//      const int X_POS = int(obj.xPos_prec);
//      const int Y_POS = int(obj.yPos_prec);
//
//      //If we somehow ended up outside the dungeon, abort.
//      if(eng->mapTests->isCellInsideMap(X_POS, Y_POS) == false) {
//        obj.cellPotential = 0;
//        return;
//      }
//
//      obj.xPos = X_POS;
//      obj.yPos = Y_POS;
//
//      //If solid met, abort and back away from the wall.
//      if(i < 99999) {
//        if(blockers[obj.xPos][obj.yPos]) {
//          //obj.blockedByCell.x = obj.xPos;
//          //obj.blockedByCell.y = obj.yPos;
//
//          if(INCLUDE_BLOCKED_CELL == true) {
//            c.x = obj.xPos;
//            c.y = obj.yPos;
//            obj.flightPath.push_back(c);
//          } else {
//            obj.xPos_prec -= obj.xIncr * 0.004;
//            obj.yPos_prec -= obj.yIncr * 0.004;
//
//            obj.xPos = int(obj.xPos_prec);
//            obj.yPos = int(obj.yPos_prec);
//          }
//
//          obj.cellPotential = 0;
//
//          i = 999999;
//        }
//      }
//
//      //If at target cell, abort
//      if(CONTINUE_UNTUL_BLOCKED == false) {
//        if(obj.xPos == obj.aimX && obj.yPos == obj.aimY) {
//          obj.cellPotential = 0;
//          c.x = obj.xPos;
//          c.y = obj.yPos;
//          obj.flightPath.push_back(c);
//          i = 999999;
//        }
//      }
//    }
//
//    if(i < 999999) {
//      //If new coordinate, push it on the vector
//      if(obj.xPos != obj.flightPath.back().x || obj.yPos != obj.flightPath.back().y) {
//        c.x = obj.xPos;
//        c.y = obj.yPos;
//        obj.flightPath.push_back(c);
//        obj.cellPotential -= 1;
//      }
//    }
//  }
//}
//
