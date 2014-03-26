#include "Marker.h"

#include "Engine.h"
#include "Input.h"
#include "InventoryHandler.h"
#include "ItemWeapon.h"
#include "Input.h"
#include "ActorPlayer.h"
#include "Attack.h"
#include "Log.h"
#include "Look.h"
#include "Thrower.h"
#include "Renderer.h"
#include "Map.h"
#include "ItemFactory.h"
#include "LineCalc.h"
#include "Utils.h"
#include "Config.h"

void Marker::readKeys(const MarkerTask markerTask, MarkerReturnData& data,
                      Item* itemThrown) {
  const KeyboardReadRetData& d = Input::readKeysUntilFound(eng);

  if(d.sdlKey_ == SDLK_RIGHT    || d.key_ == '6' || d.key_ == 'l') {
    if(d.isShiftHeld_) {
      move(1, -1, markerTask, itemThrown);
    } else if(d.isCtrlHeld_) {
      move(1, 1, markerTask, itemThrown);
    } else {
      move(1, 0, markerTask, itemThrown);
    }
  }
  if(d.sdlKey_ == SDLK_UP       || d.key_ == '8' || d.key_ == 'k') {
    move(0, -1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_LEFT     || d.key_ == '4' || d.key_ == 'h') {
    if(d.isShiftHeld_) {
      move(-1, -1, markerTask, itemThrown);
    } else if(d.isCtrlHeld_) {
      move(-1, 1, markerTask, itemThrown);
    } else {
      move(-1, 0, markerTask, itemThrown);
    }
  }
  if(d.sdlKey_ == SDLK_DOWN     || d.key_ == '2' || d.key_ == 'j') {
    move(0, 1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_PAGEUP   || d.key_ == '9' || d.key_ == 'u') {
    move(1, -1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_HOME     || d.key_ == '7' || d.key_ == 'y') {
    move(-1, -1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_END      || d.key_ == '1' || d.key_ == 'b') {
    move(-1, 1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_PAGEDOWN || d.key_ == '3' || d.key_ == 'n') {
    move(1, 1, markerTask, itemThrown);
  }
  // ------------------------------------------------------- AIM RANGED WEAPON
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'f') {
    if(markerTask == markerTask_aimRangedWeapon) {
      if(pos_ != eng.player->pos) {

        eng.log->clearLog();
        Renderer::drawMapAndInterface();

        Actor* const actor = Utils::getActorAtPos(pos_, eng);
        if(actor != NULL) {eng.player->target = actor;}

        Item* const item = eng.player->getInv().getItemInSlot(slot_wielded);
        Weapon* const weapon = dynamic_cast<Weapon*>(item);
        if(eng.attack->ranged(*eng.player, *weapon, pos_) == false) {
          eng.log->addMsg("No ammunition loaded.");
        }
      } else {
        eng.log->addMsg("I think I can persevere a little longer.");
      }
      done();
    }
  }
  // ------------------------------------------------------- LOOK
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'v') {
    if(markerTask == markerTask_look) {
      eng.look->printExtraActorDescription(pos_);
      move(0, 0, markerTask_look, itemThrown);
    }
  }
  // ------------------------------------------------------- THROW
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 't') {
    if(markerTask == markerTask_aimThrownWeapon) {
      if(pos_ == eng.player->pos) {
        eng.log->addMsg("I should throw this somewhere else.");
      } else {
        Renderer::drawMapAndInterface();
        Actor* const actor = Utils::getActorAtPos(pos_, eng);
        if(actor != NULL) {eng.player->target = actor;}
        eng.thrower->throwItem(*eng.player, pos_, *itemThrown);
        data.didThrowMissile = true;
      }

      done();
    }
  }
  // ------------------------------------------------------- THROW EXPLOSIVE
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'e') {
    if(markerTask == markerTask_aimLitExplosive) {
      Renderer::drawMapAndInterface();
      eng.thrower->playerThrowLitExplosive(pos_);
      done();
    }
  }
  if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
    cancel();
  }
}

void Marker::draw(const MarkerTask markerTask) const {
  Renderer::drawMapAndInterface(false);

  vector<Pos> trail;

  int effectiveRange = -1;

  const Pos playerPos = eng.player->pos;
  eng.lineCalc->calcNewLine(playerPos, pos_, true, 9999, false, trail);

  if(markerTask == markerTask_aimRangedWeapon) {
    Item* const item =
      eng.player->getInv().getItemInSlot(slot_wielded);
    Weapon* const weapon = dynamic_cast<Weapon*>(item);
    effectiveRange = weapon->effectiveRangeLimit;
  }

  Renderer::drawMarker(trail, effectiveRange);
  Renderer::updateScreen();
}

MarkerReturnData Marker::run(const MarkerTask markerTask, Item* itemThrown) {
  pos_ = eng.player->pos;

  MarkerReturnData data;

  if(
    markerTask == markerTask_aimRangedWeapon  ||
    markerTask == markerTask_look             ||
    markerTask == markerTask_aimThrownWeapon) {
    //Attempt to place marker at target.
    if(setPosToTargetIfVisible() == false) {
      //Else NULL the target, and attempt to place marker at closest visible enemy.
      //This sets a new target if successful.
      eng.player->target = NULL;
      setPosToClosestEnemyIfVisible();
    }
  }

  if(
    markerTask == markerTask_look ||
    markerTask == markerTask_aimRangedWeapon ||
    markerTask == markerTask_aimThrownWeapon) {
    eng.look->markerAtPos(pos_, markerTask, itemThrown);
  }

  draw(markerTask);

  isDone_ = false;
  while(isDone_ == false) {
    readKeys(markerTask, data, itemThrown);
  }

  return data;
}

void Marker::setPosToClosestEnemyIfVisible() {
  vector<Actor*> SpottedEnemies;
  eng.player->getSpottedEnemies(SpottedEnemies);
  vector<Pos> spottedEnemiesPositions;

  Utils::getActorPositions(SpottedEnemies, spottedEnemiesPositions);

  //If player sees enemies, suggest one for targeting
  if(spottedEnemiesPositions.empty() == false) {
    pos_ = Utils::getClosestPos(eng.player->pos, spottedEnemiesPositions);

    eng.player->target = Utils::getActorAtPos(pos_, eng);
  }
}

bool Marker::setPosToTargetIfVisible() {
  const Actor* const target = eng.player->target;

  if(target != NULL) {
    vector<Actor*> SpottedEnemies;
    eng.player->getSpottedEnemies(SpottedEnemies);

    if(SpottedEnemies.empty() == false) {

      for(unsigned int i = 0; i < SpottedEnemies.size(); i++) {
        if(target == SpottedEnemies.at(i)) {
          pos_ = SpottedEnemies.at(i)->pos;
          return true;
        }
      }
    }
  }
  return false;
}

void Marker::move(const int DX, const int DY, const MarkerTask markerTask,
                  const Item* itemThrown) {
  bool isMoved = false;
  const Pos newPos = pos_ + Pos(DX, DY);
  if(Utils::isPosInsideMap(newPos)) {
    pos_ = newPos;
    isMoved = true;
  }

  if(isMoved) {
    if(
      markerTask == markerTask_look             ||
      markerTask == markerTask_aimRangedWeapon  ||
      markerTask == markerTask_aimThrownWeapon) {
      eng.look->markerAtPos(pos_, markerTask, itemThrown);
    }
  }

  draw(markerTask);
}

void Marker::done() {
  Renderer::drawMapAndInterface();
  isDone_ = true;
}

void Marker::cancel() {
  eng.log->clearLog();
  Renderer::drawMapAndInterface();
  isDone_ = true;
}

