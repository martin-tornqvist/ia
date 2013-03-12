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

void Thrower::playerThrowLitExplosive(const coord aimCell) {
  const int DYNAMITE_FUSE = eng->player->dynamiteFuseTurns;
  const int FLARE_FUSE = eng->player->flareFuseTurns;

  eng->player->explosiveThrown();

  //Get flight path
  vector<coord> path = getFlightPath(eng->player->pos.x, eng->player->pos.y, aimCell.x, aimCell.y, false, false);

  //Render
  const char glyph = eng->itemData->itemDefinitions[item_dynamite]->glyph;
  sf::Color clr = DYNAMITE_FUSE != -1 ? clrRedLight : clrYellow;
  for(unsigned int i = 1; i < path.size() - 1; i++) {
    eng->renderer->drawMapAndInterface(false);
    if(eng->map->playerVision[path[i].x][path[i].y]) {
      eng->renderer->drawCharacter(glyph, renderArea_mainScreen, path[i].x, path[i].y, clr);
      eng->renderer->updateWindow();
      eng->sleep(eng->config->DELAY_PROJECTILE_DRAW);
    }
//    eng->renderer->updateWindow();
  }

  if(DYNAMITE_FUSE != -1) {
    eng->log->addMessage("I throw a lit dynamite stick.");
    if(eng->map->featuresStatic[path.back().x][path.back().y]->isBottomless() == false) {
      eng->featureFactory->spawnFeatureAt(feature_litDynamite, path.back(), new DynamiteSpawnData(DYNAMITE_FUSE));
    }
  } else if(FLARE_FUSE != -1) {
    eng->log->addMessage("I throw a lit flare.");
    if(eng->map->featuresStatic[path.back().x][path.back().y]->isBottomless() == false) {
      eng->featureFactory->spawnFeatureAt(feature_litFlare, path.back(), new DynamiteSpawnData(FLARE_FUSE));
    }
    eng->gameTime->updateLightMap();
    eng->player->FOVupdate();
    eng->renderer->drawMapAndInterface();
  } else {
    eng->log->addMessage("I throw a lit Molotov Cocktail.");
    if(eng->map->featuresStatic[path.back().x][path.back().y]->isBottomless() == false) {
      eng->audio->playSound(audio_molotovExplosion);
      eng->explosionMaker->runExplosion(path.back(), false, new StatusBurning(eng));
    }
  }

  eng->gameTime->letNextAct();
}

