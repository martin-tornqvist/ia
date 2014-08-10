#include "Init.h"

#include "UnitTest++.h"

#include <climits>
#include <string>

#include <SDL.h>

#include "Config.h"
#include "Utils.h"
#include "Renderer.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "Throwing.h"
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
#include "FeatureRigid.h"
#include "FeatureTrap.h"

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
  val = getConstrInRange(9, 4, 2);   //Min > Max -> return -1
  CHECK_EQUAL(val, -1);
  val = 10;
  constrInRange(20, val, 3);   //Min > Max -> do nothing
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
  CHECK(line.at(3) == Pos(3, 0));

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
  deltaLine = LineCalc::getFovDeltaLine(Pos(3, 0), 2);
  CHECK(!deltaLine);
  //Delta > limit of precalculated
  deltaLine = LineCalc::getFovDeltaLine(Pos(50, 0), 999);
  CHECK(!deltaLine);
}

TEST_FIXTURE(BasicFixture, Fov) {
  bool blocked[MAP_W][MAP_H];

  Utils::resetArray(blocked, false);   //Nothing blocking sight

  const int X = MAP_W_HALF;
  const int Y = MAP_H_HALF;

  Map::player->pos = Pos(X, Y);

  Fov::runPlayerFov(blocked, Map::player->pos);

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

  Map::put(new Floor(Pos(5, 7)));
  Map::put(new Floor(Pos(5, 9)));
  Map::put(new Floor(Pos(5, 10)));
  Map::player->pos = Pos(5, 10);
  Pos target(5, 8);
  Item* item = ItemFactory::mk(ItemId::throwingKnife);
  Throwing::throwItem(*(Map::player), target, *item);
  CHECK(Map::cells[5][9].item);
}

