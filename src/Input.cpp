#include "Input.h"

#include <memory>

#include "Init.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "DungeonClimb.h"
#include "Input.h"
#include "Reload.h"
#include "Kick.h"
#include "Renderer.h"
#include "Close.h"
#include "JamWithSpike.h"
#include "ItemPickup.h"
#include "InventoryHandling.h"
#include "Marker.h"
#include "Map.h"
#include "DungeonMaster.h"
#include "PlayerSpellsHandling.h"
#include "Manual.h"
#include "CharacterDescr.h"
#include "Query.h"
#include "SaveHandling.h"
#include "ItemFactory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "PlayerBon.h"
#include "Examine.h"
#include "CreateCharacter.h"
#include "Disarm.h"
#include "SdlWrapper.h"
#include "Popup.h"

using namespace std;

namespace Input {

namespace {

SDL_Event* sdlEvent_ = nullptr;

void queryQuit() {
  const vector<string> quitChoices = vector<string> {"yes", "no"};
  const int QUIT_CHOICE = Popup::showMenuMsg(
                            "Save and highscore are not kept.",
                            false, quitChoices, "Quit the current game?");
  if(QUIT_CHOICE == 0) {
    Init::quitToMainMenu = true;
    Renderer::clearScreen();
    Renderer::updateScreen();
  }
}

} //Namespace

void init() {
  if(!sdlEvent_) {sdlEvent_ = new SDL_Event;}
  setKeyRepeatDelays();
}

void cleanup() {
  if(sdlEvent_) {
    delete sdlEvent_;
    sdlEvent_ = nullptr;
  }
}

void handleKeyPress(const KeyboardReadRetData& d) {
  //----------------------------------- MOVEMENT
  if(d.sdlKey_ == SDLK_RIGHT            || d.key_ == '6' || d.key_ == 'l') {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();
      if(d.isShiftHeld_) {
        Map::player->moveDir(Dir::upRight);
      } else if(d.isCtrlHeld_) {
        Map::player->moveDir(Dir::downRight);
      } else {
        Map::player->moveDir(Dir::right);
      }
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_DOWN      || d.key_ == '2' || d.key_ == 'j') {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();
      Map::player->moveDir(Dir::down);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_LEFT      || d.key_ == '4' || d.key_ == 'h') {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();
      if(d.isShiftHeld_) {
        Map::player->moveDir(Dir::upLeft);
      } else if(d.isCtrlHeld_) {
        Map::player->moveDir(Dir::downLeft);
      } else {
        Map::player->moveDir(Dir::left);
      }
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_UP        || d.key_ == '8' || d.key_ == 'k') {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();
      Map::player->moveDir(Dir::up);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEUP    || d.key_ == '9' || d.key_ == 'u') {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();
      Map::player->moveDir(Dir::upRight);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_PAGEDOWN  || d.key_ == '3' || d.key_ == 'n') {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();
      Map::player->moveDir(Dir::downRight);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_END       || d.key_ == '1' || d.key_ == 'b') {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();
      Map::player->moveDir(Dir::downLeft);
    }
    clearEvents();
    return;
  } else if(d.sdlKey_ == SDLK_HOME      || d.key_ == '7' || d.key_ == 'y') {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();
      Map::player->moveDir(Dir::upLeft);
    }
    clearEvents();
    return;
  } else if(d.key_ == '5' || d.key_ == '.') {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();

      if(PlayerBon::hasTrait(Trait::steadyAimer)) {
        PropHandler& propHlr = Map::player->getPropHandler();

        int nrTurnsAimingOld = 0;

        if(PlayerBon::hasTrait(Trait::sharpShooter)) {
          Prop* const propAimingOld =
            propHlr.getProp(propAiming, PropSrc::applied);
          if(propAimingOld) {
            nrTurnsAimingOld =
              static_cast<PropAiming*>(propAimingOld)->nrTurnsAiming;
          }
        }

        PropAiming* const aiming = new PropAiming(propTurnsSpecific, 1);
        aiming->nrTurnsAiming += nrTurnsAimingOld;
        propHlr.tryApplyProp(aiming);
      }
      Map::player->moveDir(Dir::center);
    }
    clearEvents();
    return;
  }
  //----------------------------------- MANUAL
  else if(d.key_ == '?') {
    Log::clearLog();
    Manual::run();
    clearEvents();
    return;
  }
  //----------------------------------- OPTIONS
  else if(d.key_ == '=') {
    Log::clearLog();
    Config::runOptionsMenu();
    clearEvents();
    return;
  }
  //----------------------------------- EXAMINE
  else if(d.key_ == 'a') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      Examine::playerExamine();
    }
    clearEvents();
    return;
  }
  //----------------------------------- RELOAD
  else if(d.key_ == 'r') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      Reload::reloadWieldedWpn(*(Map::player));
    }
    clearEvents();
    return;
  }
  //----------------------------------- KICK
  else if((d.key_ == 'q')) {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      Kick::playerKick();
      Renderer::drawMapAndInterface();
    }
    clearEvents();
    return;
  }
  //----------------------------------- CLOSE
  else if(d.key_ == 'c') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      Close::playerClose();
    }
    clearEvents();
    return;
  }
  //----------------------------------- JAM
  else if(d.key_ == 'D') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      JamWithSpike::playerJam();
    }
    clearEvents();
    return;
  }
  //----------------------------------- DISARM
  else if(d.key_ == 'd') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      Disarm::playerDisarm();
    }
    clearEvents();
    return;
  }
  //----------------------------------- UNLOAD AMMO FROM GROUND
  else if(d.key_ == 'G')  {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      ItemPickup::tryUnloadWeaponOrPickupAmmoFromGround();
    }
    clearEvents();
    return;
  }
  //----------------------------------- AIM/FIRE FIREARM
  else if(d.key_ == 'f') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {

      if(Map::player->getPropHandler().allowAttackRanged(true)) {

        auto* const item = Map::player->getInv().getItemInSlot(SlotId::wielded);

        if(item) {
          const ItemDataT& itemData = item->getData();
          if(!itemData.isRangedWeapon) {
            Log::addMsg("I am not wielding a firearm.");
          } else {
            auto* wpn = static_cast<Weapon*>(item);
            if(wpn->nrAmmoLoaded >= 1 || itemData.rangedHasInfiniteAmmo) {
              Marker::run(MarkerTask::aimRangedWeapon, nullptr);
            } else if(Config::isRangedWpnAutoReload()) {
              Reload::reloadWieldedWpn(*(Map::player));
            } else {
              Log::addMsg("There is no ammo loaded.");
            }
          }
        } else {
          Log::addMsg("I am not wielding a weapon.");
        }
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- GET
  else if(d.key_ == 'g') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      const Pos& p = Map::player->pos;
      Item* const itemAtPlayer = Map::cells[p.x][p.y].item;
      if(itemAtPlayer) {
        if(itemAtPlayer->getData().id == ItemId::trapezohedron) {
          DungeonMaster::winGame();
          Init::quitToMainMenu = true;
        }
      }
      if(!Init::quitToMainMenu) {ItemPickup::tryPick();}
    }
    clearEvents();
    return;
  }
  //----------------------------------- SLOTS SCREEN
  else if(d.key_ == 'w') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      InvHandling::runSlotsScreen();
    }
    clearEvents();
    return;
  }
  //----------------------------------- INVENTORY
  else if(d.key_ == 'i') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      InvHandling::runBrowseInventory();
    }
    clearEvents();
    return;
  }
  //----------------------------------- USE
  else if(d.key_ == 'e') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      if(
        Map::player->dynamiteFuseTurns > 0 ||
        Map::player->flareFuseTurns > 0 ||
        Map::player->molotovFuseTurns > 0) {
        Marker::run(MarkerTask::aimLitExplosive, nullptr);
      } else {
        InvHandling::runUseScreen();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- SWAP TO PREPARED ITEM
  else if(d.key_ == 'z') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {

      const bool IS_FREE_TURN = PlayerBon::getBg() == Bg::warVet;

      const string swiftStr = IS_FREE_TURN ? " swiftly" : "";

      Inventory& inv = Map::player->getInv();

      Item* const wielded   = inv.getItemInSlot(SlotId::wielded);
      Item* const alt       = inv.getItemInSlot(SlotId::wieldedAlt);
      const string ALT_NAME =
        alt ? ItemData::getItemRef(*alt, ItemRefType::a) : "";
      if(wielded || alt) {
        if(wielded) {
          if(alt) {
            Log::addMsg("I" + swiftStr + " swap to my prepared weapon (" +
                        ALT_NAME + ").");
          } else {
            const string NAME = ItemData::getItemRef(*wielded, ItemRefType::a);
            Log::addMsg("I" + swiftStr + " put away my weapon (" + NAME + ").");
          }
        } else {
          Log::addMsg("I" + swiftStr + " wield my prepared weapon (" +
                      ALT_NAME + ").");
        }
        inv.swapWieldedAndPrepared(IS_FREE_TURN);
      } else {
        Log::addMsg("I have neither a wielded nor a prepared weapon.");
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- SEARCH (REALLY JUST A WAIT BUTTON)
  else if(d.key_ == 's') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      vector<Actor*> SpottedEnemies;
      Map::player->getSpottedEnemies(SpottedEnemies);
      if(SpottedEnemies.empty()) {
        const int TURNS_TO_APPLY = 5;
        const string TURNS_STR = toStr(TURNS_TO_APPLY);
        Log::addMsg("I pause for a while (" + TURNS_STR + " turns)...");
        Map::player->waitTurnsLeft = TURNS_TO_APPLY - 1;
        GameTime::actorDidAct();
      } else {
        Log::addMsg("Not while an enemy is near.");
        Renderer::drawMapAndInterface();
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- THROW ITEM
  else if(d.key_ == 't') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {

      if(Map::player->getPropHandler().allowAttackRanged(true)) {
        Inventory& playerInv = Map::player->getInv();
        Item* itemStack = playerInv.getItemInSlot(SlotId::missiles);

        if(itemStack) {
          Item* itemToThrow     = ItemFactory::copyItem(itemStack);
          itemToThrow->nrItems  = 1;

          const MarkerRetData markerReturnData =
            Marker::run(MarkerTask::aimThrownWeapon, itemToThrow);

          if(markerReturnData.didThrowMissile) {
            playerInv.decrItemInSlot(SlotId::missiles);
          } else {
            delete itemToThrow;
          }
        } else {
          Log::addMsg("I have no missiles chosen for throwing (press 'w').");
        }
      }
    }
    clearEvents();
    return;
  }
  //-----------------------------------  VIEW DESCRIPTIONS
  else if(d.key_ == 'v') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      if(Map::player->getPropHandler().allowSee()) {
        Marker::run(MarkerTask::look, nullptr);
      } else {
        Log::addMsg("Not while blind.");
      }
    }
    clearEvents();
    return;
  }
  //----------------------------------- AUTO MELEE
  else if(d.sdlKey_ == SDLK_TAB) {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      Map::player->autoMelee();
    }
    clearEvents();
    return;
  }
  //----------------------------------- RE-CAST PREVIOUS MEMORIZED SPELL
  else if(d.key_ == 'x') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      PlayerSpellsHandling::tryCastPrevSpell();
    }
    clearEvents();
    return;
  }
  //----------------------------------- MEMORIZED SPELLS
  else if(d.key_ == 'X') {
    Log::clearLog();
    if(Map::player->deadState == ActorDeadState::alive) {
      PlayerSpellsHandling::playerSelectSpellToCast();
    }
    clearEvents();
    return;
  }
  //----------------------------------- CHARACTER INFO
  else if(d.key_ == '@') {
    CharacterDescr::run();
    clearEvents();
    return;
  }
  //----------------------------------- LOG HISTORY
  else if(d.key_ == 'm') {
    Log::displayHistory();
    clearEvents();
    return;
  }
  //----------------------------------- MENU
  else if(d.sdlKey_ == SDLK_ESCAPE) {
    if(Map::player->deadState == ActorDeadState::alive) {
      Log::clearLog();

      const vector<string> choices {"Options", "Manual", "Quit", "Cancel"};
      const int CHOICE = Popup::showMenuMsg("", true, choices);

      if(CHOICE == 0) {
        //---------------------------- Options
        Config::runOptionsMenu();
        Renderer::drawMapAndInterface();
      } else if(CHOICE == 1) {
        //---------------------------- Manual
        Manual::run();
        Renderer::drawMapAndInterface();
      } else if(CHOICE == 2) {
        //---------------------------- Quit
        queryQuit();
      }
    } else {
      Init::quitToMainMenu = true;
    }
    clearEvents();
    return;
  } else if(d.key_ == 'Q' /*&& IS_DEBUG_MODE*/) {
    //----------------------------------- MENU
    queryQuit();
    clearEvents();
    return;
  }
  //----------------------------------- DESCEND CHEAT
  else if(d.sdlKey_ == SDLK_F2) {
    if(IS_DEBUG_MODE) {
      DungeonClimb::travelDown(1);
      clearEvents();
    }
    return;
  }
  //----------------------------------- XP CHEAT
  else if(d.sdlKey_ == SDLK_F3) {
    if(IS_DEBUG_MODE) {
      DungeonMaster::playerGainXp(100);
      clearEvents();
    }
    return;
  }
  //----------------------------------- VISION CHEAT
  else if(d.sdlKey_ == SDLK_F4) {
    if(IS_DEBUG_MODE) {
      if(Init::isCheatVisionEnabled) {
        for(int y = 0; y < MAP_H; ++y) {
          for(int x = 0; x < MAP_W; ++x) {
            Map::cells[x][y].isSeenByPlayer = false;
            Map::cells[x][y].isExplored     = false;
          }
        }
        Init::isCheatVisionEnabled = false;
      } else {
        Init::isCheatVisionEnabled = true;
      }
      Map::player->updateFov();
      Renderer::drawMapAndInterface();
    }
    clearEvents();
  }
  //----------------------------------- INSANITY CHEAT
  else if(d.sdlKey_ == SDLK_F5) {
    if(IS_DEBUG_MODE) {
      Map::player->incrShock(50, ShockSrc::misc);
      clearEvents();
    }
    return;
  }
  //----------------------------------- DROP ITEMS AROUND PLAYER
  else if(d.sdlKey_ == SDLK_F6) {
    if(IS_DEBUG_MODE) {
      for(int i = 1; i < int(ItemId::END); ++i) {
        const ItemDataT* const data = ItemData::data[i];
        if(
          !data->isIntrinsic &&
          (data->isPotion || data->isScroll || data->isDevice)) {
          ItemFactory::mkItemOnMap((ItemId)(i), Map::player->pos);
        }
      }
      clearEvents();
    }
    return;
  }
  //----------------------------------- TELEPORT
  else if(d.sdlKey_ == SDLK_F7) {
    if(IS_DEBUG_MODE) {
      Map::player->teleport(false);
      Log::clearLog();
      clearEvents();
    }
    return;
  }
  //----------------------------------- INFECTED
  else if(d.sdlKey_ == SDLK_F8) {
    if(IS_DEBUG_MODE) {
      Map::player->getPropHandler().tryApplyProp(
        new PropInfected(propTurnsStd));
      clearEvents();
    }
    return;
  }

  //----------------------------------- POSSESSED BY ZUUL
  else if(d.sdlKey_ == SDLK_F9) {
    if(IS_DEBUG_MODE) {
      for(Actor* actor : GameTime::actors_) {
        actor->getPropHandler().tryApplyProp(
          new PropPossessedByZuul(propTurnsIndefinite), true);
      }
    }
    return;
  }

  //----------------------------------- UNDEFINED COMMANDS
  else if(d.key_ != -1) {
    string cmdTried = " ";
    cmdTried.at(0) = d.key_;
    Log::clearLog();
    Log::addMsg("Unknown command '" + cmdTried + "'.");
    clearEvents();
    return;
  }
}

