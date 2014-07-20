#include "ActorMonster.h"

#include <algorithm>

#include "Init.h"

#include "ItemFactory.h"
#include "Inventory.h"
#include "Explosion.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "GameTime.h"
#include "ActorFactory.h"
#include "Renderer.h"
#include "CmnData.h"
#include "Map.h"
#include "Knockback.h"
#include "Gods.h"
#include "MapParsing.h"
#include "LineCalc.h"
#include "Utils.h"
#include "FeatureTrap.h"

using namespace std;

string Cultist::getCultistPhrase() {
  vector<string> phraseBucket;

  const God* const god = Gods::getCurGod();

  if(god && Rnd::coinToss()) {
    const string name = god->getName();
    const string descr = god->getDescr();
    phraseBucket.push_back(name + " save us!");
    phraseBucket.push_back(descr + " will save us!");
    phraseBucket.push_back(name + ", guide us!");
    phraseBucket.push_back(descr + " guides us!");
    phraseBucket.push_back("For " + name + "!");
    phraseBucket.push_back("For " + descr + "!");
    phraseBucket.push_back("Blood for " + name + "!");
    phraseBucket.push_back("Blood for " + descr + "!");
    phraseBucket.push_back("Perish for " + name + "!");
    phraseBucket.push_back("Perish for " + descr + "!");
    phraseBucket.push_back("In the name of " + name + "!");
  } else {
    phraseBucket.push_back("Apigami!");
    phraseBucket.push_back("Bhuudesco invisuu!");
    phraseBucket.push_back("Bhuuesco marana!");
    phraseBucket.push_back("Crudux cruo!");
    phraseBucket.push_back("Cruento paashaeximus!");
    phraseBucket.push_back("Cruento pestis shatruex!");
    phraseBucket.push_back("Cruo crunatus durbe!");
    phraseBucket.push_back("Cruo lokemundux!");
    phraseBucket.push_back("Cruo-stragaraNa!");
    phraseBucket.push_back("Gero shay cruo!");
    phraseBucket.push_back("In marana domus-bhaava crunatus!");
    phraseBucket.push_back("Caecux infirmux!");
    phraseBucket.push_back("Malax sayti!");
    phraseBucket.push_back("Marana pallex!");
    phraseBucket.push_back("Marana malax!");
    phraseBucket.push_back("Pallex ti!");
    phraseBucket.push_back("Peroshay bibox malax!");
    phraseBucket.push_back("Pestis Cruento!");
    phraseBucket.push_back("Pestis cruento vilomaxus pretiacruento!");
    phraseBucket.push_back("Pretaanluxis cruonit!");
    phraseBucket.push_back("Pretiacruento!");
    phraseBucket.push_back("StragarNaya!");
    phraseBucket.push_back("Vorox esco marana!");
    phraseBucket.push_back("Vilomaxus!");
    phraseBucket.push_back("Prostragaranar malachtose!");
    phraseBucket.push_back("Apigami!");
  }

  return phraseBucket.at(Rnd::range(0, phraseBucket.size() - 1));
}

void Cultist::mkStartItems() {
  const int PISTOL = 6;
  const int PUMP_SHOTGUN = PISTOL + 4;
  const int SAWN_SHOTGUN = PUMP_SHOTGUN + 3;
  const int MG = SAWN_SHOTGUN + (Map::dlvl < 3 ? 0 : 2);

  const int TOT = MG;
  const int RND = Map::dlvl == 0 ? PISTOL : Rnd::range(1, TOT);

  if(RND <= PISTOL) {
    inv_->putInSlot(SlotId::wielded, ItemFactory::mk(ItemId::pistol));
    if(Rnd::percentile() < 40) {
      inv_->putInGeneral(ItemFactory::mk(ItemId::pistolClip));
    }
  } else if(RND <= PUMP_SHOTGUN) {
    inv_->putInSlot(SlotId::wielded, ItemFactory::mk(ItemId::pumpShotgun));
    Item* item = ItemFactory::mk(ItemId::shotgunShell);
    item->nrItems = Rnd::range(5, 9);
    inv_->putInGeneral(item);
  } else if(RND <= SAWN_SHOTGUN) {
    inv_->putInSlot(SlotId::wielded, ItemFactory::mk(ItemId::sawedOff));
    Item* item = ItemFactory::mk(ItemId::shotgunShell);
    item->nrItems = Rnd::range(6, 12);
    inv_->putInGeneral(item);
  } else {
    inv_->putInSlot(SlotId::wielded, ItemFactory::mk(ItemId::machineGun));
  }

  if(Rnd::percentile() < 33) {
    inv_->putInGeneral(ItemFactory::mkRandomScrollOrPotion(true, true));
  }

  if(Rnd::percentile() < 8) {
    spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
  }
}

