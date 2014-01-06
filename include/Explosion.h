#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <vector>

#include "CommonData.h"
#include "Properties.h"
#include "Config.h"
#include "Colors.h"
#include "AudioIds.h"

using namespace std;

class Engine;

class ExplosionMaker {
public:
  ExplosionMaker(Engine& engine) :
    w(5), h(5), eng(engine) {
  }

  ExplosionMaker& operator=(const ExplosionMaker& other) {
    (void)other;
    return *this;
  }

  void runExplosion(
    const Pos& origin,
    const Sfx_t sfx = sfxExplosion,
    const bool DO_EXPLOSION_DMG = true,
    Prop* const prop = NULL,
    const bool OVERRIDE_EXPLOSION_RENDERING = false,
    const SDL_Color colorOverride = clrWhite);

  void runSmokeExplosion(const Pos& origin, const bool SMALL_RADIUS = false);

private:

  const int w;
  const int h;

  struct BasicData {
    BasicData(const Pos& origin, const int w, const int h) :
      x0(origin.x - (w - 1) / 2), y0(origin.y - (h - 1) / 2),
      x1(origin.x + (w - 1) / 2), y1(origin.y + (h - 1) / 2) {
    }

    BasicData& operator=(const BasicData& other) {
      (void)other;
      return *this;
    }

    const int x0;
    const int y0;
    const int x1;
    const int y1;
  };

  void renderExplosion(const BasicData* data,
                       bool reach[MAP_W][MAP_H]);

  void renderExplosionWithColorOverride(
    const BasicData* data, const SDL_Color clr,
    bool reach[MAP_W][MAP_H]);

  Engine& eng;
};

#endif
