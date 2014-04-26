#ifndef PLAYER_BONUSES_H
#define PLAYER_BONUSES_H

#include <string>
#include <vector>
#include <algorithm>

#include "AbilityValues.h"
#include "Converters.h"

#include <math.h>

using namespace std;

enum class Trait {
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
  demolitionExpert,
  dexterous,
  fearless,
  imperceptible,
  stealthy,
  mobile,
  lithe,
  warlock,
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
  strongBacked,
  endOfTraits
};

enum class Bg {
  occultist, rogue, warVet, endOfBgs
};

namespace PlayerBon {

extern vector<Trait> traitsPicked_;

void init();

void storeToSaveLines(vector<string>& lines);

void setupFromSaveLines(vector<string>& lines);

void getPickableBgs(vector<Bg>& bgsRef);

void getPickableTraits(vector<Trait>& traitsRef);

void getTraitPrereqs(const Trait id, vector<Trait>& traitsRef, Bg& bgRef);

Bg getBg();

void getTraitTitle(const Trait id, string& strRef);
void getTraitDescr(const Trait id, string& strRef);

void getBgTitle(const Bg id, string& strRef);
//The string vector reference parameter set in this function does not get
//formatted in getBgDescr. Each line still needs to be formatted by the
//calling function. The reason for using a vector reference instead of simply
//a string, is only to specify line breaks.
void getBgDescr(const Bg id, vector<string>& linesRef);

void getAllPickedTraitsTitlesLine(string& strRef);

void pickTrait(const Trait id);

void pickBg(const Bg bg);

void setAllTraitsToPicked();

bool hasTrait(const Trait t);

int getSpiOccultistCanCastAtLvl(const int LVL);

} //PlayerBon

#endif