void Thrower::throwMissile(Actor* const actorThrowing, const coord aim) {
  Inventory* inventory = actorThrowing->getInventory();
  Item* const itemStack = inventory->getItemInSlot(slot_missiles);

  if(itemStack != NULL) {

    const ItemDefinition& itemDefStack = itemStack->getDef();

    //Get path
    vector<coord> path;
    const bool INCLUDE_BLOCKED_CELL = itemDefStack.isQuaffable; //potions may shatter against walls and affect them

    bool continueUntilSolid = false;

    Actor* const actorAtAimPos = eng->mapTests->getActorAtPos(aim);
    if(actorAtAimPos != NULL) {
      if(actorAtAimPos->getDef()->actorSize >= actorSize_humanoid) {
        continueUntilSolid = true;
      }
    }

    path = getFlightPath(actorThrowing->pos.x, actorThrowing->pos.y, aim.x, aim.y, INCLUDE_BLOCKED_CELL, continueUntilSolid);

    // Copy item stack to a new item, and decrease stack by 1 (possibly destroying it)
    Item* itemThrown = eng->itemFactory->copyItem(itemStack);
    itemThrown->numberOfItems = 1;
    const ItemDefinition& itemDefThrown = itemThrown->getDef();

    inventory->decreaseItemInSlot(slot_missiles);

    // Print initial throwing message
    const string itemRef = eng->itemData->getItemRef(itemThrown, itemRef_a);
    if(actorThrowing == eng->player) {
      eng->log->clearLog();
      eng->log->addMessage("I throw " + itemRef + ".");
    } else {
      if(eng->map->playerVision[path.front().x][path.front().y] == true) {
        eng->log->addMessage("I see " + actorThrowing->getNameThe() + " throw " + itemRef + ".");
      }
    }
    eng->renderer->drawMapAndInterface(true);

    int blockedInElement = -1;

    const int WPN_BASE_ABILITY = itemDefThrown.missileBaseAttackSkill;
    const int ACTOR_ABILITY = actorThrowing->getDef()->abilityValues.getAbilityValue(ability_accuracyRanged, true, *(actorThrowing));
    const int TOTAL_ABILITY = WPN_BASE_ABILITY + ACTOR_ABILITY;
    const AbilityRollResult_t rollResult = eng->abilityRoll->roll(TOTAL_ABILITY);

    const DiceParam& dmg = itemDefThrown.missileDmg;

    const int DMG = rollResult == successCritical ? dmg.rolls * dmg.sides + dmg.plus : eng->dice(dmg.rolls, dmg.sides) + dmg.plus;
    const char glyph = itemThrown->getGlyph();
    const sf::Color clr = itemThrown->getColor();

    int chanceToDestroyItem = 0;

    //Run path, render, see what it hits (do dmg)
    for(unsigned int i = 1; i < path.size(); i++) {
      eng->renderer->drawMapAndInterface(false);

      Actor* const actorHere = eng->mapTests->getActorAtPos(path.at(i));
      if(actorHere != NULL) {
        //If actor was aimed for
        if(path.at(i) == aim) {
          if(rollResult >= successSmall) {
            if(eng->map->playerVision[path.at(i).x][path.at(i).y]) {
              eng->renderer->drawCharacter('*', renderArea_mainScreen, path[i].x, path[i].y, clrRedLight);
              eng->renderer->updateWindow();
              eng->sleep(eng->config->DELAY_PROJECTILE_DRAW * 4);
            }
            const sf::Color hitMessageClr = actorHere == eng->player ? clrMessageBad : clrMessageGood;
            eng->log->addMessage(actorHere->getNameThe() + " is hit.", hitMessageClr);
            actorHere->hit(DMG, damageType_physical); //TODO fix this if there ever is a non-physical thrown weapon

            //If the thing that hit an actor is a potion, let it make stuff happen...
            if(itemDefThrown.isQuaffable) {
              dynamic_cast<Potion*>(itemThrown)->collide(path.at(i), actorHere, itemDefThrown, eng);
              delete itemThrown;
              return;
            }

            blockedInElement = i;
            chanceToDestroyItem = 25;
            break;
          }
        }

        //If actor hit by accident
        if(actorHere->getDef()->actorSize >= actorSize_humanoid && eng->dice(1, 100) < 25) {
          if(eng->map->playerVision[path.at(i).x][path.at(i).y]) {
            eng->renderer->drawCharacter('*', renderArea_mainScreen, path.at(i).x, path.at(i).y, clrRedLight);
            eng->renderer->updateWindow();
            eng->sleep(eng->config->DELAY_PROJECTILE_DRAW * 4);
          }
          const sf::Color hitMessageClr = actorHere == eng->player ? clrMessageBad : clrMessageGood;
          eng->log->addMessage(actorHere->getNameThe() + " is hit.", hitMessageClr);
          eng->renderer->drawMapAndInterface();
          actorHere->hit(DMG / 2, damageType_physical); //TODO fix this if there ever is a non-physical thrown weapon

          //If the thing that hit an actor is a potion, let it make stuff happen...
          if(itemDefThrown.isQuaffable == true) {
            dynamic_cast<Potion*>(itemThrown)->collide(path.at(i), actorHere, itemDefThrown, eng);
            delete itemThrown;
            return;
          }

          blockedInElement = i;
          chanceToDestroyItem = 25;
          break;
        }
      }

      if(eng->map->playerVision[path[i].x][path[i].y]) {
        eng->renderer->drawCharacter(glyph, renderArea_mainScreen, path[i].x, path[i].y, clr);
        eng->renderer->updateWindow();
        eng->sleep(eng->config->DELAY_PROJECTILE_DRAW);
      }
    }

    //If no actor blocked the potion, collide it on the landscape
    if(itemThrown->getDef().isQuaffable) {
      if(blockedInElement == -1) {
        dynamic_cast<Potion*>(itemThrown)->collide(path.back(), NULL, itemDefThrown, eng);
        delete itemThrown;
        return;
      }
    }

    if(eng->dice(1, 100) <= chanceToDestroyItem) {
      delete itemThrown;
    } else {
      const int DROP_ELEMENT = blockedInElement == -1 ? path.size() - 1 : blockedInElement;
      const coord dropPos = path.at(DROP_ELEMENT);
      const MaterialType_t materialAtDropPos = eng->map->featuresStatic[dropPos.x][dropPos.y]->getMaterialType();
      if(materialAtDropPos == materialType_hard) {
        const bool IS_ALERTING_MONSTERS = actorThrowing == eng->player;
        Sound sound(itemThrown->getDef().landOnHardSurfaceSoundMessage, true, dropPos, false, IS_ALERTING_MONSTERS);
        eng->soundEmitter->emitSound(sound);
      }
      eng->itemDrop->dropItemOnMap(dropPos, &itemThrown);
    }

    eng->renderer->drawMapAndInterface();

    //End turn
    eng->gameTime->letNextAct();
  }
}