TEST_FIXTURE(BasicFixture, Explosions) {
  const int X0 = 5;
  const int Y0 = 7;

  auto* const floor = new Floor(Pos(X0, Y0));
  Map::put(new Floor(Pos(X0, Y0)));

  //Check wall destruction
  for(int i = 0; i < 2; ++i) {
    Explosion::runExplosionAt(Pos(X0, Y0), ExplType::expl);

    //Cells around the center, at a distance of 1, should be destroyed
    int r = 1;
    CHECK(Map::cells[X0 + r][Y0    ].rigid->getId() != FeatureId::wall);
    CHECK(Map::cells[X0 - r][Y0    ].rigid->getId() != FeatureId::wall);
    CHECK(Map::cells[X0    ][Y0 + r].rigid->getId() != FeatureId::wall);
    CHECK(Map::cells[X0    ][Y0 - r].rigid->getId() != FeatureId::wall);
    CHECK(Map::cells[X0 + r][Y0 + r].rigid->getId() != FeatureId::wall);
    CHECK(Map::cells[X0 + r][Y0 - r].rigid->getId() != FeatureId::wall);
    CHECK(Map::cells[X0 - r][Y0 + r].rigid->getId() != FeatureId::wall);
    CHECK(Map::cells[X0 - r][Y0 - r].rigid->getId() != FeatureId::wall);

    //Cells around the center, at a distance of 2, should NOT be destroyed
    r = 2;
    CHECK(Map::cells[X0 + r][Y0    ].rigid->getId() == FeatureId::wall);
    CHECK(Map::cells[X0 - r][Y0    ].rigid->getId() == FeatureId::wall);
    CHECK(Map::cells[X0    ][Y0 + r].rigid->getId() == FeatureId::wall);
    CHECK(Map::cells[X0    ][Y0 - r].rigid->getId() == FeatureId::wall);
    CHECK(Map::cells[X0 + r][Y0 + r].rigid->getId() == FeatureId::wall);
    CHECK(Map::cells[X0 + r][Y0 - r].rigid->getId() == FeatureId::wall);
    CHECK(Map::cells[X0 - r][Y0 + r].rigid->getId() == FeatureId::wall);
    CHECK(Map::cells[X0 - r][Y0 - r].rigid->getId() == FeatureId::wall);
  }

  //Check damage to actors
  Actor* a1 = ActorFactory::mk(ActorId::rat, Pos(X0 + 1, Y0));
  Explosion::runExplosionAt(Pos(X0, Y0), ExplType::expl);
  CHECK_EQUAL(int(ActorDeadState::destroyed), int(a1->deadState));

  //Check that corpses can be destroyed, and do not block living actors
  const int NR_CORPSES = 3;
  Actor* corpses[NR_CORPSES];
  for(int i = 0; i < NR_CORPSES; ++i) {
    corpses[i] = ActorFactory::mk(ActorId::rat, Pos(X0 + 1, Y0));
    corpses[i]->deadState = ActorDeadState::corpse;
  }
  a1 = ActorFactory::mk(ActorId::rat, Pos(X0 + 1, Y0));
  Explosion::runExplosionAt(Pos(X0, Y0), ExplType::expl);
  for(int i = 0; i < NR_CORPSES; ++i) {
    CHECK_EQUAL(int(ActorDeadState::destroyed), int(corpses[i]->deadState));
  }
  CHECK_EQUAL(int(ActorDeadState::destroyed), int(a1->deadState));

  //Check explosion applying Burning to living and dead actors
  a1        = ActorFactory::mk(ActorId::rat, Pos(X0 - 1, Y0));
  Actor* a2 = ActorFactory::mk(ActorId::rat, Pos(X0 + 1, Y0));
  for(int i = 0; i < NR_CORPSES; ++i) {
    corpses[i] = ActorFactory::mk(ActorId::rat, Pos(X0 + 1, Y0));
    corpses[i]->deadState = ActorDeadState::corpse;
  }
  Explosion::runExplosionAt(Pos(X0, Y0), ExplType::applyProp,
                            ExplSrc::misc, 0, SfxId::END,
                            new PropBurning(PropTurns::std));
  CHECK(a1->getPropHandler().getProp(propBurning, PropSrc::applied));
  CHECK(a2->getPropHandler().getProp(propBurning, PropSrc::applied));
  for(int i = 0; i < NR_CORPSES; ++i) {
    PropHandler& propHlr = corpses[i]->getPropHandler();
    CHECK(propHlr.getProp(propBurning, PropSrc::applied));
  }

  //Check that the explosion can handle the map edge (e.g. that it does not
  //destroy the edge wall, or go outside the map - possibly causing a crash)

  //North-west edge
  int x = 1;
  int y = 1;
  Map::put(new Floor(Pos(x, y)));
  Explosion::runExplosionAt(Pos(x, y), ExplType::expl);
  CHECK(Map::cells[x + 1][y    ].rigid->getId() != FeatureId::wall);
  CHECK(Map::cells[x    ][y + 1].rigid->getId() != FeatureId::wall);
  CHECK(Map::cells[x - 1][y    ].rigid->getId() == FeatureId::wall);
  CHECK(Map::cells[x    ][y - 1].rigid->getId() == FeatureId::wall);

  //South-east edge
  x = MAP_W - 2;
  y = MAP_H - 2;
  Map::put(new Floor(Pos(x, y)));
  Explosion::runExplosionAt(Pos(x, y), ExplType::expl);
  CHECK(Map::cells[x - 1][y    ].rigid->getId() != FeatureId::wall);
  CHECK(Map::cells[x    ][y - 1].rigid->getId() != FeatureId::wall);
  CHECK(Map::cells[x + 1][y    ].rigid->getId() == FeatureId::wall);
  CHECK(Map::cells[x    ][y + 1].rigid->getId() == FeatureId::wall);
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
  Map::put(new Floor(posL));

  //Conditions for finished test
  bool testedStuck              = false;
  bool testedLooseWebIntact     = false;
  bool testedLooseWebDestroyed  = false;

  while(!testedStuck || !testedLooseWebIntact || !testedLooseWebDestroyed) {

    //Spawn right floor cell
    Map::put(new Floor(posR));

    //Spawn a monster that can get stuck in the web
    Actor* const actor = ActorFactory::mk(ActorId::zombie, posL);
    Monster* const monster = static_cast<Monster*>(actor);

    //Create a spider web in the right cell
    const auto  mimicId     = Map::cells[posR.x][posR.x].rigid->getId();
    const auto& mimicData   = FeatureData::getData(mimicId);
    const auto* const mimic = static_cast<const Rigid*>(mimicData.mkObj(posR));
    Map::put(new Trap(posR, mimic, TrapId::spiderWeb));

    //Move the monster into the trap, and back again
    monster->awareOfPlayerCounter_ = INT_MAX; // > 0 req. for triggering trap
    monster->pos = posL;
    monster->moveDir(Dir::right);
    CHECK(monster->pos == posR);
    monster->moveDir(Dir::left);
    monster->moveDir(Dir::left);

    //Check conditions
    if(monster->pos == posR) {
      testedStuck = true;
    } else if(monster->pos == posL) {
      const auto featureId = Map::cells[posR.x][posR.y].rigid->getId();
      if(featureId == FeatureId::floor) {
        testedLooseWebDestroyed = true;
      } else {
        testedLooseWebIntact = true;
      }
    }

    //Remove the monster
    ActorFactory::deleteAllMonsters();
  }
  //Check that all cases have been triggered (not really necessary, it just
  //verifies that the loop above is correctly written).
  CHECK(testedStuck);
  CHECK(testedLooseWebIntact);
  CHECK(testedLooseWebDestroyed);
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
  //First, remove all present items (to have a known state)
  vector<Item*>& gen = inv.general_;
  for(Item* item : gen) {delete item;}
  gen.resize(0);
  vector<InvSlot>& slots = inv.slots_;
  for(InvSlot& slot : slots) {
    if(slot.item) {
      delete slot.item;
      slot.item = nullptr;
    }
  }
  //Put new items
  Item* item = ItemFactory::mk(ItemId::teslaCannon);
  inv.putInSlot(SlotId::wielded, item);
  //Wear asbestos suit to test properties from wearing items
  item = ItemFactory::mk(ItemId::armorAsbSuit);
  inv.putInSlot(SlotId::body, item);
  item = ItemFactory::mk(ItemId::pistolClip);
  static_cast<ItemAmmoClip*>(item)->ammo = 1;
  inv.putInGeneral(item);
  item = ItemFactory::mk(ItemId::pistolClip);
  static_cast<ItemAmmoClip*>(item)->ammo = 2;
  inv.putInGeneral(item);
  item = ItemFactory::mk(ItemId::pistolClip);
  static_cast<ItemAmmoClip*>(item)->ammo = 3;
  inv.putInGeneral(item);
  item = ItemFactory::mk(ItemId::pistolClip);
  static_cast<ItemAmmoClip*>(item)->ammo = 3;
  inv.putInGeneral(item);
  item = ItemFactory::mk(ItemId::deviceSentry);
  static_cast<Device*>(item)->condition_ = Condition::shoddy;
  inv.putInGeneral(item);
  item = ItemFactory::mk(ItemId::electricLantern);
  static_cast<Device*>(item)->condition_ = Condition::breaking;
  inv.putInGeneral(item);

  //Player
  ActorDataT& def = Map::player->getData();
  def.nameA = def.nameThe = "TEST PLAYER";
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
  propHlr.tryApplyProp(new PropDiseased(PropTurns::indefinite));
  propHlr.tryApplyProp(new PropRSleep(PropTurns::specific, 3));
  propHlr.tryApplyProp(new PropBlessed(PropTurns::std));
  propHlr.tryApplyProp(new PropWound(PropTurns::std));
  Prop* prop      = propHlr.getProp(propWound, PropSrc::applied);
  PropWound* wnd  = static_cast<PropWound*>(prop);
  CHECK(wnd);
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
  Inventory& inv  = Map::player->getInv();
  auto& genInv    = inv.general_;
  CHECK_EQUAL(6, int(genInv.size()));
  CHECK_EQUAL(int(ItemId::teslaCannon),
              int(inv.getItemInSlot(SlotId::wielded)->getData().id));
  CHECK_EQUAL(int(ItemId::armorAsbSuit),
              int(inv.getItemInSlot(SlotId::body)->getData().id));
  int nrClipWith1 = 0;
  int nrClipWith2 = 0;
  int nrClipWith3 = 0;
  bool isSentryDeviceFound    = false;
  bool isElectricLanternFound = false;
  for(Item* item : genInv) {
    ItemId id = item->getData().id;
    if(id == ItemId::pistolClip) {
      switch(static_cast<ItemAmmoClip*>(item)->ammo) {
        case 1: nrClipWith1++; break;
        case 2: nrClipWith2++; break;
        case 3: nrClipWith3++; break;
        default: {} break;
      }
    } else if(id == ItemId::deviceSentry) {
      isSentryDeviceFound = true;
      CHECK_EQUAL(int(Condition::shoddy),
                  int(static_cast<Device*>(item)->condition_));
    } else if(id == ItemId::electricLantern) {
      isElectricLanternFound = true;
      CHECK_EQUAL(int(Condition::breaking),
                  int(static_cast<DeviceLantern*>(item)->condition_));
    }
  }
  CHECK_EQUAL(1, nrClipWith1);
  CHECK_EQUAL(1, nrClipWith2);
  CHECK_EQUAL(2, nrClipWith3);
  CHECK(isSentryDeviceFound);
  CHECK(isElectricLanternFound);

  //Player
  ActorDataT& def = Map::player->getData();
  def.nameA = def.nameThe = "TEST PLAYER";
  CHECK_EQUAL("TEST PLAYER", def.nameA);
  CHECK_EQUAL("TEST PLAYER", def.nameThe);
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
  CHECK(prop);
  CHECK(prop->turnsLeft_ == -1);
  //Check currrent HP (affected by disease)
  CHECK_EQUAL((Map::player->getData().hp + 5) / 2, Map::player->getHp());
  prop = propHlr.getProp(propRSleep, PropSrc::applied);
  CHECK(prop);
  CHECK(prop->turnsLeft_ == 3);
  prop = propHlr.getProp(propBlessed, PropSrc::applied);
  CHECK(prop);
  CHECK(prop->turnsLeft_ > 0);
  prop = propHlr.getProp(propWound, PropSrc::applied);
  PropWound* wnd = static_cast<PropWound*>(prop);
  CHECK(wnd);
  CHECK(wnd->turnsLeft_ == -1);
  CHECK_EQUAL(3, wnd->getNrWounds());

  //Properties from worn item
  prop = propHlr.getProp(propRAcid, PropSrc::inv);
  CHECK(prop);
  CHECK(prop->turnsLeft_ == -1);
  prop = propHlr.getProp(propRFire, PropSrc::inv);
  CHECK(prop);
  CHECK(prop->turnsLeft_ == -1);

  //Game time
  CHECK_EQUAL(0, GameTime::getTurn());
}

