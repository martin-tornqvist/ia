#include "Actor.h"

#include "Engine.h"

#include "Renderer.h"
#include "ItemArmor.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Fov.h"
#include "PlayerVisualMemory.h"
#include "Log.h"
#include "Blood.h"
#include "FeatureTrap.h"
#include "ItemDrop.h"
#include "Explosion.h"
#include "DungeonMaster.h"
#include "Inventory.h"
#include "MapParsing.h"

using namespace std;

Actor::Actor(Engine& engine) :
  pos(),
  deadState(actorDeadState_alive),
  eng(engine),
  clr_(clrBlack),
  glyph_(' '),
  tile_(tile_empty),
  hp_(-1),
  spi_(-1),
  lairCell_(),
  propHandler_(NULL),
  data_(NULL),
  inventory_(NULL) {}

Actor::~Actor() {
  delete propHandler_;
  delete inventory_;
}

bool Actor::isSpottingHiddenActor(Actor& other) {
  const Pos& otherPos = other.pos;

  const int OTHER_SNEAK_BASE =
    other.getData().abilityVals.getVal(ability_stealth, true, other);

  const int  DIST              = eng.basicUtils->chebyshevDist(pos, otherPos);
  const int  DIST_BON          = getConstrInRange(0, (DIST - 1) * 10, 60);
  const bool IS_LGT            = eng.map->cells[otherPos.x][otherPos.y].isLight;
  const int  LGT_DIV           = IS_LGT ? 2 : 1;
  const int  OTHER_SNEAK_SKILL =
    getConstrInRange(0, (OTHER_SNEAK_BASE + DIST_BON) / LGT_DIV, 90);

  return eng.abilityRoll->roll(OTHER_SNEAK_SKILL) <= failSmall;
}

int Actor::getHpMax(const bool WITH_MODIFIERS) const {
  return WITH_MODIFIERS ? propHandler_->getChangedMaxHp(hpMax_) : hpMax_;
}

bool Actor::checkIfSeeActor(
  const Actor& other, const bool visionBlockingCells[MAP_W][MAP_H]) const {

  if(other.deadState == actorDeadState_alive) {
    if(this == &other) {
      return true;
    }

    if(this == eng.player) {
      const bool IS_MONSTER_SNEAKING =
        dynamic_cast<const Monster*>(&other)->isStealth;
      return eng.map->cells[other.pos.x][other.pos.y].isSeenByPlayer &&
             IS_MONSTER_SNEAKING == false;
    }

    if(
      dynamic_cast<const Monster*>(this)->leader ==
      eng.player && &other != eng.player) {
      const bool IS_MONSTER_SNEAKING =
        dynamic_cast<const Monster*>(&other)->isStealth;
      if(IS_MONSTER_SNEAKING) return false;
    }

    if(propHandler_->allowSee() == false) {
      return false;
    }

    if(pos.x - other.pos.x > FOV_STD_RADI_INT) return false;
    if(other.pos.x - pos.x > FOV_STD_RADI_INT) return false;
    if(other.pos.y - pos.y > FOV_STD_RADI_INT) return false;
    if(pos.y - other.pos.y > FOV_STD_RADI_INT) return false;

    if(visionBlockingCells != NULL) {
      const bool IS_BLOCKED_BY_DARKNESS = data_->canSeeInDarkness == false;
      return eng.fov->checkCell(
               visionBlockingCells, other.pos, pos, IS_BLOCKED_BY_DARKNESS);
    }
  }
  return false;
}

void Actor::getSpottedEnemies(vector<Actor*>& vectorRef) {
  vectorRef.resize(0);

  const bool IS_SELF_PLAYER = this == eng.player;

  bool visionBlockers[MAP_W][MAP_H];

  if(IS_SELF_PLAYER == false) {
    MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  }

  const int NR_ACTORS = eng.gameTime->getNrActors();
  for(int i = 0; i < NR_ACTORS; i++) {
    Actor& actor = eng.gameTime->getActorAtElement(i);
    if(&actor != this && actor.deadState == actorDeadState_alive) {

      if(IS_SELF_PLAYER) {
        if(dynamic_cast<Monster*>(&actor)->leader != this) {
          if(checkIfSeeActor(actor, NULL)) {
            vectorRef.push_back(&actor);
          }
        }
      } else {
        const bool IS_OTHER_PLAYER = &actor == eng.player;
        const bool IS_HOSTILE_TO_PLAYER =
          dynamic_cast<Monster*>(this)->leader != eng.player;
        const bool IS_OTHER_HOSTILE_TO_PLAYER =
          IS_OTHER_PLAYER ? false :
          dynamic_cast<Monster*>(&actor)->leader != eng.player;

        //Note that IS_OTHER_HOSTILE_TO_PLAYER is false if other IS the player,
        //there is no need to check if IS_HOSTILE_TO_PLAYER && IS_OTHER_PLAYER
        if(
          (IS_HOSTILE_TO_PLAYER && IS_OTHER_HOSTILE_TO_PLAYER == false) ||
          (IS_HOSTILE_TO_PLAYER == false && IS_OTHER_HOSTILE_TO_PLAYER)) {
          if(checkIfSeeActor(actor, visionBlockers)) {
            vectorRef.push_back(&actor);
          }
        }
      }
    }
  }
}