void CultistTeslaCannon::mkStartItems() {
  inv_->putInSlot(SlotId::wielded, ItemFactory::mk(ItemId::teslaCannon));
  inv_->putInGeneral(ItemFactory::mk(ItemId::teslaCanister));

  if(Rnd::oneIn(3)) {
    inv_->putInGeneral(ItemFactory::mkRandomScrollOrPotion(true, true));
  }

  if(Rnd::oneIn(10)) {
    spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
  }
}

void CultistSpikeGun::mkStartItems() {
  inv_->putInSlot(SlotId::wielded, ItemFactory::mk(ItemId::spikeGun));
  Item* item = ItemFactory::mk(ItemId::ironSpike);
  item->nrItems = 8 + Rnd::dice(1, 8);
  inv_->putInGeneral(item);
}

void CultistPriest::mkStartItems() {
  Item* item = ItemFactory::mk(ItemId::dagger);
  static_cast<Weapon*>(item)->meleeDmgPlus = 2;
  inv_->putInSlot(SlotId::wielded, item);

  inv_->putInGeneral(ItemFactory::mkRandomScrollOrPotion(true, true));
  inv_->putInGeneral(ItemFactory::mkRandomScrollOrPotion(true, true));

  spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
  spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());

  if(Rnd::percentile() < 33) {
    spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
  }
}

void FireHound::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::fireHoundBreath));
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::fireHoundBite));
}

void FrostHound::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::frostHoundBreath));
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::frostHoundBite));
}

void Zuul::place_() {
  if(ActorData::data[actor_zuul].nrLeftAllowedToSpawn > 0) {
    //Note: Do not call die() here, that would have side effects such as
    //player getting XP. Instead, simply set the dead state to destroyed.
    deadState = ActorDeadState::destroyed;
    Actor* actor = ActorFactory::mk(actor_cultistPriest, pos);
    PropHandler& propHandler = actor->getPropHandler();
    propHandler.tryApplyProp(
      new PropPossessedByZuul(propTurnsIndefinite), true);
    actor->restoreHp(999, false);
  }
}

void Zuul::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::zuulBite));
}

