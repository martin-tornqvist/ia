#include "UnitTest++.h"

#include "Engine.h"
#include "BasicUtils.h"
#include "Renderer.h"

TEST(Dice) {
  Dice dice;
  int rnd = 0;
  for(int i = 0; i < 10000; i++) {
    //Check that Dice can generate numbers within given ranges.
    // - Test both with raw numbers and with Range parameter.
    // - Test both positive and negative values.
    rnd = dice.range(1000, 2000);         CHECK(rnd >= 1000 && rnd <= 2000);
    rnd = dice.range(Range(1000, 2000));  CHECK(rnd >= 1000 && rnd <= 2000);
    rnd = dice.range(-100, -50);          CHECK(rnd >= -100 && rnd <= -50);
    rnd = dice.range(Range(-100, -50));   CHECK(rnd >= -100 && rnd <= -50);
    rnd = dice.range(-100, 2000);         CHECK(rnd >= -100 && rnd <= 2000);
    rnd = dice.range(Range(-100, 2000));  CHECK(rnd >= -100 && rnd <= 2000);

    //Check that Dice can generate a percentage value
    rnd = dice.percentile();              CHECK(rnd >= 1 && rnd <= 100);
  }
}

TEST(DistanceCalculations) {
  Engine eng;
  eng.initConfig();
  eng.initRenderer();
  eng.initAudio();
  eng.initGame();
  eng.cleanupGame();
  eng.cleanupAudio();
  eng.cleanupRenderer();
  eng.cleanupConfig();
}

int main() {
  return UnitTest::RunAllTests();
}