void Actor::place(const Pos& pos_, ActorData& data) {
  pos             = pos_;
  data_           = &data;
  inventory_      = new Inventory(data_->isHumanoid);
  propHandler_    = new PropHandler(this, eng);
  deadState       = actorDeadState_alive;
  clr_            = data_->color;
  glyph_          = data_->glyph;
  tile_           = data_->tile;
  hp_             = hpMax_  = data_->hp;
  spi_            = spiMax_ = data_->spi;
  lairCell_       = pos;

  if(this != eng.player) {spawnStartItems();}

  place_();

  updateColor();
}

void Actor::teleport(const bool MOVE_TO_POS_AWAY_FROM_MONSTERS) {
  (void)MOVE_TO_POS_AWAY_FROM_MONSTERS;

  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksBodyType(getBodyType(), true, eng), blockers);
  vector<Pos> freeCells;
  eng.basicUtils->makeVectorFromBoolMap(false, blockers, freeCells);
  const Pos CELL = freeCells.at(eng.dice(1, freeCells.size()) - 1);

  if(this == eng.player) {
    eng.player->updateFov();
    eng.renderer->drawMapAndInterface();
    eng.playerVisualMemory->updateVisualMemory();
  }

  pos = CELL;

  if(this == eng.player) {
    eng.player->updateFov();
    eng.renderer->drawMapAndInterface();
    eng.playerVisualMemory->updateVisualMemory();
    eng.log->addMsg("I suddenly find myself in a different location!");
    propHandler_->tryApplyProp(new PropConfused(eng, propTurnsSpecified, 8));
  }
}

void Actor::updateColor() {
  if(deadState != actorDeadState_alive) {
    clr_ = clrRed;
    return;
  }

  if(propHandler_->changeActorClr(clr_)) {
    return;
  }

  clr_ = data_->color;
}

bool Actor::restoreHp(const int HP_RESTORED,
                      const bool ALLOW_MESSAGES,
                      const bool IS_ALLOWED_ABOVE_MAX) {
  bool isHpGained = IS_ALLOWED_ABOVE_MAX;

  const int DIF_FROM_MAX = getHpMax(true) - HP_RESTORED;

  //If hp is below limit, but restored hp will push it over the limit,
  //hp is set to max.
  if(
    IS_ALLOWED_ABOVE_MAX == false &&
    getHp() > DIF_FROM_MAX &&
    getHp() < getHpMax(true)) {

    hp_ = getHpMax(true);
    isHpGained = true;
  }

  //If hp is below limit, and restored hp will NOT push it
  //over the limit - restored hp is added to current.
  if(
    IS_ALLOWED_ABOVE_MAX ||
    getHp() <= DIF_FROM_MAX) {

    hp_ += HP_RESTORED;
    isHpGained = true;
  }

  updateColor();

  if(ALLOW_MESSAGES) {
    if(isHpGained) {
      if(this == eng.player) {
        eng.log->addMsg("I feel healthier!", clrMsgGood);
      } else {
        if(eng.player->checkIfSeeActor(*this, NULL)) {
          eng.log->addMsg(data_->name_the + " looks healthier.");
        }
      }
      eng.renderer->drawMapAndInterface();
    }
  }

  return isHpGained;
}

bool Actor::restoreSpi(const int SPI_RESTORED,
                       const bool ALLOW_MESSAGES,
                       const bool IS_ALLOWED_ABOVE_MAX) {
  bool isSpiGained = IS_ALLOWED_ABOVE_MAX;

  const int DIF_FROM_MAX = getSpiMax() - SPI_RESTORED;

  //If spi is below limit, but restored spi will push it over the limit,
  //spi is set to max.
  if(
    IS_ALLOWED_ABOVE_MAX == false &&
    getSpi() > DIF_FROM_MAX &&
    getSpi() < getSpiMax()) {

    spi_ = getSpiMax();
    isSpiGained = true;
  }

  //If spi is below limit, and restored spi will NOT push it
  //over the limit - restored spi is added to current.
  if(
    IS_ALLOWED_ABOVE_MAX ||
    getSpi() <= DIF_FROM_MAX) {

    spi_ += SPI_RESTORED;
    isSpiGained = true;
  }

  if(ALLOW_MESSAGES) {
    if(isSpiGained) {
      if(this == eng.player) {
        eng.log->addMsg("I feel more spirited!", clrMsgGood);
      } else {
        if(eng.player->checkIfSeeActor(*this, NULL)) {
          eng.log->addMsg(data_->name_the + " looks more spirited.");
        }
      }
      eng.renderer->drawMapAndInterface();
    }
  }

  return isSpiGained;
}

