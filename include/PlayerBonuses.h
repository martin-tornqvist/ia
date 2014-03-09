#ifndef PLAYER_BONUSES_H
#define PLAYER_BONUSES_H

#include <string>
#include <vector>
#include <algorithm>

#include "AbilityValues.h"
#include "Converters.h"
#include "Engine.h"

#include <math.h>

using namespace std;

enum TraitId {
  traitAdeptMeleeFighter,
  traitExpertMeleeFighter,
  traitMasterMeleeFighter,
  traitAdeptMarksman,
  traitExpertMarksman,
  traitMasterMarksman,
  traitSteadyAimer,
  traitSharpShooter,
//  traitBreachExpert,
  traitCoolHeaded,
  traitCourageous,
  traitDemolitionExpert,
  traitDexterous,
  traitFearless,
  traitImperceptible,
  traitStealthy,
  traitMobile,
  traitLithe,
  traitWarlock,
  traitBloodSorcerer,
//  traitSpiritCannibal,
  traitSeer,
  traitObservant,
  traitPerceptive,
  traitVigilant,
  traitTreasureHunter,
  traitSelfAware,
  traitHealer,
  traitRapidRecoverer,
  traitSurvivalist,
  traitPotentSpirit,
  traitStrongSpirit,
  traitMightySpirit,
  traitTough,
  traitRugged,
  traitStrongBacked,
  endOfTraits
};

enum Bg {
  bgOccultist, bgRogue, bgWarVet, endOfBgs
};

class PlayerBonHandler {
public:
  PlayerBonHandler(Engine& engine);

  void addSaveLines(vector<string>& lines) {
    lines.push_back(toString(bg_));
    lines.push_back(toString(traitsPicked_.size()));
    for(TraitId t : traitsPicked_) {lines.push_back(toString(t));}
  }

  void setParamsFromSaveLines(vector<string>& lines) {
    bg_ = Bg(toInt(lines.front()));
    lines.erase(lines.begin());

    const int NR_TRAITS = toInt(lines.front());
    lines.erase(lines.begin());

    for(int i = 0; i < NR_TRAITS; i++) {
      traitsPicked_.push_back(TraitId(toInt(lines.front())));
      lines.erase(lines.begin());
    }
  }

  void getPickableBgs(vector<Bg>& bgsRef) const;
  void getPickableTraits(vector<TraitId>& traitsRef) const;

  void getTraitPrereqs(const TraitId id,
                       vector<TraitId>& traitsRef,
                       Bg& bgRef) const;

  inline Bg getBg() const {return bg_;}

  void getTraitTitle(const TraitId id, string& strRef) const;
  void getTraitDescr(const TraitId id, string& strRef) const;

  void getBgTitle(const Bg id, string& strRef) const;
  //The string vector reference parameter set in this function does not get
  //formatted in getBgDescr. Each line still needs to be formatted by the
  //calling function. The reason for using a vector reference instead of simply
  //a string, is only to specify line breaks.
  void getBgDescr(const Bg id, vector<string>& linesRef) const;

  void getAllPickedTraitsTitlesLine(string& strRef) const;

  void pickTrait(const TraitId id);

  void pickBg(const Bg bg);

  void setAllTraitsToPicked() {
    for(int i = 0; i < endOfTraits; i++) {traitsPicked_.push_back(TraitId(i));}
  }

  inline bool hasTrait(const TraitId t) const {
    return find(traitsPicked_.begin(), traitsPicked_.end(), t)
           != traitsPicked_.end();
  }

  vector<TraitId> traitsPicked_;

private:
  Bg bg_;

  Engine& eng;
};

#endif
