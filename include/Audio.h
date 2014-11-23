#ifndef AUDIO_H
#define AUDIO_H

#include "CmnTypes.h"

enum class SfxId
{
  //Monster sounds
  dogSnarl,
  wolfHowl,
  zombieGrowl,
  ghoulGrowl,
  oozeGurgle,
  flappingWings,

  //Weapon and attack sounds
  hitSmall, hitMedium, hitHard,
  missLight, missMedium, missHeavy,
  hitSharp,
  pistolFire,
  pistolReload,
  shotgunSawedOffFire,
  shotgunPumpFire,
  shotgunReload,
  machineGunFire,
  machineGunReload,

  //Environment sounds
  metalClank,
  ricochet,
  explosion,
  explosionMolotov,
  doorOpen, doorClose, doorBang, doorBreak,
  tombOpen,

  //User interface sounds
  backpack,
  pickup,
  electricLantern,
  potionQuaff,
  spellGeneric,
  insanityRise,
  glop,
  death,

  //Ambient sounds
  AMB_START,
  amb001, amb002, amb003, amb004, amb005, amb006, amb007, amb008, amb009, amb010, amb011,
  amb012, amb013, amb014, amb015, amb016, amb017, amb018, amb019, amb020, amb021, amb022,
  amb023, amb024, amb025, amb026, amb027, amb028, amb029, amb030, amb031, amb032, amb033,
  amb034, amb035, amb036, amb037,
  AMB_END,

  musCthulhiana_Madness,

  END
};

namespace Audio
{

void init();
void cleanup();

//Returns channel playing on
int play(const SfxId sfx, const int VOL_PERCENT_TOT = 100,
         const int VOL_PERCENT_L = 50);

void play(const SfxId sfx, const Dir dir, const int DISTANCE_PERCENT);

void tryPlayAmb(const int ONE_IN_N_CHANCE_TO_PLAY);

void fadeOutChannel(const int CHANNEL_NR);

}

#endif
