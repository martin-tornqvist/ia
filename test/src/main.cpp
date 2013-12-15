#include "UnitTest++.h"

#include "engine.h"
#include "BasicUtils.h"
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
#include "CommonTypes.h"
#include "MapParsing.h"
#include "Fov.h"
#include "LineCalc.h"

struct BasicFixture {
  BasicFixture() {
    eng = new Engine;
    eng->initConfig();
    eng->initRenderer();
    eng->initAudio();
    eng->initGame();
    eng->gameTime->insertActorInLoop(eng->player);
    eng->player->pos = Pos(1, 1);
    eng->map->resetMap();
  }
  ~BasicFixture() {
    eng->cleanupGame();
    eng->cleanupAudio();
    eng->cleanupRenderer();
    eng->cleanupConfig();
    delete eng;
  }
  Engine* eng;
};

TEST(RollDice) {
  Dice dice;
  int val = dice.range(100, 200);
  CHECK(val >= 100 && val <= 200);
  val = dice.range(-1, 1);
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
  Engine eng;
  BasicUtils utils(eng);
  CHECK_EQUAL(utils.chebyshevDist(Pos(1, 2), Pos(2, 3)), 1);
  CHECK_EQUAL(utils.chebyshevDist(Pos(1, 2), Pos(2, 4)), 2);
  CHECK_EQUAL(utils.chebyshevDist(Pos(1, 2), Pos(1, 2)), 0);
  CHECK_EQUAL(utils.chebyshevDist(Pos(10, 3), Pos(1, 4)), 9);
}

TEST(Directions) {
  DirConverter dirConv;
  const int X0 = 20;
  const int Y0 = 20;
  const Pos fromPos(X0, Y0);
  string str = "";
  dirConv.getCompassDirName(fromPos, Pos(X0 + 1, Y0), str);
  CHECK_EQUAL("E", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 + 1, Y0 + 1), str);
  CHECK_EQUAL("SE", str);
  dirConv.getCompassDirName(fromPos, Pos(X0    , Y0 + 1), str);
  CHECK_EQUAL("S", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 - 1, Y0 + 1), str);
  CHECK_EQUAL("SW", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 - 1, Y0), str);
  CHECK_EQUAL("W", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 - 1, Y0 - 1), str);
  CHECK_EQUAL("NW", str);
  dirConv.getCompassDirName(fromPos, Pos(X0    , Y0 - 1), str);
  CHECK_EQUAL("N", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 + 1, Y0 - 1), str);
  CHECK_EQUAL("NE", str);

  dirConv.getCompassDirName(fromPos, Pos(X0 + 3, Y0 + 1), str);
  CHECK_EQUAL("E", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 + 2, Y0 + 3), str);
  CHECK_EQUAL("SE", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 + 1, Y0 + 3), str);
  CHECK_EQUAL("S", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 - 3, Y0 + 2), str);
  CHECK_EQUAL("SW", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 - 3, Y0 + 1), str);
  CHECK_EQUAL("W", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 - 3, Y0 - 2), str);
  CHECK_EQUAL("NW", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 + 1, Y0 - 3), str);
  CHECK_EQUAL("N", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 + 3, Y0 - 2), str);
  CHECK_EQUAL("NE", str);

  dirConv.getCompassDirName(fromPos, Pos(X0 + 10000, Y0), str);
  CHECK_EQUAL("E", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 + 10000, Y0 + 10000), str);
  CHECK_EQUAL("SE", str);
  dirConv.getCompassDirName(fromPos, Pos(X0        , Y0 + 10000), str);
  CHECK_EQUAL("S", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 - 10000, Y0 + 10000), str);
  CHECK_EQUAL("SW", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 - 10000, Y0), str);
  CHECK_EQUAL("W", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 - 10000, Y0 - 10000), str);
  CHECK_EQUAL("NW", str);
  dirConv.getCompassDirName(fromPos, Pos(X0        , Y0 - 10000), str);
  CHECK_EQUAL("N", str);
  dirConv.getCompassDirName(fromPos, Pos(X0 + 10000, Y0 - 10000), str);
  CHECK_EQUAL("NE", str);
}

