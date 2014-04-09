#include "Input.h"

#include <memory>

#include "Engine.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "DungeonClimb.h"
#include "Input.h"
#include "Reload.h"
#include "Bash.h"
#include "Renderer.h"
#include "Close.h"
#include "JamWithSpike.h"
#include "ItemPickup.h"
#include "InventoryHandler.h"
#include "Marker.h"
#include "Map.h"
#include "DungeonMaster.h"
#include "PlayerSpellsHandler.h"
#include "Manual.h"
#include "CharacterDescr.h"
#include "Query.h"
#include "SaveHandler.h"
#include "ItemFactory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "PlayerBon.h"
#include "Examine.h"
#include "PlayerCreateCharacter.h"
#include "Disarm.h"
#include "SdlWrapper.h"
#include "Hide.h"
#include "Popup.h"

using namespace std;

namespace Input {

namespace {

SDL_Event* event_ = NULL;

void queryQuit(Engine& eng) {
  const vector<string> quitChoices = vector<string> {"yes", "no"};
  const int QUIT_CHOICE = eng.popup->showMenuMsg(
                            "Save and highscore are not kept.",
                            false, quitChoices, "Quit the current game?");
  if(QUIT_CHOICE == 0) {
    eng.quitToMainMenu_ = true;
    Renderer::clearScreen();
    Renderer::updateScreen();
  }
}

} //Namespace

void init() {
  if(event_ == NULL) {
    event_ = new SDL_Event;
  }
  setKeyRepeatDelays();
}

void cleanup() {
  if(event_ != NULL) {
    delete event_;
    event_ = NULL;
  }
}

void handleKeyPress(const KeyboardReadRetData& d, Engine& eng) {
  //----------------------------------- MOVEMENT
  if(d.sdlKey_ == SDLK_RIGHT            || d.key_ == '6' || d.key_ == 'l') {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();
      if(d.isShiftHeld_) {
        eng.player->moveDir(Dir::upRight);
      } else if(d.isCtrlHeld_) {
        eng.player->moveDir(Dir::downRight);
      } else {
        eng.player->moveDir(Dir::right);
      }
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_DOWN      || d.key_ == '2' || d.key_ == 'j') {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();
      eng.player->moveDir(Dir::down);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_LEFT      || d.key_ == '4' || d.key_ == 'h') {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();
      if(d.isShiftHeld_) {
        eng.player->moveDir(Dir::upLeft);
      } else if(d.isCtrlHeld_) {
        eng.player->moveDir(Dir::downLeft);
      } else {
        eng.player->moveDir(Dir::left);
      }
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_UP        || d.key_ == '8' || d.key_ == 'k') {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();
      eng.player->moveDir(Dir::up);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEUP    || d.key_ == '9' || d.key_ == 'u') {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();
      eng.player->moveDir(Dir::upRight);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEDOWN  || d.key_ == '3' || d.key_ == 'n') {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();
      eng.player->moveDir(Dir::downRight);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_END       || d.key_ == '1' || d.key_ == 'b') {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();
      eng.player->moveDir(Dir::downLeft);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_HOME      || d.key_ == '7' || d.key_ == 'y') {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();
      eng.player->moveDir(Dir::upLeft);
    }
    clearEvents();
    return;
  } else if(d.key_ == '5' || d.key_ == '.') {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();

      if(PlayerBon::hasTrait(Trait::steadyAimer)) {
        PropHandler& propHlr = eng.player->getPropHandler();

        int nrTurnsAimingOld = 0;

        if(PlayerBon::hasTrait(Trait::sharpShooter)) {
          Prop* const propAimingOld =
            propHlr.getProp(propAiming, PropSrc::applied);
          if(propAimingOld != NULL) {
            nrTurnsAimingOld =
              dynamic_cast<PropAiming*>(propAimingOld)->nrTurnsAiming;
          }
        }

        PropAiming* const aiming = new PropAiming(eng, propTurnsSpecific, 1);
        aiming->nrTurnsAiming += nrTurnsAimingOld;
        propHlr.tryApplyProp(aiming);
      }
      eng.player->moveDir(Dir::center);
    }
    clearEvents();
    return;
  }
  //----------------------------------- EXAMINE
  else if(d.key_ == 'a') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.examine->playerExamine();
    }
    clearEvents();
    return;
  }
  //----------------------------------- RELOAD
  else if(d.key_ == 'r') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.reload->reloadWieldedWpn(*(eng.player));
    }
    clearEvents();
    return;
  }
  //----------------------------------- BASH
  else if((d.key_ == 'q')) {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.bash->playerBash();
      Renderer::drawMapAndInterface();
    }
    clearEvents();
    return;
  }
  //----------------------------------- CLOSE
  else if(d.key_ == 'c') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.close->playerClose();
    }
    clearEvents();
    return;
  }
  //----------------------------------- JAM
  else if(d.key_ == 'D') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.jamWithSpike->playerJam();
    }
    clearEvents();
    return;
  }
  //----------------------------------- DISARM
  else if(d.key_ == 'd') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      Disarm::playerDisarm(eng);
    }
    clearEvents();
    return;
  }
  //----------------------------------- UNLOAD AMMO FROM GROUND
  else if(d.key_ == 'G')  {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.itemPickup->tryUnloadWeaponOrPickupAmmoFromGround();
    }
    clearEvents();
    return;
  }
  //----------------------------------- AIM/FIRE FIREARM
  else if(d.key_ == 'f') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {

      if(eng.player->getPropHandler().allowAttackRanged(true)) {

        Item* const item =
          eng.player->getInv().getItemInSlot(slot_wielded);

        if(item == NULL) {
          eng.log->addMsg("I am not wielding a weapon.");
        } else {
          const ItemData& itemData = item->getData();
          if(itemData.isRangedWeapon == false) {
            eng.log->addMsg("I am not wielding a firearm.");
          } else {
            Weapon* wpn = dynamic_cast<Weapon*>(item);
            if(wpn->nrAmmoLoaded >= 1 || itemData.rangedHasInfiniteAmmo) {
              eng.marker->run(MarkerTask::aimRangedWeapon, NULL);
            } else if(Config::isRangedWpnAutoReload()) {
              eng.reload->reloadWieldedWpn(*(eng.player));
            } else {
              eng.log->addMsg("There is no ammo loaded.");
            }
          }
        }
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- GET
  else if(d.key_ == 'g') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      const Pos& p = eng.player->pos;
      Item* const itemAtPlayer = eng.map->cells[p.x][p.y].item;
      if(itemAtPlayer != NULL) {
        if(itemAtPlayer->getData().id == ItemId::trapezohedron) {
          eng.dungeonMaster->winGame();
          eng.quitToMainMenu_ = true;
        }
      }
      if(eng.quitToMainMenu_ == false) {
        eng.itemPickup->tryPick();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- SLOTS SCREEN
  else if(d.key_ == 'w') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.inventoryHandler->runSlotsScreen();
    }
    clearEvents();
    return;
  }
  //----------------------------------- INVENTORY
  else if(d.key_ == 'i') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.inventoryHandler->runBrowseInventory();
    }
    clearEvents();
    return;
  }
  //----------------------------------- HIDE (ROGUE)