vector<coord> Thrower::getFlightPath(int originX, int originY, int aimX, int aimY, const bool INCLUDE_BLOCKED_CELL, const bool CONTINUE_UNTUL_BLOCKED) {
  ThrownObject obj;
  obj.flightPath.resize(0);

  double deltaX = (double(aimX) - double(originX));
  double deltaY = (double(aimY) - double(originY));

  obj.xPos = originX;
  obj.yPos = originY;
  obj.xPos_prec = double(originX) + 0.5;
  obj.yPos_prec = double(originY) + 0.5;
  obj.hypot = sqrt(deltaX * deltaX + deltaY * deltaY);
  obj.xIncr = (deltaX / obj.hypot);
  obj.yIncr = (deltaY / obj.hypot);
  obj.aimX = aimX;
  obj.aimY = aimY;

  //Get maximum cell flightlength
  const int abilityCellPotenetial = 12; //Placeholder value***
  const int aimedCellPotential = CONTINUE_UNTUL_BLOCKED == true ? 9999 : eng->basicUtils->manhattanDistance(originX, originY, aimX, aimY);
  obj.cellPotential = min(abilityCellPotenetial, aimedCellPotential);

  //Add origin cell as first cell in flightpath
  coord c;
  c.x = obj.xPos;
  c.y = obj.yPos;
  obj.flightPath.push_back(c);

  bool done = false;
  while(done == false) {
    setLineUntilSolidAndDeductCells(obj, INCLUDE_BLOCKED_CELL, CONTINUE_UNTUL_BLOCKED);

    if(obj.cellPotential <= 0) {
      done = true;
    }
  }

  return obj.flightPath;
}

void Thrower::setLineUntilSolidAndDeductCells(ThrownObject& obj, const bool INCLUDE_BLOCKED_CELL, const bool CONTINUE_UNTUL_BLOCKED) {
  coord c;

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeShootBlockerFeaturesArray(blockers);

  for(double i = 0; i <= 9999; i += 0.004) {
    if(obj.cellPotential <= 0) {
      i = 999999;
    }

    if(i < 999999) {
      obj.xPos_prec += obj.xIncr * 0.004;
      obj.yPos_prec += obj.yIncr * 0.004;

      const int X_POS = int(obj.xPos_prec);
      const int Y_POS = int(obj.yPos_prec);

      //If we somehow ended up outside the dungeon, abort.
      if(eng->mapTests->isCellInsideMainScreen(X_POS, Y_POS) == false) {
        obj.cellPotential = 0;
        return;
      }

      obj.xPos = X_POS;
      obj.yPos = Y_POS;

      //If solid met, abort and back away from the wall.
      if(i < 99999) {
        if(blockers[obj.xPos][obj.yPos]) {
          //obj.blockedByCell.x = obj.xPos;
          //obj.blockedByCell.y = obj.yPos;

          if(INCLUDE_BLOCKED_CELL == true) {
            c.x = obj.xPos;
            c.y = obj.yPos;
            obj.flightPath.push_back(c);
          } else {
            obj.xPos_prec -= obj.xIncr * 0.004;
            obj.yPos_prec -= obj.yIncr * 0.004;

            obj.xPos = int(obj.xPos_prec);
            obj.yPos = int(obj.yPos_prec);
          }

          obj.cellPotential = 0;

          i = 999999;
        }
      }

      //If at target cell, abort
      if(CONTINUE_UNTUL_BLOCKED == false) {
        if(obj.xPos == obj.aimX && obj.yPos == obj.aimY) {
          obj.cellPotential = 0;
          c.x = obj.xPos;
          c.y = obj.yPos;
          obj.flightPath.push_back(c);
          i = 999999;
        }
      }
    }

    if(i < 999999) {
      //If new coordinate, push it on the vector
      if(obj.xPos != obj.flightPath.back().x || obj.yPos != obj.flightPath.back().y) {
        c.x = obj.xPos;
        c.y = obj.yPos;
        obj.flightPath.push_back(c);
        obj.cellPotential -= 1;
      }
    }
  }
}

