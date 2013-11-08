#include "UnitTest++.h"

#include "Engine.h"
#include "BasicUtils.h"
#include "Renderer.h"
#include "Map.h"

struct BasicFixture {
  BasicFixture() {

    eng.initConfig();
    eng.initRenderer();
    eng.initAudio();
    eng.initGame();
    eng.map->clearMap();
  }
  ~BasicFixture() {
    eng.cleanupGame();
    eng.cleanupAudio();
    eng.cleanupRenderer();
    eng.cleanupConfig();
  }
  Engine eng;
};

//TEST_FIXTURE(BasicFixture, DistanceCalculations) {
//  CHECK_EQUAL(eng.basicUtils->chebyshevDist(Pos(1, 2), Pos(2, 3)), 1);
//  CHECK_EQUAL(eng.basicUtils->chebyshevDist(Pos(1, 2), Pos(2, 4)), 2);
//  CHECK_EQUAL(eng.basicUtils->chebyshevDist(Pos(1, 2), Pos(1, 2)), 0);
//  CHECK_EQUAL(eng.basicUtils->chebyshevDist(Pos(10, 3), Pos(1, 4)), 9);
//}

TEST(DistanceCalculations) {
  Engine eng;
  BasicUtils utils(&eng);
  CHECK_EQUAL(utils.chebyshevDist(Pos(1, 2), Pos(2, 3)), 1);
  CHECK_EQUAL(utils.chebyshevDist(Pos(1, 2), Pos(2, 4)), 2);
  CHECK_EQUAL(utils.chebyshevDist(Pos(1, 2), Pos(1, 2)), 0);
  CHECK_EQUAL(utils.chebyshevDist(Pos(10, 3), Pos(1, 4)), 9);
}

int main() {
  return UnitTest::RunAllTests();
}
