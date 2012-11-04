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
#include "ConstTypes.h"
#include "Map.h"
#include "Blood.h"
#include "FeatureFactory.h"

using namespace std;

void CultistShotgun::actorSpecific_spawnStartItems() {
  Item* item = eng->itemFactory->spawnItem(item_sawedOff);
  inventory_->putItemInSlot(slot_wielded, item, true);

  item = eng->itemFactory->spawnItem(item_shotgunShell);
  item->numberOfItems = 10;
  inventory_->putItemInGeneral(item);

  if(eng->dice(1, 100) < 33) {
    inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(eng->dice(1, 100) < 10) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
  }
}

void CultistMachineGun::actorSpecific_spawnStartItems() {
  Item* item = eng->itemFactory->spawnItem(item_machineGun);
  inventory_->putItemInSlot(slot_wielded, item, true);

  if(eng->dice(1, 100) < 33) {
    inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(eng->dice(1, 100) < 10) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
  }
}

void CultistTeslaCannon::actorSpecific_spawnStartItems() {
  Item* item = eng->itemFactory->spawnItem(item_teslaCanon);
  inventory_->putItemInSlot(slot_wielded, item, true);

  item = eng->itemFactory->spawnItem(item_teslaCanister);
  item->numberOfItems = 4;
  inventory_->putItemInGeneral(item);

  if(eng->dice(1, 100) < 33) {
    inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(eng->dice(1, 100) < 10) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
  }
}

void CultistSpikeGun::actorSpecific_spawnStartItems() {
  inventory_->putItemInSlot(slot_wielded, eng->itemFactory->spawnItem(item_spikeGun), true);
  Item* item = eng->itemFactory->spawnItem(item_ironSpike);
  item->numberOfItems = 8 + eng->dice(1, 8);
  inventory_->putItemInGeneral(item);
}

void CultistPistol::actorSpecific_spawnStartItems() {
  Item* item = eng->itemFactory->spawnItem(item_pistol);
  inventory_->putItemInSlot(slot_wielded, item, true);

  if(eng->dice(1, 100) < 38) {
    item = eng->itemFactory->spawnItem(item_pistolClip);
    inventory_->putItemInGeneral(item);
  }

  if(eng->dice(1, 100) < 33) {
    inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(eng->dice(1, 100) < 10) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
  }
}

void CultistPriest::actorSpecific_spawnStartItems() {
  Item* item = eng->itemFactory->spawnItem(item_dagger);
  inventory_->putItemInSlot(slot_wielded, item, true);

  inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));
  inventory_->putItemInGeneral(eng->itemFactory->spawnRandomScrollOrPotion(true, true));

  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
  spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());

  if(eng->dice(1, 100) < 33) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
  }
}

void FireHound::actorSpecific_spawnStartItems()
{
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_hellHoundFireBreath));
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_hellHoundBite));
}

void FireVampire::monsterDeath()
{
  eng->explosionMaker->runExplosion(pos, false, new StatusBurning(eng), true, clrYellow);
}

void FireVampire::actorSpecific_spawnStartItems()
{
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_fireVampireTouch));
}

bool Ghost::actorSpecificAct() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(eng->mapTests->isCellsNeighbours(pos, eng->player->pos, false)) {
        if(eng->dice(1, 100) < 35) {

          bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
          eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);
          const bool PLAYER_SEES_ME = eng->player->checkIfSeeActor(*this, blockers);
          const string refer = PLAYER_SEES_ME ? getNameThe() : "It";
          eng->log->addMessage(refer + " reaches for me... ");
          const AbilityRollResult_t rollResult = eng->abilityRoll->roll(
              eng->player->getDef()->abilityValues.getAbilityValue(ability_dodgeAttack, true, *this));
          const bool PLAYER_DODGES = rollResult >= successSmall;
          if(PLAYER_DODGES) {
            eng->log->addMessage("I dodge!", clrMessageGood);
          } else {
            bool deflectedByArmor = false;
            Item* const playerArmor = eng->player->getInventory()->getItemInSlot(slot_armorBody);
            if(playerArmor != NULL) {
              const int ARMOR_CHANCE_TO_DEFLECT = playerArmor->getDef().armorData.chanceToDeflectTouchAttacks;
              if(eng->dice(1, 100) < ARMOR_CHANCE_TO_DEFLECT) {
                deflectedByArmor = true;
              }
            }

            if(deflectedByArmor) {
              const string armorName = playerArmor->getDef().name.name;
              eng->log->addMessage("The touch is deflected by my " + armorName + "!");
            } else {
              if(eng->dice.coinToss()) {
                eng->player->getStatusEffectsHandler()->attemptAddEffect(new StatusSlowed(eng));
              } else {
                eng->player->getStatusEffectsHandler()->attemptAddEffect(new StatusCursed(eng));
              }
              restoreHP(999);
            }
          }
          eng->gameTime->letNextAct();
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
  eng->spellHandler->addAllCommonSpellsForMonsters(spellsKnown);
}

