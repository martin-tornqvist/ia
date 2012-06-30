#include "PlayerBonuses.h"

#include <cassert>

#include "Engine.h"
#include "TextFormatting.h"

PlayerBonus::PlayerBonus(Abilities_t ability,  string titleGroup, string title, string description,
                         Engine* engine, int startSkill, int bon1, string descrBon1, int bon2,
                         string descrBon2, int bon3, string descrBon3) :
	rank_(0), picked_(false), ability_(ability), titleGroup_(titleGroup), title_(title) {

	abilityBonusAtRanks_.push_back(startSkill);

	if(bon1 != 0) {
		abilityBonusAtRanks_.push_back(bon1);
	}
	if(bon2 != 0) {
		abilityBonusAtRanks_.push_back(bon2);
	}
	if(bon3 != 0) {
		abilityBonusAtRanks_.push_back(bon3);
	}

   descriptionRanks_.push_back(descrBon1);
   descriptionRanks_.push_back(descrBon2);
   descriptionRanks_.push_back(descrBon3);

	description_ = engine->textFormatting->lineToLines(description, MAP_X_CELLS - 2);
}

PlayerBonusHandler::PlayerBonusHandler(Engine* eng) {
	const string TITLE_COMBAT = "COMBAT";
	const string TITLE_EXPLORATION_MOVEMENT = "EXPLORATION & MOVEMENT";
	const string TITLE_LORE = "LORE";
	const string TITLE_SURVIVAL = "SURVIVAL";

	string s = "";
   string r1, r2, r3;

//	s = "You are occasionally able to do extra damage to unaware creatures in melee.";
//	bonuses_.push_back(PlayerBonus(ability_backstabbing, TITLE_COMBAT, "Backstabbing", s, eng, 0, 75));

	s = "You are more skilled at evading melee attacks and traps.";
	r1 = "";
	r2 = "Dodging causes retaliation attack.";
	r3 = "Can evade explosions.";
	bonuses_.push_back(PlayerBonus(ability_dodge, TITLE_COMBAT, "Dodging", s, eng, 5, 25, r1, 45, r2, 65, r3));

	s = "You have better accuracy with melee attacks.";
	r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_accuracyMelee, TITLE_COMBAT, "Melee Accuracy", s, eng, 35, 45, 55));

	s = "You have better accuracy with firearms and thrown weapons.";
	r1 = "Thrown weapons do max damage more often.";
	r2 = "Occasionally reload instantly.";
	r3 = "";
	bonuses_.push_back(PlayerBonus(ability_accuracyRanged, TITLE_COMBAT, "Ranged Accuracy", s, eng, 50, 60, 70, 80, 90));

	s = "You become more efficient with your weapons. With small weapons you occasionally get a free turn,";
	s += " and with heavy weapons you do full damage more often. For medium weapons any of the two can occur.";
	s += " You also occasionally wield, reload or swap weapons in the blink of an eye.";
   r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_weaponHandling, TITLE_COMBAT, "Weapon Handling", s, eng, 0, 25, 50, 75));

	s = "You are able to move more effectively. You occasionally get a free turn when moving.";
	r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_mobility, TITLE_EXPLORATION_MOVEMENT, "Mobility", s, eng, 0, 10, 20, 30));

	s = "You have better chances of spotting hidden traps, doors and monsters. You also tend to find more items in the dungeon.";
	r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_searching, TITLE_EXPLORATION_MOVEMENT, "Searching", s, eng, 1, 7, 14, 21));

	s = "You are more adept at moving unseen.";
	r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_sneaking, TITLE_EXPLORATION_MOVEMENT, "Sneaking", s, eng, 30, 65, 80, 90));

	s = "You are more able to fathom the deeper workings of the black arts. Spells can be cast from memory with less chance of failure. ";
	s += "You may also gain spontaneous insights into the purpose of potions.";
	r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_loreArcana, TITLE_LORE, "Arcana", s, eng, 0, 30, 50, 70, 100));

	s = "You have a better understanding of ancient language and esoteric symbolism. ";
	s += "You have better chances of identifying and memorizing written sorceries";
	r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_loreLanguage, TITLE_LORE, "Language", s, eng, 15, 90));

	s = "You have more focus and will, and are less likely to succumb to fear, confusion, etc. ";
	s += "You are also better able to keep your composure when facing hideous revelations.";
	r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_resistStatusMindAndShock, TITLE_SURVIVAL, "Fortitude", s, eng, 23, 36, 49, 62, 75));

	s = "You are quicker at mending your wounds.";
	r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_firstAid, TITLE_SURVIVAL, "Healing", s, eng, 0, 30, 60));

	s = "You have better physical endurance, better immune system, and more robust senses. You are less likely to be afflicted by";
	s += " diseases, stunning, blindness, etc. You are also better able to shrug off the elements.";
	r1 = "";
	r2 = "Occasional free turn or full damage depending on weapon size.";
	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_resistStatusBodyAndSense, TITLE_SURVIVAL, "Toughness", s, eng, 23, 36, 49, 62, 75));
}

