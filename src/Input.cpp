#include "Input.h"

#include "SDL/SDL_Keyboard.h"

#include "Engine.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "DungeonClimb.h"
#include "Input.h"
#include "Reload.h"
#include "Bash.h"
#include "Render.h"
#include "Close.h"
#include "JamWithSpike.h"
#include "ItemPickup.h"
#include "InventoryHandler.h"
#include "Marker.h"
#include "Map.h"
#include "DungeonMaster.h"
#include "PlayerPowersHandler.h"
#include "Manual.h"
#include "CharacterInfo.h"
#include "Query.h"
#include "SaveHandler.h"
#include "ItemFactory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "PlayerBonuses.h"
#include "Examine.h"
#include "PlayerCreateCharacter.h"
#include "Disarm.h"

void Input::clearLogMessages() {
  if(eng->player->deadState == actorDeadState_alive) {
    eng->log->clearLog();
  }
}

Input::Input(Engine* engine, bool* quitToMainMenu) : eng(engine), quitToMainMenu_(quitToMainMenu)  {
  SDL_EnableKeyRepeat(110, 50);
}

void Input::handleMapModeInputUntilFound() {
  const KeyboardReadReturnData& d = readKeysUntilFound();
  handleKeyPress(d);
}

void Input::handleKeyPress(const KeyboardReadReturnData& d) {
  //----------------------------------------MOVEMENT
  if(d.sdlKey_ == SDLK_RIGHT || d.key_ == '6') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      if(d.isShiftHeld_) {
        eng->player->moveDirection(1, -1);
      } else if(d.isCtrlHeld_) {
        eng->player->moveDirection(1, 1);
      } else {
        eng->player->moveDirection(1, 0);
      }
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_DOWN || d.key_ == '2') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(0, 1);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_LEFT || d.key_ == '4') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      if(d.isShiftHeld_) {
        eng->player->moveDirection(-1, -1);
      } else if(d.isCtrlHeld_) {
        eng->player->moveDirection(-1, 1);
      } else {
        eng->player->moveDirection(-1, 0);
      }
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_UP || d.key_ == '8') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(0, -1);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEUP || d.key_ == '9') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(1, -1);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEUP || d.key_ == '3') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(1, 1);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEUP || d.key_ == '1') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(-1, 1);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEUP || d.key_ == '7') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(-1, -1);
    }
    clearEvents();
    return;
  } else if(d.key_ == '5' || d.key_ == '.') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(0, 0);
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_steadyAimer)) {
        eng->player->getStatusEffectsHandler()->tryAddEffect(new StatusStill(1));
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------DESCEND
  else if(d.key_ == '>') {
    tracer << "Input: User pressed '>'" << endl;
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      tracer << "Input: Calling DungeonClimb::tryUseDownStairs()" << endl;
      eng->dungeonClimb->tryUseDownStairs();
    }
    clearEvents();
    return;
  }
  //----------------------------------------EXAMINE
  else if(d.key_ == 'a') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->player->getStatusEffectsHandler()->allowSee()) {
        eng->examine->playerExamine();
        eng->renderer->drawMapAndInterface();
      } else {
        eng->log->addMessage("Not while blind.");
        eng->renderer->drawMapAndInterface();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------RELOAD
  else if(d.key_ == 'r') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->reload->reloadWeapon(eng->player);
    }
    clearEvents();
    return;
  }
  //----------------------------------------BASH
  else if(d.key_ == 'b') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->bash->playerBash();
      eng->renderer->drawMapAndInterface();
    }
    clearEvents();
    return;
  }
  //----------------------------------------CLOSE
  else if(d.key_ == 'c') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->close->playerClose();
      eng->renderer->drawMapAndInterface();
    }
    clearEvents();
    return;
  }
  //----------------------------------------JAM
  else if(d.key_ == 'j') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->jamWithSpike->playerJam();
      eng->renderer->drawMapAndInterface();
    }
    clearEvents();
    return;
  }
  //----------------------------------------DISARM