bool Vortex::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(pullCooldown > 0) {
      pullCooldown--;
    }

    if(pullCooldown <= 0) {
      if(awareOfPlayerCounter_ > 0) {
        TRACE << "Vortex: pullCooldown: " << pullCooldown << endl;
        TRACE << "Vortex: Is player aware" << endl;
        const Pos& playerPos = Map::player->pos;
        if(!Utils::isPosAdj(pos, playerPos, true)) {

          const int CHANCE_TO_KNOCK = 25;
          if(Rnd::percentile() < CHANCE_TO_KNOCK) {
            TRACE << "Vortex: Passed random chance to pull" << endl;

            const Pos playerDelta = playerPos - pos;
            Pos knockBackFromPos = playerPos;
            if(playerDelta.x > 1)   {knockBackFromPos.x++;}
            if(playerDelta.x < -1)  {knockBackFromPos.x--;}
            if(playerDelta.y > 1)   {knockBackFromPos.y++;}
            if(playerDelta.y < -1)  {knockBackFromPos.y--;}

            if(knockBackFromPos != playerPos) {
              TRACE << "Vortex: Good pos found to knockback player from (";
              TRACE << knockBackFromPos.x << ",";
              TRACE << knockBackFromPos.y << ")" << endl;
              TRACE << "Vortex: Player position: ";
              TRACE << playerPos.x << "," << playerPos.y << ")" << endl;
              bool visionBlockers[MAP_W][MAP_H];
              MapParse::parse(CellPred::BlocksVision(), visionBlockers);
              if(isSeeingActor(*(Map::player), visionBlockers)) {
                TRACE << "Vortex: I am seeing the player" << endl;
                if(Map::player->isSeeingActor(*this, nullptr)) {
                  Log::addMsg("The Vortex attempts to pull me in!");
                } else {
                  Log::addMsg("A powerful wind is pulling me!");
                }
                TRACE << "Vortex: Attempt pull (knockback)" << endl;
                KnockBack::tryKnockBack(
                  *(Map::player), knockBackFromPos, false, false);
                pullCooldown = 5;
                GameTime::actorDidAct();
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

void DustVortex::die_() {
  Explosion::runExplosionAt(
    pos, ExplType::applyProp, ExplSrc::misc, 0, SfxId::END,
    new PropBlind(propTurnsStd), &clrGray);
}

void DustVortex::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::dustVortexEngulf));
}

void FireVortex::die_() {
  Explosion::runExplosionAt(
    pos, ExplType::applyProp, ExplSrc::misc, 0, SfxId::END,
    new PropBurning(propTurnsStd), &clrRedLgt);
}

void FireVortex::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::fireVortexEngulf));
}

void FrostVortex::die_() {
  //TODO Add explosion with cold damage
}

void FrostVortex::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::frostVortexEngulf));
}

bool Ghost::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {

      if(Utils::isPosAdj(pos, Map::player->pos, false)) {
        if(Rnd::percentile() < 30) {

          bool blocked[MAP_W][MAP_H];
          MapParse::parse(CellPred::BlocksVision(), blocked);
          const bool PLAYER_SEES_ME =
            Map::player->isSeeingActor(*this, blocked);
          const string refer = PLAYER_SEES_ME ? getNameThe() : "It";
          Log::addMsg(refer + " reaches for me... ");
          const AbilityRollResult rollResult =
            AbilityRoll::roll(Map::player->getData().abilityVals.getVal(
                                AbilityId::dodgeAttack, true, *this));
          const bool PLAYER_DODGES = rollResult >= successSmall;
          if(PLAYER_DODGES) {
            Log::addMsg("I dodge!", clrMsgGood);
          } else {
            Map::player->getPropHandler().tryApplyProp(
              new PropSlowed(propTurnsStd));
          }
          GameTime::actorDidAct();
          return true;
        }
      }
    }
  }
  return false;
}

void Ghost::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::ghostClaw));
}

void Phantasm::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::phantasmSickle));
}

void Wraith::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::wraithClaw));
  spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
  spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
}

void MiGo::mkStartItems() {
  Item* item = ItemFactory::mk(ItemId::miGoElectricGun);
  inv_->putInIntrinsics(item);

  spellsKnown.push_back(new SpellTeleport);
  spellsKnown.push_back(new SpellMiGoHypnosis);
  spellsKnown.push_back(new SpellHealSelf);

  if(Rnd::coinToss()) {
    spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
  }
}

void FlyingPolyp::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::polypTentacle));
}

void Rat::mkStartItems() {
  Item* item = nullptr;
  if(Rnd::percentile() < 15) {
    item = ItemFactory::mk(ItemId::ratBiteDiseased);
  } else {
    item = ItemFactory::mk(ItemId::ratBite);
  }
  inv_->putInIntrinsics(item);
}

void RatThing::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::ratThingBite));
}

void Shadow::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::shadowClaw));
}

void Ghoul::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::ghoulClaw));
}