void MiGo::actorSpecific_spawnStartItems()
{
  Item* item = eng->itemFactory->spawnItem(item_miGoElectricGun);
  inventory_->putItemInIntrinsics(item);

  spellsKnown.push_back(new SpellTeleport);

  if(eng->dice.coinToss()) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
  }
}

void Rat::actorSpecific_spawnStartItems() {
  Item* item = NULL;
  if(eng->dice(1, 100) < 20) {
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

  for(int i = eng->dice(1, 3); i > 0; i--) {
    spellsKnown.push_back(eng->spellHandler->getRandomSpellForMonsters());
  }
}

void MummyUnique::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_mummyMaul));
  eng->spellHandler->addAllCommonSpellsForMonsters(spellsKnown);
}

void DeepOne::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_deepOneJavelinAttack));
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_deepOneSpearAttack));
}

void GiantBat::actorSpecific_spawnStartItems()
{
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_giantBatBite));
}

void Byakhee::actorSpecific_spawnStartItems()
{
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_byakheeClaw));
}

void HuntingHorror::actorSpecific_spawnStartItems()
{
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

          vector<coord> line = eng->mapTests->getLine(pos.x, pos.y, eng->player->pos.x, eng->player->pos.y, true, 9999);

          for(unsigned int i = 0; i < line.size(); i++) {
            const coord c = line.at(i);
            if(blockers[c.x][c.y] == false) {
              eng->log->addMessage("Keziah summons Brown Jenkin!");
              Monster* jenkin = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_brownJenkin, c));
              eng->explosionMaker->runSmokeExplosion(c);
              eng->renderer->drawMapAndInterface();
              hasSummonedJenkin = true;
              jenkin->playerAwarenessCounter = 999;
              jenkin->leader = this;
              eng->gameTime->letNextAct();
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
  eng->spellHandler->addAllCommonSpellsForMonsters(spellsKnown);
}

void OozeGray::actorSpecific_spawnStartItems()
{
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_oozeGraySpewPus));
}

void OozeClear::actorSpecific_spawnStartItems()
{
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_oozeClearSpewPus));
}

void OozePutrid::actorSpecific_spawnStartItems()
{
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_oozePutridSpewPus));
}

bool Spider::actorSpecificAct() {
  return false;
}

void GreenSpider::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_greenSpiderBite));
}

void BlackSpider::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_blackSpiderBite));
}


void WhiteSpider::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_whiteSpiderBite));
}

void LengSpider::actorSpecific_spawnStartItems() {
  inventory_->putItemInIntrinsics(eng->itemFactory->spawnItem(item_lengSpiderBite));
}

void Wolf::actorSpecific_spawnStartItems() {
  Item* item = eng->itemFactory->spawnItem(item_wolfBite);
  inventory_->putItemInIntrinsics(item);
}

