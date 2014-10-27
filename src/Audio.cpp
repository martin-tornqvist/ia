#include "Audio.h"

#include <time.h>

#include <SDL_mixer.h>

#include "Init.h"
#include "Map.h"
#include "Render.h"
#include "Utils.h"

using namespace std;

namespace Audio {

namespace {

vector<Mix_Chunk*> audioChunks;

int curChannel    = 0;
int timeAtLastAmb = -1;

SfxId getAmbSfxSuitableForDlvl() {
  vector<SfxId> sfxBucket;
  sfxBucket.clear();

  if(Map::dlvl >= 1 && Map::dlvl < LAST_ROOM_AND_CORRIDOR_LVL) {
    sfxBucket.push_back(SfxId::amb002);
    sfxBucket.push_back(SfxId::amb003);
    sfxBucket.push_back(SfxId::amb004);
    sfxBucket.push_back(SfxId::amb005);
    sfxBucket.push_back(SfxId::amb006);
    sfxBucket.push_back(SfxId::amb007);
    sfxBucket.push_back(SfxId::amb008);
    sfxBucket.push_back(SfxId::amb009);
    sfxBucket.push_back(SfxId::amb010);
    sfxBucket.push_back(SfxId::amb011);
    sfxBucket.push_back(SfxId::amb012);
    sfxBucket.push_back(SfxId::amb013);
    sfxBucket.push_back(SfxId::amb014);
    sfxBucket.push_back(SfxId::amb015);
    sfxBucket.push_back(SfxId::amb017);
    sfxBucket.push_back(SfxId::amb018);
    sfxBucket.push_back(SfxId::amb019);
    sfxBucket.push_back(SfxId::amb021);
    sfxBucket.push_back(SfxId::amb022);
    sfxBucket.push_back(SfxId::amb023);
    sfxBucket.push_back(SfxId::amb024);
    sfxBucket.push_back(SfxId::amb026);
    sfxBucket.push_back(SfxId::amb027);
    sfxBucket.push_back(SfxId::amb028);
    sfxBucket.push_back(SfxId::amb031);
    sfxBucket.push_back(SfxId::amb033);
    sfxBucket.push_back(SfxId::amb034);
    sfxBucket.push_back(SfxId::amb035);
    sfxBucket.push_back(SfxId::amb036);
    sfxBucket.push_back(SfxId::amb037);
  } else if(Map::dlvl > FIRST_CAVERN_LVL) {
    sfxBucket.push_back(SfxId::amb001);
    sfxBucket.push_back(SfxId::amb002);
    sfxBucket.push_back(SfxId::amb003);
    sfxBucket.push_back(SfxId::amb004);
    sfxBucket.push_back(SfxId::amb005);
    sfxBucket.push_back(SfxId::amb006);
    sfxBucket.push_back(SfxId::amb007);
    sfxBucket.push_back(SfxId::amb010);
    sfxBucket.push_back(SfxId::amb011);
    sfxBucket.push_back(SfxId::amb012);
    sfxBucket.push_back(SfxId::amb013);
    sfxBucket.push_back(SfxId::amb016);
    sfxBucket.push_back(SfxId::amb017);
    sfxBucket.push_back(SfxId::amb019);
    sfxBucket.push_back(SfxId::amb020);
    sfxBucket.push_back(SfxId::amb024);
    sfxBucket.push_back(SfxId::amb025);
    sfxBucket.push_back(SfxId::amb026);
    sfxBucket.push_back(SfxId::amb028);
    sfxBucket.push_back(SfxId::amb029);
    sfxBucket.push_back(SfxId::amb030);
    sfxBucket.push_back(SfxId::amb032);
    sfxBucket.push_back(SfxId::amb033);
    sfxBucket.push_back(SfxId::amb034);
    sfxBucket.push_back(SfxId::amb035);
    sfxBucket.push_back(SfxId::amb037);
  }

  if(sfxBucket.empty()) {return SfxId::END;}

  const int ELEMENT = Rnd::range(0, sfxBucket.size() - 1);
  return sfxBucket.at(ELEMENT);
}

void loadAudioFile(const SfxId sfx, const string& filename) {
  const string fileRelPath = "audio/" + filename;

  Render::clearScreen();
  Render::drawText("Loading " + fileRelPath + "...", Panel::screen,
                   Pos(0), clrWhite);

  Render::updateScreen();

  audioChunks.at(int(sfx)) = Mix_LoadWAV((fileRelPath).data());

  if(!audioChunks.at(int(sfx))) {
    TRACE << "Problem loading audio file with name " + filename << endl
          << Mix_GetError() << endl;
  }
}

} //Namespace

void init() {
  cleanup();

  if(Config::isAudioEnabled()) {

    audioChunks.resize(int(SfxId::END));

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
    loadAudioFile(SfxId::insanityRise,            "sfx_insanityRise.ogg");
    loadAudioFile(SfxId::glop,                    "sfx_glop.ogg");
    loadAudioFile(SfxId::death,                   "sfx_death.ogg");

    int a = 1;
    const int FIRST = int(SfxId::startOfAmbSfx) + 1;
    const int LAST  = int(SfxId::END)   - 1;
    for(int i = FIRST; i <= LAST; ++i) {
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
  for(Mix_Chunk* chunk : audioChunks) {Mix_FreeChunk(chunk);}
  audioChunks.clear();

  curChannel    =  0;
  timeAtLastAmb = -1;
}

int play(const SfxId sfx, const int VOL_PERCENT_TOT,
         const int VOL_PERCENT_L) {
  int ret = -1;

  if(
    !audioChunks.empty()        &&
    sfx != SfxId::endOfAmbSfx   &&
    sfx != SfxId::startOfAmbSfx &&
    sfx != SfxId::END           &&
    !Config::isBotPlaying()) {

    const int VOL_TOT = (255 * VOL_PERCENT_TOT)   / 100;
    const int VOL_L   = (VOL_PERCENT_L * VOL_TOT) / 100;
    const int VOL_R   = VOL_TOT - VOL_L;

    Mix_SetPanning(curChannel, VOL_L, VOL_R);

    Mix_PlayChannel(curChannel, audioChunks.at(int(sfx)), 0);

    ret = curChannel;

    ++curChannel;

    if(curChannel >= AUDIO_ALLOCATED_CHANNELS) {curChannel = 0;}
  }

  return ret;
}

void play(const SfxId sfx, const Dir dir, const int DISTANCE_PERCENT) {
  if(!audioChunks.empty() && dir != Dir::END) {
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
      case Dir::END:        volPercentL = 50;  break;
    }
    play(sfx, VOL_PERCENT_TOT, volPercentL);
  }
}

void tryPlayAmb(const int ONE_IN_N_CHANCE_TO_PLAY) {

  if(!audioChunks.empty() && Rnd::oneIn(ONE_IN_N_CHANCE_TO_PLAY)) {

    const int TIME_NOW                  = time(nullptr);
    const int TIME_REQ_BETWEEN_AMB_SFX  = 20;

    if(TIME_NOW - TIME_REQ_BETWEEN_AMB_SFX > timeAtLastAmb) {
      timeAtLastAmb = TIME_NOW;
      const int VOL_PERCENT = Rnd::oneIn(5) ? Rnd::range(50,  99) : 100;
      play(getAmbSfxSuitableForDlvl(), VOL_PERCENT);
    }
  }
}

void fadeOutChannel(const int CHANNEL_NR) {
  if(!audioChunks.empty()) {Mix_FadeOutChannel(CHANNEL_NR, 5000);}
}

} //Audio