void Actor::changeMaxHp(const int CHANGE, const bool ALLOW_MESSAGES) {
  hpMax_  = max(1, hpMax_ + CHANGE);
  hp_     = max(1, hp_ + CHANGE);

  if(ALLOW_MESSAGES) {
    if(this == eng.player) {
      if(CHANGE > 0) {
        eng.log->addMsg("I feel more vigorous!");
      }
      if(CHANGE < 0) {
        eng.log->addMsg("I feel frailer!");
      }
    } else {
      if(eng.player->checkIfSeeActor(*this, NULL)) {
        if(CHANGE > 0) {
          eng.log->addMsg(getNameThe() + " looks more vigorous.");
        }
        if(CHANGE < 0) {
          eng.log->addMsg(getNameThe() + " looks frailer.");
        }
      }
    }
  }
}

void Actor::changeMaxSpi(const int CHANGE, const bool ALLOW_MESSAGES) {
  spiMax_ = max(1, spiMax_ + CHANGE);
  spi_    = max(1, spi_ + CHANGE);

  if(ALLOW_MESSAGES) {
    if(this == eng.player) {
      if(CHANGE > 0) {
        eng.log->addMsg("My spirit is stronger!");
      }
      if(CHANGE < 0) {
        eng.log->addMsg("My spirit is weaker!");
      }
    } else {
      if(eng.player->checkIfSeeActor(*this, NULL)) {
        if(CHANGE > 0) {
          eng.log->addMsg(getNameThe() + " appears to grow in spirit.");
        }
        if(CHANGE < 0) {
          eng.log->addMsg(getNameThe() + " appears to shrink in spirit.");
        }
      }
    }
  }
}

bool Actor::hit(int dmg, const DmgTypes_t dmgType, const bool ALLOW_WOUNDS) {
  traceVerbose << "Actor::hit()..." << endl;
  traceVerbose << "Actor: Damage from parameter: " << dmg << endl;

  if(
    dmgType == dmgType_light &&
    propHandler_->hasProp(propLightSensitive) == false) {
    return false;
  }

  onMonsterHit(dmg);
  traceVerbose << "Actor: Damage after onMonsterHit(): " << dmg << endl;

  dmg = max(1, dmg);

  if(dmgType == dmgType_spirit) {
    return hitSpi(dmg);
  }

  //Property resists?
  const bool ALLOW_DMG_RES_MSG = deadState == actorDeadState_alive;
  if(propHandler_->tryResistDmg(dmgType, ALLOW_DMG_RES_MSG)) {
    return false;
  }

  //Filter damage through worn armor
  if(isHumanoid()) {
    Armor* armor =
      dynamic_cast<Armor*>(inventory_->getItemInSlot(slot_armorBody));
    if(armor != NULL) {
      traceVerbose << "Actor: Has armor, running hit on armor" << endl;

      if(dmgType == dmgType_physical) {
        dmg = armor->takeDurabilityHitAndGetReducedDamage(dmg);
      }

      if(armor->isDestroyed()) {
        trace << "Actor: Armor was destroyed" << endl;
        if(this == eng.player) {
          eng.log->addMsg("My " + eng.itemDataHandler->getItemRef(
                            *armor, itemRef_plain) + " is torn apart!");
        }
        delete armor;
        armor = NULL;
        inventory_->getSlot(slot_armorBody)->item = NULL;
      }
    }
  }

  propHandler_->onHit();

  hit_(dmg, ALLOW_WOUNDS);

  //Damage to corpses
  if(deadState != actorDeadState_alive) {
    if(dmg >= getHpMax(true) / 2) {
      deadState = actorDeadState_mangled;
      glyph_ = ' ';
      if(isHumanoid()) {
        eng.gore->makeGore(pos);
      }
    }
    traceVerbose << "Actor::hit() [DONE]" << endl;
    return false;
  }

  if(this != eng.player || eng.config->isBotPlaying == false) {
    hp_ -= dmg;
  }

  const bool IS_ON_BOTTOMLESS =
    eng.map->cells[pos.x][pos.y].featureStatic->isBottomless();
  const bool IS_MANGLED =
    IS_ON_BOTTOMLESS == true ? true :
    (dmg > ((getHpMax(true) * 5) / 4) ? true : false);
  if(getHp() <= 0) {
    die(IS_MANGLED, IS_ON_BOTTOMLESS == false, IS_ON_BOTTOMLESS == false);
    die_();
    traceVerbose << "Actor::hit() [DONE]" << endl;
    return true;
  } else {
    traceVerbose << "Actor::hit() [DONE]" << endl;
    return false;
  }
}

