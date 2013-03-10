#include "ItemDevice.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Render.h"

//---------------------------------------------------- BASE CLASS
bool Device::toggle(Engine* const engine) {
  if(isActivated_) {
    isActivated_ = false;
    nrTurnsToNextEffect_ = -1;
  } else {
    isActivated_ = true;
    nrTurnsToNextEffect_ = getRandomNrTurnsToNextEffect(engine);
  }
  return false;
}

int Device::getRandomNrTurnsToNextEffect(Engine* const engine) const {
  return engine->dice.getInRange(6, 10);
}

void Device::newTurn(Engine* const engine) {
  if(isActivated_) {
    if(--nrTurnsToNextEffect_ <= 0) {
      runEffect(engine);
      nrTurnsToNextEffect_ = getRandomNrTurnsToNextEffect(engine);
    }
  }
}

void Device::runEffect(Engine* const engine) {
  tracer << "Device::runEffect()..." << endl;
  specificEffect(engine);
  tracer << "Device::runEffect() [DONE]" << endl;
}

//---------------------------------------------------- SENTRY DEVICE
void DeviceSentry::specificEffect(Engine* const engine) {
  const int DMG = engine->dice(1, 6) + 2;

  engine->player->getSpotedEnemies();
  vector<Actor*>& targetCandidates = engine->player->spotedEnemies;
  const unsigned int NR_TARGET_CANDIDATES = targetCandidates.size();
  if(NR_TARGET_CANDIDATES > 0) {
    Actor* const actor = targetCandidates.at(engine->dice.getInRange(0, NR_TARGET_CANDIDATES - 1));
    const coord& pos = actor->pos;
    engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(vector<coord>(1, pos), clrYellow, 1, engine);
    actor->hit(DMG, damageType_electric);
  }
}