//  else if(d.key_ == 'h') {
//    eng.log->clearLog();
//    if(eng.player->deadState == ActorDeadState::alive) {
//      if(PlayerBon::getBg() == Bg::rogue) {
//        eng.hide->playerTryHide();
//      } else {
//        eng.log->addMsg("I do not have this ability.");
//      }
//    }
//    clearEvents();
//    return;
//  }
  //----------------------------------- USE
  else if(d.key_ == 'e') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      if(
        eng.player->dynamiteFuseTurns > 0 ||
        eng.player->flareFuseTurns > 0 ||
        eng.player->molotovFuseTurns > 0) {
        eng.marker->run(MarkerTask::aimLitExplosive, NULL);
      } else {
        eng.inventoryHandler->runUseScreen();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- SWAP TO PREPARED ITEM
  else if(d.key_ == 'z') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {

      const bool IS_FREE_TURN = false; //PlayerBon::hasTrait(Trait::traitnimble);

      const string swiftStr = IS_FREE_TURN ? " swiftly" : "";

      Item* const itemWielded = eng.player->getInv().getItemInSlot(slot_wielded);
      Item* const itemAlt = eng.player->getInv().getItemInSlot(slot_wieldedAlt);
      const string ITEM_WIELDED_NAME =
        itemWielded == NULL ? "" :
        eng.itemDataHandler->getItemRef(*itemWielded, ItemRefType::a);
      const string ITEM_ALT_NAME =
        itemAlt == NULL ? "" :
        eng.itemDataHandler->getItemRef(*itemAlt, ItemRefType::a);
      if(itemWielded == NULL && itemAlt == NULL) {
        eng.log->addMsg("I have neither a wielded nor a prepared weapon.");
      } else {
        if(itemWielded == NULL) {
          eng.log->addMsg(
            "I" + swiftStr + " wield my prepared weapon (" + ITEM_ALT_NAME + ").");
        } else {
          if(itemAlt == NULL) {
            eng.log->addMsg(
              "I" + swiftStr + " put away my weapon (" + ITEM_WIELDED_NAME + ").");
          } else {
            eng.log->addMsg(
              "I" + swiftStr + " swap to my prepared weapon (" + ITEM_ALT_NAME + ").");
          }
        }
        eng.player->getInv().swapWieldedAndPrepared(IS_FREE_TURN, eng);
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- SEARCH (REALLY JUST A WAIT BUTTON)
  else if(d.key_ == 's') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      vector<Actor*> SpottedEnemies;
      eng.player->getSpottedEnemies(SpottedEnemies);
      if(SpottedEnemies.empty()) {
        const int TURNS_TO_APPLY = 5;
        const string TURNS_STR = toStr(TURNS_TO_APPLY);
        eng.log->addMsg("I pause for a while (" + TURNS_STR + " turns).");
        eng.player->waitTurnsLeft = TURNS_TO_APPLY - 1;
        eng.gameTime->actorDidAct();
      } else {
        eng.log->addMsg("Not while an enemy is near.");
        Renderer::drawMapAndInterface();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- THROW ITEM
  else if(d.key_ == 't') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {

      if(eng.player->getPropHandler().allowAttackRanged(true)) {
        Inventory& playerInv = eng.player->getInv();
        Item* itemStack = playerInv.getItemInSlot(slot_missiles);

        if(itemStack == NULL) {
          eng.log->addMsg(
            "I have no missiles chosen for throwing (press 'w').");
        } else {
          Item* itemToThrow = eng.itemFactory->copyItem(itemStack);
          itemToThrow->nrItems = 1;

          const MarkerReturnData markerReturnData =
            eng.marker->run(MarkerTask::aimThrownWeapon, itemToThrow);

          if(markerReturnData.didThrowMissile) {
            playerInv.decrItemInSlot(slot_missiles);
          } else {
            delete itemToThrow;
          }
        }
      }
    }
    clearEvents();
    return;
  }
  //-----------------------------------  VIEW DESCRIPTIONS
  else if(d.key_ == 'v') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      if(eng.player->getPropHandler().allowSee()) {
        eng.marker->run(MarkerTask::look, NULL);
      } else {
        eng.log->addMsg("Not while blind.");
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- AUTO MELEE
  else if(d.sdlKey_ == SDLK_TAB) {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.player->autoMelee();
    }
    clearEvents();
    return;
  }
  //----------------------------------- RE-CAST PREVIOUS MEMORIZED SPELL
  else if(d.key_ == 'x') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.playerSpellsHandler->tryCastPrevSpell();
    }
    clearEvents();
    return;
  }
  //----------------------------------- MEMORIZED SPELLS
  else if(d.key_ == 'X') {
    eng.log->clearLog();
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.playerSpellsHandler->playerSelectSpellToCast();
    }
    clearEvents();
    return;
  }
  //----------------------------------- CHARACTER INFO
  else if(d.key_ == '@') {
    eng.characterDescr->run();
    clearEvents();
    return;
  }
  //----------------------------------- LOG HISTORY
  else if(d.key_ == 'm') {
    eng.log->displayHistory();
    clearEvents();
    return;
  }
  //----------------------------------- MENU
  else if(d.sdlKey_ == SDLK_ESCAPE) {
    if(eng.player->deadState == ActorDeadState::alive) {
      eng.log->clearLog();

      const vector<string> choices {"Options", "Manual", "Quit", "Cancel"};
      const int CHOICE = eng.popup->showMenuMsg("", true, choices);

      if(CHOICE == 0) {
        //---------------------------- Options
        Config::runOptionsMenu(eng);
        Renderer::drawMapAndInterface();
      } else if(CHOICE == 1) {
        //---------------------------- Manual
        eng.manual->run();
        Renderer::drawMapAndInterface();
      } else if(CHOICE == 2) {
        //---------------------------- Quit
        queryQuit(eng);
      }
    } else {
      eng.quitToMainMenu_ = true;
    }
    clearEvents();
    return;
  } else if(d.key_ == 'Q' && IS_DEBUG_MODE) {
    //----------------------------------- MENU
    queryQuit(eng);
    clearEvents();
    return;
  }
  //----------------------------------- DESCEND CHEAT
  else if(d.sdlKey_ == SDLK_F2) {
    if(IS_DEBUG_MODE) {
      eng.dungeonClimb->travelDown(1);
      clearEvents();
    }
    return;
  }
  //----------------------------------- XP CHEAT
  else if(d.sdlKey_ == SDLK_F3) {
    if(IS_DEBUG_MODE) {
      eng.dungeonMaster->playerGainXp(100);
      clearEvents();
    }
    return;
  }
  //----------------------------------- VISION CHEAT
  else if(d.sdlKey_ == SDLK_F4) {
    if(IS_DEBUG_MODE) {
      if(eng.isCheatVisionEnabled_) {
        for(int y = 0; y < MAP_H; y++) {
          for(int x = 0; x < MAP_W; x++) {
            eng.map->cells[x][y].isSeenByPlayer = false;
            eng.map->cells[x][y].isExplored     = false;
          }
        }
        eng.isCheatVisionEnabled_ = false;
      } else {
        eng.isCheatVisionEnabled_ = true;
      }
      eng.player->updateFov();
      Renderer::drawMapAndInterface();
    }
    clearEvents();
  }
  //----------------------------------- INSANITY CHEAT
  else if(d.sdlKey_ == SDLK_F5) {
    if(IS_DEBUG_MODE) {
      eng.player->incrShock(50, ShockSrc::misc);
      clearEvents();
    }
    return;
  }
  //----------------------------------- DROP ITEMS AROUND PLAYER
  else if(d.sdlKey_ == SDLK_F6) {
    if(IS_DEBUG_MODE) {
      for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
        const ItemData* const data = eng.itemDataHandler->dataList[i];
        if(
          data->isIntrinsic == false &&
          (data->isPotion || data->isScroll || data->isDevice)) {
          eng.itemFactory->spawnItemOnMap((ItemId)(i), eng.player->pos);
        }
      }
      clearEvents();
    }
    return;
  }
  //----------------------------------- TELEPORT
  else if(d.sdlKey_ == SDLK_F7) {
    if(IS_DEBUG_MODE) {
      eng.player->teleport(false);
      eng.log->clearLog();
      clearEvents();
    }
    return;
  }
  //----------------------------------- INFECTED
  else if(d.sdlKey_ == SDLK_F8) {
    if(IS_DEBUG_MODE) {
      eng.player->getPropHandler().tryApplyProp(
        new PropInfected(eng, propTurnsStd));
      clearEvents();
    }
    return;
  }

  //----------------------------------- POSSESSED BY ZUUL
  else if(d.sdlKey_ == SDLK_F9) {
    if(IS_DEBUG_MODE) {
      for(Actor * actor : eng.gameTime->actors_) {
        actor->getPropHandler().tryApplyProp(
          new PropPossessedByZuul(eng, propTurnsIndefinite), true);
      }
    }
    return;
  }

  //----------------------------------- UNDEFINED COMMANDS
  else if(d.key_ != -1) {
    string cmdTried = " ";
    cmdTried.at(0) = d.key_;
    eng.log->clearLog();
    eng.log->addMsg("Unknown command '" + cmdTried + "'.");
    clearEvents();
    return;
  }
}

