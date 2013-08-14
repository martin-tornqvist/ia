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
#include "Render.h"
#include "ItemScroll.h"
#include "Map.h"
#include "ItemFactory.h"

void Marker::readKeys(const MarkerTask_t markerTask, MarkerReturnData& data,
                      Item* itemThrown) {
  const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

  if(d.sdlKey_ == SDLK_RIGHT || d.key_ == '6') {
    if(d.isShiftHeld_) {
      move(1, -1, markerTask, itemThrown);
    } else if(d.isCtrlHeld_) {
      move(1, 1, markerTask, itemThrown);
    } else {
      move(1, 0, markerTask, itemThrown);
    }
  }
  if(d.sdlKey_ == SDLK_UP || d.key_ == '8') {
    move(0, -1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_LEFT || d.key_ == '4') {
    if(d.isShiftHeld_) {
      move(-1, -1, markerTask, itemThrown);
    } else if(d.isCtrlHeld_) {
      move(-1, 1, markerTask, itemThrown);
    } else {
      move(-1, 0, markerTask, itemThrown);
    }
  }
  if(d.sdlKey_ == SDLK_DOWN || d.key_ == '2') {
    move(0, 1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_PAGEUP || d.key_ == '9') {
    move(1, -1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_HOME || d.key_ == '7') {
    move(-1, -1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_END || d.key_ == '1') {
    move(-1, 1, markerTask, itemThrown);
  }
  if(d.sdlKey_ == SDLK_PAGEDOWN || d.key_ == '3') {
    move(1, 1, markerTask, itemThrown);
  }
  // ------------------------------------------------------- AIM RANGED WEAPON
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'f') {
    if(markerTask == markerTask_aimRangedWeapon) {
      if(pos_ != eng->player->pos) {

        eng->log->clearLog();
        eng->renderer->drawMapAndInterface();

        const Actor* const actor = eng->mapTests->getActorAtPos(pos_);
        if(actor != NULL) {
          eng->player->target = actor;
        }

        Weapon* const weapon = dynamic_cast<Weapon*>(
                                 eng->player->getInventory()->getItemInSlot(slot_wielded));
        if(eng->attack->ranged(*eng->player, *weapon, pos_) == false) {
          eng->log->addMessage("No ammunition loaded.");
        }
      } else {
        eng->log->addMessage("I think I can persevere a little longer.");
      }
      done();
    }
  }
  // ------------------------------------------------------- LOOK
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'l') {
    if(markerTask == markerTask_look) {
      eng->look->printExtraActorDescription(pos_);
      move(0, 0, markerTask_look, itemThrown);
    }
  }
  // ------------------------------------------------------- THROW
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 't') {
    if(markerTask == markerTask_aimThrownWeapon) {
      if(pos_ == eng->player->pos) {
        eng->log->addMessage("I should throw this somewhere else.");
      } else {
        eng->renderer->drawMapAndInterface();
        const Actor* const actor = eng->mapTests->getActorAtPos(pos_);
        if(actor != NULL) {
          eng->player->target = actor;
        }
        eng->thrower->throwItem(*eng->player, pos_, *itemThrown);
        data.didThrowMissile = true;
      }

      done();
    }
  }
  // ------------------------------------------------------- THROW LIT EXPLOSIVE
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'e') {
    if(markerTask == markerTask_aimLitExplosive) {
      eng->renderer->drawMapAndInterface();
      eng->thrower->playerThrowLitExplosive(pos_);
      done();
    }
  }
  // ------------------------------------------------------- AZATHOTS BLAST SPELL
//  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'x') {
//    if(markerTask == markerTask_spellAzathothsBlast) {
//      if(eng->map->playerVision[pos_.x][pos_.y]) {
//        eng->renderer->drawMapAndInterface();
//        ScrollOfAzathothsBlast::castAt(pos_, eng);
//        done();
//      }
//    }
//  }
  if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
    cancel();
  }
}