void Mummy::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::mummyMaul));

  spellsKnown.push_back(SpellHandling::getSpellFromId(SpellId::disease));

  for(int i = Rnd::range(1, 2); i > 0; --i) {
    spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
  }
}

void MummyUnique::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::mummyMaul));

  spellsKnown.push_back(SpellHandling::getSpellFromId(SpellId::disease));

  spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
  spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
  spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
}

bool Khephren::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(!hasSummonedLocusts) {

        bool blocked[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), blocked);

        if(isSeeingActor(*(Map::player), blocked)) {
          MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);

          const int SPAWN_AFTER_X =
            Map::player->pos.x + FOV_STD_RADI_INT + 1;
          for(int y = 0; y  < MAP_H; ++y) {
            for(int x = 0; x <= SPAWN_AFTER_X; ++x) {
              blocked[x][y] = true;
            }
          }

          vector<Pos> freeCells;
          Utils::mkVectorFromBoolMap(false, blocked, freeCells);

          sort(begin(freeCells), end(freeCells), IsCloserToPos(pos));

          const int NR_OF_SPAWNS = 15;
          if(freeCells.size() >= NR_OF_SPAWNS + 1) {
            Log::addMsg("Khephren calls a plague of Locusts!");
            Map::player->incrShock(ShockValue::shockValue_heavy,
                                   ShockSrc::misc);
            for(int i = 0; i < NR_OF_SPAWNS; ++i) {
              Actor* const actor =
                ActorFactory::mk(actor_locust, freeCells.at(0));
              Monster* const monster = static_cast<Monster*>(actor);
              monster->awareOfPlayerCounter_ = 999;
              monster->leader = this;
              freeCells.erase(begin(freeCells));
            }
            Renderer::drawMapAndInterface();
            hasSummonedLocusts = true;
            GameTime::actorDidAct();
            return true;
          }
        }
      }
    }
  }

  return false;
}



void DeepOne::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::deepOneJavelinAtt));
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::deepOneSpearAtt));
}

void GiantBat::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::giantBatBite));
}

void Byakhee::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::byakheeClaw));
}

void GiantMantis::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::giantMantisClaw));
}

void Chthonian::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::chthonianBite));
}

void HuntingHorror::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::huntingHorrorBite));
}

bool KeziahMason::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(!hasSummonedJenkin) {

        bool blocked[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), blocked);

        if(isSeeingActor(*(Map::player), blocked)) {

          MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);

          vector<Pos> line;
          LineCalc::calcNewLine(pos, Map::player->pos, true, 9999,
                                false, line);

          const int LINE_SIZE = line.size();
          for(int i = 0; i < LINE_SIZE; ++i) {
            const Pos c = line.at(i);
            if(!blocked[c.x][c.y]) {
              //TODO Make a generalized summoning functionality
              Log::addMsg("Keziah summons Brown Jenkin!");
              Actor* const actor = ActorFactory::mk(actor_brownJenkin, c);
              Monster* jenkin = static_cast<Monster*>(actor);
              Renderer::drawMapAndInterface();
              hasSummonedJenkin = true;
              jenkin->awareOfPlayerCounter_ = 999;
              jenkin->leader = this;
              GameTime::actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }

  return false;
}

void KeziahMason::mkStartItems() {
  spellsKnown.push_back(new SpellTeleport);
  spellsKnown.push_back(new SpellHealSelf);
  spellsKnown.push_back(new SpellSummonRandom);
  spellsKnown.push_back(new SpellAzathothsWrath);
  spellsKnown.push_back(SpellHandling::getRandomSpellForMonster());
}

void Ooze::onStandardTurn() {
  restoreHp(1, false);
}

void OozeBlack::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::oozeBlackSpewPus));
}

void OozeClear::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::oozeClearSpewPus));
}

void OozePutrid::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::oozePutridSpewPus));
}

void OozePoison::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::oozePoisonSpewPus));
}

void ColourOOSpace::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::colourOOSpaceTouch));
}

const Clr& ColourOOSpace::getClr() {
  return curColor;
}