TEST(FormatText) {
  Engine eng;
  TextFormatting t;
  string str = "one two three four";
  int lineMaxWidth = 100;
  vector<string> formattedLines;
  t.lineToLines(str, lineMaxWidth, formattedLines);
  CHECK_EQUAL(str, formattedLines.at(0));
  CHECK_EQUAL(int(formattedLines.size()), 1);

  lineMaxWidth = 13;
  t.lineToLines(str, lineMaxWidth, formattedLines);
  CHECK_EQUAL("one two three", formattedLines.at(0));
  CHECK_EQUAL("four", formattedLines.at(1));
  CHECK_EQUAL(int(formattedLines.size()), 2);

  lineMaxWidth = 15;
  t.lineToLines(str, lineMaxWidth, formattedLines);
  CHECK_EQUAL("one two three", formattedLines.at(0));
  CHECK_EQUAL("four", formattedLines.at(1));
  CHECK_EQUAL(int(formattedLines.size()), 2);

  lineMaxWidth = 11;
  t.lineToLines(str, lineMaxWidth, formattedLines);
  CHECK_EQUAL("one two", formattedLines.at(0));
  CHECK_EQUAL("three four", formattedLines.at(1));
  CHECK_EQUAL(int(formattedLines.size()), 2);
}

TEST_FIXTURE(BasicFixture, LineCalculation) {
  Pos origin(0, 0);
  vector<Pos> line;

  eng->lineCalc->calcNewLine(origin, Pos(3, 0), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(1, 0));
  CHECK(line.at(2) == Pos(2, 0));
  CHECK(line.at(3) ==  Pos(3, 0));

  eng->lineCalc->calcNewLine(origin, Pos(-3, 0), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(-1, 0));
  CHECK(line.at(2) == Pos(-2, 0));
  CHECK(line.at(3) == Pos(-3, 0));

  eng->lineCalc->calcNewLine(origin, Pos(0, 3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(0, 1));
  CHECK(line.at(2) == Pos(0, 2));
  CHECK(line.at(3) == Pos(0, 3));

  eng->lineCalc->calcNewLine(origin, Pos(0, -3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(0, -1));
  CHECK(line.at(2) == Pos(0, -2));
  CHECK(line.at(3) == Pos(0, -3));

  eng->lineCalc->calcNewLine(origin, Pos(3, 3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(1, 1));
  CHECK(line.at(2) == Pos(2, 2));
  CHECK(line.at(3) == Pos(3, 3));

  eng->lineCalc->calcNewLine(Pos(9, 9), Pos(6, 12), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == Pos(9, 9));
  CHECK(line.at(1) == Pos(8, 10));
  CHECK(line.at(2) == Pos(7, 11));
  CHECK(line.at(3) == Pos(6, 12));

  eng->lineCalc->calcNewLine(origin, Pos(-3, 3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(-1, 1));
  CHECK(line.at(2) == Pos(-2, 2));
  CHECK(line.at(3) == Pos(-3, 3));

  eng->lineCalc->calcNewLine(origin, Pos(3, -3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(1, -1));
  CHECK(line.at(2) == Pos(2, -2));
  CHECK(line.at(3) == Pos(3, -3));

  eng->lineCalc->calcNewLine(origin, Pos(-3, -3), true, 999, true, line);
  CHECK(line.size() == 4);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(-1, -1));
  CHECK(line.at(2) == Pos(-2, -2));
  CHECK(line.at(3) == Pos(-3, -3));

  //Test disallowing outside map
  eng->lineCalc->calcNewLine(Pos(1, 0), Pos(-9, 0), true, 999, false, line);
  CHECK(line.size() == 2);
  CHECK(line.at(0) == Pos(1, 0));
  CHECK(line.at(1) == Pos(0, 0));

  //Test travel limit parameter
  eng->lineCalc->calcNewLine(origin, Pos(20, 0), true, 2, true, line);
  CHECK(line.size() == 3);
  CHECK(line.at(0) == origin);
  CHECK(line.at(1) == Pos(1, 0));
  CHECK(line.at(2) == Pos(2, 0));

  //Test precalculated FOV line offsets
  const vector<Pos>* deltaLine =
    eng->lineCalc->getFovDeltaLine(Pos(3, 3), FOV_STANDARD_RADI_DB);
  CHECK(deltaLine->size() == 4);
  CHECK(deltaLine->at(0) == Pos(0, 0));
  CHECK(deltaLine->at(1) == Pos(1, 1));
  CHECK(deltaLine->at(2) == Pos(2, 2));
  CHECK(deltaLine->at(3) == Pos(3, 3));

  deltaLine =
    eng->lineCalc->getFovDeltaLine(Pos(-3, 3), FOV_STANDARD_RADI_DB);
  CHECK(deltaLine->size() == 4);
  CHECK(deltaLine->at(0) == Pos(0, 0));
  CHECK(deltaLine->at(1) == Pos(-1, 1));
  CHECK(deltaLine->at(2) == Pos(-2, 2));
  CHECK(deltaLine->at(3) == Pos(-3, 3));

  deltaLine =
    eng->lineCalc->getFovDeltaLine(Pos(3, -3), FOV_STANDARD_RADI_DB);
  CHECK(deltaLine->size() == 4);
  CHECK(deltaLine->at(0) == Pos(0, 0));
  CHECK(deltaLine->at(1) == Pos(1, -1));
  CHECK(deltaLine->at(2) == Pos(2, -2));
  CHECK(deltaLine->at(3) == Pos(3, -3));

  deltaLine =
    eng->lineCalc->getFovDeltaLine(Pos(-3, -3), FOV_STANDARD_RADI_DB);
  CHECK(deltaLine->size() == 4);
  CHECK(deltaLine->at(0) == Pos(0, 0));
  CHECK(deltaLine->at(1) == Pos(-1, -1));
  CHECK(deltaLine->at(2) == Pos(-2, -2));
  CHECK(deltaLine->at(3) == Pos(-3, -3));

  //Check constraints for retrieving FOV offset lines
  //Delta > parameter max distance
  deltaLine =
    eng->lineCalc->getFovDeltaLine(Pos(3, 0), 2);
  CHECK(deltaLine == NULL);
  //Delta > limit of precalculated
  deltaLine =
    eng->lineCalc->getFovDeltaLine(Pos(50, 0), 999);
  CHECK(deltaLine == NULL);
}

TEST_FIXTURE(BasicFixture, Fov) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];

  eng->basicUtils->resetArray(blockers, false); //Nothing blocking sight

  const int X = MAP_X_CELLS_HALF;
  const int Y = MAP_Y_CELLS_HALF;

  eng->player->pos = Pos(X, Y);

  eng->fov->runPlayerFov(blockers, eng->player->pos);

  const int R = FOV_STANDARD_RADI_INT;

  CHECK(eng->map->cells[X    ][Y    ].isSeenByPlayer);
  CHECK(eng->map->cells[X + 1][Y    ].isSeenByPlayer);
  CHECK(eng->map->cells[X - 1][Y    ].isSeenByPlayer);
  CHECK(eng->map->cells[X    ][Y + 1].isSeenByPlayer);
  CHECK(eng->map->cells[X    ][Y - 1].isSeenByPlayer);
  CHECK(eng->map->cells[X + 2][Y + 2].isSeenByPlayer);
  CHECK(eng->map->cells[X - 2][Y + 2].isSeenByPlayer);
  CHECK(eng->map->cells[X + 2][Y - 2].isSeenByPlayer);
  CHECK(eng->map->cells[X - 2][Y - 2].isSeenByPlayer);
  CHECK(eng->map->cells[X + R][Y    ].isSeenByPlayer);
  CHECK(eng->map->cells[X - R][Y    ].isSeenByPlayer);
  CHECK(eng->map->cells[X    ][Y + R].isSeenByPlayer);
  CHECK(eng->map->cells[X    ][Y - R].isSeenByPlayer);
}

TEST_FIXTURE(BasicFixture, ThrowItems) {
  //-----------------------------------------------------------------
  // Throwing a throwing knife at a wall should make it land
  // in front of the wall - i.e. the cell it travelled through
  // before encountering the wall.
  //
  // .  <- (5, 7)
  // # <- If aiming at wall here... (5, 8)
  // . <- ...throwing knife should finally land here (5, 9)
  // @ <- Player position (5, 10).
  //-----------------------------------------------------------------

  eng->featureFactory->spawnFeatureAt(feature_stoneFloor, Pos(5, 7));
  eng->featureFactory->spawnFeatureAt(feature_stoneFloor, Pos(5, 9));
  eng->featureFactory->spawnFeatureAt(feature_stoneFloor, Pos(5, 10));
  eng->player->pos = Pos(5, 10);
  Pos target(5, 8);
  Item* item = eng->itemFactory->spawnItem(item_throwingKnife);
  eng->thrower->throwItem(*(eng->player), target, *item);
  CHECK(eng->map->cells[5][9].item != NULL);
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
  eng->featureFactory->spawnFeatureAt(feature_stoneFloor, posL);

  //Conditions for finished test
  bool isTestedStuck              = false;
  bool isTestedLooseWebIntact     = false;
  bool isTestedLooseWebDestroyed  = false;

  while(
    isTestedStuck             == false ||
    isTestedLooseWebIntact    == false ||
    isTestedLooseWebDestroyed == false) {

    //Spawn right floor cell
    eng->featureFactory->spawnFeatureAt(feature_stoneFloor, posR);

    //Spawn a monster that can get stuck in the web
    Actor* const actor = eng->actorFactory->spawnActor(actor_zombie, posL);
    Monster* const monster = dynamic_cast<Monster*>(actor);

    //Create a spider web in the right cell
    const Feature_t mimicId =
      eng->map->cells[posR.x][posR.x].featureStatic->getId();
    const FeatureData* const mimicData =
      eng->featureDataHandler->getData(mimicId);
    TrapSpawnData* const trapSpawnData = new TrapSpawnData(
      mimicData, trap_spiderWeb);
    eng->featureFactory->spawnFeatureAt(feature_trap, posR, trapSpawnData);

    //Move the monster into the trap, and back again
    monster->playerAwarenessCounter = INT_MAX; // > 0 req. for triggering trap
    monster->pos = posL;
    monster->moveDir(dirRight);
    CHECK(monster->pos == posR);
    monster->moveDir(dirLeft);    //Move left to try to break loose
    CHECK(monster->pos == posR);  //Even if loose, monster is stuck this move
    monster->moveDir(dirLeft);    //Move left again to actually move if free

    //Check conditions
    if(monster->pos == posR) {
      isTestedStuck = true;
    } else if(monster->pos == posL) {
      const Feature_t featureId =
        eng->map->cells[posR.x][posR.y].featureStatic->getId();
      if(featureId == feature_trashedSpiderWeb) {
        isTestedLooseWebDestroyed = true;
      } else {
        isTestedLooseWebIntact = true;
      }
    }

    //Remove the monster
    eng->actorFactory->deleteAllMonsters();
  }
  //Check that all cases have been triggered (not really necessary, it just
  //verifies that the loop above is correctly written).
  CHECK_EQUAL(isTestedStuck, true);
  CHECK_EQUAL(isTestedLooseWebIntact, true);
  CHECK_EQUAL(isTestedLooseWebDestroyed, true);
}

TEST_FIXTURE(BasicFixture, ConnectRoomsWithCorridor) {
  Rect roomArea1(Pos(1, 1), Pos(10, 10));
  Rect roomArea2(Pos(15, 4), Pos(23, 14));

  for(int y = roomArea1.x0y0.y; y <= roomArea1.x1y1.y; y++) {
    for(int x = roomArea1.x0y0.x; x <= roomArea1.x1y1.x; x++) {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, Pos(x, y));
    }
  }

  for(int y = roomArea2.x0y0.y; y <= roomArea2.x1y1.y; y++) {
    for(int x = roomArea2.x0y0.x; x <= roomArea2.x1y1.x; x++) {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, Pos(x, y));
    }
  }

  Room room1(roomArea1);
  Room room2(roomArea2);

  MapGenUtilCorridorBuilder(*eng).buildZCorridorBetweenRooms(
    room1, room2, dirRight);
}

//TEST_FIXTURE(BasicFixture, MakeBspMap) {
//  MapGenBsp mapGen(eng->;
//  for(int i = 0; i < 1000; i++) {
//    eng->player->pos = Pos(eng->dice.range(1, MAP_X_CELLS - 1),
//                           eng->dice.range(1, MAP_Y_CELLS - 1));
//    mapGen.run();
//  }
//}

int main() {
  trace << "Running all tests" << endl;
  return UnitTest::RunAllTests();
}


