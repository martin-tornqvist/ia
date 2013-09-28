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
  playerBon_alchemist,
//  playerBon_athletic,
//playerBon_clearThinker,
  playerBon_coolHeaded,
//playerBon_courageous,
//playerBon_deadlyThrower,
  playerBon_dexterous,
//  playerBon_elusive,
  playerBon_fearless,
  playerBon_healer,
//  playerBon_imperceptible,
//  playerBon_lithe,
  playerBon_marksman,
//  playerBon_mobile,
//  playerBon_nimbleHanded,
  playerBon_observant,
  playerBon_occultist,
  playerBon_rapidRecoverer,
//  playerBon_rugged,
  playerBon_selfAware,
//  playerBon_sharpshooter,
  playerBon_spirited,
//  playerBon_steadyAimer,
  playerBon_stealthy,
  playerBon_strongBacked,
//  playerBon_strongMinded,
//playerBon_swiftAssailant,
//playerBon_swiftRetaliator,
  playerBon_tough,
  playerBon_treasureHunter,
//playerBon_tumbler,
  playerBon_vigilant,
//  playerBon_warlock,
  endOfPlayerBons
};

enum PlayerBonType_t {
  playerBonType_trait, playerBonType_skill
};

class PlayerBon {
public:
  PlayerBon(string title, PlayerBonType_t type, string effectDescr) :
    title_(title), type_(type), effectDescr_(effectDescr), isPicked_(false) {}
  PlayerBon() {}
  string title_;
  PlayerBonType_t type_;
  string effectDescr_;
  bool isPicked_;
};

class PlayerBonHandler {
public:
  PlayerBonHandler(Engine* engine);

  void addSaveLines(vector<string>& lines) {
    for(unsigned int i = 0; i < endOfPlayerBons; i++) {
      lines.push_back(bons_[i].isPicked_ ? intToString(1) : intToString(0));
    }
  }

  void setParametersFromSaveLines(vector<string>& lines) {
    for(unsigned int i = 0; i < endOfPlayerBons; i++) {
      bons_[i].isPicked_ = lines.front() == intToString(0) ? false : true;
      lines.erase(lines.begin());
    }
  }

  inline bool isBonPicked(const PlayerBon_t bon) const {
    return bons_[bon].isPicked_;
  }

  void setAllToUnpicked() {
    for(unsigned int i = 0; i < endOfPlayerBons; i++) {
      bons_[i].isPicked_ = false;
    }
  }

  inline string getBonTitle(const PlayerBon_t bon) const {
    return bons_[bon].title_;
  }

  inline PlayerBonType_t getBonType(const PlayerBon_t bon) const {
    return bons_[bon].type_;
  }

  void getAllPickedBonTitlesList(vector<string>& titles);
  void getAllPickedBonTitlesLine(string& str);

  string getBonEffectDescr(const PlayerBon_t bon) const {
    return bons_[bon].effectDescr_;
  }

  void pickBon(const PlayerBon_t bon);

  void setAllBonsToPicked() {
    for(unsigned int i = 0; i < endOfPlayerBons; i++) {
      bons_[i].isPicked_ = true;
    }
  }

private:
  Engine* eng;

  inline void addBon(const PlayerBon_t bon, const string title,
                     const PlayerBonType_t type,
                     const string effectDescr) {
    bons_[bon] = PlayerBon(title, type, effectDescr);
  }

  PlayerBon bons_[endOfPlayerBons];
};

#endif