void ColourOOSpace::onStandardTurn() {
  curColor.r = Rnd::range(40, 255);
  curColor.g = Rnd::range(40, 255);
  curColor.b = Rnd::range(40, 255);

  restoreHp(1, false);

  if(Map::player->isSeeingActor(*this, nullptr)) {
    Map::player->getPropHandler().tryApplyProp(
      new PropConfused(propTurnsStd));
  }
}

bool Spider::onActorTurn_() {
  return false;
}

void GreenSpider::mkStartItems() {
  inv_->putInIntrinsics(
    ItemFactory::mk(ItemId::greenSpiderBite));
}

void WhiteSpider::mkStartItems() {
  inv_->putInIntrinsics(
    ItemFactory::mk(ItemId::whiteSpiderBite));
}

void RedSpider::mkStartItems() {
  inv_->putInIntrinsics(
    ItemFactory::mk(ItemId::redSpiderBite));
}

void ShadowSpider::mkStartItems() {
  inv_->putInIntrinsics(
    ItemFactory::mk(ItemId::shadowSpiderBite));
}

void LengSpider::mkStartItems() {
  inv_->putInIntrinsics(
    ItemFactory::mk(ItemId::lengSpiderBite));
}

void Wolf::mkStartItems() {
  inv_->putInIntrinsics(
    ItemFactory::mk(ItemId::wolfBite));
}

