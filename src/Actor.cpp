#include "Actor.h"

#include "Engine.h"
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

using namespace std;

Actor::~Actor() {
  delete statusEffectsHandler_;
  delete inventory_;
}

void Actor::newTurn() {
  if(statusEffectsHandler_->allowAct()) {
    updateColor();
    act();
  } else {
    eng->gameTime->letNextAct();
  }
}

bool Actor::checkIfSeeActor(const Actor& other, bool visionBlockingCells[MAP_X_CELLS][MAP_Y_CELLS]) const {
  if(other.deadState == actorDeadState_alive) {
    if(this == &other) {
      return true;
    }

    if(this == eng->player) {
      const bool IS_MONSTER_SNEAKING = dynamic_cast<const Monster*>(&other)->isStealth;
      return eng->map->playerVision[other.pos.x][other.pos.y] && IS_MONSTER_SNEAKING == false;
    }

    if(dynamic_cast<const Monster*>(this)->leader == eng->player && &other != eng->player) {
      const bool IS_MONSTER_SNEAKING = dynamic_cast<const Monster*>(&other)->isStealth;
      if(IS_MONSTER_SNEAKING) return false;
    }

    if(statusEffectsHandler_->allowSee() == false) {
      return false;
    }

    if(pos.x - other.pos.x > FOV_STANDARD_RADI_INT) return false;
    if(other.pos.x - pos.x > FOV_STANDARD_RADI_INT)	return false;
    if(other.pos.y - pos.y > FOV_STANDARD_RADI_INT)	return false;
    if(pos.y - other.pos.y > FOV_STANDARD_RADI_INT)	return false;

    if(visionBlockingCells != NULL) {
      return eng->fov->checkOneCell(visionBlockingCells, other.pos, pos, true);
    }
  }
  return false;
}

void Actor::getSpotedEnemies() {
  spotedEnemies.resize(0);

  const bool IS_SELF_PLAYER = this == eng->player;

  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];

  if(IS_SELF_PLAYER == false) {
    eng->mapTests->makeVisionBlockerArray(pos, visionBlockers);
  }

  const unsigned int SIZE_OF_LOOP = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < SIZE_OF_LOOP; i++) {
    Actor* const actor = eng->gameTime->getActorAt(i);
    if(actor != this && actor->deadState == actorDeadState_alive) {

      if(IS_SELF_PLAYER) {
        if(dynamic_cast<Monster*>(actor)->leader != this) {
          if(checkIfSeeActor(*actor, NULL)) {
            spotedEnemies.push_back(actor);
          }
        }
      } else {
        const bool IS_OTHER_PLAYER = actor == eng->player;
        const bool IS_SELF_HOSTILE_TO_PLAYER = dynamic_cast<Monster*>(this)->leader != eng->player;
        const bool IS_OTHER_HOSTILE_TO_PLAYER = IS_OTHER_PLAYER ? false : dynamic_cast<Monster*>(actor)->leader != eng->player;

        //Note that IS_OTHER_HOSTILE_TO_PLAYER is false if the other IS the player,
        //so there is no need to check if IS_SELF_HOSTILE_TO_PLAYER && IS_OTHER_PLAYER
        if(
          (IS_SELF_HOSTILE_TO_PLAYER == true && IS_OTHER_HOSTILE_TO_PLAYER == false) ||
          (IS_SELF_HOSTILE_TO_PLAYER == false && IS_OTHER_HOSTILE_TO_PLAYER == true)) {
          if(checkIfSeeActor(*actor, visionBlockers)) {
            spotedEnemies.push_back(actor);
          }
        }
      }
    }
  }
}

void Actor::getSpotedEnemiesPositions() {
  spotedEnemiesPositions.resize(0);
  getSpotedEnemies();

  const unsigned int SIZE_OF_LOOP = spotedEnemies.size();
  for(unsigned int i = 0; i < SIZE_OF_LOOP; i++) {
    const Actor* const actor = spotedEnemies.at(i);
    if(actor != NULL) {
      spotedEnemiesPositions.push_back(actor->pos);
    }
  }
}

void Actor::place(const coord& pos_, ActorDefinition* const actorDefinition, Engine* engine) {
  eng = engine;
  pos = pos_;
  def_ = actorDefinition;
  inventory_ = new Inventory(def_->isHumanoid);
  statusEffectsHandler_ = new StatusEffectsHandler(this, eng);
  deadState = actorDeadState_alive;
  clr_ = def_->color;
  glyph_ = def_->glyph;
  tile_ = def_->tile;
  hp_ = def_->hpMax;
  hpMax_ = def_->hpMax;
  lairCell_ = pos;

  if(this != eng->player) {
    actorSpecific_spawnStartItems();
  }

  updateColor();
}

