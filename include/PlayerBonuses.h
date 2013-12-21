#ifndef PLAYER_BONUSES_H
#define PLAYER_BONUSES_H

#include <string>
#include <vector>

#include "AbilityValues.h"
#include "Converters.h"
#include "Engine.h"

#include <math.h>

using namespace std;

enum Trait_t {
  traitAdeptMeleeCombatant,
  traitExpertMeleeCombatant,
  traitMasterMeleeCombatant,
  traitAdeptMarksman,
  traitExpertMarksman,
  traitMasterMarksman,
  traitSteadyAimer,
  traitSniper,
  traitBreachExpert,
  traitCoolHeaded,
  traitCourageous,
  traitDemolitionExpert,
  traitDexterous,
  traitFearless,
  traitImperceptible,
  traitStealthy,
  traitMobile,
  traitLithe,
  traitMythologist,
  traitObservant,
  traitVigilant,
  traitTreasureHunter,
  traitSelfAware,
  traitSelfPossessed,
  traitHealer,
  traitRapidRecoverer,
  traitSurvivalist,
  traitSpirited,
  traitTough,
  traitRugged,
  traitStrongBacked,
  endOfTraits
};

enum Bg_t {
  bgOccultist, bgRogue, bgSoldier, endOfBgs
};

class PlayerBonHandler {
public:
  PlayerBonHandler(Engine& engine);

  void addSaveLines(vector<string>& lines) {
    lines.push_back(toString(bg_));

    for(int i = 0; i < endOfTraits; i++) {
      lines.push_back(traitsPicked_[i] ? toString(1) : toString(0));
    }
  }

  void setParametersFromSaveLines(vector<string>& lines) {
    bg_ = Bg_t(toInt(lines.front()));
    lines.erase(lines.begin());

    for(int i = 0; i < endOfTraits; i++) {
      traitsPicked_[i] = lines.front() == toString(1);
      lines.erase(lines.begin());
    }
  }

  inline bool hasTrait(const Trait_t id) const {
    return traitsPicked_[id];
  }

  void getAllPickableTraits(vector<Trait_t>& traitsRef);

  void getTraitPrereqs(const Trait_t id,
                       vector<Trait_t>& traitsToFill);

  inline bool getBg() const {return bg_;}

  void getTraitTitle(const Trait_t id, string& strRef) const;
  void getTraitDescr(const Trait_t id, string& strRef) const;

  void getBgTitle(const Bg_t id, string& strRef) const;
  void getBgDescr(const Bg_t id, string& strRef) const;

  void getAllPickedTraitsTitlesList(vector<string>& titlesRef);
  void getAllPickedTraitsTitlesLine(string& strRef);

  void pickTrait(const Trait_t id);

  void pickBg(const Bg_t bg);

  void setAllTraitsToPicked() {
    for(int i = 0; i < endOfTraits; i++) {
      traitsPicked_[i] = true;
    }
  }

  //  void setAllTraitsToNotPicked() {
  //    for(int i = 0; i < endOfTraits; i++) {
  //      traitsPicked_[i] = false;
  //    }
  //  }

private:
  bool traitsPicked_[endOfTraits];

  Bg_t bg_;

  Engine& eng;
};

#endif