bool WormMass::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(Rnd::percentile() < chanceToSpawnNew) {

        bool blocked[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksActor(*this, true), blocked);

        Pos mkPos;
        for(int dx = -1; dx <= 1; ++dx) {
          for(int dy = -1; dy <= 1; ++dy) {
            mkPos.set(pos + Pos(dx, dy));
            if(!blocked[mkPos.x][mkPos.y]) {
              Actor* const actor =
                ActorFactory::mk(data_->id, mkPos);
              WormMass* const worm = static_cast<WormMass*>(actor);
              chanceToSpawnNew -= 4;
              worm->chanceToSpawnNew = chanceToSpawnNew;
              worm->awareOfPlayerCounter_ = awareOfPlayerCounter_;
              GameTime::actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void WormMass::mkStartItems() {
  inv_->putInIntrinsics(
    ItemFactory::mk(ItemId::wormMassBite));
}

bool GiantLocust::onActorTurn_() {
  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(Rnd::percentile() < chanceToSpawnNew) {

        bool blocked[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksActor(*this, true), blocked);

        Pos mkPos;
        for(int dx = -1; dx <= 1; ++dx) {
          for(int dy = -1; dy <= 1; ++dy) {
            mkPos.set(pos + Pos(dx, dy));
            if(!blocked[mkPos.x][mkPos.y]) {
              Actor* const actor =
                ActorFactory::mk(data_->id, mkPos);
              GiantLocust* const locust = static_cast<GiantLocust*>(actor);
              chanceToSpawnNew -= 2;
              locust->chanceToSpawnNew = chanceToSpawnNew;
              locust->awareOfPlayerCounter_ = awareOfPlayerCounter_;
              GameTime::actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void GiantLocust::mkStartItems() {
  inv_->putInIntrinsics(
    ItemFactory::mk(ItemId::giantLocustBite));
}

bool LordOfShadows::onActorTurn_() {
  return false;
}

void LordOfShadows::mkStartItems() {

}

bool LordOfSpiders::onActorTurn_() {
  if(deadState == ActorDeadState::alive && awareOfPlayerCounter_ > 0) {

    if(Rnd::coinToss()) {

      const Pos playerPos = Map::player->pos;

      if(Map::player->isSeeingActor(*this, nullptr)) {
        Log::addMsg(data_->spellCastMessage);
      }

      for(int dy = -1; dy <= 1; ++dy) {
        for(int dx = -1; dx <= 1; ++dx) {

          if(Rnd::fraction(3, 4)) {

            const Pos p(playerPos + Pos(dx, dy));
            const auto* const mimicFeature = Map::cells[p.x][p.y].featureStatic;

            if(mimicFeature->canHaveStaticFeature()) {
              const auto& mimicData = FeatureData::getData(mimicFeature->getId());
              Trap* const f = new Trap(p, mimicData, TrapId::spiderWeb);
              Map::put(f);
              f->reveal(false);
            }
          }
        }
      }
    }
  }
  return false;
}

void LordOfSpiders::mkStartItems() {

}

bool LordOfSpirits::onActorTurn_() {
  return false;
}

void LordOfSpirits::mkStartItems() {

}

bool LordOfPestilence::onActorTurn_() {
  return false;
}

void LordOfPestilence::mkStartItems() {

}

bool Zombie::onActorTurn_() {
  return tryResurrect();
}

bool MajorClaphamLee::onActorTurn_() {
  if(tryResurrect()) {
    return true;
  }

  if(deadState == ActorDeadState::alive) {
    if(awareOfPlayerCounter_ > 0) {
      if(!hasSummonedTombLegions) {

        bool visionBlockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), visionBlockers);

        if(isSeeingActor(*(Map::player), visionBlockers)) {
          Log::addMsg("Major Clapham Lee calls forth his Tomb-Legions!");
          vector<ActorId> monsterIds;
          monsterIds.resize(0);

          monsterIds.push_back(actor_deanHalsey);

          const int NR_OF_EXTRA_SPAWNS = 4;

          for(int i = 0; i < NR_OF_EXTRA_SPAWNS; ++i) {
            const int ZOMBIE_TYPE = Rnd::range(1, 3);
            ActorId id = actor_zombie;
            switch(ZOMBIE_TYPE) {
              case 1: id = actor_zombie;        break;
              case 2: id = actor_zombieAxe;     break;
              case 3: id = actor_bloatedZombie; break;
            }
            monsterIds.push_back(id);
          }
          ActorFactory::summonMonsters(pos, monsterIds, true, this);
          Renderer::drawMapAndInterface();
          hasSummonedTombLegions = true;
          Map::player->incrShock(ShockValue::shockValue_heavy, ShockSrc::misc);
          GameTime::actorDidAct();
          return true;
        }
      }
    }
  }

  return false;
}

bool Zombie::tryResurrect() {
  if(deadState == ActorDeadState::corpse) {
    if(!hasResurrected) {
      deadTurnCounter += 1;
      if(deadTurnCounter > 5) {
        if(pos != Map::player->pos && Rnd::percentile() < 7) {
          deadState = ActorDeadState::alive;
          hp_ = (getHpMax(true) * 3) / 4;
          glyph_ = data_->glyph;
          tile_ = data_->tile;
          clr_ = data_->color;
          hasResurrected = true;
          data_->nrKills--;
          if(Map::cells[pos.x][pos.y].isSeenByPlayer) {
            Log::addMsg(
              getNameThe() + " rises again!!", clrWhite, true);
            Map::player->incrShock(ShockValue::shockValue_some, ShockSrc::misc);
          }

          awareOfPlayerCounter_ = data_->nrTurnsAwarePlayer * 2;
          GameTime::actorDidAct();
          return true;
        }
      }
    }
  }
  return false;
}

void Zombie::die_() {
  //If resurrected once and has corpse, blow up the corpse
  if(hasResurrected && deadState == ActorDeadState::corpse) {
    deadState = ActorDeadState::destroyed;
    Map::mkBlood(pos);
    Map::mkGore(pos);
  }
}

void ZombieClaw::mkStartItems() {
  Item* item = nullptr;
  if(Rnd::percentile() < 20) {
    item = ItemFactory::mk(ItemId::zombieClawDiseased);
  } else {
    item = ItemFactory::mk(ItemId::zombieClaw);
  }
  inv_->putInIntrinsics(item);
}

void ZombieAxe::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::zombieAxe));
}

void BloatedZombie::mkStartItems() {
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::bloatedZombiePunch));
  inv_->putInIntrinsics(ItemFactory::mk(ItemId::bloatedZombieSpit));
}

