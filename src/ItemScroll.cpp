#include "ItemScroll.h"

#include "ItemData.h"
#include "Engine.h"
#include "StatusEffects.h"
#include "Log.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorFactory.h"
#include "DungeonClimb.h"
#include "FeatureTrap.h"
#include "Postmortem.h"
#include "Query.h"
#include "PlayerBonuses.h"
#include "Marker.h"


const int BLAST_ANIMATION_DELAY_FACTOR = 3;


void ScrollOfMayhem::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;

  const int NR_OF_SWEEPS = 5;

  engine->log->addMessage("Destruction rages around me!");

  const int PLAYER_X = engine->player->pos.x;
  const int PLAYER_Y = engine->player->pos.y;

  const int AREA_RADI = 8;

  for(int i = 0; i < NR_OF_SWEEPS; i++) {
    for(int y = max(1, PLAYER_Y - AREA_RADI); y < min(MAP_Y_CELLS - 1, PLAYER_Y + AREA_RADI); y++) {
      for(int x = max(1, PLAYER_X - AREA_RADI); x < min(MAP_X_CELLS - 1, PLAYER_X + AREA_RADI); x++) {
        const coord c(x, y);
        bool isAdjToWalkableCell = false;
        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {
            if(engine->map->featuresStatic[x + dx][y + dy]->isMoveTypePassable(moveType_walk)) {
              isAdjToWalkableCell = true;
            }
          }
        }
        if(isAdjToWalkableCell) {
          const int CHANCE_TO_DESTROY = 10;
          if(engine->dice(1, 100) < CHANCE_TO_DESTROY) {
            engine->map->switchToDestroyedFeatAt(c);
          }
        }
      }
    }
  }

  for(int y = max(1, PLAYER_Y - AREA_RADI); y < min(MAP_Y_CELLS - 1, PLAYER_Y + AREA_RADI); y++) {
    for(int x = max(1, PLAYER_X - AREA_RADI); x < min(MAP_X_CELLS - 1, PLAYER_X + AREA_RADI); x++) {
      if(engine->map->featuresStatic[x][y]->canHaveBlood()) {
        const int CHANCE_FOR_BLOOD = 10;
        if(engine->dice(1, 100) < CHANCE_FOR_BLOOD) {
          engine->map->featuresStatic[x][y]->setHasBlood(true);
        }
      }
    }
  }

  for(unsigned int i = 0; i < engine->gameTime->getLoopSize(); i++) {
    Actor* actor = engine->gameTime->getActorAt(i);
    if(actor != engine->player) {
      if(engine->player->checkIfSeeActor(*actor, NULL)) {
        actor->getStatusEffectsHandler()->attemptAddEffect(new StatusBurning(engine));
      }
    }
  }

  engine->soundEmitter->emitSound(Sound("", true, engine->player->pos, true, true));

  setRealDefinitionNames(engine, false);
}

void ScrollOfPestilence::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

  const int RADI = 4;
  const int x0 = max(0, engine->player->pos.x - RADI);
  const int y0 = max(0, engine->player->pos.y - RADI);
  const int x1 = min(MAP_X_CELLS - 1, engine->player->pos.x + RADI);
  const int y1 = min(MAP_Y_CELLS - 1, engine->player->pos.y + RADI);

  for(int x = x0; x <= x1; x++) {
    for(int y = y0; y <= y1; y++) {
      if(blockers[x][y] == false) {
        if(engine->dice(1, 3) == 1) {
          if(engine->dice.coinToss()) {
            if(engine->dice.coinToss()) {
              engine->actorFactory->spawnActor(actor_greenSpider, coord(x, y));
            } else {
              engine->actorFactory->spawnActor(actor_whiteSpider, coord(x, y));
            }
          } else {
            engine->actorFactory->spawnActor(actor_redSpider, coord(x, y));
          }
        } else {
          engine->actorFactory->spawnActor(actor_rat, coord(x, y));
        }
      }
    }
  }

  engine->log->addMessage("Disgusting critters appear around me!");
  setRealDefinitionNames(engine, false);
}

