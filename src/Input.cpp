#include "Input.h"

#include "SFML/Window/Event.hpp"

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

void Input::clearLogMessages() {
  if(eng->player->deadState == actorDeadState_alive) {
    eng->log->clearLog();
  }
}

Input::Input(Engine* engine, bool* quitToMainMenu) : eng(engine), quitToMainMenu_(quitToMainMenu)  {

}

void Input::handleMapModeInputUntilFound() {
  const KeyboardReadReturnData& d = readKeysUntilFound();
  handleKeyPress(d);
}

void Input::handleKeyPress(const KeyboardReadReturnData& d) {
  //----------------------------------------MOVEMENT
  if(d.sfmlKey_ == sf::Keyboard::Right || d.key_ == '6') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      if(d.isShiftHeld_) {
        eng->player->moveDirection(1, -1);
      }
      else if(d.isCtrlHeld_) {
        eng->player->moveDirection(1, 1);
      }
      else {
        eng->player->moveDirection(1, 0);
      }
    }
    clearEvents();
    return;
  }
  else if(d.sfmlKey_ == sf::Keyboard::Down || d.key_ == '2') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(0, 1);
    }
    clearEvents();
    return;
  }
  else if(d.sfmlKey_ == sf::Keyboard::Left || d.key_ == '4') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      if(d.isShiftHeld_) {
        eng->player->moveDirection(-1, -1);
      }
      else if(d.isCtrlHeld_) {
        eng->player->moveDirection(-1, 1);
      }
      else {
        eng->player->moveDirection(-1, 0);
      }
    }
    clearEvents();
    return;
  }
  else if(d.sfmlKey_ == sf::Keyboard::Up || d.key_ == '8') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(0, -1);
    }
    clearEvents();
    return;
  }
  else if(d.sfmlKey_ == sf::Keyboard::PageUp || d.key_ == '9') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(1, -1);
    }
    clearEvents();
    return;
  }
  else if(d.sfmlKey_ == sf::Keyboard::PageUp || d.key_ == '3') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(1, 1);
    }
    clearEvents();
    return;
  }
  else if(d.sfmlKey_ == sf::Keyboard::PageUp || d.key_ == '1') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(-1, 1);
    }
    clearEvents();
    return;
  }
  else if(d.sfmlKey_ == sf::Keyboard::PageUp || d.key_ == '7') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(-1, -1);
    }
    clearEvents();
    return;
  }
  else if(d.key_ == '5') {
    if(eng->player->deadState == actorDeadState_alive) {
      clearLogMessages();
      eng->player->moveDirection(0, 0);
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_steadyAimer)) {
        eng->player->getStatusEffectsHandler()->attemptAddEffect(new StatusStill(1));
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
      tracer << "Input: Calling DungeonClimb::attemptUseDownStairs()" << endl;
      eng->dungeonClimb->attemptUseDownStairs();
    }
    clearEvents();
    return;
  }
  //----------------------------------------EXAMINE
  else if(d.key_ == 'a') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->examine->playerExamine();
      eng->renderer->drawMapAndInterface();
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
  //----------------------------------------UNLOAD AMMO FROM GROUND
  else if(d.key_ == 'u') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->itemPickup->tryUnloadWeaponOrPickupAmmoFromGround();
    }
    clearEvents();
    return;
  }
  //----------------------------------------EXPLOSIVES
  else if(d.key_ == 'e') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      //If player has no explosive readied, ask player to select one
      if(eng->player->dynamiteFuseTurns == -1 && eng->player->molotovFuseTurns == -1 && eng->player->flareFuseTurns == -1) {
        eng->inventoryHandler->runPlayerInventory(inventoryPurpose_readyExplosive);
      } else {
        //Else, ask player to throw explosive
        eng->marker->place(markerTask_throwLitExplosive);
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------AIM/FIRE FIREARM
  else if(d.key_ == 'f') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {

      if(eng->player->getStatusEffectsHandler()->allowAttackRanged(true) == true) {

        Weapon* firearm = NULL;

        firearm = dynamic_cast<Weapon*>(eng->player->getInventory()->getItemInSlot(slot_wielded));

        if(firearm != NULL) {
          if(firearm->getDef().isRangedWeapon == true) {
            if(firearm->ammoLoaded >= 1 || firearm->getDef().rangedHasInfiniteAmmo) {
              if(firearm->getDef().isMachineGun && firearm->ammoLoaded < 5) {
                eng->log->addMessage("Need to load more ammo.");
              } else {
                eng->marker->place(markerTask_aim);
              }
            } else {
              //If no ammo loaded, try a reload instead
              eng->reload->reloadWeapon(eng->player);
            }
          } else firearm = NULL;
        }

        if(firearm == NULL) {
          eng->log->addMessage("I am not wielding a firearm (press 'w').");
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
        if(itemAtPlayer->getDef().devName == item_trapezohedron) {
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
  //----------------------------------------EXAMINE INVENTORY
  else if(d.key_ == 'i') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_look);
    }
    clearEvents();
    return;
  }
  //----------------------------------------WIELD WEAPON
  else if(d.key_ == 'w') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_wieldWear);
    }
    clearEvents();
    return;
  }
  //----------------------------------------PREPARE WEAPON
  else if(d.key_ == 'W') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_wieldAlt);
    }
    clearEvents();
    return;
  }
  //----------------------------------------SWAP TO PREPARED ITEM
  else if(d.key_ == 'z') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {

      const bool IS_FREE_TURN = eng->playerBonusHandler->isBonusPicked(playerBonus_nimble);

      const string swiftStr = IS_FREE_TURN ? " swiftly" : "";

      Item* const itemWielded = eng->player->getInventory()->getItemInSlot(slot_wielded);
      Item* const itemAlt = eng->player->getInventory()->getItemInSlot(slot_wieldedAlt);
      const string ITEM_WIELDED_NAME = itemWielded == NULL ? "" : eng->itemData->itemInterfaceName(itemWielded, IS_FREE_TURN);
      const string ITEM_ALT_NAME = itemAlt == NULL ? "" : eng->itemData->itemInterfaceName(itemAlt, true);

      if(itemWielded == NULL && itemAlt == NULL) {
        eng->log->addMessage("I have neither a wielded nor a prepared weapon.");
      } else {
        if(itemWielded == NULL) {
          eng->log->addMessage("I" + swiftStr + " wield my prepared weapon (" + ITEM_ALT_NAME + ").");
        } else {
          if(itemAlt == NULL) {
            eng->log->addMessage("I" + swiftStr + " put away my weapon (" + ITEM_WIELDED_NAME + ").");
          } else {
            eng->log->addMessage("I" + swiftStr + " swap to my prepared weapon (" + ITEM_ALT_NAME + ").");
          }
        }
        eng->player->getInventory()->swapWieldedAndPrepared(IS_FREE_TURN == false, eng);
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------WIELD MISSILE WEAPON
  else if(d.key_ == 'm') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_missileSelect);
    }
    clearEvents();
    return;
  }
  //----------------------------------------UN-WIELD MISSILE WEAPON
  else if(d.key_ == 'M') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      InventorySlot* const missileSlot = eng->player->getInventory()->getSlot(slot_missiles);
      Item* const item = eng->player->getInventory()->getItemInSlot(slot_missiles);
      const bool ITEM_MOVED = eng->player->getInventory()->moveItemToGeneral(missileSlot);
      if(ITEM_MOVED == true) {
        eng->log->addMessage("I quit using " + eng->itemData->itemInterfaceName(item, true) + " as missile weapon.");
      } else {
        eng->log->addMessage("There is no missile weapon to quit using.");
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
        eng->gameTime->letNextAct();
      } else {
        eng->log->addMessage("Not while an enemy is near.");
        eng->renderer->drawMapAndInterface();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------READ
  else if(d.key_ == 'R') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->player->getStatusEffectsHandler()->allowSee()) {
        eng->playerPowersHandler->run(false);
      } else {
        eng->log->addMessage("I can not read while blind.");
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------QUAFF
  else if(d.key_ == 'q') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_quaff);
    }
    clearEvents();
    return;
  }
  //----------------------------------------THROW ITEM
  else if(d.key_ == 't') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {

      if(eng->player->getStatusEffectsHandler()->allowAttackRanged(true)) {
        Item* item = eng->player->getInventory()->getItemInSlot(slot_missiles);

        if(item == NULL) {
          eng->log->addMessage("I have no weapon chosen for throwing (press 'm').");
        } else {
          eng->marker->place(markerTask_throw);
        }
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------DROP ITEM
  else if(d.key_ == 'd') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_selectDrop);
    }
    clearEvents();
    return;
  }
  //---------------------------------------- LOOK AROUND
  else if(d.key_ == 'l') {
    clearLogMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->player->getStatusEffectsHandler()->allowSee() == true) {
        eng->marker->place(markerTask_look);
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
      if(eng->player->getHp() >= eng->player->getHpMax()) {
        eng->log->addMessage("I am already at good health.");
        eng->renderer->drawMapAndInterface();
      } else {
        eng->player->getSpotedEnemies();
        if(eng->player->spotedEnemies.size() == 0) {
          const int TURNS_TO_HEAL = eng->player->getHealingTimeTotal();
          const string TURNS_STR = intToString(TURNS_TO_HEAL);
          eng->log->addMessage("I rest here and attend my wounds (" + TURNS_STR + " turns)...");
          eng->player->firstAidTurnsLeft = TURNS_TO_HEAL - 1;
          eng->gameTime->letNextAct();
        } else {
          eng->log->addMessage("Not while an enemy is near.");
          eng->renderer->drawMapAndInterface();
        }
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------------AUTO MELEE
  else if(d.sfmlKey_ == sf::Keyboard::Tab) {
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
      eng->playerPowersHandler->run(true);
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
  else if(d.sfmlKey_ == sf::Keyboard::Escape || d.key_ == 'Q') {
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
  //----------------------------------------UNDEFINED COMMANDS
  else if(d.key_ != -1) {
    string cmdTried = " ";
    cmdTried.at(0) = d.key_;
    eng->log->addMessage("Unknown command '" + cmdTried + "'. Press '?' for commands.");
    clearEvents();
    return;
  }
}

void Input::clearEvents() const {
  sf::Event event;
  while(eng->renderer->renderWindow_->pollEvent(event)) {
  }
}

KeyboardReadReturnData Input::readKeysUntilFound() const {
  while(true) {
    sf::Event event;
    while(eng->renderer->renderWindow_->pollEvent(event)) {
      if(event.type == sf::Event::TextEntered) {
        // ASCII char entered?
        // Decimal unicode:
        // '!' = 33
        // '~' = 126
        if(event.text.unicode >= 33 && event.text.unicode <= 126) {
          clearEvents();
          return KeyboardReadReturnData(static_cast<char>(event.text.unicode));
        }
        continue;
      }
      else if(event.type == sf::Event::KeyPressed) {
        // Other key pressed? (escape, return, space, etc)
        const sf::Keyboard::Key sfmlKey = event.key.code;

        // Don't register shift, control or alt as actual key events
        if(
          sfmlKey == sf::Keyboard::LShift ||
          sfmlKey == sf::Keyboard::RShift ||
          sfmlKey == sf::Keyboard::LControl ||
          sfmlKey == sf::Keyboard::RControl ||
          sfmlKey == sf::Keyboard::LAlt ||
          sfmlKey == sf::Keyboard::RAlt) {
          continue;
        }

        const bool IS_SHIFT_HELD =
          sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
          sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
        const bool IS_CTRL_HELD =
          sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
          sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);

        KeyboardReadReturnData ret(-1, sfmlKey, IS_SHIFT_HELD, IS_CTRL_HELD);

        if(sfmlKey >= sf::Keyboard::F1 && sfmlKey <= sf::Keyboard::F15) {
          // F-keys
          return ret;
        }
        else {
          switch(sfmlKey) {
          default: continue; break;
          case sf::Keyboard::LSystem: continue; break;
          case sf::Keyboard::RSystem: continue; break;
          case sf::Keyboard::Menu: continue; break;
          case sf::Keyboard::Pause: continue; break;
          case sf::Keyboard::Space: return ret; break;
          case sf::Keyboard::Return: return ret; break;
          case sf::Keyboard::Back: return ret; break;
          case sf::Keyboard::Tab: return ret; break;
          case sf::Keyboard::PageUp: return ret; break;
          case sf::Keyboard::PageDown: return ret; break;
          case sf::Keyboard::End: return ret; break;
          case sf::Keyboard::Home: return ret; break;
          case sf::Keyboard::Insert: return ret; break;
          case sf::Keyboard::Delete: return ret; break;
          case sf::Keyboard::Left: return ret; break;
          case sf::Keyboard::Right: return ret; break;
          case sf::Keyboard::Up: return ret; break;
          case sf::Keyboard::Down: return ret; break;
          case sf::Keyboard::Escape: return ret; break;
          }
        }
      }
    }
  }
}

//void Input::read() {
//  const bool IS_SHIFT_HELD = SDL_GetModState() & KMOD_SHIFT;
//  const bool IS_CTRL_HELD = SDL_GetModState() & KMOD_CTRL;
//
//  while(SDL_PollEvent(&m_event)) {
//    switch(m_event.type) {
//    case SDL_QUIT:
//      *quitToMainMenu_ = true;
//      break;
//
//    case SDL_KEYDOWN: {
//      SDLKey key = m_event.key.keysym.sym;
//
//      bool isCheatKeyPressed = false;
//
//      if(IS_DEBUG_MODE) {
//        //----------------------------------------DESCEND CHEAT
//        if(key == SDLK_F2) {
//          isCheatKeyPressed = true;
//          eng->dungeonClimb->travelDown(1);
//          clearKeyEvents();
//        }
//
//        //----------------------------------------XP CHEAT
//        if(key == SDLK_F3) {
//          isCheatKeyPressed = true;
//          eng->dungeonMaster->playerGainsExp(500);
//          clearKeyEvents();
//        }
//
//        //----------------------------------------VISION CHEAT
//        if(key == SDLK_F4) {
//          isCheatKeyPressed = true;
//          eng->cheat_vision = !eng->cheat_vision;
//          clearKeyEvents();
//        }
//
//        //----------------------------------------INSANITY "CHEAT"
//        if(key == SDLK_F5) {
//          isCheatKeyPressed = true;
//          eng->player->shock(shockValue_heavy, 0);
//          clearKeyEvents();
//        }
//
//        //----------------------------------------DROP ALL SCROLLS AND POTIONS ON PLAYER
//        if(key == SDLK_F6) {
//          isCheatKeyPressed = true;
//          for(unsigned int i = 1; i < endOfItemDevNames; i++) {
//            const ItemDefinition* const def = eng->itemData->itemDefinitions[i];
//            if(def->isQuaffable == true || def->isReadable == true) {
//              eng->itemFactory->spawnItemOnMap(static_cast<ItemDevNames_t>(i), eng->player->pos);
//            }
//          }
//          clearKeyEvents();
//        }
//      }
//
//      if(isCheatKeyPressed == false) {
//        if(
//          key == SDLK_RIGHT || key == SDLK_UP || key == SDLK_LEFT || key == SDLK_DOWN ||
//          key == SDLK_PAGEUP || key == SDLK_PAGEDOWN || key == SDLK_HOME || key == SDLK_END) {
//          handleKeyPress(static_cast<Uint16>(m_event.key.keysym.sym), IS_SHIFT_HELD, IS_CTRL_HELD);
//        } else {
//          handleKeyPress(m_event.key.keysym.unicode, IS_SHIFT_HELD, IS_CTRL_HELD);
//        }
//      }
//    }
//    break;
//
//    default: {
//    }
//    break;
//    }
//  }
//}