void Actor::changeMaxHP(const int CHANGE, const bool ALLOW_MESSAGES) {
  hpMax_ = max(1, getHpMax());
  hp_ = max(1, getHp());

  if(ALLOW_MESSAGES) {
    if(this == eng->player) {
      if(CHANGE > 0) {
        eng->log->addMessage("I feel more vigorous!", clrMessageGood);
      }
      if(CHANGE < 0) {
        eng->log->addMessage("I feel frailer!", clrMessageBad);
      }
    } else {
      if(eng->map->playerVision[pos.x][pos.y] == true) {
        if(CHANGE > 0) {
          eng->log->addMessage(getNameThe() + " looks more vigorous.");
        }
        if(CHANGE < 0) {
          eng->log->addMessage(getNameThe() + " looks frailer.");
        }
      }
    }
  }
}

void Actor::teleportToRandom() {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArray(this, blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  vector<coord> freeCells;
  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);
  const coord CELL = freeCells.at(eng->dice(1, freeCells.size()) - 1);

  if(this == eng->player) {
    eng->player->FOVupdate();
    eng->renderer->drawMapAndInterface();
    eng->playerVisualMemory->updateVisualMemory();
  }

  pos = CELL;

  if(this == eng->player) {
    eng->player->FOVupdate();
    eng->renderer->drawMapAndInterface();
    eng->playerVisualMemory->updateVisualMemory();
    eng->log->addMessage("I suddenly find myself in a different location!");
    eng->renderer->flip();
    statusEffectsHandler_->attemptAddEffect(new StatusConfused(8 + eng->dice(1, 8)));
  }
}

void Actor::updateColor() {
  if(deadState != actorDeadState_alive) {
    clr_ = clrRed;
    return;
  }

  const SDL_Color clrFromStatusEffect = statusEffectsHandler_->getColor();
  if(clrFromStatusEffect.r != 0 || clrFromStatusEffect.g != 0 || clrFromStatusEffect.b != 0) {
    clr_ = clrFromStatusEffect;
    return;
  }

  clr_ = def_->color;
}

bool Actor::restoreHP(int hpRestored, const bool ALLOW_MESSAGE) {
  bool IS_HP_GAINED = false;

  const int DIF_FROM_MAX = hpMax_ - hpRestored;

  //If hp is below limit, but restored hp will push it
  //over the limit - hp is set to max.
  if(getHp() > DIF_FROM_MAX && getHp() < getHpMax()) {
    hp_ = getHpMax();
    IS_HP_GAINED = true;
  }

  //If hp is below limit, and restored hp will NOT push it
  //over the limit - restored hp is added to current.
  if(getHp() <= DIF_FROM_MAX) {
    hp_ += hpRestored;
    IS_HP_GAINED = true;
  }

  updateColor();

  if(ALLOW_MESSAGE) {
    if(IS_HP_GAINED) {
      if(this == eng->player) {
        eng->log->addMessage("I feel healthier!", clrMessageGood);
      } else {
        if(eng->player->checkIfSeeActor(*this, NULL)) {
          eng->log->addMessage(def_->name_the + " looks healthier.");
        }
      }
      eng->renderer->drawMapAndInterface();
    }
  }

  return IS_HP_GAINED;
}

bool Actor::hit(int dmg, const DamageTypes_t damageType) {
  tracer << "Actor::hit()..." << endl;
  monsterHit();

  dmg = max(1, dmg);

  //Filter damage through worn armor
  if(isHumanoid()) {
    Armor* armor = dynamic_cast<Armor*>(inventory_->getItemInSlot(slot_armorBody));
    if(armor != NULL) {
      tracer << "Actor: Has armor, running hit on armor" << endl;

      dmg = armor->takeDurabilityHitAndGetReducedDamage(dmg, damageType);

      if(armor->isDestroyed()) {
        tracer << "Actor: Armor was destroyed" << endl;
        if(this == eng->player) {
          eng->log->addMessage("My " + armor->getDef().name.name + " is torn apart!");
        }
        delete armor;
        armor = NULL;
        inventory_->getSlot(slot_armorBody)->item = NULL;
      }
    }
  }

  // Filter damage through intrinsic armor
  // [TODOO] Stuff goes here


  statusEffectsHandler_->isHit();

  actorSpecific_hit(dmg);

  //Damage to corpses
  if(deadState != actorDeadState_alive) {
    if(dmg >= getHpMax() / 2) {
      deadState = actorDeadState_mangled;
      glyph_ = ' ';
      if(isHumanoid()) {
        eng->gore->makeGore(pos);
      }
    }
    tracer << "Actor::hit() [DONE]" << endl;
    return false;
  }

  if(this != eng->player || eng->config->BOT_PLAYING == false) {
    hp_ -= dmg;
  }

  const bool IS_ON_BOTTOMLESS = eng->map->featuresStatic[pos.x][pos.y]->isBottomless();
  const bool IS_MANGLED = IS_ON_BOTTOMLESS == true ? true : (dmg > ((getHpMax() * 5) / 4) ? true : false);
  if(getHp() <= 0) {
    die(IS_MANGLED, !IS_ON_BOTTOMLESS, !IS_ON_BOTTOMLESS);
    actorSpecificDie();
    tracer << "Actor::hit() [DONE]" << endl;
    return true;
  } else {
    tracer << "Actor::hit() [DONE]" << endl;
    return false;
  }
}

