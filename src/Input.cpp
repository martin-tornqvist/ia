#include "Input.h"

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

extern void profiler(bool init);

void Input::clearMessages() {
  if(eng->player->deadState == actorDeadState_alive) {
    eng->log->clearLog();
  }
}

void Input::handleKeyPress(Uint16 key, const bool SHIFT, const bool CTRL) {
  char str[255];

  switch(key) {
    //----------------------------------------MOVEMENT
  case SDLK_6:
  case SDLK_KP6:
  case SDLK_RIGHT: {
    if(eng->player->deadState == actorDeadState_alive) {
      clearMessages();

      if(SHIFT == true) {
	eng->player->moveDirection(1, -1);
      } else {
	if(CTRL == true) {
	  eng->player->moveDirection(1, 1);
	} else {
	  eng->player->moveDirection(1, 0);
	}
      }
    }
    clearKeyEvents();
  }
    break;

  case SDLK_9:
  case SDLK_KP9:
  case SDLK_PAGEUP: {
    if(eng->player->deadState == actorDeadState_alive) {
      clearMessages();
      eng->player->moveDirection(1, -1);
    }
    clearKeyEvents();
  }
    break;

  case SDLK_8:
  case SDLK_KP8:
  case SDLK_UP: {
    if(eng->player->deadState == actorDeadState_alive) {
      clearMessages();
      eng->player->moveDirection(0, -1);
    }
    clearKeyEvents();
  }
    break;

  case SDLK_7:
  case SDLK_KP7:
  case SDLK_HOME: {
    if(eng->player->deadState == actorDeadState_alive) {
      clearMessages();
      eng->player->moveDirection(-1, -1);
    }
    clearKeyEvents();
  }
    break;

  case SDLK_4:
  case SDLK_KP4:
  case SDLK_LEFT: {
    if(eng->player->deadState == actorDeadState_alive) {
      clearMessages();
      if(SHIFT == true) {
	eng->player->moveDirection(-1, -1);
      } else {
	if(CTRL == true) {
	  eng->player->moveDirection(-1, 1);
	} else {
	  eng->player->moveDirection(-1, 0);
	}
      }
    }
    clearKeyEvents();
  }
    break;

  case SDLK_1:
  case SDLK_KP1:
  case SDLK_END: {
    if(eng->player->deadState == actorDeadState_alive) {
      clearMessages();
      eng->player->moveDirection(-1, 1);
    }
    clearKeyEvents();
  }
    break;

  case SDLK_2:
  case SDLK_KP2:
  case SDLK_DOWN: {
    if(eng->player->deadState == actorDeadState_alive) {
      clearMessages();
      eng->player->moveDirection(0, 1);
    }
    clearKeyEvents();
  }
    break;

  case SDLK_3:
  case SDLK_KP3:
  case SDLK_PAGEDOWN: {
    if(eng->player->deadState == actorDeadState_alive) {
      clearMessages();
      eng->player->moveDirection(1, 1);
    }
    clearKeyEvents();
  }
    break;

  case SDLK_5:
  case SDLK_KP5:
  case int('.'): {
    if(eng->player->deadState == actorDeadState_alive) {
      clearMessages();
      eng->player->moveDirection(0, 0);
      eng->player->getStatusEffectsHandler()->attemptAddEffect(new StatusStill(1));
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------DESCEND
  case int('>'): {
    tracer << "Input: User pressed '>'" << endl;
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      tracer << "Input: Calling DungeonClimb::attemptUseDownStairs()" << endl;
      eng->dungeonClimb->attemptUseDownStairs();
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------APPLY/USE/EXAMINE
  //	case int('a'): {
  //		clearMessages();
  //		if(eng->player->deadState == actorDeadState_alive) {
  //			eng->examine->playerExamine();
  //			eng->renderer->flip();
  //		}
  //		clearKeyEvents();
  //	}
  //	break;

  //----------------------------------------RELOAD
  case SDLK_r: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->reload->reloadWeapon(eng->player);
    }
    clearKeyEvents();
  }
    break;

    //----------------------------------------BASH
  case int('b'): {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->bash->playerBash();
      eng->renderer->flip();
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------CLOSE
  case int('c'): {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->close->playerClose();
      eng->renderer->flip();
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------JAM
  case int('j'): {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->jamWithSpike->playerJam();
      eng->renderer->flip();
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------UNLOAD AMMO FROM GROUND
  case int('u'): {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->itemPickup->tryUnloadWeaponFromGround();
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------EXPLOSIVES
  case SDLK_e: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {

      //If player has no explosive readied, ask player to select one
      if(eng->player->dynamiteFuseTurns == -1 && eng->player->molotovFuseTurns == -1 && eng->player->flareFuseTurns == -1) {
	eng->inventoryHandler->runPlayerInventory(inventoryPurpose_readyExplosive);
      } else {
	//Else, ask player to throw explosive
	eng->marker->place(markerTask_throwLitExplosive);
      }
    }
    clearKeyEvents();
  }
    break;

    //----------------------------------------AIM/FIRE FIREARM
  case SDLK_f: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {

      if(eng->player->getStatusEffectsHandler()->allowAttackRanged(true) == true) {

	Weapon* firearm = NULL;

	firearm = dynamic_cast<Weapon*>(eng->player->getInventory()->getItemInSlot(slot_wielded));

	if(firearm != NULL) {
	  if(firearm->getInstanceDefinition().isRangedWeapon == true) {
	    if(firearm->ammoLoaded >= 1 || firearm->getInstanceDefinition().rangedHasInfiniteAmmo) {
	      if(firearm->getInstanceDefinition().isMachineGun && firearm->ammoLoaded < 5) {
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
    clearKeyEvents();
  }
    break;

    //----------------------------------------GET
  case SDLK_g: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      Item* const itemAtPlayer = eng->map->items[eng->player->pos.x][eng->player->pos.y];
      if(itemAtPlayer != NULL) {
	if(itemAtPlayer->getInstanceDefinition().devName == item_trapezohedron) {
	  eng->dungeonMaster->winGame();
	  *quitToMainMenu_ = true;
	}
      }
      if(*quitToMainMenu_ == false) {
	eng->itemPickup->tryPick();
      }
    }
    clearKeyEvents();
  }
    break;

    //----------------------------------------EXAMINE INVENTORY
  case SDLK_i: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_look);
    }
    clearKeyEvents();
  }
    break;

    //----------------------------------------WIELD WEAPON
  case SDLK_w: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_wieldWear);
    }
    clearKeyEvents();
  }
    break;

    //----------------------------------------PREPARE WEAPON
  case int('Z'): {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_wieldAlt);
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------SWAP TO PREPARED ITEM
  case int('z'): {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {

      const AbilityRollResult_t rollResult = eng->abilityRoll->roll(eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(
																	ability_weaponHandling, true));
      const bool IS_FREE_TURN_DUE_TO_SKILL = rollResult >= successSmall;
      const SDL_Color clr = IS_FREE_TURN_DUE_TO_SKILL ? clrMagenta : clrWhite;
      const string swiftStr = IS_FREE_TURN_DUE_TO_SKILL ? " swiftly" : "";

      Item* const itemWielded = eng->player->getInventory()->getItemInSlot(slot_wielded);
      Item* const itemAlt = eng->player->getInventory()->getItemInSlot(slot_wieldedAlt);
      const string ITEM_WIELDED_NAME = itemWielded == NULL ? "" : eng->itemData->itemInterfaceName(itemWielded, IS_FREE_TURN_DUE_TO_SKILL);
      const string ITEM_ALT_NAME = itemAlt == NULL ? "" : eng->itemData->itemInterfaceName(itemAlt, true);

      if(itemWielded == NULL && itemAlt == NULL) {
	eng->log->addMessage("I have neither a wielded nor a prepared weapon.");
      } else {
	if(itemWielded == NULL) {
	  eng->log->addMessage("I" + swiftStr + " wield my prepared weapon (" + ITEM_ALT_NAME + ").", clr);
	} else {
	  if(itemAlt == NULL) {
	    eng->log->addMessage("I" + swiftStr + " put away my weapon (" + ITEM_WIELDED_NAME + ").", clr);
	  } else {
	    eng->log->addMessage("I" + swiftStr + " swap to my prepared weapon (" + ITEM_ALT_NAME + ").", clr);
	  }
	}
	eng->player->getInventory()->swapWieldedAndPrepared(IS_FREE_TURN_DUE_TO_SKILL == false, eng);
      }
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------WIELD MISSILE WEAPON
  case int('m'): {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_missileSelect);
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------UN-WIELD MISSILE WEAPON
  case int('M'): {
    clearMessages();
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
    clearKeyEvents();
  }
  break;

  //----------------------------------------WEAR ITEM
  //    case int('W'): {
  //        clearMessages();
  //        if(eng->player->deadState == actorDeadState_alive) {
  //            eng->inventoryHandler->runPlayerInventory(inventoryPurpose_wear);
  //        }
  //        clearKeyEvents();
  //    }
  //    break;

  //----------------------------------------SEARCH (REALLY JUST A WAIT BUTTON)
  case SDLK_s: {
    clearMessages();
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
	eng->renderer->flip();
      }
    }
    clearKeyEvents();
  }
    break;

    //----------------------------------------USE ITEM
    //	case int('U'): {
    //		clearMessages();
    //		if(eng->player->deadState == actorDeadState_alive) {
    //			eng->inventoryHandler->runPlayerInventory(inventoryPurpose_use);
    //		}
    //		clearKeyEvents();
    //	}
    //	break;

    //----------------------------------------READ
  case int('R'): {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->player->getStatusEffectsHandler()->allowSee()) {
	eng->playerPowersHandler->run(false);
      } else {
	eng->log->addMessage("I can not read while blind.");
      }
    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------EAT
  //	case int('E'): {
  //		clearMessages();
  //		if(eng->player->deadState == actorDeadState_alive) {
  //			eng->inventoryHandler->runPlayerInventory(inventoryPurpose_eat);
  //		}
  //		clearKeyEvents();
  //	}
  //	break;

  //----------------------------------------QUAFF
  case SDLK_q: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_quaff);
    }
    clearKeyEvents();
  }
    break;

    //----------------------------------------THROW ITEM
  case SDLK_t: {
    clearMessages();
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
    clearKeyEvents();
  }
    break;

    //----------------------------------------DROP ITEM
  case SDLK_d: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->inventoryHandler->runPlayerInventory(inventoryPurpose_selectDrop);
    }
    clearKeyEvents();
  }
    break;

    //---------------------------------------- LOOK AROUND
  case SDLK_l: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->player->getStatusEffectsHandler()->allowSee() == true) {
	eng->marker->place(markerTask_look);
      } else {
	eng->log->addMessage("I am blind.");
      }
    }
    clearKeyEvents();
  }
    break;

    //---------------------------------------- TRIGGER TRAP ON PURPOSE
    //	case int('T'): {
    //TODO reimplement
    //		clearMessages();
    //		if(eng->player->deadState == actorDeadState_alive) {
    //			Trap* const trap = eng->map->traps[eng->player->pos.x][eng->player->pos.y];
    //			if(trap != NULL) {
    //				if(trap->isHidden() == false) {
    //					trap->triggerOnPurpose(eng->player);
    //					eng->gameTime->letNextAct();
    //				} else {
    //					eng->log->addMessage("I find no trap here to trigger.");
    //				}
    //			} else {
    //				eng->log->addMessage("I find no trap here to trigger.");
    //			}
    //		}
    //		clearKeyEvents();
    //	}
    //	break;

    //----------------------------------------FIRST AID
  case SDLK_h: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->player->getHP() >= eng->player->getHP_max()) {
	eng->log->addMessage("I am already at good health.");
	eng->renderer->flip();
      } else {
	eng->player->getSpotedEnemies();
	if(eng->player->spotedEnemies.size() == 0) {
	  const int TURNS_BEFORE_BON = 70;
	  //					const int PLAYER_FIRST_AID_SKILL = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(ability_firstAid, true);
	  //               const int TURNS_AFTER_BON = max(20, static_cast<int>(static_cast<float>(TURNS_BEFORE_BON * (100 - PLAYER_FIRST_AID_SKILL)) / 100.0));
	  const int PLAYER_HEALING_RANK = eng->playerBonusHandler->getBonusRankForAbility(ability_firstAid);
	  const int TURNS_AFTER_BON = PLAYER_HEALING_RANK >= 1 ? TURNS_BEFORE_BON / 2 : TURNS_BEFORE_BON;
	  const string TURNS_STR = intToString(TURNS_AFTER_BON);
	  eng->log->addMessage("I rest here and attend my wounds (" + TURNS_STR + " turns)...");
	  eng->player->firstAidTurnsLeft = TURNS_AFTER_BON - 1;
	  eng->gameTime->letNextAct();
	} else {
	  eng->log->addMessage("Not while an enemy is near.");
	  eng->renderer->flip();
	}
      }
      clearKeyEvents();
    }
  }
    break;

    //----------------------------------------AUTO MELEE
  case SDLK_TAB: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->player->autoMelee();
      clearKeyEvents();
    }
  }
    break;

    //----------------------------------------KICK
  case SDLK_k: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->player->getStatusEffectsHandler()->allowAttackMelee(true)) {
	eng->player->kick();
      }
      clearKeyEvents();
    }
  }
    break;

    //----------------------------------------KICK STATS
  case int('K'): {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      const ItemDefinition* const d = eng->itemData->itemDefinitions[item_playerKick];
      const DiceParam dmg = d->meleeDmg;
      const Abilities_t abilityUsed = d->meleeAbilityUsed;
      const int PLAYER_SKILL = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(abilityUsed, true);
      const int TOTAL_SKILL = PLAYER_SKILL + d->meleeBaseAttackSkill;
      const string DMG_STR = intToString(dmg.rolls) + "d" + intToString(dmg.sides);
      eng->log->addMessage("Kick stats: " + DMG_STR + "  " + intToString(TOTAL_SKILL) + "%");

    }
    clearKeyEvents();
  }
  break;

  //----------------------------------------POWERS - MEMORIZED
  case SDLK_x: {
    clearMessages();
    if(eng->player->deadState == actorDeadState_alive) {
      eng->playerPowersHandler->run(true);
    }
    clearKeyEvents();
  }
    break;

    //----------------------------------------MANUAL
  case int('?'):
    eng->manual->run();
  break;

  //----------------------------------------CHARACTER INFO
  case int('@'):
    eng->characterInfo->run();
  break;

  //----------------------------------------LOG HISTORY
  case int('L'):
    eng->log->displayHistory();
  break;

  //----------------------------------------QUIT
  case int('Q'): {
    if(eng->player->deadState == actorDeadState_alive) {
      eng->log->clearLog();
      eng->log->addMessage("Quit the current game (y/n)? Save and highscore are not kept.", clrWhiteHigh);
      eng->renderer->flip();
      if(eng->query->yesOrNo()) {
	*quitToMainMenu_ = true;
      } else {
	eng->log->clearLog();
	eng->renderer->drawMapAndInterface();
      }
    } else {
      *quitToMainMenu_ = true;
    }
  }
  break;

  //----------------------------------------SAVE AND QUIT
  case int('S'): {
    if(eng->player->deadState == actorDeadState_alive) {
      if(eng->map->featuresStatic[eng->player->pos.x][eng->player->pos.y]->getId() == feature_stairsDown) {
	eng->log->clearLog();
	eng->log->addMessage("Save and quit (y/n)?", clrWhiteHigh);
	eng->renderer->flip();
	if(eng->query->yesOrNo() == true) {
	  if(eng->saveHandler->save() == true) {
	    *quitToMainMenu_ = true;
	  }
	} else {
	  eng->log->clearLog();
	  eng->renderer->drawMapAndInterface();
	}
      } else {
	eng->log->addMessage("Saving can only be done on stairs.");
	eng->renderer->flip();
      }
    }
  }
  ;
  break;

  //----------------------------------------UNDEFINED COMMANDS
  default:
    if(key < SDLK_UP) {
      str[0] = char(key);
    }
    str[1] = '\0';
    if(str[0] >= 33) { //&& str[0] <= 126) {
      clearMessages();
      string cmdTried = str;
      eng->log->addMessage("Unknown command '" + cmdTried + "'. Press '?' for commands.");
    }
    break;
  }
  if(eng->gameTime->getCurrentActor() == eng->player) {
    eng->renderer->flip();
  }
}

