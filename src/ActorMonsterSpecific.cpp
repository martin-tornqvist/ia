#include "ActorMonster.h"

#include <algorithm>

#include "Engine.h"
#include "ItemFactory.h"
#include "Inventory.h"
#include "Explosion.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "GameTime.h"
#include "ActorFactory.h"
#include "Render.h"
#include "CommonData.h"
#include "Map.h"
#include "Blood.h"
#include "FeatureFactory.h"
#include "Knockback.h"
#include "Gods.h"

using namespace std;

string Cultist::getCultistPhrase(Engine* const engine) {
  vector<string> phraseCandidates;

  const God* const god = engine->gods->getCurrentGod();
  if(god != NULL && engine->dice.coinToss()) {
    const string name = god->getName();
    const string descr = god->getDescr();
    phraseCandidates.push_back(name + " save us!");
    phraseCandidates.push_back(descr + " will save us!");
    phraseCandidates.push_back(name + ", guide us!");
    phraseCandidates.push_back(descr + " guides us!");
    phraseCandidates.push_back("For " + name + "!");
    phraseCandidates.push_back("For " + descr + "!");
    phraseCandidates.push_back("Blood for " + name + "!");
    phraseCandidates.push_back("Blood for " + descr + "!");
    phraseCandidates.push_back("Perish for " + name + "!");
    phraseCandidates.push_back("Perish for " + descr + "!");
    phraseCandidates.push_back("In the name of " + name + "!");
  } else {
    phraseCandidates.push_back("Apigami!");
    phraseCandidates.push_back("Bhuudesco invisuu!");
    phraseCandidates.push_back("Bhuuesco marana!");
    phraseCandidates.push_back("Crudux cruo!");
    phraseCandidates.push_back("Cruento paashaeximus!");
    phraseCandidates.push_back("Cruento pestis shatruex!");
    phraseCandidates.push_back("Cruo crunatus durbe!");
    phraseCandidates.push_back("Cruo lokemundux!");
    phraseCandidates.push_back("Cruo-stragaraNa!");
    phraseCandidates.push_back("Gero shay cruo!");
    phraseCandidates.push_back("In marana domus-bhaava crunatus!");
    phraseCandidates.push_back("Caecux infirmux!");
    phraseCandidates.push_back("Malax sayti!");
    phraseCandidates.push_back("Marana pallex!");
    phraseCandidates.push_back("Marana malax!");
    phraseCandidates.push_back("Pallex ti!");
    phraseCandidates.push_back("Peroshay bibox malax!");
    phraseCandidates.push_back("Pestis Cruento!");
    phraseCandidates.push_back("Pestis cruento vilomaxus pretiacruento!");
    phraseCandidates.push_back("Pretaanluxis cruonit!");
    phraseCandidates.push_back("Pretiacruento!");
    phraseCandidates.push_back("StragarNaya!");
    phraseCandidates.push_back("Vorox esco marana!");
    phraseCandidates.push_back("Vilomaxus!");
    phraseCandidates.push_back("Prostragaranar malachtose!");
    phraseCandidates.push_back("Apigami!");
  }

  return phraseCandidates.at(engine->dice.range(0, phraseCandidates.size() - 1));
}

void Cultist::actorSpecific_spawnStartItems() {
  const int DLVL = eng->map->getDLVL();

  const int PISTOL = 6;
  const int PUMP_SHOTGUN = PISTOL + 4;
  const int SAWN_SHOTGUN = PUMP_SHOTGUN + 3;
  const int MG = SAWN_SHOTGUN + (DLVL < 3 ? 0 : 2);

  const int TOT = MG;
  const int RND = DLVL == 0 ? PISTOL : eng->dice.range(1, TOT);

  if(RND <= PISTOL) {
    inventory_->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_pistol), true);
    if(eng->dice.percentile() < 40) {
      inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_pistolClip));
    }
  } else if(RND <= PUMP_SHOTGUN) {
    inventory_->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_pumpShotgun), true);
    Item* item = eng->itemFactory->spawnItem(item_shotgunShell);
    item->nrItems = eng->dice.range(5, 9);
    inventory_->putItemInGeneral(item);
  } else if(RND <= SAWN_SHOTGUN) {
    inventory_->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_sawedOff), true);
    Item* item = eng->itemFactory->spawnItem(item_shotgunShell);
    item->nrItems = eng->dice.range(6, 12);
    inventory_->putItemInGeneral(item);
  } else {
    inventory_->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_machineGun), true);
  }

  if(eng->dice.percentile() < 33) {
    inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(eng->dice.percentile() < 8) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
  }
}

