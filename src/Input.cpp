#include "Input.h"

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
#include "PlayerBonuses.h"
#include "Examine.h"
#include "PlayerCreateCharacter.h"
#include "Disarm.h"
#include "SdlWrapper.h"
#include "Hide.h"

Input::Input(Engine& engine, bool* quitToMainMenu) :
  eng(engine), quitToMainMenu_(quitToMainMenu)  {
  setKeyRepeatDelays();
}

void Input::setKeyRepeatDelays() {
  trace << "Input::setKeyRepeatDelays()..." << endl;
  SDL_EnableKeyRepeat(
    eng.config->keyRepeatDelay, eng.config->keyRepeatInterval);
  trace << "Input::setKeyRepeatDelays() [DONE]" << endl;
}

void Input::handleMapModeInputUntilFound() {
  const KeyboardReadReturnData& d = readKeysUntilFound();
  handleKeyPress(d);
}

void Input::handleKeyPress(const KeyboardReadReturnData& d) {
  //----------------------------------- MOVEMENT
  if(d.sdlKey_ == SDLK_RIGHT || d.key_ == '6') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();
      if(d.isShiftHeld_) {
        eng.player->moveDir(dirUpRight);
      } else if(d.isCtrlHeld_) {
        eng.player->moveDir(dirDownRight);
      } else {
        eng.player->moveDir(dirRight);
      }
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_DOWN || d.key_ == '2') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();
      eng.player->moveDir(dirDown);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_LEFT || d.key_ == '4') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();
      if(d.isShiftHeld_) {
        eng.player->moveDir(dirUpLeft);
      } else if(d.isCtrlHeld_) {
        eng.player->moveDir(dirDownLeft);
      } else {
        eng.player->moveDir(dirLeft);
      }
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_UP || d.key_ == '8') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();
      eng.player->moveDir(dirUp);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEUP || d.key_ == '9') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();
      eng.player->moveDir(dirUpRight);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEDOWN || d.key_ == '3') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();
      eng.player->moveDir(dirDownRight);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_END || d.key_ == '1') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();
      eng.player->moveDir(dirDownLeft);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_HOME || d.key_ == '7') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();
      eng.player->moveDir(dirUpLeft);
    }
    clearEvents();
    return;
  } else if(d.key_ == '5' || d.key_ == '.') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();

      PlayerBonHandler& bonHlr = *eng.playerBonHandler;
      if(bonHlr.hasTrait(traitSteadyAimer)) {
        PropHandler& propHlr = eng.player->getPropHandler();

        int nrTurnsAimingOld = 0;

        if(bonHlr.hasTrait(traitSharpShooter)) {
          Prop* const propAimingOld = propHlr.getAppliedProp(propAiming);
          if(propAimingOld != NULL) {
            nrTurnsAimingOld =
              dynamic_cast<PropAiming*>(propAimingOld)->nrTurnsAiming;
          }
        }

        PropAiming* const aiming = new PropAiming(eng, propTurnsSpecified, 1);
        aiming->nrTurnsAiming += nrTurnsAimingOld;
        propHlr.tryApplyProp(aiming);
      }
      eng.player->moveDir(dirCenter);
    }
    clearEvents();
    return;
  }
  //----------------------------------- EXAMINE
  else if(d.key_ == 'a') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      eng.examine->playerExamine();
    }
    clearEvents();
    return;
  }
  //----------------------------------- RELOAD
  else if(d.key_ == 'r') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      eng.reload->reloadWieldedWpn(*(eng.player));
    }
    clearEvents();
    return;
  }
  //----------------------------------- BASH
  else if(d.key_ == 'b') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      eng.bash->playerBash();
      eng.renderer->drawMapAndInterface();
    }
    clearEvents();
    return;
  }
  //----------------------------------- CLOSE
  else if(d.key_ == 'c') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      eng.close->playerClose();
    }
    clearEvents();
    return;
  }
  //----------------------------------- JAM
  else if(d.key_ == 'j') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      eng.jamWithSpike->playerJam();
    }
    clearEvents();
    return;
  }
  //----------------------------------- DISARM
  else if(d.key_ == 'd') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      Disarm::playerDisarm(eng);
    }
    clearEvents();
    return;
  }
  //----------------------------------- UNLOAD AMMO FROM GROUND
  else if(d.key_ == 'u') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      eng.itemPickup->tryUnloadWeaponOrPickupAmmoFromGround();
    }
    clearEvents();
    return;
  }
  //----------------------------------- AIM/FIRE FIREARM
  else if(d.key_ == 'f') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {

      if(eng.player->getPropHandler().allowAttackRanged(true)) {

        Item* const item =
          eng.player->getInv().getItemInSlot(slot_wielded);

        if(item == NULL) {
          eng.log->addMsg("I am not wielding a weapon.");
        } else {
          Weapon* wpn = dynamic_cast<Weapon*>(item);
          if(
            wpn->nrAmmoLoaded >= 1 ||
            wpn->getData().rangedHasInfiniteAmmo
          ) {
            eng.marker->run(markerTask_aimRangedWeapon, NULL);
          } else if(eng.config->useRangedWpnAutoReload) {
            eng.reload->reloadWieldedWpn(*(eng.player));
          } else {
            eng.log->addMsg("There is no ammo loaded.");
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
    if(eng.player->deadState == actorDeadState_alive) {
      const Pos& p = eng.player->pos;
      Item* const itemAtPlayer = eng.map->cells[p.x][p.y].item;
      if(itemAtPlayer != NULL) {
        if(itemAtPlayer->getData().id == item_trapezohedron) {
          eng.dungeonMaster->winGame();
          *quitToMainMenu_ = true;
        }
      }
      if(*quitToMainMenu_ == false) {
        eng.itemPickup->tryPick();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- SLOTS SCREEN
  else if(d.key_ == 'w') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      eng.inventoryHandler->runSlotsScreen();
    }
    clearEvents();
    return;
  }
  //----------------------------------- INVENTORY
  else if(d.key_ == 'i') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      eng.inventoryHandler->runBrowseInventory();
    }
    clearEvents();
    return;
  }
  //----------------------------------- HIDE (ROGUE)
//  else if(d.key_ == 'h') {
//    eng.log->clearLog();
//    if(eng.player->deadState == actorDeadState_alive) {
//      if(eng.playerBonHandler->getBg() == bgRogue) {
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
    if(eng.player->deadState == actorDeadState_alive) {
      if(
        eng.player->dynamiteFuseTurns > 0 ||
        eng.player->flareFuseTurns > 0 ||
        eng.player->molotovFuseTurns > 0) {
        eng.marker->run(markerTask_aimLitExplosive, NULL);
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
    if(eng.player->deadState == actorDeadState_alive) {

      const bool IS_FREE_TURN = false; //eng.playerBonHandler->hasTrait(traitnimble);

      const string swiftStr = IS_FREE_TURN ? " swiftly" : "";

      Item* const itemWielded = eng.player->getInv().getItemInSlot(slot_wielded);
      Item* const itemAlt = eng.player->getInv().getItemInSlot(slot_wieldedAlt);
      const string ITEM_WIELDED_NAME =
        itemWielded == NULL ? "" :
        eng.itemDataHandler->getItemRef(*itemWielded, itemRef_a);
      const string ITEM_ALT_NAME =
        itemAlt == NULL ? "" :
        eng.itemDataHandler->getItemRef(*itemAlt, itemRef_a);
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
    if(eng.player->deadState == actorDeadState_alive) {
      vector<Actor*> SpottedEnemies;
      eng.player->getSpottedEnemies(SpottedEnemies);
      if(SpottedEnemies.empty()) {
        const int TURNS_TO_APPLY = 5;
        const string TURNS_STR = toString(TURNS_TO_APPLY);
        eng.log->addMsg("I pause for a while (" + TURNS_STR + " turns).");
        eng.player->waitTurnsLeft = TURNS_TO_APPLY - 1;
        eng.gameTime->actorDidAct();
      } else {
        eng.log->addMsg("Not while an enemy is near.");
        eng.renderer->drawMapAndInterface();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- THROW ITEM
  else if(d.key_ == 't') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {

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
            eng.marker->run(markerTask_aimThrownWeapon, itemToThrow);

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
  //-----------------------------------  LOOK AROUND
  else if(d.key_ == 'l') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      if(eng.player->getPropHandler().allowSee()) {
        eng.marker->run(markerTask_look, NULL);
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
    if(eng.player->deadState == actorDeadState_alive) {
      eng.player->autoMelee();
    }
    clearEvents();
    return;
  }
  //----------------------------------- POWERS - MEMORIZED
  else if(d.key_ == 'x') {
    eng.log->clearLog();
    if(eng.player->deadState == actorDeadState_alive) {
      if(eng.player->getPropHandler().allowRead(true)) {
        eng.playerSpellsHandler->run();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- MANUAL
  else if(d.key_ == '?') {
    eng.manual->run();
    eng.renderer->drawMapAndInterface();
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
  else if(d.key_ == 'L') {
    eng.log->displayHistory();
    clearEvents();
    return;
  }
  //----------------------------------- QUIT
  else if(d.sdlKey_ == SDLK_ESCAPE || d.key_ == 'Q') {
    if(eng.player->deadState == actorDeadState_alive) {
      eng.log->clearLog();
      eng.log->addMsg(
        "Quit the current game (y/n)? Save and highscore are not kept.",
        clrWhiteHigh);
      eng.renderer->drawMapAndInterface();
      if(eng.query->yesOrNo()) {
        *quitToMainMenu_ = true;
      } else {
        eng.log->clearLog();
        eng.renderer->drawMapAndInterface();
      }
    } else {
      *quitToMainMenu_ = true;
    }
    clearEvents();
    return;
  }
  //----------------------------------- SAVE AND QUIT
  else if(d.key_ == 'S') {
    if(eng.player->deadState == actorDeadState_alive) {
      const Pos& p = eng.player->pos;
      const FeatureStatic* const f = eng.map->cells[p.x][p.y].featureStatic;
      if(f->getId() == feature_stairsDown) {
        eng.log->clearLog();
        eng.log->addMsg("Save and quit (y/n)?", clrWhiteHigh);
        eng.renderer->drawMapAndInterface();
        if(eng.query->yesOrNo()) {
          eng.saveHandler->save();
          *quitToMainMenu_ = true;
        } else {
          eng.log->clearLog();
          eng.renderer->drawMapAndInterface();
        }
      } else {
        eng.log->clearLog();
        eng.renderer->drawMapAndInterface();
      }
    } else {
      eng.log->addMsg("Saving can only be done on stairs.");
      eng.renderer->drawMapAndInterface();
    }
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
      if(eng.isCheatVisionEnabled) {
        for(int y = 0; y < MAP_H; y++) {
          for(int x = 0; x < MAP_W; x++) {
            eng.map->cells[x][y].isSeenByPlayer = false;
            eng.map->cells[x][y].isExplored     = false;
          }
        }
        eng.isCheatVisionEnabled = false;
      } else {
        eng.isCheatVisionEnabled = true;
      }
      eng.player->updateFov();
      eng.renderer->drawMapAndInterface();
    }
    clearEvents();
  }
  //----------------------------------- INSANITY CHEAT
  else if(d.sdlKey_ == SDLK_F5) {
    if(IS_DEBUG_MODE) {
      eng.player->incrShock(50, shockSrc_misc);
      clearEvents();
    }
    return;
  }
  //----------------------------------- MTH CHEAT
  else if(d.sdlKey_ == SDLK_F6) {
    if(IS_DEBUG_MODE) {
      eng.player->incrMth(8, false);
      clearEvents();
    }
    return;
  }
  //----------------------------------- DROP ALL SCROLLS AND POTIONS ON PLAYER
  else if(d.sdlKey_ == SDLK_F7) {
    if(IS_DEBUG_MODE) {
      for(unsigned int i = 1; i < endOfItemIds; i++) {
        const ItemData* const data = eng.itemDataHandler->dataList[i];
        if(
          data->isIntrinsic == false &&
          (data->isPotion || data->isScroll)) {
          eng.itemFactory->spawnItemOnMap((ItemId)(i), eng.player->pos);
        }
      }
      clearEvents();
    }
    return;
  }
  //----------------------------------- TELEPORT
  else if(d.sdlKey_ == SDLK_F8) {
    if(IS_DEBUG_MODE) {
      eng.player->teleport(false);
      eng.log->clearLog();
      clearEvents();
    }
    return;
  }
  //----------------------------------- INFECTED
  else if(d.sdlKey_ == SDLK_F9) {
    if(IS_DEBUG_MODE) {
      eng.player->getPropHandler().tryApplyProp(
        new PropInfected(eng, propTurnsStd));
      clearEvents();
    }
    return;
  }

  //----------------------------------- POSSESSED BY ZUUL
  else if(d.sdlKey_ == SDLK_F10) {
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
    eng.log->addMsg(
      "Unknown command '" + cmdTried + "'. Press '?' for commands.");
    clearEvents();
    return;
  }
}

void Input::clearEvents() {
  while(SDL_PollEvent(&event_)) {}
}

KeyboardReadReturnData Input::readKeysUntilFound() {
  while(true) {

    eng.sleep(1);

    while(SDL_PollEvent(&event_)) {
      //Closing the window sends escape key event
      if(event_.type == SDL_QUIT) {
        return KeyboardReadReturnData(SDLK_ESCAPE);
      } else if(event_.type == SDL_KEYDOWN) {
        // ASCII char entered?
        // Decimal unicode:
        // '!' = 33
        // '~' = 126
        const Uint16 UNICODE = event_.key.keysym.unicode;
        if(UNICODE >= 33 && UNICODE < 126) {
          clearEvents();
          return KeyboardReadReturnData(char(UNICODE));
        } else {
          // Other key pressed? (escape, return, space, etc)
          const SDLKey sdlKey = event_.key.keysym.sym;

          // Don't register shift, control or alt as actual key events
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

          KeyboardReadReturnData ret(-1, sdlKey, IS_SHIFT_HELD, IS_CTRL_HELD);

          if(sdlKey >= SDLK_F1 && sdlKey <= SDLK_F15) {
            // F-keys
            return ret;
          } else {
            switch(sdlKey) {
              case SDLK_RETURN:
              case SDLK_KP_ENTER: {
                if(IS_ALT_HELD) {
                  eng.config->toggleFullscreen();
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
