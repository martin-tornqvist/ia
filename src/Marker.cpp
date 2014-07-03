#include "Marker.h"

#include <vector>

#include "Input.h"
#include "InventoryHandling.h"
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

using namespace std;

namespace Marker {

namespace {

Pos   pos_;
bool  isDone_ = false;

void done() {
  Renderer::drawMapAndInterface();
  isDone_ = true;
}

void cancel() {
  Log::clearLog();
  Renderer::drawMapAndInterface();
  isDone_ = true;
}

void setPosToClosestEnemyIfVisible() {
  vector<Actor*> spottedEnemies;
  Map::player->getSpottedEnemies(spottedEnemies);
  vector<Pos> spottedEnemiesPositions;

  Utils::getActorPositions(spottedEnemies, spottedEnemiesPositions);

  //If player sees enemies, suggest one for targeting
  if(!spottedEnemiesPositions.empty()) {
    pos_ = Utils::getClosestPos(Map::player->pos, spottedEnemiesPositions);

    Map::player->target = Utils::getActorAtPos(pos_);
  }
}

void move(const int DX, const int DY, const MarkerTask markerTask,
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
      Look::onMarkerAtPos(pos_, markerTask, itemThrown);
    }
  }

  draw(markerTask);
}

bool setPosToTargetIfVisible() {
  const Actor* const target = Map::player->target;

  if(target) {
    vector<Actor*> spottedEnemies;
    Map::player->getSpottedEnemies(spottedEnemies);

    if(!spottedEnemies.empty()) {

      for(size_t i = 0; i < spottedEnemies.size(); ++i) {
        if(target == spottedEnemies.at(i)) {
          pos_ = spottedEnemies.at(i)->pos;
          return true;
        }
      }
    }
  }
  return false;
}

void readKeys(const MarkerTask markerTask, MarkerRetData& data,
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

        Log::clearLog();
        Renderer::drawMapAndInterface();

        Actor* const actor = Utils::getActorAtPos(pos_);
        if(actor) {Map::player->target = actor;}

        Item* const item = Map::player->getInv().getItemInSlot(SlotId::wielded);
        Weapon* const weapon = static_cast<Weapon*>(item);
        if(!Attack::ranged(*Map::player, *weapon, pos_)) {
          Log::addMsg("No ammunition loaded.");
        }
      } else {
        Log::addMsg("I think I can persevere a little longer.");
      }
      done();
    }
  }
  // ------------------------------------------------------- LOOK
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'v') {
    if(markerTask == MarkerTask::look) {
      Look::printExtraActorDescription(pos_);
      move(0, 0, MarkerTask::look, itemThrown);
    }
  }
  // ------------------------------------------------------- THROW
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 't') {
    if(markerTask == MarkerTask::aimThrownWeapon) {
      if(pos_ == Map::player->pos) {
        Log::addMsg("I should throw this somewhere else.");
      } else {
        Renderer::drawMapAndInterface();
        Actor* const actor = Utils::getActorAtPos(pos_);
        if(actor) {Map::player->target = actor;}
        Throwing::throwItem(*Map::player, pos_, *itemThrown);
        data.didThrowMissile = true;
      }

      done();
    }
  }
  // ------------------------------------------------------- THROW EXPLOSIVE
  if(d.sdlKey_ == SDLK_RETURN || d.key_ == 'e') {
    if(markerTask == MarkerTask::aimLitExplosive) {
      Renderer::drawMapAndInterface();
      Throwing::playerThrowLitExplosive(pos_);
      done();
    }
  }
  if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
    cancel();
  }
}

} //namespace

const Pos& getPos() {return pos_;}

void draw(const MarkerTask markerTask) {
  Renderer::drawMapAndInterface(false);

  vector<Pos> trail;

  int effectiveRange = -1;

  const Pos playerPos = Map::player->pos;
  LineCalc::calcNewLine(playerPos, pos_, true, 9999, false, trail);

  if(markerTask == MarkerTask::aimRangedWeapon) {
    Item* const item =
      Map::player->getInv().getItemInSlot(SlotId::wielded);
    Weapon* const weapon = static_cast<Weapon*>(item);
    effectiveRange = weapon->effectiveRangeLimit;
  }

  Renderer::drawMarker(trail, effectiveRange);
  Renderer::updateScreen();
}

MarkerRetData run(const MarkerTask markerTask, Item* itemThrown) {
  pos_ = Map::player->pos;

  MarkerRetData data;

  if(
    markerTask == MarkerTask::aimRangedWeapon  ||
    markerTask == MarkerTask::look             ||
    markerTask == MarkerTask::aimThrownWeapon) {
    //Attempt to place marker at target.
    if(!setPosToTargetIfVisible()) {
      //Else attempt to place marker at closest visible enemy.
      //This sets a new target if successful.
      Map::player->target = nullptr;
      setPosToClosestEnemyIfVisible();
    }
  }

  if(
    markerTask == MarkerTask::look ||
    markerTask == MarkerTask::aimRangedWeapon ||
    markerTask == MarkerTask::aimThrownWeapon) {
    Look::onMarkerAtPos(pos_, markerTask, itemThrown);
  }

  draw(markerTask);

  isDone_ = false;
  while(!isDone_) {readKeys(markerTask, data, itemThrown);}

  return data;
}

} //Marker