void ScrollOfDeepDescent::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;
  if(engine->map->getDungeonLevel() <= LAST_CAVERN_LEVEL - 3) {
    engine->dungeonClimb->travelDown(1);
    engine->log->addMessage("I sink downwards!");
    setRealDefinitionNames(engine, false);
  } else {
    engine->log->addMessage("Nothing happens.");
  }
}

void ScrollOfTeleportation::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;
  engine->player->teleportToRandom();
  setRealDefinitionNames(engine, false);
}

void ScrollOfStatusOnAllVisibleMonsters::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;
  engine->player->getSpotedEnemies();
  const vector<Actor*>& actors = engine->player->spotedEnemies;

  if(actors.size() > 0) {
    setRealDefinitionNames(engine, false);
    vector<coord> actorPositions;

    for(unsigned int i = 0; i < actors.size(); i++) {
      actorPositions.push_back(actors.at(i)->pos);
    }

    engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(actorPositions, clrMagenta,
        BLAST_ANIMATION_DELAY_FACTOR, engine);

    for(unsigned int i = 0; i < actors.size(); i++) {
      StatusEffect* const effect = getStatusEffect(engine);
      actors.at(i)->getStatusEffectsHandler()->attemptAddEffect(effect);
    }

  } else {
    failedToLearnRealName(engine);
  }
}

StatusEffect* ScrollOfConfuseEnemies::getStatusEffect(Engine* const engine) {
  return new StatusConfused(engine);
}

StatusEffect* ScrollOfParalyzeEnemies::getStatusEffect(Engine* const engine) {
  return new StatusParalyzed(engine);
}

StatusEffect* ScrollOfSlowEnemies::getStatusEffect(Engine* const engine) {
  return new StatusSlowed(engine);
}

void ScrollOfDetectItems::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;

  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      Item* item = engine->map->items[x][y];
      if(item != NULL) {
        engine->map->playerVision[x][y] = true;
        engine->map->explored[x][y] = true;
      }
    }
  }
  engine->renderer->drawMapAndInterface();
  engine->player->FOVupdate();
  engine->renderer->drawMapAndInterface();

  engine->log->addMessage("All items are revealed to me.");
  setRealDefinitionNames(engine, false);
}

void ScrollOfBlessing::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;

  engine->player->getStatusEffectsHandler()->attemptAddEffect(new StatusBlessed(engine));
  setRealDefinitionNames(engine, false);
}

void ScrollOfDetectTraps::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;
  bool somethingRevealed = false;

  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      FeatureStatic* const f = engine->map->featuresStatic[x][y];
      if(f->getId() == feature_trap) {
        Trap* const trap = dynamic_cast<Trap*>(f);
        trap->reveal(false);
        somethingRevealed = true;
      }
    }
  }

  if(somethingRevealed) {
    engine->log->addMessage("All traps are revealed to me.");
    setRealDefinitionNames(engine, false);
  } else {
    failedToLearnRealName(engine);
  }
}

void ScrollOfClairvoyance::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;
  engine->player->getStatusEffectsHandler()->attemptAddEffect(new StatusClairvoyant(engine), true, false);
  setRealDefinitionNames(engine, false);
}

void ScrollOfAzathothsBlast::specificRead(const bool FROM_MEMORY, Engine* const engine) {
  (void)FROM_MEMORY;
  engine->player->getSpotedEnemies();
  const vector<Actor*>& actors = engine->player->spotedEnemies;

  if(actors.size() > 0) {
    vector<coord> actorPositions;

    for(unsigned int i = 0; i < actors.size(); i++) {
      actorPositions.push_back(actors.at(i)->pos);
    }

    engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(actorPositions, clrRedLight,
        BLAST_ANIMATION_DELAY_FACTOR, engine);

    for(unsigned int i = 0; i < actors.size(); i++) {
      const string monsterName = actors.at(i)->getNameThe();
      engine->log->addMessage(monsterName + " is struck by a roaring blast!", clrMessageGood);
      actors.at(i)->getStatusEffectsHandler()->attemptAddEffect(new StatusParalyzed(1), false, false);
      actors.at(i)->hit(engine->dice(1, 8), damageType_physical);
    }

    setRealDefinitionNames(engine, false);
  } else {
    failedToLearnRealName(engine);
  }
}