//  else if(d.key_ == 'd') {
//    clearLogMessages();
//    if(eng->player->deadState == actorDeadState_alive) {
//      if(eng->player->getStatusEffectsHandler()->allowSee()) {
//        eng->disarm->playerDisarm();
//        eng->renderer->drawMapAndInterface();
//      } else {
//      }
//    }
//    clearEvents();
//    return;
//  }
  //----------------------------------------UNLOAD AMMO FROM GROUND
  else if(d.key_ == 'u') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->itemPickup->tryUnloadWeaponOrPickupAmmoFromGround();
    }
    clearEvents();
    return;
  }
  //----------------------------------------AIM/FIRE FIREARM
  else if(d.key_ == 'f') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {

      if(eng->player->getStatusEffectsHandler()->allowAttackRanged(true)) {

        Weapon* firearm = NULL;

        firearm = dynamic_cast<Weapon*>(eng->player->getInventory()->getItemInSlot(slot_wielded));

        if(firearm != NULL) {
          if(firearm->getDef().isRangedWeapon) {
            if(firearm->ammoLoaded >= 1 || firearm->getDef().rangedHasInfiniteAmmo) {
              if(firearm->getDef().isMachineGun && firearm->ammoLoaded < 5) {
                eng->log->addMessage("Need to load more ammo.");
              } else {
                eng->marker->run(markerTask_aimRangedWeapon, NULL);
              }
            } else {
              //If no ammo loaded, try a reload instead
              eng->reload->reloadWeapon(eng->player);
            }
          } else firearm = NULL;
        }

        if(firearm == NULL) {
          eng->log->addMessage("I am not wielding a firearm.");
        }
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------GET
  else if(d.key_ == 'g') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      Item* const itemAtPlayer = eng->map->items[eng->player->pos.x][eng->player->pos.y];
      if(itemAtPlayer != NULL) {
        if(itemAtPlayer->getDef().id == item_trapezohedron) {
          eng->dungeonMaster->winGame();
          *quitToMainMenu_ = true;
        }
      }
      if(*quitToMainMenu_ == false) {
        eng->itemPickup->tryPick();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------SLOTS SCREEN
  else if(d.key_ == 'w') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runSlotsScreen();
    }
    clearEvents();
    return;
  }
  //----------------------------------------INVENTORY
  else if(d.key_ == 'i') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runBrowseInventoryMode();
    }
    clearEvents();
    return;
  }
  //----------------------------------------USE
  else if(d.key_ == 'e') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(
        eng->player->dynamiteFuseTurns > 0 ||
        eng->player->flareFuseTurns > 0 ||
        eng->player->molotovFuseTurns > 0) {
        eng->marker->run(markerTask_aimLitExplosive, NULL);
      } else {
        eng->inventoryHandler->runUseScreen();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------SWAP TO PREPARED ITEM
  else if(d.key_ == 'z') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {

      const bool IS_FREE_TURN = false; //eng->playerBonusHandler->isBonusPicked(playerBonus_nimble);

      const string swiftStr = IS_FREE_TURN ? " swiftly" : "";

      Item* const itemWielded = eng->player->getInventory()->getItemInSlot(slot_wielded);
      Item* const itemAlt = eng->player->getInventory()->getItemInSlot(slot_wieldedAlt);
      const string ITEM_WIELDED_NAME =
        itemWielded == NULL ? "" :
        eng->itemData->getItemRef(*itemWielded, itemRef_a);
      const string ITEM_ALT_NAME =
        itemAlt == NULL ? "" :
        eng->itemData->getItemRef(*itemAlt, itemRef_a);
      if(itemWielded == NULL && itemAlt == NULL) {
        eng->log->addMessage("I have neither a wielded nor a prepared weapon.");
      } else {
        if(itemWielded == NULL) {
          eng->log->addMessage(
            "I" + swiftStr + " wield my prepared weapon (" + ITEM_ALT_NAME + ").");
        } else {
          if(itemAlt == NULL) {
            eng->log->addMessage(
              "I" + swiftStr + " put away my weapon (" + ITEM_WIELDED_NAME + ").");
          } else {
            eng->log->addMessage(
              "I" + swiftStr + " swap to my prepared weapon (" + ITEM_ALT_NAME + ").");
          }
        }
        eng->player->getInventory()->swapWieldedAndPrepared(IS_FREE_TURN == false, eng);
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------SEARCH (REALLY JUST A WAIT BUTTON)
  else if(d.key_ == 's') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->player->getSpotedEnemies();
      if(eng->player->spotedEnemies.size() == 0) {
        const int TURNS_TO_APPLY = 10;
        const string TURNS_STR = intToString(TURNS_TO_APPLY);
        eng->log->addMessage("I pause for a while and search around (" + TURNS_STR + " turns).");
        eng->player->waitTurnsLeft = TURNS_TO_APPLY - 1;
        eng->gameTime->endTurnOfCurrentActor();
      } else {
        eng->log->addMessage("Not while an enemy is near.");
        eng->renderer->drawMapAndInterface();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------THROW ITEM
  else if(d.key_ == 't') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {

      if(eng->player->getStatusEffectsHandler()->allowAttackRanged(true)) {
        Inventory* const playerInv = eng->player->getInventory();
        Item* itemStack = playerInv->getItemInSlot(slot_missiles);

        if(itemStack == NULL) {
          eng->log->addMessage(
            "I have no missiles chosen for throwing (press 'v').");
        } else {
          Item* itemToThrow = eng->itemFactory->copyItem(itemStack);
          itemToThrow->numberOfItems = 1;

          const MarkerReturnData markerReturnData =
            eng->marker->run(markerTask_aimThrownWeapon, itemToThrow);

          if(markerReturnData.didThrowMissile) {
            playerInv->decreaseItemInSlot(slot_missiles);
          } else {
            delete itemToThrow;
          }
        }
      }
    }
    clearEvents();
    return;
  }
  //---------------------------------------- LOOK AROUND
  else if(d.key_ == 'l') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->player->getStatusEffectsHandler()->allowSee()) {
        eng->marker->run(markerTask_look, NULL);
      } else {
        eng->log->addMessage("I am blind.");
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------FIRST AID
  else if(d.key_ == 'h') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->player->getStatusEffectsHandler()->hasEffect(statusPoisoned)) {
        eng->log->addMessage("Not while poisoned.");
        eng->renderer->drawMapAndInterface();
      } else {
        bool allowHeal = false;
        const bool IS_DISEASED = eng->player->getStatusEffectsHandler()->hasEffect(statusDiseased);

        if(eng->player->getHp() < eng->player->getHpMax(true)) {
          allowHeal = true;
        } else if(IS_DISEASED && eng->playerBonusHandler->isBonusPicked(playerBonus_curer)) {
          allowHeal = true;
        }

        if(allowHeal) {
          eng->player->getSpotedEnemies();
          if(eng->player->spotedEnemies.size() == 0) {
            const int TURNS_TO_HEAL = eng->player->getHealingTimeTotal();
            const string TURNS_STR = intToString(TURNS_TO_HEAL);
            eng->log->addMessage("I rest here and attend my wounds (" + TURNS_STR + " turns)...");
            eng->player->firstAidTurnsLeft = TURNS_TO_HEAL - 1;
            eng->gameTime->endTurnOfCurrentActor();
          } else {
            eng->log->addMessage("Not while an enemy is near.");
            eng->renderer->drawMapAndInterface();
          }
        } else {
          if(IS_DISEASED) {
            eng->log->addMessage("I cannot heal this disease.");
          } else {
            eng->log->addMessage("I am already at good health.");
          }
          eng->renderer->drawMapAndInterface();
        }
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------AUTO MELEE
  else if(d.sdlKey_ == SDLK_TAB) {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->player->autoMelee();
    }
    clearEvents();
    return;
  }
  //----------------------------------------POWERS - MEMORIZED
  else if(d.key_ == 'x') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->playerPowersHandler->run();
    }
    clearEvents();
    return;
  }
  //----------------------------------------MANUAL
  else if(d.key_ == '?') {
    eng->manual->run();
    eng->renderer->drawMapAndInterface();
    clearEvents();
    return;
  }
  //----------------------------------------CHARACTER INFO
  else if(d.key_ == '@') {
    eng->characterInfo->run();
    clearEvents();
    return;
  }
  //----------------------------------------LOG HISTORY
  else if(d.key_ == 'L') {
    eng->log->displayHistory();
    clearEvents();
    return;
  }
  //----------------------------------------QUIT
  else if(d.sdlKey_ == SDLK_ESCAPE || d.key_ == 'Q') {
    if(eng->player->deadState == actorDeadState_alive) {
      eng->log->clearLog();
      eng->log->addMessage("Quit the current game (y/n)? Save and highscore are not kept.", clrWhiteHigh);
      eng->renderer->drawMapAndInterface();
      if(eng->query->yesOrNo()) {
        *quitToMainMenu_ = true;
      } else {
        eng->log->clearLog();
        eng->renderer->drawMapAndInterface();
      }
    } else {
      *quitToMainMenu_ = true;
    }
    clearEvents();
    return;
  }
  //----------------------------------------SAVE AND QUIT
  else if(d.key_ == 'S') {
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->map->featuresStatic[eng->player->pos.x][eng->player->pos.y]->getId() == feature_stairsDown) {
        eng->log->clearLog();
        eng->log->addMessage("Save and quit (y/n)?", clrWhiteHigh);
        eng->renderer->drawMapAndInterface();
        if(eng->query->yesOrNo()) {
          eng->saveHandler->save();
          *quitToMainMenu_ = true;
        } else {
          eng->log->clearLog();
          eng->renderer->drawMapAndInterface();
        }
      } else {
        eng->log->clearLog();
        eng->renderer->drawMapAndInterface();
      }
    } else {
      eng->log->addMessage("Saving can only be done on stairs.");
      eng->renderer->drawMapAndInterface();
    }
    clearEvents();
    return;
  }
  //----------------------------------------DESCEND CHEAT
  else if(d.sdlKey_ == SDLK_F2) {
    if(IS_DEBUG_MODE) {
      eng->dungeonClimb->travelDown(1);
      clearEvents();
    }
    return;
  }
  //----------------------------------------XP CHEAT
  else if(d.sdlKey_ == SDLK_F3) {
    if(IS_DEBUG_MODE) {
      eng->dungeonMaster->playerGainsExp(500);
      clearEvents();
    }
    return;
  }
  //----------------------------------------VISION CHEAT
  else if(d.sdlKey_ == SDLK_F4) {
    if(IS_DEBUG_MODE) {
      if(eng->isCheatVisionEnabled) {
        for(int y = 0; y < MAP_Y_CELLS; y++) {
          for(int x = 0; x < MAP_X_CELLS; x++) {
            eng->map->explored[x][y] = false;
          }
        }
        eng->isCheatVisionEnabled = false;
      } else {
        eng->isCheatVisionEnabled = true;
      }
    }
    clearEvents();
  }
  //----------------------------------------INSANITY CHEAT
  else if(d.sdlKey_ == SDLK_F5) {
    if(IS_DEBUG_MODE) {
      eng->player->incrShock(50);
      clearEvents();
    }
    return;
  }
  //----------------------------------------MTH CHEAT
  else if(d.sdlKey_ == SDLK_F6) {
    if(IS_DEBUG_MODE) {
      eng->player->incrMth(8);
      clearEvents();
    }
    return;
  }
  //----------------------------------------DROP ALL SCROLLS AND POTIONS ON PLAYER
  else if(d.sdlKey_ == SDLK_F7) {
    if(IS_DEBUG_MODE) {
      for(unsigned int i = 1; i < endOfItemIds; i++) {
        const ItemDefinition* const def = eng->itemData->itemDefinitions[i];
        if(def->isIntrinsic == false && (def->isQuaffable || def->isReadable)) {
          eng->itemFactory->spawnItemOnMap(static_cast<ItemId_t>(i), eng->player->pos);
        }
      }
      clearEvents();
    }
    return;
  }

  //----------------------------------------UNDEFINED COMMANDS
  else if(d.key_ != -1) {
    string cmdTried = " ";
    cmdTried.at(0) = d.key_;
    eng->log->clearLog();
    eng->log->addMessage("Unknown command '" + cmdTried + "'. Press '?' for commands.");
    clearEvents();
    return;
  }
}

void Input::clearEvents() {
//  sf::Event event;
  while(SDL_PollEvent(&event_)) {
  }
}

KeyboardReadReturnData Input::readKeysUntilFound() {
  while(true) {

    eng->sleep(1);

    while(SDL_PollEvent(&event_)) {
      if(event_.type == SDL_KEYDOWN) {
        // ASCII char entered?
        // Decimal unicode:
        // '!' = 33
        // '~' = 126
        if(event_.key.keysym.unicode >= 33 && event_.key.keysym.unicode < 126) {
          return KeyboardReadReturnData(static_cast<char>(event_.key.keysym.unicode));
          clearEvents();
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
                  eng->config->toggleFullscreen();
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
