#include "Audio.h"

#include "SDL/SDL_mixer.h"

#include "Engine.h"

Audio::Audio(Engine* engine) : eng(engine) {
  for(int i = 0; i < endOfSfx; i++) {
    audioChunks[i] = NULL;
  }

  loadAllAudio();
}

Audio::~Audio() {
  freeAssets();
}

void Audio::play(const Sfx_t sfx) {
  Mix_PlayChannel(-1, audioChunks[sfx], 0);
}

void Audio::loadAllAudio() {
  freeAssets();

  loadAudioFile(sfxPistolFire,              "sfx_pistolFire.ogg");
  loadAudioFile(sfxShotgunSawedOffFire,     "sfx_shotgunSawedOffFire.ogg");
  loadAudioFile(sfxShotgunPumpFire,         "sfx_shotgunPumpFire.ogg");

  loadAudioFile(sfxRicochet,                "sfx_ricochet.ogg");

  loadAudioFile(sfxDoorOpen,                "sfx_doorOpen.ogg");
  loadAudioFile(sfxDoorClose,               "sfx_doorClose.ogg");
  loadAudioFile(sfxDoorBang,                "sfx_doorBang.ogg");
  loadAudioFile(sfxDoorBreak,               "sfx_doorBreak.ogg");
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
