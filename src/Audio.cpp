#include "Audio.h"

#include <time.h>

#include <SDL_mixer.h>

#include "Engine.h"
#include "Map.h"
#include "Renderer.h"
#include "Utils.h"

namespace Audio {

//------------------------------------------------------------------------LOCAL
namespace {

vector<Mix_Chunk*> audioChunks;

int curChannel    = 0;
int timeAtLastAmb = -1;

SfxId getAmbSfxSuitableForDlvl(Engine& eng) {
  vector<SfxId> sfxCandidates;
  sfxCandidates.resize(0);

  const int DLVL = eng.map->getDlvl();
  if(DLVL >= 1 && DLVL < LAST_ROOM_AND_CORRIDOR_LEVEL) {
    sfxCandidates.push_back(SfxId::amb002);
    sfxCandidates.push_back(SfxId::amb003);
    sfxCandidates.push_back(SfxId::amb004);
    sfxCandidates.push_back(SfxId::amb005);
    sfxCandidates.push_back(SfxId::amb006);
    sfxCandidates.push_back(SfxId::amb007);
    sfxCandidates.push_back(SfxId::amb008);
    sfxCandidates.push_back(SfxId::amb009);
    sfxCandidates.push_back(SfxId::amb010);
    sfxCandidates.push_back(SfxId::amb011);
    sfxCandidates.push_back(SfxId::amb012);
    sfxCandidates.push_back(SfxId::amb013);
    sfxCandidates.push_back(SfxId::amb014);
    sfxCandidates.push_back(SfxId::amb015);
    sfxCandidates.push_back(SfxId::amb017);
    sfxCandidates.push_back(SfxId::amb018);
    sfxCandidates.push_back(SfxId::amb019);
    sfxCandidates.push_back(SfxId::amb021);
    sfxCandidates.push_back(SfxId::amb022);
    sfxCandidates.push_back(SfxId::amb023);
    sfxCandidates.push_back(SfxId::amb024);
    sfxCandidates.push_back(SfxId::amb026);
    sfxCandidates.push_back(SfxId::amb027);
    sfxCandidates.push_back(SfxId::amb028);
    sfxCandidates.push_back(SfxId::amb031);
    sfxCandidates.push_back(SfxId::amb033);
    sfxCandidates.push_back(SfxId::amb034);
    sfxCandidates.push_back(SfxId::amb035);
    sfxCandidates.push_back(SfxId::amb036);
    sfxCandidates.push_back(SfxId::amb037);
  } else if(DLVL > FIRST_CAVERN_LEVEL) {
    sfxCandidates.push_back(SfxId::amb001);
    sfxCandidates.push_back(SfxId::amb002);
    sfxCandidates.push_back(SfxId::amb003);
    sfxCandidates.push_back(SfxId::amb004);
    sfxCandidates.push_back(SfxId::amb005);
    sfxCandidates.push_back(SfxId::amb006);
    sfxCandidates.push_back(SfxId::amb007);
    sfxCandidates.push_back(SfxId::amb010);
    sfxCandidates.push_back(SfxId::amb011);
    sfxCandidates.push_back(SfxId::amb012);
    sfxCandidates.push_back(SfxId::amb013);
    sfxCandidates.push_back(SfxId::amb016);
    sfxCandidates.push_back(SfxId::amb017);
    sfxCandidates.push_back(SfxId::amb019);
    sfxCandidates.push_back(SfxId::amb020);
    sfxCandidates.push_back(SfxId::amb024);
    sfxCandidates.push_back(SfxId::amb025);
    sfxCandidates.push_back(SfxId::amb026);
    sfxCandidates.push_back(SfxId::amb028);
    sfxCandidates.push_back(SfxId::amb029);
    sfxCandidates.push_back(SfxId::amb030);
    sfxCandidates.push_back(SfxId::amb032);
    sfxCandidates.push_back(SfxId::amb033);
    sfxCandidates.push_back(SfxId::amb034);
    sfxCandidates.push_back(SfxId::amb035);
    sfxCandidates.push_back(SfxId::amb037);
  }

  if(sfxCandidates.empty()) {
    return SfxId::endOfSfxId;
  }

  const int ELEMENT = Rnd::range(0, sfxCandidates.size() - 1);
  return sfxCandidates.at(ELEMENT);
}

void loadAudioFile(const SfxId sfx, const string& filename) {
  const string fileRelPath = "audio/" + filename;

  Renderer::clearScreen();
  Renderer::drawText("Loading " + fileRelPath + "...", Panel::screen,
                     Pos(0, 0), clrWhite);

  Renderer::updateScreen();

  audioChunks.at(int(sfx)) = Mix_LoadWAV((fileRelPath).data());

  if(audioChunks.at(int(sfx)) == NULL) {
    trace << "[WARNING] Problem loading audio file with name " + filename;
    trace << ", in Audio::loadAudio()" << endl;
    trace << "SDL_mixer: " << Mix_GetError() << endl;
  }
}

} //Namespace

//---------------------------------------------------------------------- GLOBAL
void init() {
  cleanup();

  if(Config::isAudioEnabled()) {

    audioChunks.resize(int(SfxId::endOfSfxId));

    //Monster sounds
    loadAudioFile(SfxId::dogSnarl,                "sfx_dogSnarl.ogg");
    loadAudioFile(SfxId::wolfHowl,                "sfx_wolfHowl.ogg");
    loadAudioFile(SfxId::zombieGrowl,             "sfx_zombieGrowl.ogg");
    loadAudioFile(SfxId::ghoulGrowl,              "sfx_ghoulGrowl.ogg");
    loadAudioFile(SfxId::oozeGurgle,              "sfx_oozeGurgle.ogg");
    loadAudioFile(SfxId::flappingWings,           "sfx_flappingWings.ogg");

    //Weapon and attack sounds
    loadAudioFile(SfxId::hitSmall,                "sfx_hitSmall.ogg");
    loadAudioFile(SfxId::hitMedium,               "sfx_hitMedium.ogg");
    loadAudioFile(SfxId::hitHard,                 "sfx_hitHard.ogg");
    loadAudioFile(SfxId::missLight,               "sfx_missLight.ogg");
    loadAudioFile(SfxId::missMedium,              "sfx_missMedium.ogg");
    loadAudioFile(SfxId::missHeavy,               "sfx_missHeavy.ogg");
    loadAudioFile(SfxId::hitSharp,                "sfx_hitSharp.ogg");
    loadAudioFile(SfxId::pistolFire,              "sfx_pistolFire.ogg");
    loadAudioFile(SfxId::pistolReload,            "sfx_pistolReload.ogg");
    loadAudioFile(SfxId::shotgunSawedOffFire,     "sfx_shotgunSawedOffFire.ogg");
    loadAudioFile(SfxId::shotgunPumpFire,         "sfx_shotgunPumpFire.ogg");
    loadAudioFile(SfxId::shotgunReload,           "sfx_shotgunReload.ogg");
    loadAudioFile(SfxId::machineGunFire,          "sfx_machineGunFire.ogg");
    loadAudioFile(SfxId::machineGunReload,        "sfx_machineGunReload.ogg");

    //Environment sounds
    loadAudioFile(SfxId::metalClank,              "sfx_metalClank.ogg");
    loadAudioFile(SfxId::ricochet,                "sfx_ricochet.ogg");
    loadAudioFile(SfxId::explosion,               "sfx_explosion.ogg");
    loadAudioFile(SfxId::explosionMolotov,        "sfx_explosionMolotov.ogg");
    loadAudioFile(SfxId::doorOpen,                "sfx_doorOpen.ogg");
    loadAudioFile(SfxId::doorClose,               "sfx_doorClose.ogg");
    loadAudioFile(SfxId::doorBang,                "sfx_doorBang.ogg");
    loadAudioFile(SfxId::doorBreak,               "sfx_doorBreak.ogg");
    loadAudioFile(SfxId::tombOpen,                "sfx_tombOpen.ogg");

    //User interface sounds
    loadAudioFile(SfxId::backpack,                "sfx_backpack.ogg");
    loadAudioFile(SfxId::pickup,                  "sfx_pickup.ogg");
    loadAudioFile(SfxId::electricLantern,         "sfx_electricLantern.ogg");
    loadAudioFile(SfxId::potionQuaff,             "sfx_potionQuaff.ogg");
    loadAudioFile(SfxId::spellGeneric,            "sfx_spellGeneric.ogg");
    loadAudioFile(SfxId::insanityRising,          "sfx_insanityRising.ogg");
    loadAudioFile(SfxId::glop,                    "sfx_glop.ogg");
    loadAudioFile(SfxId::death,                   "sfx_death.ogg");

    int a = 1;
    const int FIRST = int(SfxId::startOfAmbSfx) + 1;
    const int LAST  = int(SfxId::endOfAmbSfx) - 1;
    for(int i = FIRST; i <= LAST; i++) {
      const string indexStr = toStr(a);
      const string indexStrPadded =
        a < 10  ? "00" + indexStr : a < 100 ? "0"  + indexStr : indexStr;
      loadAudioFile(SfxId(i), "amb_" + indexStrPadded + ".ogg");
      a++;
    }

    loadAudioFile(SfxId::musCthulhiana_Madness,
                  "Musica_Cthulhiana-Fragment-Madness.ogg");
  }
}

void cleanup() {
  for(Mix_Chunk * chunk : audioChunks) {Mix_FreeChunk(chunk);}
  audioChunks.resize(0);

  curChannel    = 0;
  timeAtLastAmb = -1;
}

int play(const SfxId sfx, const int VOL_PERCENT_TOT,
         const int VOL_PERCENT_L) {
  int ret = -1;

  if(
    audioChunks.empty() == false  &&
    sfx != SfxId::endOfSfxId      &&
    sfx != SfxId::startOfAmbSfx   &&
    sfx != SfxId::endOfAmbSfx     &&
    Config::isBotPlaying() == false) {

    const int VOL_TOT = (255 * VOL_PERCENT_TOT) / 100;
    const int VOL_L   = (VOL_PERCENT_L * VOL_TOT) / 100;
    const int VOL_R   = VOL_TOT - VOL_L;

    Mix_SetPanning(curChannel, VOL_L, VOL_R);

    Mix_PlayChannel(curChannel, audioChunks.at(int(sfx)), 0);

    ret = curChannel;

    curChannel++;

    if(curChannel >= AUDIO_ALLOCATED_CHANNELS) {
      curChannel = 0;
    }
  }

  return ret;
}

void play(const SfxId sfx, const Dir dir, const int DISTANCE_PERCENT) {
  if(audioChunks.empty() == false && dir != Dir::endOfDirs) {
    //The distance value is scaled down to avoid too much volume degradation
    const int VOL_PERCENT_TOT = 100 - ((DISTANCE_PERCENT * 2) / 3);

    int volPercentL = 0;
    switch(dir) {
      case Dir::left:       volPercentL = 85;  break;
      case Dir::upLeft:     volPercentL = 75;  break;
      case Dir::downLeft:   volPercentL = 75;  break;
      case Dir::up:         volPercentL = 50;  break;
      case Dir::center:     volPercentL = 50;  break;
      case Dir::down:       volPercentL = 50;  break;
      case Dir::upRight:    volPercentL = 25;  break;
      case Dir::downRight:  volPercentL = 25;  break;
      case Dir::right:      volPercentL = 15;  break;
      case Dir::endOfDirs:  volPercentL = 50;  break;
    }
    play(sfx, VOL_PERCENT_TOT, volPercentL);
  }
}

void tryPlayAmb(const int ONE_IN_N_CHANCE_TO_PLAY, Engine& eng) {
  if(audioChunks.empty() == false && Rnd::oneIn(ONE_IN_N_CHANCE_TO_PLAY)) {

    const int TIME_NOW = time(0);
    const int TIME_REQ_BETWEEN_AMB_SFX = 20;

    if(TIME_NOW - TIME_REQ_BETWEEN_AMB_SFX > timeAtLastAmb) {
      timeAtLastAmb = TIME_NOW;
      const int VOL_PERCENT = Rnd::oneIn(5) ? Rnd::range(1,  99) : 100;
      play(getAmbSfxSuitableForDlvl(eng), VOL_PERCENT);
    }
  }
}

void fadeOutChannel(const int CHANNEL_NR) {
  if(audioChunks.empty() == false) {Mix_FadeOutChannel(CHANNEL_NR, 5000);}
}

} //Audio