void CultistTeslaCannon::actorSpecific_spawnStartItems() {
  Item* item = eng->itemFactory->spawnItem(item_teslaCanon);
  inventory_->putItemInSlot(slot_wielded, item, true);

  inventory_->putItemInGeneral(eng->itemFactory->spawnItem(item_teslaCanister));

  if(eng->dice.percentile() < 33) {
    inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(eng->dice.percentile() < 10) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
  }
}

void CultistSpikeGun::actorSpecific_spawnStartItems() {
  inventory_->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_spikeGun), true);
  Item* item = eng->itemFactory->spawnItem(item_ironSpike);
  item->nrItems = 8 + eng->dice(1, 8);
  inventory_->putItemInGeneral(item);
}

void CultistPriest::actorSpecific_spawnStartItems() {
  Item* item = eng->itemFactory->spawnItem(item_dagger);
  dynamic_cast<Weapon*>(item)->meleeDmgPlus = 2;
  inventory_->putItemInSlot(slot_wielded, item, true);

  inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
  inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));

  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());

  if(eng->dice.percentile() < 33) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
  }
}

void FireHound::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_hellHoundFireBreath));
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_hellHoundBite));
}

bool Vortex::actorSpecificAct() {
  if(pullCooldown > 0) {
    pullCooldown--;
  }

  if(pullCooldown <= 0) {
    if(playerAwarenessCounter > 0) {
      tracer << "Vortex: pullCooldown: " << pullCooldown << endl;
      tracer << "Vortex: Is player aware" << endl;
      const Pos& playerPos = eng->player->pos;
      if(eng->mapTests->isCellsNeighbours(pos, playerPos, true) == false) {

        const int CHANCE_TO_KNOCK = 25;
        if(eng->dice.percentile() < CHANCE_TO_KNOCK) {
          tracer << "Vortex: Passed random chance to pull" << endl;

          const Pos playerDelta = playerPos - pos;
          Pos knockBackFromPos = playerPos;
          if(playerDelta.x > 1) {
            knockBackFromPos.x++;
          }
          if(playerDelta.x < -1) {
            knockBackFromPos.x--;
          }
          if(playerDelta.y > 1) {
            knockBackFromPos.y++;
          }
          if(playerDelta.y < -1) {
            knockBackFromPos.y--;
          }

          if(knockBackFromPos != playerPos) {
            tracer << "Vortex: Good pos found to pull (knockback) player from (";
            tracer << knockBackFromPos.x << "," << knockBackFromPos.y << ")" << endl;
            tracer << "Vortex: Player position: ";
            tracer << playerPos.x << "," << playerPos.y << ")" << endl;
            bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
            eng->mapTests->makeVisionBlockerArray(pos, visionBlockers);
            if(checkIfSeeActor(*(eng->player), visionBlockers)) {
              tracer << "Vortex: I am seeing the player" << endl;
              if(eng->player->checkIfSeeActor(*this, NULL)) {
                eng->log->addMessage("The Vortex attempts to pull me in!");
              } else {
                eng->log->addMessage("A powerful wind is pulling me!");
              }
              tracer << "Vortex: Attempt pull (knockback)" << endl;
              eng->knockBack->tryKnockBack(
                eng->player, knockBackFromPos, false, false);
              pullCooldown = 5;
              eng->gameTime->endTurnOfCurrentActor();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void DustVortex::monsterDeath() {
  eng->explosionMaker->runExplosion(
    pos, false, new PropBlind(eng, propTurnsStandard),
    true, clrGray);
}

void DustVortex::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_dustVortexEngulf));
}

void FireVortex::monsterDeath() {
  eng->explosionMaker->runExplosion(
    pos, false, new PropBurning(eng, propTurnsStandard),
    true, clrRedLgt);
}

void FireVortex::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_fireVortexEngulf));
}

bool Ghost::actorSpecificAct() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {

      if(eng->mapTests->isCellsNeighbours(pos, eng->player->pos, false)) {
        if(eng->dice.percentile() < 30) {

          bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
          eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);
          const bool PLAYER_SEES_ME =
            eng->player->checkIfSeeActor(*this, blockers);
          const string refer = PLAYER_SEES_ME ? getNameThe() : "It";
          eng->log->addMessage(refer + " reaches for me... ");
          const AbilityRollResult_t rollResult = eng->abilityRoll->roll(
              eng->player->getData()->abilityVals.getVal(
                ability_dodgeAttack, true, *this));
          const bool PLAYER_DODGES = rollResult >= successSmall;
          if(PLAYER_DODGES) {
            eng->log->addMessage("I dodge!", clrMessageGood);
          } else {
            if(eng->dice.coinToss()) {
              eng->player->getPropHandler()->tryApplyProp(
                new PropSlowed(eng, propTurnsStandard));
            } else {
              eng->player->getPropHandler()->tryApplyProp(
                new PropCursed(eng, propTurnsStandard));
            }
            restoreHp(999, true);
          }
          eng->gameTime->endTurnOfCurrentActor();
          return true;
        }
      }
    }
  }
  return false;
}

