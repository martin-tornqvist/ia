#include "Audio.h"

#include <time.h>

#include <SDL_mixer.h>

#include "Engine.h"
#include "Map.h"
#include "Renderer.h"
#include "Utils.h"

Audio::Audio(Engine& engine) :
  curChannel(0), timeAtLastAmb(-1), eng(engine) {

  for(int i = 0; i < endOfSfxId; i++) {
    audioChunks[i] = NULL;
  }

  initAndClearPrev();
}

Audio::~Audio() {
  freeAssets();
}

void Audio::initAndClearPrev() {
  freeAssets();

  if(Config::isAudioEnabled()) {
    //Monster sounds
    loadAudioFile(sfxDogSnarl,                "sfx_dogSnarl.ogg");
    loadAudioFile(sfxWolfHowl,                "sfx_wolfHowl.ogg");
    loadAudioFile(sfxZombieGrowl,             "sfx_zombieGrowl.ogg");
    loadAudioFile(sfxGhoulGrowl,              "sfx_ghoulGrowl.ogg");
    loadAudioFile(sfxOozeGurgle,              "sfx_oozeGurgle.ogg");
    loadAudioFile(sfxFlappingWings,           "sfx_flappingWings.ogg");

    //Weapon and attack sounds
    loadAudioFile(sfxHitSmall,                "sfx_hitSmall.ogg");
    loadAudioFile(sfxHitMedium,               "sfx_hitMedium.ogg");
    loadAudioFile(sfxHitHard,                 "sfx_hitHard.ogg");
    loadAudioFile(sfxHitSharp,                "sfx_hitSharp.ogg");
    loadAudioFile(sfxPistolFire,              "sfx_pistolFire.ogg");
    loadAudioFile(sfxPistolReload,            "sfx_pistolReload.ogg");
    loadAudioFile(sfxShotgunSawedOffFire,     "sfx_shotgunSawedOffFire.ogg");
    loadAudioFile(sfxShotgunPumpFire,         "sfx_shotgunPumpFire.ogg");
    loadAudioFile(sfxShotgunReload,           "sfx_shotgunReload.ogg");
    loadAudioFile(sfxMachineGunFire,          "sfx_machineGunFire.ogg");
    loadAudioFile(sfxMachineGunReload,        "sfx_machineGunReload.ogg");

    //Environment sounds
    loadAudioFile(sfxMetalClank,              "sfx_metalClank.ogg");
    loadAudioFile(sfxRicochet,                "sfx_ricochet.ogg");
    loadAudioFile(sfxExplosion,               "sfx_explosion.ogg");
    loadAudioFile(sfxExplosionMolotov,        "sfx_explosionMolotov.ogg");
    loadAudioFile(sfxDoorOpen,                "sfx_doorOpen.ogg");
    loadAudioFile(sfxDoorClose,               "sfx_doorClose.ogg");
    loadAudioFile(sfxDoorBang,                "sfx_doorBang.ogg");
    loadAudioFile(sfxDoorBreak,               "sfx_doorBreak.ogg");

    //User interface sounds
    loadAudioFile(sfxBackpack,                "sfx_backpack.ogg");
    loadAudioFile(sfxPickup,                  "sfx_pickup.ogg");
    loadAudioFile(sfxElectricLantern,         "sfx_electricLantern.ogg");
    loadAudioFile(sfxPotionQuaff,             "sfx_potionQuaff.ogg");
    loadAudioFile(sfxInsanityRising,          "sfx_insanityRising.ogg");
    loadAudioFile(sfxGlop,                    "sfx_glop.ogg");

    int a = 1;
    for(int i = startOfAmbSfx + 1; i < endOfAmbSfx; i++) {
      const string indexStr = toString(a);
      const string indexStrPadded =
        a < 10  ? "00" + indexStr : a < 100 ? "0"  + indexStr : indexStr;
      loadAudioFile(SfxId(i), "amb_" + indexStrPadded + ".ogg");
      a++;
    }

    loadAudioFile(musCthulhiana_Madness,
                  "Musica_Cthulhiana-Fragment-Madness.ogg");
  }
}

void Audio::loadAudioFile(const SfxId sfx, const string& filename) {
  const string fileRelPath = "audio/" + filename;

  Renderer::clearScreen();
  Renderer::drawText("Loading " + fileRelPath + "...", panel_screen,
                         Pos(1, 1), clrWhite);
  Renderer::updateScreen();

  audioChunks[sfx] = Mix_LoadWAV((fileRelPath).data());

  if(audioChunks[sfx] == NULL) {
    trace << "[WARNING] Problem loading audio file with name " + filename;
    trace << ", in Audio::loadAudio()" << endl;
    trace << "SDL_mixer: " << Mix_GetError() << endl;
  }
}

int Audio::play(const SfxId sfx, const int VOL_PERCENT_TOT,
                const int VOL_PERCENT_L) {
  int ret = -1;
  if(
    Config::isAudioEnabled() &&
    sfx != endOfSfxId && sfx != startOfAmbSfx && sfx != endOfAmbSfx &&
    Config::isBotPlaying() == false) {

    const int VOL_TOT = (255 * VOL_PERCENT_TOT) / 100;
    const int VOL_L   = (VOL_PERCENT_L * VOL_TOT) / 100;
    const int VOL_R   = VOL_TOT - VOL_L;

    Mix_SetPanning(curChannel, VOL_L, VOL_R);

    Mix_PlayChannel(curChannel, audioChunks[sfx], 0);

    ret = curChannel;

    curChannel++;

    if(curChannel >= AUDIO_ALLOCATED_CHANNELS) {
      curChannel = 0;
    }
  }

  return ret;
}

