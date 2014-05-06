#include "UnitTest++.h"

#include <climits>
#include <string>

#include <SDL.h>

#include "Init.h"
#include "Config.h"
#include "Utils.h"
#include "Renderer.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "Thrower.h"
#include "FeatureFactory.h"
#include "ItemFactory.h"
#include "TextFormatting.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "MapGen.h"
#include "Converters.h"
#include "CmnTypes.h"
#include "MapParsing.h"
#include "Fov.h"
#include "LineCalc.h"
#include "SaveHandling.h"
#include "Inventory.h"
#include "PlayerSpellsHandling.h"
#include "PlayerBon.h"
#include "Explosion.h"
#include "ItemAmmo.h"
#include "ItemDevice.h"

using namespace std;

struct BasicFixture {
  BasicFixture() {
    Init::initGame();
    Init::initSession();
    Map::player->pos = Pos(1, 1);
    Map::resetMap(); //Because map generation is not run
  }
  ~BasicFixture() {
    Init::cleanupSession();
    Init::cleanupGame();
  }
};

TEST(RollDice) {
  int val = Rnd::range(100, 200);
  CHECK(val >= 100 && val <= 200);
  val = Rnd::range(-1, 1);
  CHECK(val >= -1 && val <= 1);
}

TEST(ConstrainValInRange) {
  int val = getConstrInRange(5, 9, 10);
  CHECK_EQUAL(val, 9);
  val = getConstrInRange(5, 11, 10);
  CHECK_EQUAL(val, 10);
  val = getConstrInRange(5, 4, 10);
  CHECK_EQUAL(val, 5);

  constrInRange(2, val, 8);
  CHECK_EQUAL(val, 5);
  constrInRange(2, val, 4);
  CHECK_EQUAL(val, 4);
  constrInRange(18, val, 22);
  CHECK_EQUAL(val, 18);

  //Test faulty paramters
  val = getConstrInRange(9, 4, 2); //Min > Max -> return -1
  CHECK_EQUAL(val, -1);
  val = 10;
  constrInRange(20, val, 3); //Min > Max -> do nothing
  CHECK_EQUAL(val, 10);
}

TEST(CalculateDistances) {
  CHECK_EQUAL(Utils::kingDist(Pos(1, 2), Pos(2, 3)), 1);
  CHECK_EQUAL(Utils::kingDist(Pos(1, 2), Pos(2, 4)), 2);
  CHECK_EQUAL(Utils::kingDist(Pos(1, 2), Pos(1, 2)), 0);
  CHECK_EQUAL(Utils::kingDist(Pos(10, 3), Pos(1, 4)), 9);
}

TEST(Directions) {
  const int X0 = 20;
  const int Y0 = 20;
  const Pos fromPos(X0, Y0);
  string str = "";
  DirUtils::getCompassDirName(fromPos, Pos(X0 + 1, Y0), str);
  CHECK_EQUAL("E", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 + 1, Y0 + 1), str);
  CHECK_EQUAL("SE", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0    , Y0 + 1), str);
  CHECK_EQUAL("S", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 - 1, Y0 + 1), str);
  CHECK_EQUAL("SW", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 - 1, Y0), str);
  CHECK_EQUAL("W", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 - 1, Y0 - 1), str);
  CHECK_EQUAL("NW", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0    , Y0 - 1), str);
  CHECK_EQUAL("N", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 + 1, Y0 - 1), str);
  CHECK_EQUAL("NE", str);

  DirUtils::getCompassDirName(fromPos, Pos(X0 + 3, Y0 + 1), str);
  CHECK_EQUAL("E", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 + 2, Y0 + 3), str);
  CHECK_EQUAL("SE", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 + 1, Y0 + 3), str);
  CHECK_EQUAL("S", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 - 3, Y0 + 2), str);
  CHECK_EQUAL("SW", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 - 3, Y0 + 1), str);
  CHECK_EQUAL("W", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 - 3, Y0 - 2), str);
  CHECK_EQUAL("NW", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 + 1, Y0 - 3), str);
  CHECK_EQUAL("N", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 + 3, Y0 - 2), str);
  CHECK_EQUAL("NE", str);

  DirUtils::getCompassDirName(fromPos, Pos(X0 + 10000, Y0), str);
  CHECK_EQUAL("E", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 + 10000, Y0 + 10000), str);
  CHECK_EQUAL("SE", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0        , Y0 + 10000), str);
  CHECK_EQUAL("S", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 - 10000, Y0 + 10000), str);
  CHECK_EQUAL("SW", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 - 10000, Y0), str);
  CHECK_EQUAL("W", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 - 10000, Y0 - 10000), str);
  CHECK_EQUAL("NW", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0        , Y0 - 10000), str);
  CHECK_EQUAL("N", str);
  DirUtils::getCompassDirName(fromPos, Pos(X0 + 10000, Y0 - 10000), str);
  CHECK_EQUAL("NE", str);
}

