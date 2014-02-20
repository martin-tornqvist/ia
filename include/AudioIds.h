#ifndef AUDIO_IDS_H
#define AUDIO_IDS_H

enum class SfxId {
  //Monster sounds
  dogSnarl,
  wolfHowl,
  zombieGrowl,
  ghoulGrowl,
  oozeGurgle,
  flappingWings,

  //Weapon and attack sounds
  hitSmall,
  hitMedium,
  hitHard,
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
  doorOpen,
  doorClose,
  doorBang,
  doorBreak,

  //User interface sounds
  backpack,
  pickup,
  electricLantern,
  potionQuaff,
  insanityRising,
  glop,

  //Ambient sounds
  startOfAmbSfx,
  amb001, amb002, amb003, amb004, amb005, amb006, amb007, amb008, amb009,
  amb010, amb011, amb012, amb013, amb014, amb015, amb016, amb017, amb018,
  amb019, amb020, amb021, amb022, amb023, amb024, amb025, amb026, amb027,
  amb028, amb029, amb030,
  endOfAmbSfx,

  musCthulhiana_Madness,

  endOfSfxId
};

#endif
