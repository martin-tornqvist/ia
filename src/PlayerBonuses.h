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
enum PlayerBon_t {
  playerBon_adeptMeleeCombatant,
//playerBon_aggressive,
//  playerBon_athletic,
//playerBon_clearThinker,
  playerBon_coolHeaded,
//playerBon_courageous,
  playerBon_curer,
//playerBon_deadlyThrower,
  playerBon_dexterous,
//  playerBon_elusive,
  playerBon_healthy,
  playerBon_imperceptible,
  playerBon_lithe,
  playerBon_marksman,
  playerBon_masterfulMeleeCombatant,
  playerBon_mobile,
//  playerBon_nimbleHanded,
  playerBon_observant,
  playerBon_occultist,
  playerBon_rapidRecoverer,
  playerBon_rugged,
  playerBon_selfAware,
  playerBon_sharpshooter,
  playerBon_skillfulWoundTreater,
  playerBon_steadyAimer,
  playerBon_stealthy,
  playerBon_strongBacked,
  playerBon_strongMinded,
//playerBon_swiftAssailant,
//playerBon_swiftRetaliator,
  playerBon_tough,
  playerBon_treasureHunter,
//playerBon_tumbler,
  playerBon_unyielding,
  playerBon_vigilant,
  playerBon_vigorous,
//playerBon_wakeful,
  playerBon_warlock,
  endOfPlayerBons
};

class PlayerBonus {
public:
  PlayerBonus(string title, string description, vector<PlayerBon_t> prereqs) :
    title_(title), description_(description), prereqs_(prereqs), isPicked_(false) {
  }
  PlayerBonus() {
  }
  string title_;
  string description_;
  vector<PlayerBon_t> prereqs_;
  bool isPicked_;
protected:
};

class PlayerBonHandler {
public:
  PlayerBonHandler(Engine* engine);

  void addSaveLines(vector<string>& lines) {
    for(unsigned int i = 0; i < endOfPlayerBons; i++) {
      lines.push_back(bonuses_[i].isPicked_ ? intToString(1) : intToString(0));
    }
  }

  void setParametersFromSaveLines(vector<string>& lines) {
    for(unsigned int i = 0; i < endOfPlayerBons; i++) {
      bonuses_[i].isPicked_ = lines.front() == intToString(0) ? false : true;
      lines.erase(lines.begin());
    }
  }

  bool isBonPicked(const PlayerBon_t bonus) {
    return bonuses_[bonus].isPicked_;
  }

  void setAllToUnpicked() {
    for(unsigned int i = 0; i < endOfPlayerBons; i++) {
      bonuses_[i].isPicked_ = false;
    }
  }

  vector<PlayerBon_t> getBonusChoices() const;

  string getBonusTitle(const PlayerBon_t bonus) const {
    return bonuses_[bonus].title_;
  }

  void getAllPickedBonusTitlesList(vector<string>& titles);
  void getAllPickedBonusTitlesLine(string& str);

  string getBonusDescription(const PlayerBon_t bonus) const {
    return bonuses_[bonus].description_;
  }

  void pickBonus(const PlayerBon_t bonus);

  void setAllBonusesToPicked() {
    for(unsigned int i = 0; i < endOfPlayerBons; i++) {
      bonuses_[i].isPicked_ = true;
    }
  }

  vector<PlayerBon_t> getBonusPrereqs(const PlayerBon_t bonusId) const;

private:
  Engine* eng;

  void setBonus(const PlayerBon_t bonus, const string title,
                const string description,
                const PlayerBon_t prereq1 = endOfPlayerBons,
                const PlayerBon_t prereq2 = endOfPlayerBons,
                const PlayerBon_t prereq3 = endOfPlayerBons);

  PlayerBonus bonuses_[endOfPlayerBons];
};

#endif