TEST(FormatText) {
  string str = "one two three four";
  int lineMaxW = 100;
  vector<string> formattedLines;
  TextFormatting::lineToLines(str, lineMaxW, formattedLines);
  CHECK_EQUAL(str, formattedLines.at(0));
  CHECK_EQUAL(int(formattedLines.size()), 1);

  lineMaxW = 13;
  TextFormatting::lineToLines(str, lineMaxW, formattedLines);
  CHECK_EQUAL("one two three", formattedLines.at(0));
  CHECK_EQUAL("four", formattedLines.at(1));
  CHECK_EQUAL(int(formattedLines.size()), 2);

  lineMaxW = 15;
  TextFormatting::lineToLines(str, lineMaxW, formattedLines);
  CHECK_EQUAL("one two three", formattedLines.at(0));
  CHECK_EQUAL("four", formattedLines.at(1));
  CHECK_EQUAL(int(formattedLines.size()), 2);

  lineMaxW = 11;
  TextFormatting::lineToLines(str, lineMaxW, formattedLines);
  CHECK_EQUAL("one two", formattedLines.at(0));
  CHECK_EQUAL("three four", formattedLines.at(1));
  CHECK_EQUAL(int(formattedLines.size()), 2);

  str = "";
  TextFormatting::lineToLines(str, lineMaxW, formattedLines);
  CHECK(formattedLines.empty());
}

