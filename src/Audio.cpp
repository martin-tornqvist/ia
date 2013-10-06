#include "Audio.h"

#include "SDL/SDL_mixer.h"

#include "Engine.h"

Audio::Audio(Engine* engine) : curChannel(0), eng(engine) {
  for(int i = 0; i < endOfSfx; i++) {
    audioChunks[i] = NULL;
  }

  loadAllAudio();
}

Audio::~Audio() {
  freeAssets();
}

void Audio::play(const Sfx_t sfx) {
  play(sfx, directionCenter, 0);
}

void Audio::play(const Sfx_t sfx, const Direction_t direction,
                 const int DISTANCE_PERCENT) {
  if(sfx != endOfSfx && direction != endOfDirections) {
    //The distance value is scaled down to avoid too much volume degradation
    const int VOL_PERCENT_TOT = 100 - ((DISTANCE_PERCENT * 2) / 3);
    const int VOL_TOT         = (255 * VOL_PERCENT_TOT) / 100;

    int volPercentOfTotL = 0;
    switch(direction) {
      case directionLeft:       volPercentOfTotL = 85;  break;
      case directionUpLeft:     volPercentOfTotL = 75;  break;
      case directionDownLeft:   volPercentOfTotL = 75;  break;
      case directionUp:         volPercentOfTotL = 50;  break;
      case directionCenter:     volPercentOfTotL = 50;  break;
      case directionDown:       volPercentOfTotL = 50;  break;
      case directionUpRight:    volPercentOfTotL = 25;  break;
      case directionDownRight:  volPercentOfTotL = 25;  break;
      case directionRight:      volPercentOfTotL = 15;  break;
      case endOfDirections:     volPercentOfTotL = 50;  break;
    }

    const int VOL_L = (volPercentOfTotL * VOL_TOT) / 100;
    const int VOL_R = VOL_TOT - VOL_L;

    Mix_SetPanning(curChannel, VOL_L, VOL_R);

    Mix_PlayChannel(curChannel, audioChunks[sfx], 0);

    curChannel++;

    if(curChannel >= AUDIO_ALLOCATED_CHANNELS) {
      curChannel = 0;
    }
  }
}

void Audio::loadAllAudio() {
  freeAssets();

  loadAudioFile(sfxDogSnarl,                "sfx_dogSnarl.ogg");
  loadAudioFile(sfxWolfHowl,                "sfx_wolfHowl.ogg");
  loadAudioFile(sfxZombieGrowl,             "sfx_zombieGrowl.ogg");
  loadAudioFile(sfxGhoulGrowl,              "sfx_ghoulGrowl.ogg");

  loadAudioFile(sfxPistolFire,              "sfx_pistolFire.ogg");
  loadAudioFile(sfxPistolReload,            "sfx_pistolReload.ogg");
  loadAudioFile(sfxShotgunSawedOffFire,     "sfx_shotgunSawedOffFire.ogg");
  loadAudioFile(sfxShotgunPumpFire,         "sfx_shotgunPumpFire.ogg");

  loadAudioFile(sfxRicochet,                "sfx_ricochet.ogg");

  loadAudioFile(sfxDoorOpen,                "sfx_doorOpen.ogg");
  loadAudioFile(sfxDoorClose,               "sfx_doorClose.ogg");
  loadAudioFile(sfxDoorBang,                "sfx_doorBang.ogg");
  loadAudioFile(sfxDoorBreak,               "sfx_doorBreak.ogg");

  loadAudioFile(sfxExplosionMolotov,        "sfx_explosionMolotov.ogg");
}

void Audio::loadAudioFile(const Sfx_t sfx, const string& filename) {
  audioChunks[sfx] = Mix_LoadWAV(("audio/" + filename).data());

  if(audioChunks[sfx] == NULL) {
    trace << "[WARNING] Problem loading audio file with name " + filename;
    trace << ", in Audio::loadAudio()" << endl;
    trace << "SDL_mixer: " << Mix_GetError() << endl;
  }
}

void Audio::freeAssets() {
  for(int i = 0; i < endOfSfx; i++) {
    if(audioChunks[i] != NULL) {
      Mix_FreeChunk(audioChunks[i]);
      audioChunks[i] = NULL;
    }
  }
}