void Ghost::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_ghostClaw));
}

void Phantasm::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_phantasmSickle));
}

void Wraith::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_wraithClaw));
  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
}

void MiGo::actorSpecific_spawnStartItems() {
  Item* item = eng->itemFactory->spawnItem(item_miGoElectricGun);
  inventory_->putItemInIntrinsics(item);

  spellsKnown.push_back(new SpellTeleport);
  spellsKnown.push_back(new SpellHealSelf);

  if(eng->dice.coinToss()) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
  }
}

void Rat::actorSpecific_spawnStartItems() {
  Item* item = NULL;
  if(eng->dice.percentile() < 15) {
    item = eng->itemFactory->spawnItem(item_ratBiteDiseased);
  } else {
    item = eng->itemFactory->spawnItem(item_ratBite);
  }
  inventory_->putItemInIntrinsics(item);
}

void RatThing::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_ratThingBite));
}

void Shadow::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_shadowClaw));
}

void Ghoul::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_ghoulClaw));
}

void Mummy::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_mummyMaul));

  for(int i = eng->dice.range(1, 3); i > 0; i--) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
  }
}

void MummyUnique::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_mummyMaul));
  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
}

bool Khephren::actorSpecificAct() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(hasSummonedLocusts == false) {

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        eng->mapTests->makeVisionBlockerArray(pos, blockers);

        if(checkIfSeeActor(*(eng->player), blockers)) {
          eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_fly, blockers);

          const int SPAWN_AFTER_X = eng->player->pos.x + FOV_STANDARD_RADI_INT + 1;
          for(int y = 0; y  < MAP_Y_CELLS; y++) {
            for(int x = 0; x <= SPAWN_AFTER_X; x++) {
              blockers[x][y] = true;
            }
          }

          eng->basicUtils->reverseBoolArray(blockers);
          vector<Pos> freeCells;
          eng->mapTests->makeBoolVectorFromMapArray(blockers, freeCells);
          sort(freeCells.begin(), freeCells.end(), IsCloserToOrigin(pos, eng));

          const unsigned int NR_OF_SPAWNS = 15;
          if(freeCells.size() >= NR_OF_SPAWNS + 1) {
            eng->log->addMessage("Khephren calls a plague of Locusts!");
            eng->player->incrShock(shockValue_heavy);
            for(unsigned int i = 0; i < NR_OF_SPAWNS; i++) {
              Monster* monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_giantLocust, freeCells.at(0)));
              monster->playerAwarenessCounter = 999;
              monster->leader = this;
              freeCells.erase(freeCells.begin());
            }
            eng->renderer->drawMapAndInterface();
            hasSummonedLocusts = true;
            eng->gameTime->endTurnOfCurrentActor();
            return true;
          }
        }
      }
    }
  }

  return false;
}



void DeepOne::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_deepOneJavelinAttack));
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_deepOneSpearAttack));
}

void GiantBat::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_giantBatBite));
}

void Byakhee::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_byakheeClaw));
}

void GiantMantis::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_giantMantisClaw));
}

void HuntingHorror::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_huntingHorrorBite));
}

bool KeziahMason::actorSpecificAct() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(hasSummonedJenkin == false) {

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        eng->mapTests->makeVisionBlockerArray(pos, blockers);

        if(checkIfSeeActor(*(eng->player), blockers)) {

          eng->mapTests->makeMoveBlockerArray(this, blockers);

          vector<Pos> line = eng->mapTests->getLine(pos, eng->player->pos,
                             true, 9999);

          for(unsigned int i = 0; i < line.size(); i++) {
            const Pos c = line.at(i);
            if(blockers[c.x][c.y] == false) {
              //TODO Make a generalized summoning funtionality
              eng->log->addMessage("Keziah summons Brown Jenkin!");
              Actor* const actor =
                eng->actorFactory->spawnActor(actor_brownJenkin, c);
              Monster* jenkin = dynamic_cast<Monster*>(actor);
              eng->renderer->drawMapAndInterface();
              hasSummonedJenkin = true;
              jenkin->playerAwarenessCounter = 999;
              jenkin->leader = this;
              eng->gameTime->endTurnOfCurrentActor();
              return true;
            }
          }
        }
      }
    }
  }

  return false;
}