TEST_FIXTURE(BasicFixture, LineCalculation) {
  Pos origin(0, 0);
  vector<Pos> line;

  LineCalc::calcNewLine(origin, Pos(3, 0), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(1, 0));
  CHECK(line.at(2) == Pos(2, 0));
  CHECK(line.at(3) ==  Pos(3, 0));

  LineCalc::calcNewLine(origin, Pos(-3, 0), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(-1, 0));
  CHECK(line.at(2) == Pos(-2, 0));
  CHECK(line.at(3) == Pos(-3, 0));

  LineCalc::calcNewLine(origin, Pos(0, 3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(0, 1));
  CHECK(line.at(2) == Pos(0, 2));
  CHECK(line.at(3) == Pos(0, 3));

  LineCalc::calcNewLine(origin, Pos(0, -3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(0, -1));
  CHECK(line.at(2) == Pos(0, -2));
  CHECK(line.at(3) == Pos(0, -3));

  LineCalc::calcNewLine(origin, Pos(3, 3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(1, 1));
  CHECK(line.at(2) == Pos(2, 2));
  CHECK(line.at(3) == Pos(3, 3));

  LineCalc::calcNewLine(Pos(9, 9), Pos(6, 12), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == Pos(9, 9));
  CHECK(line.at(1) == Pos(8, 10));
  CHECK(line.at(2) == Pos(7, 11));
  CHECK(line.at(3) == Pos(6, 12));

  LineCalc::calcNewLine(origin, Pos(-3, 3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(-1, 1));
  CHECK(line.at(2) == Pos(-2, 2));
  CHECK(line.at(3) == Pos(-3, 3));

  LineCalc::calcNewLine(origin, Pos(3, -3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(1, -1));
  CHECK(line.at(2) == Pos(2, -2));
  CHECK(line.at(3) == Pos(3, -3));

  LineCalc::calcNewLine(origin, Pos(-3, -3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(-1, -1));
  CHECK(line.at(2) == Pos(-2, -2));
  CHECK(line.at(3) == Pos(-3, -3));

  //Test disallowing outside map
  LineCalc::calcNewLine(Pos(1, 0), Pos(-9, 0), true, 999, false, line);
  CHECK(line.size() == 2);
  CHECK(line.at(0) == Pos(1, 0));
  CHECK(line.at(1) == Pos(0, 0));

  //Test travel limit parameter
  LineCalc::calcNewLine(origin, Pos(20, 0), true, 2, true, line);
  CHECK(line.size() == 3);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(1, 0));
  CHECK(line.at(2) == Pos(2, 0));

  //Test precalculated FOV line offsets
  const vector<Pos>* deltaLine =
    LineCalc::getFovDeltaLine(Pos(3, 3), FOV_STD_RADI_DB);
  CHECK(deltaLine->size() == 4);
  CHECK(deltaLine->at(0) == Pos(0, 0));
  CHECK(deltaLine->at(1) == Pos(1, 1));
  CHECK(deltaLine->at(2) == Pos(2, 2));
  CHECK(deltaLine->at(3) == Pos(3, 3));

  deltaLine =
    LineCalc::getFovDeltaLine(Pos(-3, 3), FOV_STD_RADI_DB);
  CHECK(deltaLine->size() == 4);
  CHECK(deltaLine->at(0) == Pos(0, 0));
  CHECK(deltaLine->at(1) == Pos(-1, 1));
  CHECK(deltaLine->at(2) == Pos(-2, 2));
  CHECK(deltaLine->at(3) == Pos(-3, 3));

  deltaLine =
    LineCalc::getFovDeltaLine(Pos(3, -3), FOV_STD_RADI_DB);
  CHECK(deltaLine->size() == 4);
  CHECK(deltaLine->at(0) == Pos(0, 0));
  CHECK(deltaLine->at(1) == Pos(1, -1));
  CHECK(deltaLine->at(2) == Pos(2, -2));
  CHECK(deltaLine->at(3) == Pos(3, -3));

  deltaLine =
    LineCalc::getFovDeltaLine(Pos(-3, -3), FOV_STD_RADI_DB);
  CHECK(deltaLine->size() == 4);
  CHECK(deltaLine->at(0) == Pos(0, 0));
  CHECK(deltaLine->at(1) == Pos(-1, -1));
  CHECK(deltaLine->at(2) == Pos(-2, -2));
  CHECK(deltaLine->at(3) == Pos(-3, -3));

  //Check constraints for retrieving FOV offset lines
  //Delta > parameter max distance
  deltaLine =
    LineCalc::getFovDeltaLine(Pos(3, 0), 2);
  CHECK(deltaLine == NULL);
  //Delta > limit of precalculated
  deltaLine =
    LineCalc::getFovDeltaLine(Pos(50, 0), 999);
  CHECK(deltaLine == NULL);
}

TEST_FIXTURE(BasicFixture, Fov) {
  bool blockers[MAP_W][MAP_H];

  Utils::resetArray(blockers, false); //Nothing blocking sight

  const int X = MAP_W_HALF;
  const int Y = MAP_H_HALF;

  Map::player->pos = Pos(X, Y);

  Fov::runPlayerFov(blockers, Map::player->pos);

  const int R = FOV_STD_RADI_INT;

  CHECK(Map::cells[X    ][Y    ].isSeenByPlayer);
  CHECK(Map::cells[X + 1][Y    ].isSeenByPlayer);
  CHECK(Map::cells[X - 1][Y    ].isSeenByPlayer);
  CHECK(Map::cells[X    ][Y + 1].isSeenByPlayer);
  CHECK(Map::cells[X    ][Y - 1].isSeenByPlayer);
  CHECK(Map::cells[X + 2][Y + 2].isSeenByPlayer);
  CHECK(Map::cells[X - 2][Y + 2].isSeenByPlayer);
  CHECK(Map::cells[X + 2][Y - 2].isSeenByPlayer);
  CHECK(Map::cells[X - 2][Y - 2].isSeenByPlayer);
  CHECK(Map::cells[X + R][Y    ].isSeenByPlayer);
  CHECK(Map::cells[X - R][Y    ].isSeenByPlayer);
  CHECK(Map::cells[X    ][Y + R].isSeenByPlayer);
  CHECK(Map::cells[X    ][Y - R].isSeenByPlayer);
}

TEST_FIXTURE(BasicFixture, ThrowItems) {
  //-----------------------------------------------------------------
  // Throwing a throwing knife at a wall should make it land
  // in front of the wall - i.e. the cell it travelled through
  // before encountering the wall.
  //
  // . <- (5, 7)
  // # <- If aiming at wall here... (5, 8)
  // . <- ...throwing knife should finally land here (5, 9)
  // @ <- Player position (5, 10).
  //-----------------------------------------------------------------

  FeatureFactory::spawn(FeatureId::floor, Pos(5, 7));
  FeatureFactory::spawn(FeatureId::floor, Pos(5, 9));
  FeatureFactory::spawn(FeatureId::floor, Pos(5, 10));
  Map::player->pos = Pos(5, 10);
  Pos target(5, 8);
  Item* item = ItemFactory::spawnItem(ItemId::throwingKnife);
  Throwing::throwItem(*(Map::player), target, *item);
  CHECK(Map::cells[5][9].item != NULL);
}

TEST_FIXTURE(BasicFixture, Explosions) {
  const int X0 = 5;
  const int Y0 = 7;

  const FeatureId wallId  = FeatureId::wall;
  const FeatureId floorId = FeatureId::floor;

  FeatureFactory::spawn(floorId, Pos(X0, Y0));

  //Check wall destruction
  for(int i = 0; i < 2; i++) {
    Explosion::runExplosionAt(Pos(X0, Y0), ExplType::expl);

    //Cells around the center, at a distance of 1, should be destroyed
    int r = 1;
    CHECK(Map::cells[X0 + r][Y0    ].featureStatic->getId() != wallId);
    CHECK(Map::cells[X0 - r][Y0    ].featureStatic->getId() != wallId);
    CHECK(Map::cells[X0    ][Y0 + r].featureStatic->getId() != wallId);
    CHECK(Map::cells[X0    ][Y0 - r].featureStatic->getId() != wallId);
    CHECK(Map::cells[X0 + r][Y0 + r].featureStatic->getId() != wallId);
    CHECK(Map::cells[X0 + r][Y0 - r].featureStatic->getId() != wallId);
    CHECK(Map::cells[X0 - r][Y0 + r].featureStatic->getId() != wallId);
    CHECK(Map::cells[X0 - r][Y0 - r].featureStatic->getId() != wallId);

    //Cells around the center, at a distance of 2, should NOT be destroyed
    r = 2;
    CHECK(Map::cells[X0 + r][Y0    ].featureStatic->getId() == wallId);
    CHECK(Map::cells[X0 - r][Y0    ].featureStatic->getId() == wallId);
    CHECK(Map::cells[X0    ][Y0 + r].featureStatic->getId() == wallId);
    CHECK(Map::cells[X0    ][Y0 - r].featureStatic->getId() == wallId);
    CHECK(Map::cells[X0 + r][Y0 + r].featureStatic->getId() == wallId);
    CHECK(Map::cells[X0 + r][Y0 - r].featureStatic->getId() == wallId);
    CHECK(Map::cells[X0 - r][Y0 + r].featureStatic->getId() == wallId);
    CHECK(Map::cells[X0 - r][Y0 - r].featureStatic->getId() == wallId);
  }

  //Check damage to actors
  Actor* a1 = ActorFactory::spawn(actor_rat, Pos(X0 + 1, Y0));
  Explosion::runExplosionAt(Pos(X0, Y0), ExplType::expl);
  CHECK_EQUAL(int(ActorDeadState::destroyed), int(a1->deadState));

  //Check that corpses can be destroyed, and do not block living actors
  const int NR_CORPSES = 3;
  Actor* corpses[NR_CORPSES];
  for(int i = 0; i < NR_CORPSES; i++) {
    corpses[i] = ActorFactory::spawn(actor_rat, Pos(X0 + 1, Y0));
    corpses[i]->deadState = ActorDeadState::corpse;
  }
  a1 = ActorFactory::spawn(actor_rat, Pos(X0 + 1, Y0));
  Explosion::runExplosionAt(Pos(X0, Y0), ExplType::expl);
  for(int i = 0; i < NR_CORPSES; i++) {
    CHECK_EQUAL(int(ActorDeadState::destroyed), int(corpses[i]->deadState));
  }
  CHECK_EQUAL(int(ActorDeadState::destroyed), int(a1->deadState));

  //Check explosion applying Burning to living and dead actors
  a1        = ActorFactory::spawn(actor_rat, Pos(X0 - 1, Y0));
  Actor* a2 = ActorFactory::spawn(actor_rat, Pos(X0 + 1, Y0));
  for(int i = 0; i < NR_CORPSES; i++) {
    corpses[i] = ActorFactory::spawn(actor_rat, Pos(X0 + 1, Y0));
    corpses[i]->deadState = ActorDeadState::corpse;
  }
  Explosion::runExplosionAt(Pos(X0, Y0), ExplType::applyProp,
                            ExplSrc::misc, 0, SfxId::endOfSfxId,
                            new PropBurning(propTurnsStd));
  CHECK(a1->getPropHandler().getProp(propBurning, PropSrc::applied) != NULL);
  CHECK(a2->getPropHandler().getProp(propBurning, PropSrc::applied) != NULL);
  for(int i = 0; i < NR_CORPSES; i++) {
    PropHandler& propHlr = corpses[i]->getPropHandler();
    CHECK(propHlr.getProp(propBurning, PropSrc::applied) != NULL);
  }

  //Check that the explosion can handle the map edge (e.g. that it does not
  //destroy the edge wall, or go outside the map - possibly causing a crash)

  //North-west edge
  int x = 1;
  int y = 1;
  FeatureFactory::spawn(floorId, Pos(x, y));
  Explosion::runExplosionAt(Pos(x, y), ExplType::expl);
  CHECK(Map::cells[x + 1][y    ].featureStatic->getId() != wallId);
  CHECK(Map::cells[x    ][y + 1].featureStatic->getId() != wallId);
  CHECK(Map::cells[x - 1][y    ].featureStatic->getId() == wallId);
  CHECK(Map::cells[x    ][y - 1].featureStatic->getId() == wallId);

  //South-east edge
  x = MAP_W - 2;
  y = MAP_H - 2;
  FeatureFactory::spawn(floorId, Pos(x, y));
  Explosion::runExplosionAt(Pos(x, y), ExplType::expl);
  CHECK(Map::cells[x - 1][y    ].featureStatic->getId() != wallId);
  CHECK(Map::cells[x    ][y - 1].featureStatic->getId() != wallId);
  CHECK(Map::cells[x + 1][y    ].featureStatic->getId() == wallId);
  CHECK(Map::cells[x    ][y + 1].featureStatic->getId() == wallId);
}

TEST_FIXTURE(BasicFixture, MonsterStuckInSpiderWeb) {
  //-----------------------------------------------------------------
  // Test that-
  // * a monster can get stuck in a spider web,
  // * the monster can get loose, and
  // * the web can get destroyed
  //-----------------------------------------------------------------

  const Pos posL(1, 4);
  const Pos posR(2, 4);

  //Spawn left floor cell
  FeatureFactory::spawn(FeatureId::floor, posL);

  //Conditions for finished test
  bool isTestedStuck              = false;
  bool isTestedLooseWebIntact     = false;
  bool isTestedLooseWebDestroyed  = false;

  while(
    isTestedStuck             == false ||
    isTestedLooseWebIntact    == false ||
    isTestedLooseWebDestroyed == false) {

    //Spawn right floor cell
    FeatureFactory::spawn(FeatureId::floor, posR);

    //Spawn a monster that can get stuck in the web
    Actor* const actor = ActorFactory::spawn(actor_zombie, posL);
    Monster* const monster = dynamic_cast<Monster*>(actor);

    //Create a spider web in the right cell
    const FeatureId mimicId =
      Map::cells[posR.x][posR.x].featureStatic->getId();
    const FeatureDataT* const mimicData =
      FeatureData::getData(mimicId);
    TrapSpawnData* const trapSpawnData = new TrapSpawnData(
      mimicData, trap_spiderWeb);
    FeatureFactory::spawn(FeatureId::trap, posR, trapSpawnData);

    //Move the monster into the trap, and back again
    monster->awareOfPlayerCounter_ = INT_MAX; // > 0 req. for triggering trap
    monster->pos = posL;
    monster->moveDir(Dir::right);
    CHECK(monster->pos == posR);
    monster->moveDir(Dir::left);
    monster->moveDir(Dir::left);

    //Check conditions
    if(monster->pos == posR) {
      isTestedStuck = true;
    } else if(monster->pos == posL) {
      const FeatureId featureId =
        Map::cells[posR.x][posR.y].featureStatic->getId();
      if(featureId == FeatureId::floor) {
        isTestedLooseWebDestroyed = true;
      } else {
        isTestedLooseWebIntact = true;
      }
    }

    //Remove the monster
    ActorFactory::deleteAllMonsters();
  }
  //Check that all cases have been triggered (not really necessary, it just
  //verifies that the loop above is correctly written).
  CHECK(isTestedStuck);
  CHECK(isTestedLooseWebIntact);
  CHECK(isTestedLooseWebDestroyed);
}

TEST_FIXTURE(BasicFixture, SavingGame) {
  //Item data
  ItemData::data[int(ItemId::scrollTelep)]->isTried = true;
  ItemData::data[int(ItemId::scrollOpening)]->isIdentified = true;

  //Bonus
  PlayerBon::pickBg(Bg::rogue);
  PlayerBon::traitsPicked_.push_back(Trait::healer);

  //Player inventory
  Inventory& inv = Map::player->getInv();
  inv.moveToGeneral(inv.getSlot(SlotId::wielded));
  Item* item = ItemFactory::spawnItem(ItemId::teslaCannon);
  inv.putInSlot(SlotId::wielded, item);
  //Wear asbestos suit to test properties from wearing items
  inv.moveToGeneral(inv.getSlot(SlotId::armorBody));
  item = ItemFactory::spawnItem(ItemId::armorAsbSuit);
  inv.putInSlot(SlotId::armorBody, item);
  item = ItemFactory::spawnItem(ItemId::pistolClip);
  dynamic_cast<ItemAmmoClip*>(item)->ammo = 1;
  inv.putInGeneral(item);
  item = ItemFactory::spawnItem(ItemId::pistolClip);
  dynamic_cast<ItemAmmoClip*>(item)->ammo = 2;
  inv.putInGeneral(item);
  item = ItemFactory::spawnItem(ItemId::pistolClip);
  dynamic_cast<ItemAmmoClip*>(item)->ammo = 3;
  inv.putInGeneral(item);
  item = ItemFactory::spawnItem(ItemId::pistolClip);
  dynamic_cast<ItemAmmoClip*>(item)->ammo = 3;
  inv.putInGeneral(item);
  item = ItemFactory::spawnItem(ItemId::deviceSentry);
  dynamic_cast<Device*>(item)->condition_ = Condition::shoddy;
  inv.putInGeneral(item);
  item = ItemFactory::spawnItem(ItemId::electricLantern);
  dynamic_cast<Device*>(item)->condition_ = Condition::breaking;
  inv.putInGeneral(item);

  //Player
  ActorDataT& def = Map::player->getData();
  def.name_a = def.name_the = "TEST PLAYER";
  Map::player->changeMaxHp(5, false);

  //Map
  Map::dlvl = 7;

  //Actor data
  ActorData::data[endOfActorIds - 1].nrKills = 123;

  //Learned spells
  PlayerSpellsHandling::learnSpellIfNotKnown(SpellId::bless);
  PlayerSpellsHandling::learnSpellIfNotKnown(SpellId::azathothsWrath);

  //Applied properties
  PropHandler& propHlr = Map::player->getPropHandler();
  propHlr.tryApplyProp(new PropDiseased(propTurnsIndefinite));
  propHlr.tryApplyProp(new PropRSleep(propTurnsSpecific, 3));
  propHlr.tryApplyProp(new PropBlessed(propTurnsStd));
  propHlr.tryApplyProp(new PropWound(propTurnsStd));
  Prop* prop      = propHlr.getProp(propWound, PropSrc::applied);
  PropWound* wnd  = dynamic_cast<PropWound*>(prop);
  CHECK(wnd != NULL);
  CHECK_EQUAL(1, wnd->getNrWounds());
  wnd->onMore();
  CHECK_EQUAL(2, wnd->getNrWounds());
  wnd->onMore();
  CHECK_EQUAL(3, wnd->getNrWounds());

  SaveHandling::save();
  CHECK(SaveHandling::isSaveAvailable());
}

TEST_FIXTURE(BasicFixture, LoadingGame) {
  CHECK(SaveHandling::isSaveAvailable());

  const int PLAYER_MAX_HP_BEFORE_LOAD = Map::player->getHpMax(true);

  SaveHandling::load();

  //Item data
  CHECK_EQUAL(true,  ItemData::data[int(ItemId::scrollTelep)]->isTried);
  CHECK_EQUAL(false, ItemData::data[int(ItemId::scrollTelep)]->isIdentified);
  CHECK_EQUAL(true,  ItemData::data[int(ItemId::scrollOpening)]->isIdentified);
  CHECK_EQUAL(false, ItemData::data[int(ItemId::scrollOpening)]->isTried);
  CHECK_EQUAL(false, ItemData::data[int(ItemId::scrollDetMon)]->isTried);
  CHECK_EQUAL(false, ItemData::data[int(ItemId::scrollDetMon)]->isIdentified);

  //Bonus
  CHECK_EQUAL(int(Bg::rogue), int(PlayerBon::getBg()));
  CHECK(PlayerBon::hasTrait(Trait::healer));
  CHECK_EQUAL(false, PlayerBon::hasTrait(Trait::sharpShooter));
  PlayerBon::traitsPicked_.push_back(Trait::healer);

  //Player inventory
  Inventory& inv = Map::player->getInv();
  CHECK_EQUAL(int(ItemId::teslaCannon),
              int(inv.getItemInSlot(SlotId::wielded)->getData().id));
  CHECK_EQUAL(int(ItemId::armorAsbSuit),
              int(inv.getItemInSlot(SlotId::armorBody)->getData().id));
  vector<Item*> genInv = inv.getGeneral();
  int nrClipWith1 = 0;
  int nrClipWith2 = 0;
  int nrClipWith3 = 0;
  bool isSentryDeviceFound    = false;
  bool isElectricLanternFound = false;
  for(Item * item : genInv) {
    ItemId id = item->getData().id;
    if(id == ItemId::pistolClip) {
      switch(dynamic_cast<ItemAmmoClip*>(item)->ammo) {
        case 1: nrClipWith1++; break;
        case 2: nrClipWith2++; break;
        case 3: nrClipWith3++; break;
        default: {} break;
      }
    } else if(id == ItemId::deviceSentry) {
      isSentryDeviceFound = true;
      CHECK_EQUAL(int(Condition::shoddy),
                  int(dynamic_cast<Device*>(item)->condition_));
    } else if(id == ItemId::electricLantern) {
      isElectricLanternFound = true;
      CHECK_EQUAL(int(Condition::breaking),
                  int(dynamic_cast<Device*>(item)->condition_));
    }
  }
  CHECK_EQUAL(1, nrClipWith1);
  CHECK_EQUAL(1, nrClipWith2);
  CHECK_EQUAL(2, nrClipWith3);
  CHECK(isSentryDeviceFound);
  CHECK(isElectricLanternFound);

  //Player
  ActorDataT& def = Map::player->getData();
  def.name_a = def.name_the = "TEST PLAYER";
  CHECK_EQUAL("TEST PLAYER", def.name_a);
  CHECK_EQUAL("TEST PLAYER", def.name_the);
  //Check max HP (affected by disease)
  CHECK_EQUAL((PLAYER_MAX_HP_BEFORE_LOAD + 5) / 2, Map::player->getHpMax(true));

  //Map
  CHECK_EQUAL(7, Map::dlvl);

  //Actor data
  CHECK_EQUAL(123, ActorData::data[endOfActorIds - 1].nrKills);

  //Learned spells
  CHECK(PlayerSpellsHandling::isSpellLearned(SpellId::bless));
  CHECK(PlayerSpellsHandling::isSpellLearned(SpellId::azathothsWrath));
  CHECK_EQUAL(false, PlayerSpellsHandling::isSpellLearned(SpellId::mayhem));

  //Properties
  PropHandler& propHlr = Map::player->getPropHandler();
  Prop* prop = propHlr.getProp(propDiseased, PropSrc::applied);
  CHECK(prop != NULL);
  CHECK(prop->turnsLeft_ == -1);
  //Check currrent HP (affected by disease)
  CHECK_EQUAL((Map::player->getData().hp + 5) / 2, Map::player->getHp());
  prop = propHlr.getProp(propRSleep, PropSrc::applied);
  CHECK(prop != NULL);
  CHECK(prop->turnsLeft_ == 3);
  prop = propHlr.getProp(propBlessed, PropSrc::applied);
  CHECK(prop != NULL);
  CHECK(prop->turnsLeft_ > 0);
  prop = propHlr.getProp(propWound, PropSrc::applied);
  PropWound* wnd = dynamic_cast<PropWound*>(prop);
  CHECK(wnd != NULL);
  CHECK(wnd->turnsLeft_ == -1);
  CHECK_EQUAL(3, wnd->getNrWounds());

  //Properties from worn item
  prop = propHlr.getProp(propRAcid, PropSrc::inv);
  CHECK(prop != NULL);
  CHECK(prop->turnsLeft_ == -1);
  prop = propHlr.getProp(propRFire, PropSrc::inv);
  CHECK(prop != NULL);
  CHECK(prop->turnsLeft_ == -1);

  //Game time
  CHECK_EQUAL(0, GameTime::getTurn());
}

TEST_FIXTURE(BasicFixture, ConnectRoomsWithCorridor) {
  Rect roomArea1(Pos(1, 1), Pos(10, 10));
  Rect roomArea2(Pos(15, 4), Pos(23, 14));

  for(int y = roomArea1.x0y0.y; y <= roomArea1.x1y1.y; y++) {
    for(int x = roomArea1.x0y0.x; x <= roomArea1.x1y1.x; x++) {
      FeatureFactory::spawn(FeatureId::floor, Pos(x, y));
    }
  }

  for(int y = roomArea2.x0y0.y; y <= roomArea2.x1y1.y; y++) {
    for(int x = roomArea2.x0y0.x; x <= roomArea2.x1y1.x; x++) {
      FeatureFactory::spawn(FeatureId::floor, Pos(x, y));
    }
  }

  Room room1(roomArea1);
  Room room2(roomArea2);

  MapGenUtils::buildZCorridorBetweenRooms(room1, room2, Dir::right);
}

//TODO This would benefit a lot from modifying the map through some
//template parameter instead. Perhaps adapt the map template functionality
//to allow easily creating structures with string parameters, e.g.:
//
//MapTempl::build(Pos(1, 1), vector<string>("#...#"));
//
//It would be neat if this also had functionality for automatically rotate
//or flip structures, e.g.:
//MapTempl::build(Pos(1, 1), templateRotate90, templateNoFlip,
//                   vector<string>("#...#"));
//Where templateRotate90 and templateNoFlip would be enums
//TEST_FIXTURE(BasicFixture, CellPredCorridor) {
//  // #####
//  // #...#
//  // #####
//  for(int x = 3; x <= 5; x++) {
//    FeatureFactory::spawn(FeatureId::floor, Pos(x, 7));
//  }
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[2][7]));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[3][7]));
//  CHECK_EQUAL(true,  CellPred::Corridor().check(Map::cells[4][7]));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[5][7]));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[6][7]));
//
//  // #####
//  // #.#.#
//  // #####
//  FeatureFactory::spawn(FeatureId::wall,  Pos(4, 7));
//  CHECK_EQUAL(false,  CellPred::Corridor().check(Map::cells[4][7]));
//  FeatureFactory::spawn(FeatureId::floor, Pos(4, 7));
//
//  //  ###
//  // ##.##
//  // #...#
//  // ##.##
//  //  ###
//  FeatureFactory::spawn(FeatureId::floor, Pos(4, 6));
//  FeatureFactory::spawn(FeatureId::floor, Pos(4, 8));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[4][7]));
//
//  // ###
//  // #.#
//  // #.#
//  // #.#
//  // ###
//  for(int y = 6; y <= 8; y++) {
//    FeatureFactory::spawn(FeatureId::floor, Pos(20, y));
//  }
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[20][5]));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[20][6]));
//  CHECK_EQUAL(true,  CellPred::Corridor().check(Map::cells[20][7]));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[20][8]));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[20][9]));
//
//  // ###
//  // #.#
//  // ###
//  // #.#
//  // ###
//  FeatureFactory::spawn(FeatureId::wall,  Pos(20, 7));
//  CHECK_EQUAL(false,  CellPred::Corridor().check(Map::cells[20][7]));
//  FeatureFactory::spawn(FeatureId::floor, Pos(20, 7));
//
//  //  ###
//  // ##.##
//  // #...#
//  // ##.##
//  //  ###
//  FeatureFactory::spawn(FeatureId::floor, Pos(19, 7));
//  FeatureFactory::spawn(FeatureId::floor, Pos(21, 7));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[20][8]));
//
//  // ...
//  // #.#
//  // ...
//  for(int x = 30; x <= 32; x++) {
//    FeatureFactory::spawn(FeatureId::floor, Pos(x, 7));
//    FeatureFactory::spawn(FeatureId::floor, Pos(x, 9));
//  }
//  FeatureFactory::spawn(FeatureId::floor, Pos(31, 8));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[31][7]));
//  CHECK_EQUAL(true,  CellPred::Corridor().check(Map::cells[31][8]));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[31][9]));
//
//  // .#.
//  // ...
//  // .#.
//  for(int y = 17; y <= 19; y++) {
//    FeatureFactory::spawn(FeatureId::floor, Pos(30, y));
//    FeatureFactory::spawn(FeatureId::floor, Pos(32, y));
//  }
//  FeatureFactory::spawn(FeatureId::floor, Pos(31, 18));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[30][18]));
//  CHECK_EQUAL(true,  CellPred::Corridor().check(Map::cells[31][18]));
//  CHECK_EQUAL(false, CellPred::Corridor().check(Map::cells[32][18]));
//}

