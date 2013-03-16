#include "DebugModeStatPrinter.h"

#include <vector>
#include <algorithm>

#include "Engine.h"
#include "ActorData.h"
#include "ActorFactory.h"
#include "Inventory.h"
#include "Item.h"

struct IsHigherMonsterLvl {
public:
  IsHigherMonsterLvl() {
  }
  bool operator()(const ActorDefinition* const d1, const ActorDefinition* const d2) {
    return d1->monsterLvl < d2->monsterLvl;
  }
};

void DebugModeStatPrinter::run() {
  const string separator = "--------------------------------------------------";
  const string indent1 = " ";
  const string indent2 = indent1 + "      ";
//  const string indent2 = indent1 + indent1;

  statFile.open("debug_mode_stats_file.txt", ios::trunc);
  printLine("This file was created because Infra Arcana was run in Debug mode\n");
  printLine("Created at   : " + eng->basicUtils->getCurrentTime().getTimeStr(time_minute, true));
  printLine("Game version : " + eng->config->GAME_VERSION);
  printLine("\n");

  vector<ActorDefinition*> actorDefsSorted;
  for(unsigned int i = actor_player + 1; i < endOfActorIds; i++) {
    actorDefsSorted.push_back(&(eng->actorData->actorDefinitions[i]));
  }
  IsHigherMonsterLvl isHigherMonsterLvl;
  std::sort(actorDefsSorted.begin(), actorDefsSorted.end(), isHigherMonsterLvl);

  printLine("MONSTER LVL SPREAD");
  printLine(separator);
  printLine(indent1 + "LVL   NR");
  printLine(indent1 + "---------");

  vector<int> monstersPerLvl(actorDefsSorted.back()->monsterLvl, 0);
  for(unsigned int i = 0; i < actorDefsSorted.size(); i++) {
    monstersPerLvl.at(actorDefsSorted.at(i)->monsterLvl - 1)++;
  }
  for(unsigned int i = 0; i < monstersPerLvl.size(); i++) {
    const int LVL = i + 1;
    const string lvlStr = (LVL < 10 ? " " : "") + intToString(LVL);
    const string nrStr = intToString(monstersPerLvl.at(i));
    printLine(indent1 + lvlStr + "    " + nrStr);
  }
  printLine("\n" + indent1 + "Total number of monsters: " + intToString(actorDefsSorted.size()));
  printLine("\n");

  printLine("STATS FOR EACH MONSTER");
  printLine(separator);
  printLine(indent1 + "For creature's attack(s):");
  printLine(indent1 + "(M) = Melee weapon");
  printLine(indent1 + "(R) = Ranged weapon");
  printLine("");

  for(unsigned int i = 0; i < actorDefsSorted.size(); i++) {
    ActorDefinition& d = *(actorDefsSorted.at(i));

    Actor* const actor = eng->actorFactory->makeActorFromId(d.id);
    actor->place(coord(-1, -1), &d, eng);

    printLine(indent1 + actor->getNameA());
    const string lvlStr = "LVL " + intToString(d.monsterLvl);
    const string hpStr = "HP " + intToString(d.hpMax);
    const int attackSkill = d.abilityValues.getAbilityValue(ability_accuracyMelee, false, *actor);
    const string attackSkillStr = "Attack skill " + intToString(attackSkill) + "%";
    printLine(indent2 + lvlStr + ", " + hpStr + ", " + attackSkillStr);

    const Inventory* const inv = actor->getInventory();
    const unsigned int NR_INTRINSIC_ATTACKS = inv->getIntrinsicsSize();
    for(unsigned int i_intr = 0; i_intr < NR_INTRINSIC_ATTACKS; i_intr++) {
      const Item* const item = inv->getIntrinsicInElement(i_intr);
      const ItemDefinition& itemDef = item->getDef();
      const string meleeOrRangedStr = itemDef.isRangedWeapon ? "(R)" : "(M)";
      const string attackNrStr = "Attack " + intToString(i_intr + 1);
      const string dmgStr = intToString(itemDef.meleeDmg.first) + "d" + intToString(itemDef.meleeDmg.second);
      printLine(indent2 + attackNrStr + " " + meleeOrRangedStr + ": " + dmgStr);
    }

    printLine("");
  }



  statFile.close();
}


void DebugModeStatPrinter::printLine(const string& line) {
  statFile << line << endl;
}