void Actor::die(const bool MANGLED, const bool ALLOW_GORE, const bool ALLOW_DROP_ITEMS) {
  //Check all monsters and unset this actor as leader
  for(unsigned int i = 0; i < eng->gameTime->getLoopSize(); i++) {
    Actor* const actor = eng->gameTime->getActorAt(i);
    if(actor != this && actor != eng->player) {
      Monster* const monster = dynamic_cast<Monster*>(actor);
      if(monster->leader == this) {
        monster->leader = NULL;
      }
    }
  }

  if(this != eng->player) {
    if(isHumanoid() == true) {
      eng->soundEmitter->emitSound(Sound("I hear agonised screaming.", true, pos, false, false));
    }
    dynamic_cast<Monster*>(this)->leader = NULL;
  }

  bool diedOnVisibleTrap = false;

  //If died on a visible trap, always destroy the corpse
  const Feature* const f = eng->map->featuresStatic[pos.x][pos.y];
  if(f->getId() == feature_trap) {
    if(dynamic_cast<const Trap*>(f)->isHidden() == false) {
      diedOnVisibleTrap = true;
    }
  }

  //Print death messages
  if(this != eng->player) {
    //Only print if visible
    if(eng->player->checkIfSeeActor(*this, NULL)) {
      const string deathMessageOverride = def_->deathMessageOverride;
      if(deathMessageOverride != "") {
        eng->log->addMessage(deathMessageOverride);
      } else {
        eng->log->addMessage(getNameThe() + " dies.");
      }
    }
  }

  //If mangled because of damage, or if a monster died on a visible trap, gib the corpse.
  deadState = (MANGLED == true || (diedOnVisibleTrap == true && this != eng->player)) ? actorDeadState_mangled : actorDeadState_corpse;

  if(ALLOW_DROP_ITEMS) {
    eng->itemDrop->dropAllCharactersItems(this, true);
  }

  //Died with a flare inside?
  if(def_->actorSize > actorSize_floor) {
    if(statusEffectsHandler_->hasEffect(statusFlared)) {
      eng->explosionMaker->runExplosion(pos, false, new StatusBurning(eng));
      deadState = actorDeadState_mangled;
    }
  }

  if(MANGLED == false) {
    coord newCoord;
    Feature* featureHere = eng->map->featuresStatic[pos.x][pos.y];
    //TODO this should be decided with a floodfill instead
    if(featureHere->canHaveCorpse() == false) {
      for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {
          newCoord = pos + coord(dx, dy);
          featureHere = eng->map->featuresStatic[pos.x + dx][pos.y + dy];
          if(featureHere->canHaveCorpse() == true) {
            pos.set(newCoord);
            dx = 9999;
            dy = 9999;
          }
        }
      }
    }
    glyph_ = '&';
    tile_ = tile_corpse;
  } else {
    glyph_ = ' ';
    tile_ = tile_empty;
    if(isHumanoid() == true) {
      if(ALLOW_GORE) {
        eng->gore->makeGore(pos);
      }
    }
  }

  clr_ = clrRedLight;

  monsterDeath();

  //Give exp if monster, and count up nr of kills.
  if(this != eng->player) {
    eng->dungeonMaster->monsterKilled(this);
  }
}

void Actor::addLight(bool light[MAP_X_CELLS][MAP_Y_CELLS]) const {
  if(statusEffectsHandler_->hasEffect(statusBurning)) {
    for(int dy = -1; dy <= 1; dy++) {
      for(int dx = -1; dx <= 1; dx++) {
        light[pos.x + dx][pos.y + dy] = true;
      }
    }
  }

  actorSpecific_addLight(light);
}