void setKeyRepeatDelays() {
  trace << "Input::setKeyRepeatDelays()..." << endl;
  const int DELAY     = Config::getKeyRepeatDelay();
  const int INTERVAL  = Config::getKeyRepeatInterval();
  SDL_EnableKeyRepeat(DELAY, INTERVAL);
  trace << "Input::setKeyRepeatDelays() [DONE]" << endl;
}

void handleMapModeInputUntilFound(Engine& eng) {
  if(event_ != NULL) {
    const KeyboardReadRetData& d = readKeysUntilFound(eng);
    if(eng.quitToMainMenu_ == false) {
      handleKeyPress(d, eng);
    }
  }
}

void clearEvents() {
  if(event_ != NULL) {while(SDL_PollEvent(event_)) {}}
}

KeyboardReadRetData readKeysUntilFound(Engine& eng, const bool IS_O_RETURN) {
  if(event_ == NULL) {
    return KeyboardReadRetData();
  }

  while(true) {

    SdlWrapper::sleep(1);

    while(SDL_PollEvent(event_)) {
      if(event_->type == SDL_QUIT) {
        return KeyboardReadRetData(SDLK_ESCAPE);
      } else if(event_->type == SDL_KEYDOWN) {
        // ASCII char entered?
        // Decimal unicode:
        // '!' = 33
        // '~' = 126
        Uint16 unicode = event_->key.keysym.unicode;
        if((unicode == 'o' || unicode == 'O') && IS_O_RETURN) {
          return KeyboardReadRetData(-1, SDLK_RETURN, unicode == 'O', false);
        } else if(unicode >= 33 && unicode < 126) {
          clearEvents();
          return KeyboardReadRetData(char(unicode));
        } else {
          //Other key pressed? (escape, return, space, etc)
          const SDLKey sdlKey = event_->key.keysym.sym;

          //Don't register shift, control or alt as actual key events
          if(
            sdlKey == SDLK_LSHIFT ||
            sdlKey == SDLK_RSHIFT ||
            sdlKey == SDLK_LCTRL  ||
            sdlKey == SDLK_RCTRL  ||
            sdlKey == SDLK_LALT   ||
            sdlKey == SDLK_RALT) {
            continue;
          }

          SDLMod mod = SDL_GetModState();
          const bool IS_SHIFT_HELD    = mod & KMOD_SHIFT;
          const bool IS_CTRL_HELD     = mod & KMOD_CTRL;
          const bool IS_ALT_HELD      = mod & KMOD_ALT;

          KeyboardReadRetData ret(-1, sdlKey, IS_SHIFT_HELD, IS_CTRL_HELD);

          if(sdlKey >= SDLK_F1 && sdlKey <= SDLK_F15) {
            //F-keys
            return ret;
          } else {
            switch(sdlKey) {
              case SDLK_RETURN:
              case SDLK_KP_ENTER: {
                if(IS_ALT_HELD) {
                  Config::toggleFullscreen(eng);
                  clearEvents();
                  continue;
                } else {
                  ret.sdlKey_ = SDLK_RETURN;
                  return ret;
                }
              } break;
              case SDLK_MENU:         continue;   break;
              case SDLK_PAUSE:        continue;   break;
              case SDLK_SPACE:        return ret; break;
              case SDLK_BACKSPACE:    return ret; break;
              case SDLK_TAB:          return ret; break;
              case SDLK_PAGEUP:       return ret; break;
              case SDLK_PAGEDOWN:     return ret; break;
              case SDLK_END:          return ret; break;
              case SDLK_HOME:         return ret; break;
              case SDLK_INSERT:       return ret; break;
              case SDLK_DELETE:       return ret; break;
              case SDLK_LEFT:         return ret; break;
              case SDLK_RIGHT:        return ret; break;
              case SDLK_UP:           return ret; break;
              case SDLK_DOWN:         return ret; break;
              case SDLK_ESCAPE:       return ret; break;
              default:                continue;   break;
            }
          }
        }
      }
    }
  }
}

} //Input