void ScrollOfAzathothsBlast::castAt(const coord& pos, Engine* const engine) {
  engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(vector<coord>(1, pos), clrRedLight,
      BLAST_ANIMATION_DELAY_FACTOR, engine);
}

//void ScrollOfVoidChain::specificRead(const bool FROM_MEMORY, Engine* const engine) {
//  setRealDefinitionNames(engine, false);
//  if(engine->player->getStatusEffectsHandler()->allowAct()) {
//    engine->marker->place(markerTask_spellVoidChain);
//  } else {
//    engine->log->addMessage("My spell is disrupted.");
//  }
//}

//void ScrollOfVoidChain::castAt(const coord& pos, Engine* const engine) {
//  const coord playerPos = engine->player->pos;
//  const vector<coord> projectilePath =
//    engine->mapTests->getLine(playerPos.x, playerPos.y, pos.x, pos.y,
//                              true, FOV_STANDARD_RADI_INT);
//}

//void ScrollOfIbnGhazisPowder::specificRead(const bool FROM_MEMORY, Engine* const engine) {
//  setRealDefinitionNames(engine, false);
//  if(engine->player->getStatusEffectsHandler()->allowAct()) {
//    engine->query->direction();
//  } else {
//    engine->log->addMessage("My spell is disrupted.");
//  }
//}

void ScrollNameHandler::setFalseScrollName(ItemDefinition* d) {
  const unsigned int NR_NAMES = m_falseNames.size();

  const unsigned int ELEMENT = static_cast<unsigned int>(eng->dice(1, NR_NAMES) - 1);

  const string TITLE = "\"" + m_falseNames.at(ELEMENT) + "\"";

  m_falseNames.erase(m_falseNames.begin() + ELEMENT);

  d->name.name = "Manuscript titled " + TITLE;
  d->name.name_plural = "Manuscripts titled " + TITLE;
  d->name.name_a = "a Manuscript titled " + TITLE;
}

void ScrollNameHandler::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 1; i < endOfItemDevNames; i++) {
    if(eng->itemData->itemDefinitions[i]->isReadable == true) {
      lines.push_back(eng->itemData->itemDefinitions[i]->name.name);
      lines.push_back(eng->itemData->itemDefinitions[i]->name.name_plural);
      lines.push_back(eng->itemData->itemDefinitions[i]->name.name_a);
    }
  }
}

void ScrollNameHandler::setParametersFromSaveLines(vector<string>& lines) {
  for(unsigned int i = 1; i < endOfItemDevNames; i++) {
    if(eng->itemData->itemDefinitions[i]->isReadable == true) {
      eng->itemData->itemDefinitions[i]->name.name = lines.front();
      lines.erase(lines.begin());
      eng->itemData->itemDefinitions[i]->name.name_plural = lines.front();
      lines.erase(lines.begin());
      eng->itemData->itemDefinitions[i]->name.name_a = lines.front();
      lines.erase(lines.begin());
    }
  }
}

Scroll::~Scroll() {

}

int Scroll::getChanceToCastFromMemory(Engine* const engine) const {
  (void)engine;
//  const int PLAYER_SKILL = engine->player->getMythosKnowledge();
  const int BASE_CHANCE = def_->castFromMemoryChance;

  return BASE_CHANCE; //+ PLAYER_SKILL;
}

void Scroll::setCastFromMemoryChance(const int VAL) {
  def_->castFromMemoryChance = VAL;
}