void Marker::draw(const MarkerTask_t markerTask) const {
  vector<Pos> trace;
  trace.resize(0);

  int effectiveRange = -1;

//  if(markerTask == markerTask_spellAzathothsBlast) {
//    trace.push_back(Pos(pos_.x, pos_.y));
//  } else {
  const Pos playerPos = eng->player->pos;
  trace = eng->mapTests->getLine(playerPos, pos_, true, 99999);
//  }

  if(markerTask == markerTask_aimRangedWeapon) {
    Weapon* const weapon = dynamic_cast<Weapon*>(eng->player->getInventory()->getItemInSlot(slot_wielded));
    effectiveRange = weapon->effectiveRangeLimit;
  }

  eng->renderer->drawMarker(trace, effectiveRange);
}

MarkerReturnData Marker::run(const MarkerTask_t markerTask, Item* itemThrown) {
  pos_ = eng->player->pos;

  MarkerReturnData data;

  if(
    markerTask == markerTask_aimRangedWeapon  ||
    markerTask == markerTask_look             ||
    markerTask == markerTask_aimThrownWeapon) {
    //Attempt to place marker at target.
    if(setPosToTargetIfVisible() == false) {
      //Else NULL the target, and attempt to place marker at closest visible enemy.
      //This sets a new target if successful.
      eng->player->target = NULL;
      setPosToClosestEnemyIfVisible();
    }
  }

  eng->log->clearLog();

  eng->renderer->drawMapAndInterface(false);
  draw(markerTask);
  eng->renderer->updateScreen();

  if(
    markerTask == markerTask_look ||
    markerTask == markerTask_aimRangedWeapon ||
    markerTask == markerTask_aimThrownWeapon) {
    eng->look->markerAtPos(pos_, markerTask, itemThrown);
  }

  eng->renderer->drawMapAndInterface(false);
  draw(markerTask);
  eng->renderer->updateScreen();

  isDone_ = false;
  while(isDone_ == false) {
    readKeys(markerTask, data, itemThrown);
  }

  return data;
}

void Marker::setPosToClosestEnemyIfVisible() {
  vector<Actor*> spotedEnemies;
  eng->player->getSpotedEnemies(spotedEnemies);
  vector<Pos> spotedEnemiesPositions;
  eng->mapTests->getActorsPositions(spotedEnemies, spotedEnemiesPositions);

  //If player sees enemies, suggest one for targeting
  if(spotedEnemiesPositions.empty() == false) {
    pos_ = eng->mapTests->getClosestPos(eng->player->pos,
                                        spotedEnemiesPositions);

    const Actor* const actor = eng->mapTests->getActorAtPos(pos_);
    eng->player->target = actor;
  }
}

bool Marker::setPosToTargetIfVisible() {
  const Actor* const target = eng->player->target;

  if(target != NULL) {
    vector<Actor*> spotedEnemies;
    eng->player->getSpotedEnemies(spotedEnemies);

    if(spotedEnemies.empty() == false) {

      for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
        if(target == spotedEnemies.at(i)) {
          pos_ = spotedEnemies.at(i)->pos;
          return true;
        }
      }
    }
  }
  return false;
}

void Marker::move(const int DX, const int DY, const MarkerTask_t markerTask,
                  const Item* itemThrown) {
  bool isMoved = false;
  const Pos newPos = pos_ + Pos(DX, DY);
  if(eng->mapTests->isCellInsideMap(newPos)) {
    pos_ = newPos;
    isMoved = true;
  }

  if(isMoved) {
    if(
      markerTask == markerTask_look             ||
      markerTask == markerTask_aimRangedWeapon  ||
      markerTask == markerTask_aimThrownWeapon) {
      eng->look->markerAtPos(pos_, markerTask, itemThrown);
    }
  }

  eng->renderer->drawMapAndInterface(false);
  draw(markerTask);
  eng->renderer->updateScreen();
}

void Marker::done() {
  eng->renderer->drawMapAndInterface();
  isDone_ = true;
}

void Marker::cancel() {
  eng->log->clearLog();
  eng->renderer->drawMapAndInterface();
  isDone_ = true;
}

