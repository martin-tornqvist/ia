#ifndef PLAYER_BONUSES_H
#define PLAYER_BONUSES_H

#include <string>
#include <vector>

#include "AbilityValues.h"
#include "Converters.h"
#include "Engine.h"

#include <math.h>

using namespace std;

// *************************************************** //
// Keep the following values lexicographically sorted! //
// *************************************************** //
enum Trait_t {
  traitAdeptMeleeCombatant,
  traitAlchemist,
  traitCoolHeaded,
  traitDexterous,
  traitFearless,
  traitHealer,
  traitMarksman,
  traitObservant,
  traitRapidRecoverer,
  traitSelfAware,
  traitSpirited,
  traitStealthy,
  traitStrongBacked,
  traitTough,
  traitTreasureHunter,
  endOfTraits
};

enum Bg_t {
  bgOccultist, bgRogue, bgSoldier, endOfBgs
};

class PlayerBonHandler {
public:
  PlayerBonHandler(Engine* engine);

  void addSaveLines(vector<string>& lines) {
    lines.push_back(toString(bg_));

    for(int i = 0; i < endOfTraits; i++) {
      lines.push_back(traitsPicked_[i] ? toString(1) : toString(0));
    }
  }

  void setParametersFromSaveLines(vector<string>& lines) {
    bg_ = Bg_t(toInt(lines.front()));

    for(int i = 0; i < endOfTraits; i++) {
      traitsPicked_[i] = lines.front() == toString(1);
      lines.erase(lines.begin());
    }
  }

  inline bool isTraitPicked(const Trait_t id) const {
    return traitsPicked_[id];
  }

  void getAllPickableTraits(vector<Trait_t>& traitsToSet);

  void getTraitPrereqs(const Trait_t id,
                       vector<Trait_t>& traitsToFill);

  inline bool getBg() const {return bg_;}

  void getTraitTitle(const Trait_t id, string& strToSet) const;
  void getTraitDescr(const Trait_t id, string& strToSet) const;

  void getBgTitle(const Bg_t id, string& strToSet) const;
  void getBgDescr(const Bg_t id, string& strToSet) const;

  void getAllPickedTraitsTitlesList(vector<string>& titlesToSet);
  void getAllPickedTraitsTitlesLine(string& strToSet);

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

  Engine* eng;
};

#endif