void Scroll::setRealDefinitionNames(Engine* const engine, const bool IS_SILENT_IDENTIFY) {
  if(def_->isIdentified == false) {
    const string REAL_TYPE_NAME = getRealTypeName();

    const string REAL_NAME = "Manuscript of " + REAL_TYPE_NAME;
    const string REAL_NAME_PLURAL = "Manuscripts of " + REAL_TYPE_NAME;
    const string REAL_NAME_A = "a Manuscript of " + REAL_TYPE_NAME;

    def_->name.name = REAL_NAME;
    def_->name.name_plural = REAL_NAME_PLURAL;
    def_->name.name_a = REAL_NAME_A;

    if(IS_SILENT_IDENTIFY == false) {
      engine->log->addMessage("It was " + def_->name.name_a + ".");
      engine->renderer->drawMapAndInterface();
      engine->player->incrShock(shockValue_heavy);
    }

    def_->isIdentified = true;
  }
}

void Scroll::attemptMemorizeIfLearnable(Engine* const engine) {
  if(def_->isScrollLearned == false && def_->isScrollLearnable) {
    if(engine->playerBonusHandler->isBonusPicked(playerBonus_learned)) {
      const int CHANCE_TO_LEARN = 75;
      if(engine->dice.getInRange(0, 100) < CHANCE_TO_LEARN) {
        engine->log->addMessage("I learn to cast this incantation by heart!");
        def_->isScrollLearned = true;
      } else {
        engine->log->addMessage("I failed to memorize the incantation.");
      }
    }
  }
}

bool Scroll::read(const bool IS_FROM_MEMORY, Engine* const engine) {
  engine->renderer->drawMapAndInterface();

  if(IS_FROM_MEMORY) {
    return attemptReadFromMemory(engine);
  } else {
    return attemptReadFromScroll(engine);
  }
}

bool Scroll::attemptReadFromMemory(Engine* const engine) {
  const AbilityRollResult_t rollResult = engine->abilityRoll->roll(getChanceToCastFromMemory(engine));
  if(rollResult >= successSmall) {
    engine->log->addMessage("I cast " + getRealTypeName() + "...");
    specificRead(true, engine);
  } else {
    engine->log->addMessage("I miscast it.");
    engine->player->getStatusEffectsHandler()->attemptAddEffect(new StatusWeak(engine));
    if(engine->dice.coinToss()) {
      engine->log->addMessage("I feel a sharp pain in my head!", clrMessageBad);
    } else {
      engine->log->addMessage("It feels like a dagger piercing my skull!", clrMessageBad);
    }
    engine->player->getStatusEffectsHandler()->attemptAddEffect(new StatusParalyzed(engine), false, false);
    engine->player->hit(engine->dice(1, 6), damageType_pure);
  }
  if(engine->player->deadState == actorDeadState_alive) {
    engine->player->incrShock(shockValue_heavy);
    engine->gameTime->letNextAct();

    def_->castFromMemoryChance = engine->playerBonusHandler->isBonusPicked(playerBonus_erudite) ? 20 : 0;
  }
  return true;
}

bool Scroll::attemptReadFromScroll(Engine* const engine) {
  if(engine->player->getStatusEffectsHandler()->allowSee() == false) {
    engine->log->addMessage("I can not read while blind.");
    return false;
  }

  if(engine->playerBonusHandler->isBonusPicked(playerBonus_learned) == false) {
    engine->log->addMessage("I can not yet comprehend this.");
    return false;
  }

  const bool IS_IDENTIFIED_BEFORE_READING = def_->isIdentified;

  if(IS_IDENTIFIED_BEFORE_READING) {
    engine->log->addMessage("I read a scroll of " + getRealTypeName() + "...");
    specificRead(false, engine);
    attemptMemorizeIfLearnable(engine);
    engine->player->incrShock(shockValue_heavy);
  } else {
    engine->log->addMessage("I recite forbidden incantations...");
    def_->isTried = true;
    specificRead(false, engine);
    engine->player->incrShock(shockValue_heavy);
  }
  engine->gameTime->letNextAct();
  return true;
}

void Scroll::failedToLearnRealName(Engine* const engine, const string overrideFailString) {
  if(def_->isIdentified == false) {
    if(overrideFailString != "") {
      engine->log->addMessage(overrideFailString);
    } else {
      engine->log->addMessage("Was that supposed to do something?");
    }
  }
}

