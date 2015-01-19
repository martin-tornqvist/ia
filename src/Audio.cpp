#include "Audio.h"

#include <time.h>

#include <SDL_mixer.h>

#include "Init.h"
#include "Map.h"
#include "Render.h"
#include "Utils.h"

using namespace std;

namespace Audio
{

namespace
{

vector<Mix_Chunk*> audioChunks;

int curChannel    = 0;
int timeAtLastAmb = -1;

void loadAudioFile(const SfxId sfx, const string& filename)
{
    Render::clearScreen();

    const string fileRelPath  = "audio/" + filename;

    Render::drawText("Loading " + fileRelPath + "...", Panel::screen, Pos(0, 0), clrWhite);

    Render::updateScreen();

    audioChunks[int(sfx)] = Mix_LoadWAV((fileRelPath).data());

    if (!audioChunks[int(sfx)])
    {
        TRACE << "Problem loading audio file with name: " << filename       << endl
              << "Mix_GetError(): "                       << Mix_GetError() << endl;
        assert(false);
    }
}

} //Namespace

void init()
{
    cleanup();

    if (Config::isAudioEnabled())
    {
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
        loadAudioFile(SfxId::hitCorpseBreak,          "sfx_hitCorpseBreak.ogg");
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
        loadAudioFile(SfxId::migoGunFire,             "sfx_migoGun.ogg");
        loadAudioFile(SfxId::spikeGun,                "sfx_spikeGun.ogg");

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
        loadAudioFile(SfxId::fountainDrink,           "sfx_fountainDrink.ogg");
        loadAudioFile(SfxId::bossVoice1,              "sfx_bossVoice1.ogg");
        loadAudioFile(SfxId::bossVoice2,              "sfx_bossVoice2.ogg");

        //User interface sounds
        loadAudioFile(SfxId::backpack,                "sfx_backpack.ogg");
        loadAudioFile(SfxId::pickup,                  "sfx_pickup.ogg");
        loadAudioFile(SfxId::electricLantern,         "sfx_electricLantern.ogg");
        loadAudioFile(SfxId::potionQuaff,             "sfx_potionQuaff.ogg");
        loadAudioFile(SfxId::spellGeneric,            "sfx_spellGeneric.ogg");
        loadAudioFile(SfxId::insanityRise,            "sfx_insanityRising.ogg");
        loadAudioFile(SfxId::glop,                    "sfx_glop.ogg");
        loadAudioFile(SfxId::death,                   "sfx_death.ogg");

        int a = 1;
        const int FIRST = int(SfxId::AMB_START) + 1;
        const int LAST  = int(SfxId::AMB_END)   - 1;
        for (int i = FIRST; i <= LAST; ++i)
        {
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

void cleanup()
{
    for (Mix_Chunk* chunk : audioChunks) {Mix_FreeChunk(chunk);}
    audioChunks.clear();

    curChannel    =  0;
    timeAtLastAmb = -1;
}

int play(const SfxId sfx, const int VOL_PERCENT_TOT,
         const int VOL_PERCENT_L)
{
    int ret = -1;

    if (
        !audioChunks.empty()    &&
        sfx != SfxId::AMB_START &&
        sfx != SfxId::AMB_END   &&
        sfx != SfxId::END       &&
        !Config::isBotPlaying())
    {
        const int VOL_TOT = (255 * VOL_PERCENT_TOT)   / 100;
        const int VOL_L   = (VOL_PERCENT_L * VOL_TOT) / 100;
        const int VOL_R   = VOL_TOT - VOL_L;

        Mix_SetPanning(curChannel, VOL_L, VOL_R);

        Mix_PlayChannel(curChannel, audioChunks[int(sfx)], 0);

        ret = curChannel;

        ++curChannel;

        if (curChannel >= AUDIO_ALLOCATED_CHANNELS) {curChannel = 0;}
    }
    return ret;
}

void play(const SfxId sfx, const Dir dir, const int DISTANCE_PERCENT)
{
    if (!audioChunks.empty() && dir != Dir::END)
    {
        //The distance value is scaled down to avoid too much volume degradation
        const int VOL_PERCENT_TOT = 100 - ((DISTANCE_PERCENT * 2) / 3);

        int volPercentL = 0;
        switch (dir)
        {
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

void tryPlayAmb(const int ONE_IN_N_CHANCE_TO_PLAY)
{

    if (!audioChunks.empty() && Rnd::oneIn(ONE_IN_N_CHANCE_TO_PLAY))
    {

        const int TIME_NOW                  = time(nullptr);
        const int TIME_REQ_BETWEEN_AMB_SFX  = 20;

        if ((TIME_NOW - TIME_REQ_BETWEEN_AMB_SFX) > timeAtLastAmb)
        {
            timeAtLastAmb           = TIME_NOW;
            const int   VOL_PERCENT = Rnd::oneIn(5) ? Rnd::range(50,  99) : 100;
            const int   FIRST_INT   = int(SfxId::AMB_START) + 1;
            const int   LAST_INT    = int(SfxId::AMB_END)   - 1;
            const SfxId sfx         = SfxId(Rnd::range(FIRST_INT, LAST_INT));
            play(sfx , VOL_PERCENT);
        }
    }
}

void fadeOutChannel(const int CHANNEL_NR)
{
    if (!audioChunks.empty()) {Mix_FadeOutChannel(CHANNEL_NR, 5000);}
}

} //Audio
