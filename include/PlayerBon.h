#ifndef PLAYER_BON_H
#define PLAYER_BON_H

#include <string>
#include <vector>
#include <algorithm>

#include "AbilityValues.h"
#include "Converters.h"

#include <math.h>

struct ActorDataT;

enum class Trait
{
    adeptMeleeFighter,
    expertMeleeFighter,
    masterMeleeFighter,
    adeptMarksman,
    expertMarksman,
    masterMarksman,
    steadyAimer,
    sharpShooter,
    coolHeaded,
    courageous,
    demExpert,
    dexterous,
    fearless,
    imperceptible,
    vicious,
    stealthy,
    mobile,
    lithe,
    warlock,
    summoner,
    bloodSorcerer,
    seer,
    observant,
    perceptive,
    vigilant,
    treasureHunter,
    selfAware,
    healer,
    rapidRecoverer,
    survivalist,
    perseverant,
    stoutSpirit,
    strongSpirit,
    mightySpirit,
    tough,
    rugged,
    unbreakable,
    strongBacked,
    undeadBane,
    END
};

enum class Bg {occultist, rogue, warVet, END};

namespace PlayerBon
{

extern bool traits[int(Trait::END)];

void init();

void storeToSaveLines(std::vector<std::string>& lines);

void setupFromSaveLines(std::vector<std::string>& lines);

void getPickableBgs(std::vector<Bg>& bgsRef);

void getPickableTraits(std::vector<Trait>& traitsRef);

void getTraitPrereqs(const Trait id, std::vector<Trait>& traitsRef, Bg& bgRef);

Bg getBg();

void getTraitTitle(const Trait id, std::string& out);
void getTraitDescr(const Trait id, std::string& out);

void getBgTitle(const Bg id, std::string& out);

//Note: The string vector reference parameter set in this function is not formatted in
//getBgDescr. Each line still needs to be formatted by the calling function. The reason
//for using a vector reference instead of simply a string is only to specify line breaks.
void getBgDescr(const Bg id, std::vector<std::string>& linesRef);

void getAllPickedTraitsTitlesLine(std::string& out);

void pickTrait(const Trait id);

void pickBg(const Bg bg);

void setAllTraitsToPicked();

int getSpiOccultistCanCastAtLvl(const int LVL);

bool getsUndeadBaneBon(const Actor& attacker, const ActorDataT& actorData);

} //PlayerBon

#endif
