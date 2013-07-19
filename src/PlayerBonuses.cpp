#include "PlayerBonuses.h"

#include <cassert>
#include <algorithm>

#include "Engine.h"
#include "TextFormatting.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"

using namespace std;

void PlayerBonHandler::pickBonus(const PlayerBon_t bonus) {
  bonuses_[bonus].isPicked_ = true;

  switch(bonus) {
    case playerBon_healthy: {
      eng->player->changeMaxHP(2, false);
    }
    break;
    case playerBon_vigorous: {
      eng->player->changeMaxHP(2, false);
    }
    break;
    case playerBon_selfAware: {
      bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeVisionBlockerArray(eng->player->pos, visionBlockers);
      eng->player->getStatusEffectsHandler()->endEffect(statusConfused, visionBlockers);
    }
    break;
    default: {
    } break;
  }
}

PlayerBonHandler::PlayerBonHandler(Engine* engine) : eng(engine) {
  setBonus(playerBon_dexterous, "Dexterous", "+25% chance to evade attacks and traps, every fifth move is a free action");
  setBonus(playerBon_lithe, "Lithe", "+20% chance to evade attacks and traps, every fourth move is a free action", playerBon_dexterous);
  setBonus(playerBon_mobile, "Mobile", "Every second move is a free action", playerBon_lithe, playerBon_tough);
//  setBonus(playerBon_swiftRetaliator, "Swift retaliator", "Dodging causes retaliation attacks if melee weapon is wielded");
//  setBonus(playerBon_elusive, "Elusive", "+30% chance to evade attacks while moving", playerBon_dexterous);
//  setBonus(playerBon_tumbler, "Tumbler", "Can evade explosions", playerBon_athletic);
  setBonus(playerBon_adeptMeleeCombatant, "Adept melee combatant", "+15% hit chance with melee weapons");
  setBonus(playerBon_masterfulMeleeCombatant, "Masterful melee combatant", "+15% hit chance with melee weapons", playerBon_adeptMeleeCombatant);
//  setBonus(playerBon_swiftAssailant, "Swift assailant", "Killing a monster with a melee weapon is considered a free turn");
//  setBonus(playerBon_aggressive, "Aggressive", "+X melee dmg when attacking in same direction as previous move");
  setBonus(playerBon_marksman, "Marksman", "+15% hit chance with firearms and thrown wepaons");
  setBonus(playerBon_sharpshooter, "Sharpshooter", "+15% hit chance with firearms and thrown wepaons", playerBon_marksman);
  setBonus(playerBon_steadyAimer, "Steady aimer", "Waiting a turn gives +20% hit chance with firearms and thrown weapons", playerBon_marksman);
//  setBonus(playerBon_deadlyThrower, "Deadly thrower", "X% chance for 2x max dmg with thrown weapons", playerBon_adeptRangedCombatant);
//  setBonus(playerBon_nimbleHanded, "Nimble-handed", "Can disarm mechanical traps, can use lockpicks, X% chance to reload or swap weapons instantly", playerBon_dexterous);
//  setBonus(playerBon_quick, "Quick", "10% chance for free turn when moving");
  setBonus(playerBon_observant, "Observant", "You occasionally spot clues about hidden passages, and your attentiveness is higher when examining objects (for example when searching a chest for traps)");
  setBonus(playerBon_treasureHunter, "Treasure hunter", "50% more items found on map", playerBon_observant);
  setBonus(playerBon_vigilant, "Vigilant", "You cannot be backstabbed", playerBon_observant, playerBon_dexterous);
  setBonus(playerBon_stealthy, "Stealthy", "+45% chance to avoid being spotted by monsters");
  setBonus(playerBon_imperceptible, "Imperceptible", "+20% chance to avoid being spotted by monsters", playerBon_stealthy, playerBon_dexterous);
  setBonus(playerBon_occultist, "Occultist", "Casting chance of memorized spells increases over time");
  setBonus(playerBon_warlock, "Warlock", "+20% chance to cast memorized spells, can disarm magic traps", playerBon_occultist, playerBon_strongMinded);
  setBonus(playerBon_strongMinded, "Strong-minded", "+20% mental status resistance, -5% shock received");
  setBonus(playerBon_unyielding, "Unyielding", "+20% mental status resistance, -5% shock received", playerBon_strongMinded);
  setBonus(playerBon_coolHeaded, "Cool-headed", "-20% shock received", playerBon_strongMinded);
  setBonus(playerBon_selfAware, "Self-aware", "Cannot be confused, number of remaining turns for status effects is displayed", playerBon_strongMinded, playerBon_observant);
//  setBonus(playerBon_wakeful, "Wakeful", "Cannot faint");
//  setBonus(playerBon_clearThinker, "Clear thinker", "Cannot be confused");
//  setBonus(playerBon_courageous, "Courageous", "Cannot be terrified");
  setBonus(playerBon_skillfulWoundTreater, "Skillful wound treater", "Healing takes half the normal time");
  setBonus(playerBon_curer, "Curer", "Can heal disease", playerBon_skillfulWoundTreater);
  setBonus(playerBon_rapidRecoverer, "Rapid recoverer", "Passive HP regeneration", playerBon_curer, playerBon_healthy);
  setBonus(playerBon_tough, "Tough", "+20% physical status resistance, +20% chance to force open doors, +10% carry weight limit");
  setBonus(playerBon_strongBacked, "Strong-backed", "+30% carry weight limit", playerBon_tough);
  setBonus(playerBon_rugged, "Rugged", "+20% physical status resistance, +10% carry weight limit", playerBon_tough);
  setBonus(playerBon_healthy, "Healthy", "+2 HP", playerBon_tough);
  setBonus(playerBon_vigorous, "Vigorous", "+2 HP", playerBon_healthy);
}

