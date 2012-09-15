#include "PlayerBonuses.h"

#include <cassert>

#include "Engine.h"
#include "TextFormatting.h"

PlayerBonus::PlayerBonus(Abilities_t ability,  string titleGroup, string title, string descriptionGeneral,
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

	descriptionRanks_.push_back(engine->textFormatting->lineToLines("Rank 1: " + descrBon1, MAP_X_CELLS - 2));
	descriptionRanks_.push_back(engine->textFormatting->lineToLines("Rank 2: " + descrBon2, MAP_X_CELLS - 2));
	descriptionRanks_.push_back(engine->textFormatting->lineToLines("Rank 3: " + descrBon3, MAP_X_CELLS - 2));

	//descriptionGeneral = bon2 == 0 ? descriptionGeneral : "For each rank: " + descriptionGeneral;
	descriptionGeneral_ = engine->textFormatting->lineToLines(descriptionGeneral, MAP_X_CELLS - 2);
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

	s = "You have better chances to evade melee attacks and traps.";
//	r1 = "Being \"Still\" gives higher dodge chance.";
//	r2 = "Dodging causes retaliation attacks.";
//	r3 = "Can evade explosions for halved damage.";
	bonuses_.push_back(PlayerBonus(ability_dodge, TITLE_COMBAT, "Dodging", s, eng, 5, 25, r1, 45, r2, 65, r3));

	s = "You have better accuracy with melee attacks.";
//	r1 = "Attacks using light weapons are occasionally free.";
//	r2 = "When \"Charging\", medium & heavy weapons do full damage.";
//	r3 = "No weapon degradation from normal hits.";
	bonuses_.push_back(PlayerBonus(ability_accuracyMelee, TITLE_COMBAT, "Melee Accuracy", s, eng, 35, 45, r1, 55, r2, 65, r3));

	s = "You have better accuracy with firearms and thrown weapons.";
//	r1 = "Being \"Still\" gives aim bonus.";
//	r2 = "Thrown weapons have a chance to do double damage.";
//	r3 = "Occasionally reload instantly.";
	bonuses_.push_back(PlayerBonus(ability_accuracyRanged, TITLE_COMBAT, "Ranged Accuracy", s, eng, 40, 55, r1, 70, r2, 85, r3));

//	s = "You are quicker at drawing your weapons. "
//	r1 = "";
//	r2 = "";
//	r3 = "";
//	bonuses_.push_back(PlayerBonus(ability_weaponHandling, TITLE_COMBAT, "Weapon Handling", s, eng, 0, 75));

	s = "You get around more effectively. You occasionally get a free turn when moving.";
//	r1 = "";
//	r2 = "";
//	r3 = "";
	bonuses_.push_back(PlayerBonus(ability_mobility, TITLE_EXPLORATION_MOVEMENT, "Mobility", s, eng, 0, 20, r1));

	s = "You have better chances of spotting hidden things. You also tend to find more items.";
//	r1 = "";
//	r2 = "Can spot hidden constructions from two moves distance.";
//	r3 = "Can not be backstabbed.";
	bonuses_.push_back(PlayerBonus(ability_searching, TITLE_EXPLORATION_MOVEMENT, "Searching", s, eng, 1, 6, r1, 12, r2, 18, r3));

	s = "You are more adept at moving unseen. Attacking unaware creatures does extra damage.";
//	r1 = "Attacking an unaware creature does double damage.";
//	r2 = "Unaware opponents makes no sound when you dispatch them.";
//	r3 = "";
	bonuses_.push_back(PlayerBonus(ability_sneaking, TITLE_EXPLORATION_MOVEMENT, "Sneaking", s, eng, 30, 75, r1, 90, r2));

	s = "You have a better grasp of ancient language and esoteric symbolism. ";
	s += "Your chances to identify and memorize manuscripts increases.";
//	r1 = "";
//	r2 = "";
//	r3 = "";
	bonuses_.push_back(PlayerBonus(ability_language, TITLE_LORE, "Language", s, eng, 0, 40, r1, 80, r2));

	s = "You have more focus and will, and are less likely to succumb to fear, confusion, etc. ";
	s += "You are also better able to keep your composure when facing hideous revelations.";
//	r1 = "";
//	r2 = "Can not faint.";
//	r3 = "Can not be confused.";
	bonuses_.push_back(PlayerBonus(ability_resistStatusMind, TITLE_SURVIVAL, "Fortitude", s, eng, 25, 45, r1, 65, r2, 85, r3));

	s = "Your healing skills improve. With the first rank, healing takes half time. With the second rank, ";
	s += "you are able to heal disease. With the third rank, you regenerate hit points passively over time.";
//	r1 = "Healing takes half time.";
//	r2 = "Can heal disease.";
//	r3 = "Regenerate hit points passively.";
	bonuses_.push_back(PlayerBonus(ability_firstAid, TITLE_SURVIVAL, "Healing", s, eng, 0, 1, r1, 2, r2, 3, r3));

	s = "You have better physical endurance. You are less likely to be afflicted by diseases, stunning, ";
	s += "blindness, etc. You also gain a one-time bonus of 2 hit points each time this ability is picked.";
//	r1 = "";
//	r2 = "No sprains from bashing objects.";
//	r3 = "";
	bonuses_.push_back(PlayerBonus(ability_resistStatusBody, TITLE_SURVIVAL, "Toughness", s, eng, 25, 45, r1, 65, r2, 85, r3));
}

