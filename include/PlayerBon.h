#ifndef PLAYER_BONUSES_H
#define PLAYER_BONUSES_H

#include <string>
#include <vector>
#include <algorithm>

#include "AbilityValues.h"
#include "Converters.h"

#include <math.h>

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
  strongBacked,
  END
};

enum class Bg {occultist, rogue, warVet, END};

namespace PlayerBon
{

extern std::vector<Trait> traitsPicked_;

void init();

void storeToSaveLines(std::vector<std::string>& lines);

void setupFromSaveLines(std::vector<std::string>& lines);

void getPickableBgs(std::vector<Bg>& bgsRef);

void getPickableTraits(std::vector<Trait>& traitsRef);

void getTraitPrereqs(const Trait id, std::vector<Trait>& traitsRef, Bg& bgRef);

Bg getBg();

void getTraitTitle(const Trait id, std::string& strRef);
void getTraitDescr(const Trait id, std::string& strRef);

void getBgTitle(const Bg id, std::string& strRef);
//The std::string std::vector reference parameter set in this function does not get
//formatted in getBgDescr. Each line still needs to be formatted by the
//calling function. The reason for using a std::vector reference instead of simply
//a std::string, is only to specify line breaks.
void getBgDescr(const Bg id, std::vector<std::string>& linesRef);

void getAllPickedTraitsTitlesLine(std::string& strRef);

void pickTrait(const Trait id);

void pickBg(const Bg bg);

void setAllTraitsToPicked();

bool hasTrait(const Trait t);

int getSpiOccultistCanCastAtLvl(const int LVL);

} //PlayerBon

#endif