bool Actor::hitSpi(const int DMG) {
  if(this != eng.player || eng.config->isBotPlaying == false) {
    spi_ = max(0, spi_ - DMG);
  }
  if(getSpi() <= 0) {
    if(this == eng.player) {
      eng.log->addMsg(
        "All my spirit is depleted, I am devoid of life!");
    } else {
      if(eng.player->checkIfSeeActor(*this, NULL)) {
        eng.log->addMsg(getNameThe() + " has no spirit left!");
      }
    }
    die(false, false, true);
    return true;
  }
  return false;
}

void Actor::die(const bool IS_MANGLED, const bool ALLOW_GORE,
                const bool ALLOW_DROP_ITEMS) {
  //Check all monsters and unset this actor as leader
  const int NR_ACTORS = eng.gameTime->getNrActors();
  for(int i = 0; i < NR_ACTORS; i++) {
    Actor* const actor = &(eng.gameTime->getActorAtElement(i));
    if(actor != this && actor != eng.player) {
      Monster* const monster = dynamic_cast<Monster*>(actor);
      if(monster->leader == this) {
        monster->leader = NULL;
      }
    }
  }

  if(this != eng.player) {
    if(isHumanoid() == true) {
      eng.soundEmitter->emitSound(
        Sound("I hear agonised screaming.", endOfSfx, true, pos, false, false));
    }
    dynamic_cast<Monster*>(this)->leader = NULL;
  }

  bool diedOnVisibleTrap = false;

  //If died on a visible trap, always destroy the corpse
  const Feature* const f = eng.map->cells[pos.x][pos.y].featureStatic;
  if(f->getId() == feature_trap) {
    if(dynamic_cast<const Trap*>(f)->isHidden() == false) {
      diedOnVisibleTrap = true;
    }
  }

  bool isPlayerSeeDyingActor = true;

  //Print death messages
  if(this != eng.player) {
    //Only print if visible
    if(eng.player->checkIfSeeActor(*this, NULL)) {
      isPlayerSeeDyingActor = true;
      const string deathMessageOverride = data_->deathMessageOverride;
      if(deathMessageOverride.empty() == false) {
        eng.log->addMsg(deathMessageOverride);
      } else {
        eng.log->addMsg(getNameThe() + " dies.");
      }
    }
  }

  //If mangled because of damage, or if a monster died on a visible trap,
  //gib the corpse.
  deadState =
    (IS_MANGLED || (diedOnVisibleTrap && this != eng.player)) ?
    actorDeadState_mangled : actorDeadState_corpse;

  if(ALLOW_DROP_ITEMS) {
    eng.itemDrop->dropAllCharactersItems(this, true);
  }

  if(IS_MANGLED) {
    glyph_ = ' ';
    tile_ = tile_empty;
    if(isHumanoid() == true) {
      if(ALLOW_GORE) {
        eng.gore->makeGore(pos);
      }
    }
  } else {
    if(this != eng.player) {
      Pos newPos;
      Feature* featureHere = eng.map->cells[pos.x][pos.y].featureStatic;
      //TODO this should be decided with a floodfill instead
      if(featureHere->canHaveCorpse() == false) {
        for(int dx = -1; dx <= 1; dx++) {
          for(int dy = -1; dy <= 1; dy++) {
            newPos = pos + Pos(dx, dy);
            featureHere =
              eng.map->cells[pos.x + dx][pos.y + dy].featureStatic;
            if(featureHere->canHaveCorpse()) {
              pos.set(newPos);
              dx = 9999;
              dy = 9999;
            }
          }
        }
      }
    }
    glyph_ = '&';
    tile_ = tile_corpse2;
  }

  clr_ = clrRedLgt;

  onMonsterDeath();

  propHandler_->onDeath(isPlayerSeeDyingActor);

  //Give exp if monster, and count up nr of kills.
  if(this != eng.player) {
    eng.dungeonMaster->monsterKilled(this);
  }

  eng.renderer->drawMapAndInterface();
}

void Actor::addLight(bool light[MAP_W][MAP_H]) const {
  if(propHandler_->hasProp(propBurning)) {
    for(int dy = -1; dy <= 1; dy++) {
      for(int dx = -1; dx <= 1; dx++) {
        light[pos.x + dx][pos.y + dy] = true;
      }
    }
  }

  addLight_(light);
}
