#include "PlayerBonuses.h"

#include <cassert>
#include <algorithm>

#include "Engine.h"
#include "TextFormatting.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"

using namespace std;

void PlayerBonHandler::pickBon(const PlayerBon_t bon) {
  bons_[bon].isPicked_ = true;

  switch(bon) {
    case playerBon_vigorous: {
      eng->player->changeMaxHp(3, false);
    } break;

    case playerBon_selfAware: {
      bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeVisionBlockerArray(eng->player->pos, visionBlockers);
      eng->player->getStatusHandler()->endEffect(statusConfused, visionBlockers);
    } break;

    default: {} break;
  }
}

PlayerBonHandler::PlayerBonHandler(Engine* engine) : eng(engine) {
  string descr = "";

  descr  = "+25% chance to evade attacks and traps, ";
  descr += "every fourth move is a free action";
  addBon(playerBon_dexterous, "Dexterous", playerBonType_trait, descr);

  descr  = "You occasionally spot clues about hidden passages, and ";
  descr += "your attentiveness is higher when examining objects ";
  descr += "(for example when searching a chest for traps)";
  addBon(playerBon_observant, "Observant", playerBonType_trait, descr);

  descr  = "You cannot be backstabbed";
  addBon(playerBon_vigilant, "Vigilant", playerBonType_trait, descr);

  descr  = "+20% mental status resistance, -5% shock";
  addBon(playerBon_unyielding, "Unyielding", playerBonType_trait, descr);

  descr  = "-20% shock";
  addBon(playerBon_coolHeaded, "Cool-headed", playerBonType_trait, descr);

  descr  = "Cannot be confused, number of remaining turns for status effects ";
  descr += "is displayed";
  addBon(playerBon_selfAware, "Self-aware", playerBonType_trait, descr);

  descr  = "+20% physical status resistance, +20% chance to force open doors, ";
  descr += "+10% carry weight limit";
  addBon(playerBon_tough, "Tough", playerBonType_trait, descr);

  descr  = "+3 hit points";
  addBon(playerBon_vigorous, "Vigorous", playerBonType_trait, descr);

//  addBon(playerBon_swiftRetaliator, "Swift retaliator", "Dodging causes retaliation attacks if melee weapon is wielded");

//  addBon(playerBon_elusive, "Elusive", "+30% chance to evade attacks while moving", playerBon_dexterous);

//  addBon(playerBon_tumbler, "Tumbler", "Can evade explosions", playerBon_athletic);

  descr  = "+15% hit chance with melee weapons";
  addBon(playerBon_adeptMeleeCombatant, "Adept melee combatant",
         playerBonType_skill, descr);

//  addBon(playerBon_swiftAssailant, "Swift assailant", "Killing a monster with a melee weapon is considered a free turn");

//  addBon(playerBon_aggressive, "Aggressive", "+X melee dmg when attacking in same direction as previous move");

  descr  = "+15% hit chance with firearms and thrown weapons, ";
  descr += "standing still for a turn gives another 10%";
  addBon(playerBon_marksman, "Marksman", playerBonType_skill, descr);

//  addBon(playerBon_steadyAimer, "Steady aimer", playerBonType_skill,
//         "Waiting a turn gives +20% hit chance with firearms and thrown weapons");

//  addBon(playerBon_deadlyThrower, "Deadly thrower", "X% chance for 2x max dmg with thrown weapons", playerBon_adeptRangedCombatant);

//  addBon(playerBon_nimbleHanded, "Nimble-handed", "Can disarm mechanical traps, can use lockpicks, X% chance to reload or swap weapons instantly", playerBon_dexterous);

  descr  = "50% more items found on map";
  addBon(playerBon_treasureHunter, "Treasure hunter",
         playerBonType_skill, descr);

  descr  = "Half of all potions are known";
  addBon(playerBon_alchemist, "Alchemist", playerBonType_skill, descr);

  descr  = "You have less chance to be spotted by monsters";
  addBon(playerBon_stealthy, "Stealthy", playerBonType_skill, descr);

  descr  = "Casting chance of memorized spells increases over time";
  addBon(playerBon_occultist, "Occultist", playerBonType_skill, descr);

//  addBon(playerBon_wakeful, "Wakeful", "Cannot faint");

//  addBon(playerBon_clearThinker, "Clear thinker", "Cannot be confused");

//  addBon(playerBon_courageous, "Courageous", "Cannot be terrified");

  descr  = "Healing takes half the normal time";
  addBon(playerBon_skillfulWoundTreater, "Skillful wound treater",
         playerBonType_skill, descr);

  descr  = "Can heal disease";
  addBon(playerBon_curer, "Curer", playerBonType_skill, descr);

  descr  = "Double hit point regeneration rate";
  addBon(playerBon_rapidRecoverer, "Rapid recoverer",
         playerBonType_skill, descr);

  descr  = "+30% carry weight limit";
  addBon(playerBon_strongBacked, "Strong-backed", playerBonType_trait, descr);
}

void PlayerBonHandler::getAllPickedBonTitlesList(vector<string>& titles) {
  titles.resize(0);
  for(unsigned int i = 0; i < endOfPlayerBons; i++) {
    const PlayerBon& bon = bons_[i];
    if(bon.isPicked_) {
      titles.push_back(bon.title_);
    }
  }
}

void PlayerBonHandler::getAllPickedBonTitlesLine(string& str) {
  str = "";
  for(unsigned int i = 0; i < endOfPlayerBons; i++) {
    const PlayerBon& bon = bons_[i];
    if(bon.isPicked_) {
      const string title = "\"" + bon.title_ + "\"";
      str += str == "" ? title : (", " + title);
    }
  }
}