//TODO See comment above CellPredCorridor test
//TEST_FIXTURE(BasicFixture, CellPredNook) {
//  // #####
//  // #...#
//  // #####
//  for(int x = 3; x <= 5; x++) {
//    FeatureFactory::spawn(FeatureId::floor, Pos(x, 7));
//  }
//  CHECK_EQUAL(false, CellPred::Nook().check(Map::cells[2][7]));
//  CHECK_EQUAL(true,  CellPred::Nook().check(Map::cells[3][7]));
//  CHECK_EQUAL(false, CellPred::Nook().check(Map::cells[4][7]));
//  CHECK_EQUAL(true,  CellPred::Nook().check(Map::cells[5][7]));
//  CHECK_EQUAL(false, CellPred::Nook().check(Map::cells[6][7]));
//
//  // ##.
//  // #..
//  // ##.
//  FeatureFactory::spawn(FeatureId::floor, Pos(4, 6));
//  FeatureFactory::spawn(FeatureId::floor, Pos(4, 8));
//  CHECK_EQUAL(true,  CellPred::Nook().check(Map::cells[3][7]));
//
//  // ###
//  // #.#
//  // #.#
//  // #.#
//  // ###
//  for(int y = 6; y <= 8; y++) {
//    FeatureFactory::spawn(FeatureId::floor, Pos(20, y));
//  }
//  CHECK_EQUAL(false, CellPred::Nook().check(Map::cells[20][5]));
//  CHECK_EQUAL(true,  CellPred::Nook().check(Map::cells[20][6]));
//  CHECK_EQUAL(false, CellPred::Nook().check(Map::cells[20][7]));
//  CHECK_EQUAL(true,  CellPred::Nook().check(Map::cells[20][8]));
//  CHECK_EQUAL(false, CellPred::Nook().check(Map::cells[20][9]));
//
//  // ###
//  // #.#
//  // ...
//  FeatureFactory::spawn(FeatureId::floor, Pos(19, 7));
//  FeatureFactory::spawn(FeatureId::floor, Pos(21, 7));
//  CHECK_EQUAL(true,  CellPred::Nook().check(Map::cells[20][6]));
//
//  // ###
//  // #.#
//  // ###
//  FeatureFactory::spawn(FeatureId::floor, Pos(20, 12));
//  CHECK_EQUAL(false, CellPred::Nook().check(Map::cells[20][12]));
//}