bool WormMass::actorSpecificAct() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(eng->dice(1, 100) < chanceToSpawnNew) {

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        eng->mapTests->makeMoveBlockerArray(this, blockers);

        for(int dx = -1; dx <= 1; dx++) {
          for(int dy = -1; dy <= 1; dy++) {
            if(blockers[pos.x + dx][pos.y + dy] == false) {
              WormMass* const spawn =
                dynamic_cast<WormMass*>(eng->actorFactory->spawnActor(def_->devName, pos + coord(dx, dy)));
              chanceToSpawnNew -= 5;
              spawn->chanceToSpawnNew = chanceToSpawnNew;
              eng->gameTime->letNextAct();
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

bool LordOfShadows::actorSpecificAct() {

  return false;
}

void LordOfShadows::actorSpecific_spawnStartItems() {

}

bool LordOfSpiders::actorSpecificAct() {
  if(playerAwarenessCounter > 0) {

    if(eng->dice.coinToss()) {

      const coord playerPos = eng->player->pos;

      if(eng->player->checkIfSeeActor(*this, NULL)) {
        eng->log->addMessage(def_->spellCastMessage);
      }

      for(int dy = -1; dy <= 1; dy++) {
        for(int dx = -1; dx <= 1; dx++) {

          if(eng->dice(1, 100) < 75) {

            const coord c(playerPos + coord(dx, dy));
            const FeatureStatic* const mimicFeature = eng->map->featuresStatic[c.x][c.y];

            if(mimicFeature->canHaveStaticFeature()) {

              const FeatureDef* const mimicDef = eng->featureData->getFeatureDef(mimicFeature->getId());

              Feature* const f = eng->featureFactory->spawnFeatureAt(feature_trap, c, new TrapSpawnData(mimicDef, trap_spiderWeb));

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
  return attemptResurrect();
}

bool MajorClaphamLee::actorSpecificAct() {
  if(attemptResurrect()) {
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
          vector<coord> freeCells;
          eng->mapTests->makeMapVectorFromArray(blockers, freeCells);
          sort(freeCells.begin(), freeCells.end(), IsCloserToOrigin(pos, eng));

          const unsigned int NR_OF_SPAWNS = 5;
          if(freeCells.size() >= NR_OF_SPAWNS + 1) {
            eng->log->addMessage("Major Clapham Lee calls forth his Tomb-Legions!");
            eng->player->shock(shockValue_heavy, 0);
            for(unsigned int i = 0; i < NR_OF_SPAWNS; i++) {
              if(i == 0) {
                Monster* monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_deanHalsey, freeCells.at(0)));
                monster->playerAwarenessCounter = 999;
                monster->leader = this;
                freeCells.erase(freeCells.begin());
              } else {
                const int ZOMBIE_TYPE = eng->dice.getInRange(0, 2);
                ActorDevNames_t devName = actor_zombie;
                switch(ZOMBIE_TYPE) {
                case 0:
                  devName = actor_zombie;
                  break;
                case 1:
                  devName = actor_zombieAxe;
                  break;
                case 2:
                  devName = actor_bloatedZombie;
                  break;
                }
                Monster* monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(devName, freeCells.at(0)));
                monster->playerAwarenessCounter = 999;
                monster->leader = this;
                freeCells.erase(freeCells.begin());
              }
            }
            eng->renderer->drawMapAndInterface();
            hasSummonedTombLegions = true;
            eng->gameTime->letNextAct();
            return true;
          }
        }
      }
    }
  }

  return false;
}

bool Zombie::attemptResurrect() {
  if(deadState == actorDeadState_corpse) {
    if(hasResurrected == false) {
      deadTurnCounter += 1;
      if(deadTurnCounter > 5) {
        if(pos != eng->player->pos && eng->dice(1, 100) <= 7) {
          deadState = actorDeadState_alive;
          hp_ = (getHpMax() * 3) / 4;
          glyph_ = def_->glyph;
          tile_ = def_->tile;
          clr_ = def_->color;
          hasResurrected = true;
          def_->nrOfKills--;
          if(eng->map->playerVision[pos.x][pos.y] == true) {
            eng->log->addMessage(getNameThe() + " rises again!!", clrWhite, messageInterrupt_force);
            eng->player->shock(shockValue_some, 0);
          }

          playerAwarenessCounter = def_->nrTurnsAwarePlayer * 2;
          eng->gameTime->letNextAct();
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
  if(eng->dice(1, 100) < 30) {
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