void PlayerBonHandler::setBonus(const PlayerBon_t bonus, const string title,
                                  const string description,
                                  const PlayerBon_t prereq1,
                                  const PlayerBon_t prereq2,
                                  const PlayerBon_t prereq3) {

  vector<PlayerBon_t> prereqs;
  prereqs.resize(0);
  if(prereq1 != endOfPlayerBons) {
    prereqs.push_back(prereq1);
  }
  if(prereq2 != endOfPlayerBons) {
    prereqs.push_back(prereq2);
  }
  if(prereq3 != endOfPlayerBons) {
    prereqs.push_back(prereq3);
  }

  bonuses_[bonus] = PlayerBonus(title, description, prereqs);
}

vector<PlayerBon_t> PlayerBonHandler::getBonusChoices() const {
  vector<PlayerBon_t> ret;
  for(unsigned int i = 0; i < endOfPlayerBons; i++) {
    const PlayerBonus& bon = bonuses_[i];
    if(bon.isPicked_ == false) {
      bool isPrereqsMet = true;
      for(unsigned int ii = 0; ii < bon.prereqs_.size(); ii++) {
        if(bonuses_[bon.prereqs_.at(ii)].isPicked_ == false) {
          isPrereqsMet = false;
        }
      }
      if(isPrereqsMet) {
        ret.push_back(static_cast<PlayerBon_t>(i));
      }
    }
  }

  std::sort(ret.begin(), ret.end());
  return ret;
}

vector<PlayerBon_t> PlayerBonHandler::getBonusPrereqs(
  const PlayerBon_t bonusId) const {
  return bonuses_[bonusId].prereqs_;
}

void PlayerBonHandler::getAllPickedBonusTitlesList(vector<string>& titles) {
  titles.resize(0);
  for(unsigned int i = 0; i < endOfPlayerBons; i++) {
    const PlayerBonus& bon = bonuses_[i];
    if(bon.isPicked_) {
      titles.push_back(bon.title_);
    }
  }
}

void PlayerBonHandler::getAllPickedBonusTitlesLine(string& str) {
  str = "";
  for(unsigned int i = 0; i < endOfPlayerBons; i++) {
    const PlayerBonus& bon = bonuses_[i];
    if(bon.isPicked_) {
      const string title = "\"" + bon.title_ + "\"";
      str += str == "" ? title : (", " + title);
    }
  }
}