TEST_FIXTURE(BasicFixture, FloodFilling) {
  bool b[MAP_W][MAP_H];
  Utils::resetArray(b, false);
  for(int y = 0; y < MAP_H; ++y) {b[0][y] = b[MAP_W - 1][y] = false;}
  for(int x = 0; x < MAP_W; ++x) {b[x][0] = b[x][MAP_H - 1] = false;}
  int flood[MAP_W][MAP_H];
  FloodFill::run(Pos(20, 10), b, flood, 999, Pos(-1, -1), true);
  CHECK_EQUAL(0, flood[20][10]);
  CHECK_EQUAL(1, flood[19][10]);
  CHECK_EQUAL(1, flood[21][10]);
  CHECK_EQUAL(1, flood[20][11]);
  CHECK_EQUAL(1, flood[21][11]);
  CHECK_EQUAL(4, flood[24][12]);
  CHECK_EQUAL(4, flood[24][14]);
  CHECK_EQUAL(5, flood[24][15]);
  CHECK_EQUAL(0, flood[0][0]);
  CHECK_EQUAL(0, flood[MAP_W - 1][MAP_H - 1]);
}

TEST_FIXTURE(BasicFixture, PathFinding) {
  vector<Pos> path;
  bool b[MAP_W][MAP_H];
  Utils::resetArray(b, false);
  for(int y = 0; y < MAP_H; ++y) {b[0][y] = b[MAP_W - 1][y] = false;}
  for(int x = 0; x < MAP_W; ++x) {b[x][0] = b[x][MAP_H - 1] = false;}

  PathFind::run(Pos(20, 10), Pos(25, 10), b, path);

  CHECK(!path.empty());
  CHECK(path.back() != Pos(20, 10));
  CHECK_EQUAL(25, path.front().x);
  CHECK_EQUAL(10, path.front().y);
  CHECK_EQUAL(5, int(path.size()));

  PathFind::run(Pos(20, 10), Pos(5, 3), b, path);

  CHECK(!path.empty());
  CHECK(path.back() != Pos(20, 10));
  CHECK_EQUAL(5, path.front().x);
  CHECK_EQUAL(3, path.front().y);
  CHECK_EQUAL(15, int(path.size()));

  b[10][5] = true;

  PathFind::run(Pos(7, 5), Pos(20, 5), b, path);

  CHECK(!path.empty());
  CHECK(path.back() != Pos(7, 5));
  CHECK_EQUAL(20, path.front().x);
  CHECK_EQUAL(5, path.front().y);
  CHECK_EQUAL(13, int(path.size()));
  CHECK(find(begin(path), end(path), Pos(10, 5)) == end(path));

  b[19][4] = b[19][5] =  b[19][6] = true;

  PathFind::run(Pos(7, 5), Pos(20, 5), b, path);

  CHECK(!path.empty());
  CHECK(path.back() != Pos(7, 5));
  CHECK_EQUAL(20, path.front().x);
  CHECK_EQUAL(5, path.front().y);
  CHECK_EQUAL(14, int(path.size()));
  CHECK(find(begin(path), end(path), Pos(19, 4)) == end(path));
  CHECK(find(begin(path), end(path), Pos(19, 5)) == end(path));
  CHECK(find(begin(path), end(path), Pos(19, 6)) == end(path));

  PathFind::run(Pos(40, 10), Pos(43, 15), b, path, false);

  CHECK(!path.empty());
  CHECK(path.back() != Pos(40, 10));
  CHECK_EQUAL(43, path.front().x);
  CHECK_EQUAL(15, path.front().y);
  CHECK_EQUAL(8, int(path.size()));

  b[41][10] = b[40][11]  = true;

  PathFind::run(Pos(40, 10), Pos(43, 15), b, path, false);

  CHECK(!path.empty());
  CHECK(path.back() != Pos(40, 10));
  CHECK_EQUAL(43, path.front().x);
  CHECK_EQUAL(15, path.front().y);
  CHECK_EQUAL(10, int(path.size()));
}