void KeziahMason::actorSpecific_spawnStartItems() {
  spellsKnown.push_back(new SpellTeleport);
  spellsKnown.push_back(new SpellHealSelf);
  spellsKnown.push_back(new SpellSummonRandom);
  spellsKnown.push_back(new SpellAzathothsBlast);
  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonster());
}

void Ooze::actorSpecificOnStandardTurn() {
  restoreHp(1, false);
}

void OozeBlack::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_oozeBlackSpewPus));
}

void OozeClear::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_oozeClearSpewPus));
}

void OozePutrid::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_oozePutridSpewPus));
}

void OozePoison::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_oozePoisonSpewPus));
}

void ColourOutOfSpace::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_colourOutOfSpaceTouch));
}

const SDL_Color& ColourOutOfSpace::getColor() {
  return currentColor;
}

void ColourOutOfSpace::actorSpecificOnStandardTurn() {
  currentColor.r = eng->dice.range(40, 255);
  currentColor.g = eng->dice.range(40, 255);
  currentColor.b = eng->dice.range(40, 255);

  restoreHp(1, false);

  if(eng->player->checkIfSeeActor(*this, NULL)) {
    eng->player->getPropHandler()->tryApplyProp(
      new PropConfused(eng, propTurnsStandard));
  }
}

bool Spider::actorSpecificAct() {
  return false;
}

void GreenSpider::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_greenSpiderBite));
}

void WhiteSpider::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_whiteSpiderBite));
}

void RedSpider::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_redSpiderBite));
}

void ShadowSpider::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_shadowSpiderBite));
}

void LengSpider::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_lengSpiderBite));
}

void Wolf::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng->itemFactory->spawnItem(item_wolfBite));
}

