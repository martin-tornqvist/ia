#include "PlayerBonuses.h"

#include <cassert>
#include <algorithm>

#include "Engine.h"
#include "TextFormatting.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"

using namespace std;

PlayerBonusHandler::PlayerBonusHandler(Engine* engine) : eng(engine) {
  setBonus(playerBonus_agile, "Agile", "+30% chance to evade attacks and traps, every fourth move is a free action.");
  setBonus(playerBonus_lithe, "Lithe", "+20% chance to evade attacks and traps, every third move is a free action.", playerBonus_agile);
//  setBonus(playerBonus_swiftRetaliator, "Swift retaliator", "Dodging causes retaliation attacks if melee weapon is wielded");
//  setBonus(playerBonus_elusive, "Elusive", "+30% chance to evade attacks while moving", playerBonus_agile);
//  setBonus(playerBonus_tumbler, "Tumbler", "Can evade explosions", playerBonus_athletic);
  setBonus(playerBonus_adeptMeleeCombatant, "Adept melee combatant", "+15% hit chance with melee weapons");
  setBonus(playerBonus_masterfulMeleeCombatant, "Masterful melee combatant", "+15% hit chance with melee weapons", playerBonus_adeptMeleeCombatant);
//  setBonus(playerBonus_swiftAssailant, "Swift assailant", "Killing a monster with a melee weapon is considered a free turn");
//  setBonus(playerBonus_aggressive, "Aggressive", "+X melee dmg when attacking in same direction as previous move");
  setBonus(playerBonus_marksman, "Marksman", "+15% hit chance with firearms and thrown wepaons");
  setBonus(playerBonus_sharpshooter, "Sharpshooter", "+15% hit chance with firearms and thrown wepaons", playerBonus_marksman);
  setBonus(playerBonus_steadyAimer, "Steady aimer", "Waiting a turn gives 100% hit chance with firearms and thrown weapons", playerBonus_marksman);
//  setBonus(playerBonus_deadlyThrower, "Deadly thrower", "X% chance for 2x max dmg with thrown weapons", playerBonus_adeptRangedCombatant);
//  setBonus(playerBonus_nimble, "Nimble", "X% chance to reload instantly, swapping is always instant", playerBonus_adeptMeleeCombatant, playerBonus_adeptRangedCombatant);
//  setBonus(playerBonus_quick, "Quick", "10% chance for free turn when moving");
  setBonus(playerBonus_observant, "Observant", "You occasionally spot clues about hidden passages");
  setBonus(playerBonus_treasureHunter, "Treasure hunter", "+20% more items found", playerBonus_observant);
  setBonus(playerBonus_vigilant, "Vigilant", "You can not be backstabbed", playerBonus_observant, playerBonus_agile);
  setBonus(playerBonus_stealthy, "Stealthy", "+45% chance to avoid being spoted by monsters");
  setBonus(playerBonus_imperceptible, "Imperceptible", "+20% chance to avoid being spoted by monsters", playerBonus_stealthy);
  setBonus(playerBonus_learned, "Learned", "You can read and memorize manuscripts");
  setBonus(playerBonus_erudite, "Erudite", "Spell cooldowns starts from 20%", playerBonus_learned);
  setBonus(playerBonus_strongMinded, "Strong-minded", "+20% mental status resistance, -5% shock received");
  setBonus(playerBonus_unyielding, "Unyielding", "+20% mental status resistance, -5% shock received", playerBonus_strongMinded);
  setBonus(playerBonus_coolHeaded, "Cool-headed", "-20% shock received", playerBonus_strongMinded);
//  setBonus(playerBonus_wakeful, "Wakeful", "Can not faint");
//  setBonus(playerBonus_clearThinker, "Clear thinker", "Can not be confused");
//  setBonus(playerBonus_courageous, "Courageous", "Can not be terrified");
  setBonus(playerBonus_adeptWoundTreater, "Adept wound treater", "Healing takes half the normal time");
  setBonus(playerBonus_curer, "Curer", "Can heal disease", playerBonus_adeptWoundTreater);
  setBonus(playerBonus_rapidRecoverer, "Rapid recoverer", "Passive HP regeneration", playerBonus_curer, playerBonus_healthy);
  setBonus(playerBonus_tough, "Tough", "+20% physical status resistance, +20% chance to bash doors, +15% carry weight limit");
  setBonus(playerBonus_rugged, "Rugged", "+20% physical status resistance, +15% carry weight limit", playerBonus_tough);
  setBonus(playerBonus_healthy, "Healthy", "+2 HP", playerBonus_tough);
  setBonus(playerBonus_vigorous, "Vigorous", "+2 HP", playerBonus_healthy);
//  setBonus(playerBonus_strongBacked, "Strong-backed", "Encumbrance limit is increased to X%, carry limit is increased to Y%", playerBonus_tough);
}

void PlayerBonusHandler::setBonus(const PlayerBonuses_t bonus, const string title, const string description,
                                  const PlayerBonuses_t prereq1, const PlayerBonuses_t prereq2, const PlayerBonuses_t prereq3) {

  vector<PlayerBonuses_t> prereqs;
  prereqs.resize(0);
  if(prereq1 != endOfPlayerBonuses) {
    prereqs.push_back(prereq1);
  }
  if(prereq2 != endOfPlayerBonuses) {
    prereqs.push_back(prereq2);
  }
  if(prereq3 != endOfPlayerBonuses) {
    prereqs.push_back(prereq3);
  }

  bonuses_[bonus] = PlayerBonus(title, description, prereqs);
}

void PlayerBonusHandler::pickBonus(const PlayerBonuses_t bonus) {
  bonuses_[bonus].isPicked_ = true;

  switch(bonus) {
  case playerBonus_healthy: {
    eng->player->changeMaxHP(2, false);
  }
  break;
  case playerBonus_vigorous: {
    eng->player->changeMaxHP(2, false);
  }
  break;
  default: {
  } break;
  }
}

vector<PlayerBonuses_t> PlayerBonusHandler::getBonusChoices() const {
  vector<PlayerBonuses_t> ret;

//  if(eng->dungeonMaster->getLevel() == 1) {
//    ret.push_back(playerBonus_adeptMeleeCombatant);
//    ret.push_back(playerBonus_tough);
//    ret.push_back(playerBonus_learned);
//    ret.push_back(playerBonus_stealthy);
//    ret.push_back(playerBonus_agile);
//    ret.push_back(playerBonus_strongMinded);
//  } else {
  vector<PlayerBonuses_t> candidates;
  for(unsigned int i = 0; i < endOfPlayerBonuses; i++) {
    const PlayerBonus& bon = bonuses_[i];
    if(bon.isPicked_ == false) {
      bool isPrereqsMet = true;
      for(unsigned int ii = 0; ii < bon.prereqs_.size(); ii++) {
        if(bonuses_[bon.prereqs_.at(ii)].isPicked_ == false) {
          isPrereqsMet = false;
        }
      }
      if(isPrereqsMet) {
        candidates.push_back(static_cast<PlayerBonuses_t>(i));
      }
    }
  }

  const int NR_OF_CHOICES = 6;

  for(int i = 0; i < NR_OF_CHOICES; i++) {
    if(candidates.empty()) {
      if(ret.empty() == false) {
        std::sort(ret.begin(), ret.end());
      }
      return ret;
    }
    const int ELEMENT = eng->dice.getInRange(0, candidates.size() - 1);
    ret.push_back(candidates.at(ELEMENT));
    candidates.erase(candidates.begin() + ELEMENT);
  }
//  }

  std::sort(ret.begin(), ret.end());
  return ret;
}