TEST_FIXTURE(BasicFixture, FindRoomCorrEntries) {
  //------------------------------------------------ Square, normal sized room
  Rect roomRect(20, 5, 30, 10);
  Room room(roomRect);

  for(int y = roomRect.p0.y; y <= roomRect.p1.y; ++y) {
    for(int x = roomRect.p0.x; x <= roomRect.p1.x; ++x) {
      Map::put(new Floor(Pos(x, y)));
      Map::roomMap[x][y] = &room;
    }
  }

  vector<Pos> entryList;
  MapGenUtils::getValidRoomCorrEntries(room, entryList);
  bool entryMap[MAP_W][MAP_H];
  Utils::mkBoolMapFromVector(entryList, entryMap);

  CHECK(!entryMap[19][4]);
  CHECK(entryMap[19][5]);
  CHECK(entryMap[20][4]);
  CHECK(!entryMap[20][5]);
  CHECK(entryMap[21][4]);
  CHECK(entryMap[25][4]);
  CHECK(!entryMap[25][8]);
  CHECK(entryMap[29][4]);
  CHECK(entryMap[30][4]);
  CHECK(!entryMap[31][4]);

  //Check that a cell in the middle of the room is not an entry
  Map::roomMap[25][7] = nullptr;
  MapGenUtils::getValidRoomCorrEntries(room, entryList);
  Utils::mkBoolMapFromVector(entryList, entryMap);

  CHECK(!entryMap[25][7]);

  Map::roomMap[25][7] = &room;
  Map::put(new Wall(Pos(25, 7)));
  MapGenUtils::getValidRoomCorrEntries(room, entryList);
  Utils::mkBoolMapFromVector(entryList, entryMap);

  CHECK(!entryMap[25][7]);

  Map::roomMap[25][7] = nullptr;
  MapGenUtils::getValidRoomCorrEntries(room, entryList);
  Utils::mkBoolMapFromVector(entryList, entryMap);

  CHECK(!entryMap[25][7]);

  //Check that the room can share an antry point with a nearby room
  roomRect = Rect(10, 5, 18, 10);
  Room nearbyRoom(roomRect);

  for(int y = roomRect.p0.y; y <= roomRect.p1.y; ++y) {
    for(int x = roomRect.p0.x; x <= roomRect.p1.x; ++x) {
      Map::put(new Floor(Pos(x, y)));
      Map::roomMap[x][y] = &nearbyRoom;
    }
  }

  MapGenUtils::getValidRoomCorrEntries(room, entryList);
  Utils::mkBoolMapFromVector(entryList, entryMap);

  vector<Pos> entryListNearbyRoom;
  MapGenUtils::getValidRoomCorrEntries(nearbyRoom, entryListNearbyRoom);
  bool entryMapNearbyRoom[MAP_W][MAP_H];
  Utils::mkBoolMapFromVector(entryListNearbyRoom, entryMapNearbyRoom);

  for(int y = 5; y <= 10; ++y) {
    CHECK(entryMap[19][y]);
    CHECK(entryMapNearbyRoom[19][y]);
  }

  //------------------------------------------------ Room with only one cell
  room = Room(Rect(60, 10, 60, 10));
  Map::put(new Floor(Pos(60, 10)));
  Map::roomMap[60][10] = &room;
  MapGenUtils::getValidRoomCorrEntries(room, entryList);
  Utils::mkBoolMapFromVector(entryList, entryMap);

  // 59 60 61
  // #  #  # 9
  // #  .  # 10
  // #  #  # 11
  CHECK(!entryMap[59][9]);
  CHECK(entryMap[60][9]);
  CHECK(!entryMap[61][9]);
  CHECK(entryMap[59][10]);
  CHECK(!entryMap[60][10]);
  CHECK(entryMap[61][10]);
  CHECK(!entryMap[59][11]);
  CHECK(entryMap[60][11]);
  CHECK(!entryMap[61][11]);

  //Add an adjacent floor above the room
  // 59 60 61
  // #  .  # 9
  // #  .  # 10
  // #  #  # 11
  Map::put(new Floor(Pos(60, 9)));
  MapGenUtils::getValidRoomCorrEntries(room, entryList);
  Utils::mkBoolMapFromVector(entryList, entryMap);

  CHECK(!entryMap[59][9]);
  CHECK(!entryMap[60][9]);
  CHECK(!entryMap[61][9]);
  CHECK(entryMap[59][10]);
  CHECK(!entryMap[60][10]);
  CHECK(entryMap[61][10]);
  CHECK(!entryMap[59][11]);
  CHECK(entryMap[60][11]);
  CHECK(!entryMap[61][11]);

  //Mark the adjacent floor as a room and check again
  Room adjRoom(Rect(60, 9, 60, 9));
  Map::roomMap[60][9] = &adjRoom;
  MapGenUtils::getValidRoomCorrEntries(room, entryList);
  Utils::mkBoolMapFromVector(entryList, entryMap);

  CHECK(!entryMap[59][9]);
  CHECK(!entryMap[60][9]);
  CHECK(!entryMap[61][9]);
  CHECK(entryMap[59][10]);
  CHECK(!entryMap[60][10]);
  CHECK(entryMap[61][10]);
  CHECK(!entryMap[59][11]);
  CHECK(entryMap[60][11]);
  CHECK(!entryMap[61][11]);

  //Make the room wider, entries should not be placed next to adjacent floor
  // 58 59 60 61
  // #  #  .  # 9
  // #  .  .  # 10
  // #  #  #  # 11
  room.r_.p0.x = 59;
  Map::put(new Floor(Pos(59, 10)));
  Map::roomMap[59][10] = &room;
  MapGenUtils::getValidRoomCorrEntries(room, entryList);
  Utils::mkBoolMapFromVector(entryList, entryMap);

  CHECK(!entryMap[58][9]);
  CHECK(entryMap[59][9]);
  CHECK(!entryMap[60][9]);
  CHECK(!entryMap[61][9]);
  CHECK(entryMap[58][10]);
  CHECK(!entryMap[59][10]);
  CHECK(!entryMap[60][10]);
  CHECK(entryMap[61][10]);
  CHECK(!entryMap[58][11]);
  CHECK(entryMap[59][11]);
  CHECK(entryMap[60][11]);
  CHECK(!entryMap[61][11]);

  //Remove the adjacent room, and check that the blocked entries are now placed
  //TODO
}