TEST_FIXTURE(BasicFixture, MapParseGetCellsWithinDistOfOthers) {
  bool in[MAP_W][MAP_H];
  bool out[MAP_W][MAP_H];

  Utils::resetArray(in, false); //Make sure all values are 0

  in[20][10] = true;

  MapParse::getCellsWithinDistOfOthers(in, out, Range(0, 1));
  CHECK_EQUAL(false, out[18][10]);
  CHECK_EQUAL(true,  out[19][10]);
  CHECK_EQUAL(false, out[20][ 8]);
  CHECK_EQUAL(true,  out[20][ 9]);
  CHECK_EQUAL(true,  out[20][10]);
  CHECK_EQUAL(true,  out[20][11]);
  CHECK_EQUAL(true,  out[21][11]);

  MapParse::getCellsWithinDistOfOthers(in, out, Range(1, 1));
  CHECK_EQUAL(true,  out[19][10]);
  CHECK_EQUAL(false, out[20][10]);
  CHECK_EQUAL(true,  out[21][11]);

  MapParse::getCellsWithinDistOfOthers(in, out, Range(1, 5));
  CHECK_EQUAL(true,  out[23][10]);
  CHECK_EQUAL(true,  out[24][10]);
  CHECK_EQUAL(true,  out[25][10]);
  CHECK_EQUAL(true,  out[25][ 9]);
  CHECK_EQUAL(true,  out[25][11]);
  CHECK_EQUAL(false, out[26][10]);
  CHECK_EQUAL(true,  out[16][10]);
  CHECK_EQUAL(true,  out[15][10]);
  CHECK_EQUAL(true,  out[15][ 9]);
  CHECK_EQUAL(true,  out[15][11]);
  CHECK_EQUAL(false, out[14][10]);

  in[23][10] = true;

  MapParse::getCellsWithinDistOfOthers(in, out, Range(1, 1));
  CHECK_EQUAL(false, out[18][10]);
  CHECK_EQUAL(true,  out[19][10]);
  CHECK_EQUAL(false, out[20][10]);
  CHECK_EQUAL(true,  out[21][10]);
  CHECK_EQUAL(true,  out[22][10]);
  CHECK_EQUAL(false, out[23][10]);
  CHECK_EQUAL(true,  out[24][10]);
  CHECK_EQUAL(false, out[25][10]);
}

#ifdef _WIN32
#undef main
#endif
int main() {
  trace << "Running all tests" << endl;
  UnitTest::RunAllTests();
  return 0;
}


