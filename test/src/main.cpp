#include "UnitTest++.h"

#include "Engine.h"
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

struct BasicFixture {
  BasicFixture() {
    eng = new Engine;
    eng->initConfig();
    eng->initRenderer();
    eng->initAudio();
    eng->initGame();
    eng->gameTime->insertActorInLoop(eng->player);
    eng->player->pos = Pos(1, 1);
    eng->map->clearMap();
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
  int val = getValConstrInRange(5, 9, 10);
  CHECK_EQUAL(val, 9);
  val = getValConstrInRange(5, 11, 10);
  CHECK_EQUAL(val, 10);
  val = getValConstrInRange(5, 4, 10);
  CHECK_EQUAL(val, 5);

  constrInRange(2, val, 8);
  CHECK_EQUAL(val, 5);
  constrInRange(2, val, 4);
  CHECK_EQUAL(val, 4);
  constrInRange(18, val, 22);
  CHECK_EQUAL(val, 18);

  //Test faulty paramters
  val = getValConstrInRange(9, 4, 2); //Min > Max -> return -1
  CHECK_EQUAL(val, -1);
  val = 10;
  constrInRange(20, val, 3); //Min > Max -> do nothing
  CHECK_EQUAL(val, 10);
}


TEST(CalculateDistances) {
  Engine eng;
  BasicUtils utils(&eng);
  CHECK_EQUAL(utils.chebyshevDist(Pos(1, 2), Pos(2, 3)), 1);
  CHECK_EQUAL(utils.chebyshevDist(Pos(1, 2), Pos(2, 4)), 2);
  CHECK_EQUAL(utils.chebyshevDist(Pos(1, 2), Pos(1, 2)), 0);
  CHECK_EQUAL(utils.chebyshevDist(Pos(10, 3), Pos(1, 4)), 9);
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
  CHECK(eng->map->items[5][9] != NULL);
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
      eng->map->featuresStatic[posR.x][posR.x]->getId();
    const FeatureData* const mimicData =
      eng->featureDataHandler->getData(mimicId);
    TrapSpawnData* const trapSpawnData = new TrapSpawnData(
      mimicData, trap_spiderWeb);
    eng->featureFactory->spawnFeatureAt(feature_trap, posR, trapSpawnData);

    //Move the monster into the trap, and back again
    monster->playerAwarenessCounter = INT_MAX; // > 0 req. for triggering trap
    monster->pos = posL;
    monster->moveToCell(posR);
    CHECK(monster->pos == posR);
    monster->moveToCell(posL);   //Move left to try to break loose
    CHECK(monster->pos == posR); //Even if loose, monster is stuck this move
    monster->moveToCell(posL);   //Move left again to actually move if free

    //Check conditions
    if(monster->pos == posR) {
      isTestedStuck = true;
    } else if(monster->pos == posL) {
      const Feature_t featureId =
        eng->map->featuresStatic[posR.x][posR.y]->getId();
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

  MapGenUtilCorridorBuilder(eng).buildZCorridorBetweenRooms(
    room1, room2, directionRight);
}

//TEST_FIXTURE(BasicFixture, MakeBspMap) {
//  MapGenBsp mapGen(eng);
//  for(int i = 0; i < 100; i++) {
//    eng->player->pos = Pos(eng->dice.range(1, MAP_X_CELLS - 1),
//                           eng->dice.range(1, MAP_Y_CELLS - 1));
//    mapGen.run();
//  }
//}

int main() {
  return UnitTest::RunAllTests();
}