void Input::read() {
  const bool SHIFT = SDL_GetModState() & KMOD_SHIFT;
  const bool CTRL = SDL_GetModState() & KMOD_CTRL;

  while(SDL_PollEvent(&m_event)) {
    switch(m_event.type) {
    case SDL_QUIT:
      *quitToMainMenu_ = true;
      break;

    case SDL_KEYDOWN: {
      SDLKey key = m_event.key.keysym.sym;

      bool cheatKeyPressed = false;
      /*
      //----------------------------------------DESCEND CHEAT
      if(key == SDLK_F2) {
      cheatKeyPressed = true;
      eng->dungeonClimb->travelDown(1);
      clearKeyEvents();
      }

      //----------------------------------------XP CHEAT
      if(key == SDLK_F3) {
      cheatKeyPressed = true;
      eng->dungeonMaster->playerGainsExp(500);
      clearKeyEvents();
      }

      //----------------------------------------VISION CHEAT
      if(key == SDLK_F4) {
      cheatKeyPressed = true;
      eng->cheat_vision = !eng->cheat_vision;
      clearKeyEvents();
      }

      //----------------------------------------INSANITY "CHEAT"
      //			if(key == SDLK_F5) {
      //				cheatKeyPressed = true;
      //				eng->player->shock(shockValue_heavy, 0);
      //				clearKeyEvents();
      //			}

      //----------------------------------------DROP ALL SCROLLS AND POTIONS ON PLAYER
      if(key == SDLK_F6) {
      cheatKeyPressed = true;
      for(unsigned int i = 1; i < endOfItemDevNames; i++) {
      const ItemDefinition* const def = eng->itemData->itemDefinitions[i];
      if(def->isQuaffable == true || def->isReadable == true) {
      eng->itemFactory->spawnItemOnMap(static_cast<ItemDevNames_t>(i), eng->player->pos);
      }
      }
      clearKeyEvents();
      }
      if(key == SDLK_F7) {
      cheatKeyPressed = true;

      const coord pos = eng->player->pos + coord(1,0);
      if(eng->map->featuresStatic[pos.x][pos.y]->isMoveTypePassable(moveType_walk)) {
      Monster* const monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_zombieAxe, pos));
      monster->leader = eng->player;
      }

      clearKeyEvents();
      }

      */

      if(cheatKeyPressed == false) {
	if(
	   key == SDLK_RIGHT || key == SDLK_UP || key == SDLK_LEFT || key == SDLK_DOWN ||
	   key == SDLK_PAGEUP || key == SDLK_PAGEDOWN || key == SDLK_HOME || key == SDLK_END) {
	  handleKeyPress(static_cast<Uint16>(m_event.key.keysym.sym), SHIFT, CTRL);
	} else {
	  handleKeyPress(m_event.key.keysym.unicode, SHIFT, CTRL);
	}
      }
    }
      break;

    default: {
    }
      break;
    }
  }
}
