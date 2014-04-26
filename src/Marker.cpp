#include "Marker.h"

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
  const KeyboardReadRetData& d = Input::readKeysUntilFound();

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
    if(markerTask == MarkerTask::aimRangedWeapon) {
      if(pos_ != Map::player->pos) {

        eng.log->clearLog();
        Renderer::drawMapAndInterface();

        Actor* const actor = Utils::getActorAtPos(pos_, eng);
        if(actor != NULL) {Map::player->target = actor;}

        Item* const item = Map::player->getInv().getItemInSlot(SlotId::wielded);
        Weapon* const weapon = dynamic_cast<Weapon*>(item);
        if(eng.attack->ranged(*Map::player, *weapon, pos_) == false) {
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
    if(markerTask == MarkerTask::look) {
      eng.look->printExtraActorDescription(pos_);
      move(0, 0, MarkerTask::look, itemThrown);
    }
  }
  // ------------------------------------------------------- THROW
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 't') {
    if(markerTask == MarkerTask::aimThrownWeapon) {
      if(pos_ == Map::player->pos) {
        eng.log->addMsg("I should throw this somewhere else.");
      } else {
        Renderer::drawMapAndInterface();
        Actor* const actor = Utils::getActorAtPos(pos_, eng);
        if(actor != NULL) {Map::player->target = actor;}
        eng.thrower->throwItem(*Map::player, pos_, *itemThrown);
        data.didThrowMissile = true;
      }

      done();
    }
  }
  // ------------------------------------------------------- THROW EXPLOSIVE
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'e') {
    if(markerTask == MarkerTask::aimLitExplosive) {
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

  const Pos playerPos = Map::player->pos;
  eng.lineCalc->calcNewLine(playerPos, pos_, true, 9999, false, trail);

  if(markerTask == MarkerTask::aimRangedWeapon) {
    Item* const item =
      Map::player->getInv().getItemInSlot(SlotId::wielded);
    Weapon* const weapon = dynamic_cast<Weapon*>(item);
    effectiveRange = weapon->effectiveRangeLimit;
  }

  Renderer::drawMarker(trail, effectiveRange);
  Renderer::updateScreen();
}

MarkerReturnData Marker::run(const MarkerTask markerTask, Item* itemThrown) {
  pos_ = Map::player->pos;

  MarkerReturnData data;

  if(
    markerTask == MarkerTask::aimRangedWeapon  ||
    markerTask == MarkerTask::look             ||
    markerTask == MarkerTask::aimThrownWeapon) {
    //Attempt to place marker at target.
    if(setPosToTargetIfVisible() == false) {
      //Else NULL the target, and attempt to place marker at closest visible enemy.
      //This sets a new target if successful.
      Map::player->target = NULL;
      setPosToClosestEnemyIfVisible();
    }
  }

  if(
    markerTask == MarkerTask::look ||
    markerTask == MarkerTask::aimRangedWeapon ||
    markerTask == MarkerTask::aimThrownWeapon) {
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
  Map::player->getSpottedEnemies(SpottedEnemies);
  vector<Pos> spottedEnemiesPositions;

  Utils::getActorPositions(SpottedEnemies, spottedEnemiesPositions);

  //If player sees enemies, suggest one for targeting
  if(spottedEnemiesPositions.empty() == false) {
    pos_ = Utils::getClosestPos(Map::player->pos, spottedEnemiesPositions);

    Map::player->target = Utils::getActorAtPos(pos_, eng);
  }
}

bool Marker::setPosToTargetIfVisible() {
  const Actor* const target = Map::player->target;

  if(target != NULL) {
    vector<Actor*> SpottedEnemies;
    Map::player->getSpottedEnemies(SpottedEnemies);

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
      markerTask == MarkerTask::look             ||
      markerTask == MarkerTask::aimRangedWeapon  ||
      markerTask == MarkerTask::aimThrownWeapon) {
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