bool WormMass::actorSpecificAct() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(eng->dice.percentile() < chanceToSpawnNew) {

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        eng->mapTests->makeMoveBlockerArray(this, blockers);

        for(int dx = -1; dx <= 1; dx++) {
          for(int dy = -1; dy <= 1; dy++) {
            if(blockers[pos.x + dx][pos.y + dy] == false) {
              Actor* const actor =
                eng->actorFactory->spawnActor(data_->id, pos + Pos(dx, dy));
              WormMass* const worm = dynamic_cast<WormMass*>(actor);
              chanceToSpawnNew -= 2;
              worm->chanceToSpawnNew = chanceToSpawnNew;
              eng->gameTime->endTurnOfCurrentActor();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void WormMass::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_wormMassBite));
}

bool GiantLocust::actorSpecificAct() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(eng->dice.percentile() < chanceToSpawnNew) {

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        eng->mapTests->makeMoveBlockerArray(this, blockers);

        for(int dx = -1; dx <= 1; dx++) {
          for(int dy = -1; dy <= 1; dy++) {
            if(blockers[pos.x + dx][pos.y + dy] == false) {
              Actor* const actor =
                eng->actorFactory->spawnActor(data_->id, pos + Pos(dx, dy));
              GiantLocust* const locust = dynamic_cast<GiantLocust*>(actor);
              chanceToSpawnNew -= 2;
              locust->chanceToSpawnNew = chanceToSpawnNew;
              eng->gameTime->endTurnOfCurrentActor();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void GiantLocust::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_giantLocustBite));
}

bool LordOfShadows::actorSpecificAct() {

  return false;
}

void LordOfShadows::actorSpecific_spawnStartItems() {

}

bool LordOfSpiders::actorSpecificAct() {
  if(playerAwarenessCounter > 0) {

    if(eng->dice.coinToss()) {

      const Pos playerPos = eng->player->pos;

      if(eng->player->checkIfSeeActor(*this, NULL)) {
        eng->log->addMessage(data_->spellCastMessage);
      }

      for(int dy = -1; dy <= 1; dy++) {
        for(int dx = -1; dx <= 1; dx++) {

          if(eng->dice.percentile() < 75) {

            const Pos c(playerPos + Pos(dx, dy));
            const FeatureStatic* const mimicFeature =
              eng->map->featuresStatic[c.x][c.y];

            if(mimicFeature->canHaveStaticFeature()) {

              const FeatureData* const mimicData =
                eng->featureDataHandler->getData(
                  mimicFeature->getId());

              Feature* const f =
                eng->featureFactory->spawnFeatureAt(
                  feature_trap, c,
                  new TrapSpawnData(mimicData, trap_spiderWeb));

              dynamic_cast<Trap*>(f)->reveal(false);
            }
          }
        }
      }
    }
  }
  return false;
}

void LordOfSpiders::actorSpecific_spawnStartItems() {

}

bool LordOfSpirits::actorSpecificAct() {

  return false;
}

void LordOfSpirits::actorSpecific_spawnStartItems() {

}

bool LordOfPestilence::actorSpecificAct() {

  return false;
}

void LordOfPestilence::actorSpecific_spawnStartItems() {

}

bool Zombie::actorSpecificAct() {
  return tryResurrect();
}

bool MajorClaphamLee::actorSpecificAct() {
  if(tryResurrect()) {
    return true;
  }

  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(hasSummonedTombLegions == false) {

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        eng->mapTests->makeVisionBlockerArray(pos, blockers);

        if(checkIfSeeActor(*(eng->player), blockers)) {
          eng->mapTests->makeMoveBlockerArray(this, blockers);
          eng->basicUtils->reverseBoolArray(blockers);
          vector<Pos> freeCells;
          eng->mapTests->makeBoolVectorFromMapArray(blockers, freeCells);
          sort(freeCells.begin(), freeCells.end(), IsCloserToOrigin(pos, eng));

          const unsigned int NR_OF_SPAWNS = 5;
          if(freeCells.size() >= NR_OF_SPAWNS + 1) {
            eng->log->addMessage("Major Clapham Lee calls forth his Tomb-Legions!");
            eng->player->incrShock(shockValue_heavy);
            for(unsigned int i = 0; i < NR_OF_SPAWNS; i++) {
              if(i == 0) {
                Monster* monster = dynamic_cast<Monster*>(
                                     eng->actorFactory->spawnActor(actor_deanHalsey, freeCells.at(0)));
                monster->playerAwarenessCounter = 999;
                monster->leader = this;
                freeCells.erase(freeCells.begin());
              } else {
                const int ZOMBIE_TYPE = eng->dice.range(0, 2);
                ActorId_t id = actor_zombie;
                switch(ZOMBIE_TYPE) {
                  case 0: id = actor_zombie;        break;
                  case 1: id = actor_zombieAxe;     break;
                  case 2: id = actor_bloatedZombie; break;
                }
                Monster* monster = dynamic_cast<Monster*>(
                                     eng->actorFactory->spawnActor(id, freeCells.at(0)));
                monster->playerAwarenessCounter = 999;
                monster->leader = this;
                freeCells.erase(freeCells.begin());
              }
            }
            eng->renderer->drawMapAndInterface();
            hasSummonedTombLegions = true;
            eng->gameTime->endTurnOfCurrentActor();
            return true;
          }
        }
      }
    }
  }

  return false;
}

bool Zombie::tryResurrect() {
  if(deadState == actorDeadState_corpse) {
    if(hasResurrected == false) {
      deadTurnCounter += 1;
      if(deadTurnCounter > 5) {
        if(pos != eng->player->pos && eng->dice.percentile() < 7) {
          deadState = actorDeadState_alive;
          hp_ = (getHpMax(true) * 3) / 4;
          glyph_ = data_->glyph;
          tile_ = data_->tile;
          clr_ = data_->color;
          hasResurrected = true;
          data_->nrOfKills--;
          if(eng->map->playerVision[pos.x][pos.y] == true) {
            eng->log->addMessage(getNameThe() + " rises again!!", clrWhite, messageInterrupt_force);
            eng->player->incrShock(shockValue_some);
          }

          playerAwarenessCounter = data_->nrTurnsAwarePlayer * 2;
          eng->gameTime->endTurnOfCurrentActor();
          return true;
        }
      }
    }
  }
  return false;
}

void Zombie::actorSpecificDie() {
  //If resurrected once and has corpse, blow up the corpse
  if(hasResurrected && deadState == actorDeadState_corpse) {
    deadState = actorDeadState_mangled;
    eng->gore->makeBlood(pos);
    eng->gore->makeGore(pos);
  }
}

void ZombieClaw::actorSpecific_spawnStartItems() {
  Item* item = NULL;
  if(eng->dice.percentile() < 20) {
    item = eng->itemFactory->spawnItem(item_zombieClawDiseased);
  } else {
    item = eng->itemFactory->spawnItem(item_zombieClaw);
  }
  inventory_->putItemInIntrinsics(item);
}

void ZombieAxe::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_zombieAxe));
}

void BloatedZombie::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_bloatedZombiePunch));
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_bloatedZombieSpit));
}