void Audio::playFromDir(const SfxId sfx, const Dir dir,
                        const int DISTANCE_PERCENT) {
  if(dir != endOfDirs) {
    //The distance value is scaled down to avoid too much volume degradation
    const int VOL_PERCENT_TOT = 100 - ((DISTANCE_PERCENT * 2) / 3);

    int volPercentL = 0;
    switch(dir) {
      case dirLeft:       volPercentL = 85;  break;
      case dirUpLeft:     volPercentL = 75;  break;
      case dirDownLeft:   volPercentL = 75;  break;
      case dirUp:         volPercentL = 50;  break;
      case dirCenter:     volPercentL = 50;  break;
      case dirDown:       volPercentL = 50;  break;
      case dirUpRight:    volPercentL = 25;  break;
      case dirDownRight:  volPercentL = 25;  break;
      case dirRight:      volPercentL = 15;  break;
      case endOfDirs:     volPercentL = 50;  break;
    }
    play(sfx, VOL_PERCENT_TOT, volPercentL);
  }
}

void Audio::tryPlayAmb(const int ONE_IN_N_CHANCE_TO_PLAY) {
  if(Rnd::oneIn(ONE_IN_N_CHANCE_TO_PLAY)) {

    const int TIME_NOW = time(0);
    const int TIME_REQ_BETWEEN_AMB_SFX = 35;

    if(TIME_NOW - TIME_REQ_BETWEEN_AMB_SFX > timeAtLastAmb) {
      timeAtLastAmb = TIME_NOW;
      const int VOL_PERCENT = Rnd::oneIn(5) ?
                              Rnd::range(51, 100) :
                              Rnd::range(1, 50);
      play(getAmbSfxSuitableForDlvl(), VOL_PERCENT);
    }
  }
}

SfxId Audio::getAmbSfxSuitableForDlvl() const {
  vector<SfxId> sfxCandidates;
  sfxCandidates.resize(0);

  const int DLVL = eng.map->getDlvl();
  if(DLVL >= 1 && DLVL < LAST_ROOM_AND_CORRIDOR_LEVEL) {
    sfxCandidates.push_back(amb002);
    sfxCandidates.push_back(amb003);
    sfxCandidates.push_back(amb004);
    sfxCandidates.push_back(amb005);
    sfxCandidates.push_back(amb006);
    sfxCandidates.push_back(amb007);
    sfxCandidates.push_back(amb008);
    sfxCandidates.push_back(amb009);
    sfxCandidates.push_back(amb010);
    sfxCandidates.push_back(amb011);
    sfxCandidates.push_back(amb012);
    sfxCandidates.push_back(amb013);
    sfxCandidates.push_back(amb014);
    sfxCandidates.push_back(amb015);
    sfxCandidates.push_back(amb017);
    sfxCandidates.push_back(amb018);
    sfxCandidates.push_back(amb019);
    sfxCandidates.push_back(amb021);
    sfxCandidates.push_back(amb022);
    sfxCandidates.push_back(amb023);
    sfxCandidates.push_back(amb024);
    sfxCandidates.push_back(amb026);
    sfxCandidates.push_back(amb027);
    sfxCandidates.push_back(amb028);
  } else if(DLVL > FIRST_CAVERN_LEVEL) {
    sfxCandidates.push_back(amb001);
    sfxCandidates.push_back(amb002);
    sfxCandidates.push_back(amb003);
    sfxCandidates.push_back(amb004);
    sfxCandidates.push_back(amb005);
    sfxCandidates.push_back(amb006);
    sfxCandidates.push_back(amb007);
    sfxCandidates.push_back(amb010);
    sfxCandidates.push_back(amb011);
    sfxCandidates.push_back(amb012);
    sfxCandidates.push_back(amb013);
    sfxCandidates.push_back(amb016);
    sfxCandidates.push_back(amb017);
    sfxCandidates.push_back(amb019);
    sfxCandidates.push_back(amb020);
    sfxCandidates.push_back(amb024);
    sfxCandidates.push_back(amb025);
    sfxCandidates.push_back(amb026);
    sfxCandidates.push_back(amb028);
    sfxCandidates.push_back(amb029);
    sfxCandidates.push_back(amb030);
  }

  if(sfxCandidates.empty()) {
    return endOfSfxId;
  }

  const int ELEMENT = Rnd::range(0, sfxCandidates.size() - 1);
  return sfxCandidates.at(ELEMENT);
}

void Audio::fadeOutChannel(const int CHANNEL_NR) {
  Mix_FadeOutChannel(CHANNEL_NR, 5000);
}

void Audio::freeAssets() {
  for(int i = 0; i < endOfSfxId; i++) {
    if(audioChunks[i] != NULL) {
      Mix_FreeChunk(audioChunks[i]);
      audioChunks[i] = NULL;
    }
  }
}