void setKeyRepeatDelays() {
  TRACE_FUNC_BEGIN;
  const int DELAY     = Config::getKeyRepeatDelay();
  const int INTERVAL  = Config::getKeyRepeatInterval();
  SDL_EnableKeyRepeat(DELAY, INTERVAL);
  TRACE_FUNC_END;
}

void handleMapModeInputUntilFound() {
  if(sdlEvent_) {
    const KeyboardReadRetData& d = readKeysUntilFound();
    if(!Init::quitToMainMenu) {handleKeyPress(d);}
  }
}

void clearEvents() {
  if(sdlEvent_) {while(SDL_PollEvent(sdlEvent_)) {}}
}

KeyboardReadRetData readKeysUntilFound(const bool IS_O_RETURN) {
  if(!sdlEvent_) {return KeyboardReadRetData();}

  while(true) {
    SdlWrapper::sleep(1);

    while(SDL_PollEvent(sdlEvent_)) {
      if(sdlEvent_->type == SDL_QUIT) {
        return KeyboardReadRetData(SDLK_ESCAPE);
      } else if(sdlEvent_->type == SDL_KEYDOWN) {
        // ASCII char entered?
        // Decimal unicode:
        // '!' = 33
        // '~' = 126
        Uint16 unicode = sdlEvent_->key.keysym.unicode;
        if((unicode == 'o' || unicode == 'O') && IS_O_RETURN) {
          return KeyboardReadRetData(-1, SDLK_RETURN, unicode == 'O', false);
        } else if(unicode >= 33 && unicode < 126) {
          clearEvents();
          return KeyboardReadRetData(char(unicode));
        } else {
          //Other key pressed? (escape, return, space, etc)
          const SDLKey sdlKey = sdlEvent_->key.keysym.sym;

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
                  Config::toggleFullscreen();
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