TEST_FIXTURE(BasicFixture, ConnectRoomsWithCorridor) {
  Rect roomArea1(Pos(1, 1), Pos(10, 10));
  Rect roomArea2(Pos(15, 4), Pos(23, 14));

  Room room0(roomArea1);
  Room room1(roomArea2);

  for(int y = roomArea1.p0.y; y <= roomArea1.p1.y; ++y) {
    for(int x = roomArea1.p0.x; x <= roomArea1.p1.x; ++x) {
      Map::put(new Floor(Pos(x, y)));
      Map::roomMap[x][y] = &room0;
    }
  }

  for(int y = roomArea2.p0.y; y <= roomArea2.p1.y; ++y) {
    for(int x = roomArea2.p0.x; x <= roomArea2.p1.x; ++x) {
      Map::put(new Floor(Pos(x, y)));
      Map::roomMap[x][y] = &room1;
    }
  }

  MapGenUtils::mkPathFindCor(room0, room1);

  int flood[MAP_W][MAP_H];
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);
  FloodFill::run(5, blocked, flood, INT_MAX, -1, true);
  CHECK(flood[20][10] > 0);
}

TEST_FIXTURE(BasicFixture, MapParseGetCellsWithinDistOfOthers) {
  bool in[MAP_W][MAP_H];
  bool out[MAP_W][MAP_H];

  Utils::resetArray(in, false);  //Make sure all values are 0

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

//-----------------------------------------------------------------------------
// Some code exercise - Ichi! Ni! San!
//-----------------------------------------------------------------------------
//TEST_FIXTURE(BasicFixture, MapGenStd) {
//  for(int i = 0; i < 100; ++i) {MapGen::Std::run();}
//}

#ifdef _WIN32
#undef main
#endif
int main() {
  UnitTest::RunAllTests();
  return 0;
}


