#include "Interface.h"

#include "SDL/SDL.h"

#include "Engine.h"

#include "ConstTypes.h"

#include "Colors.h"
#include "ItemWeapon.h"
#include "ItemArmor.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "DungeonMaster.h"

using namespace std;

void Interface::drawInfoLines() {
   eng->renderer->clearRenderArea(renderArea_characterLines);

	const int CHARACTER_LINE_X0 = 1;
	const int CHARACTER_LINE_Y0 = 0;

	int xPos = CHARACTER_LINE_X0;
	int yPos = CHARACTER_LINE_Y0;
	string str = "";

	//Name
	str = eng->player->getNameA();
	eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrRedLight);

	//Health
	xPos += 1 + str.length();
	const string hp = intToString(eng->player->getHP());
	const string hpMax = intToString(eng->player->getHP_max());
	eng->renderer->drawText("HP:", renderArea_characterLines, xPos, yPos, clrGray);
	xPos += 3;
	str = hp + "/" + hpMax;
	eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrRedLight);

	//Sanity
	const int INS_SHORT = eng->player->insanityShort + eng->player->insanityShortTemp;
	const int INS_LONG = eng->player->insanityLong;
	xPos += 1 + str.length();
	eng->renderer->drawText("INS:", renderArea_characterLines, xPos, yPos, clrGray);
	xPos += 4;
	str = intToString(INS_LONG) + "%";
	eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrMagenta);
	xPos += str.length();
	const SDL_Color shortSanClr = INS_SHORT < 50 ? clrGreenLight : INS_SHORT < 75 ? clrYellow : clrMagenta;
	str = "(" + intToString(INS_SHORT) + "%)";
	eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, shortSanClr);
	xPos += 1 + str.length();

    //Encumbrance
    eng->renderer->drawText("ENC:", renderArea_characterLines, xPos, yPos, clrGray);
    xPos += 4;
    const int TOTAL_W = eng->player->getInventory()->getTotalItemWeight();
    const int MAX_W = PLAYER_CARRY_WEIGHT_STANDARD;
    const int ENC = static_cast<int>((static_cast<double>(TOTAL_W) / static_cast<double>(MAX_W)) * 100.0);
    str = intToString(ENC) + "%";
    eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, ENC >= 100 ? clrRedLight : clrWhite);
    xPos += 1 + str.length();

	//Wielded weapon
	xPos += 4;
	//Store x position, because missile wpn info will be directly beaneath wielded wpn info
	const int X_POS_MISSILE = xPos;

	Weapon* weapon = dynamic_cast<Weapon*>(eng->player->getInventory()->getItemInSlot(slot_wielded));
	if(weapon == NULL) {
		eng->renderer->drawText("Unarmed", renderArea_characterLines, xPos, yPos, clrWhite);
	} else {
		string wpnDmgStr, wpnSkillStr, wpnAmmoStr;
		Abilities_t abilityUsed;
		int dmgRolls, dmgSides, dmgPlus, actorAttackSkill, baseAttackSkill, totalAttackSkill;

		const ItemDefinition* itemDef = &(weapon->getInstanceDefinition());

        //Weapon name
//        eng->renderer->drawText("WPN:", renderArea_characterLines, xPos, yPos, clrGray);
//        xPos += 4;
//
//        const string wpnName = eng->itemData->itemInterfaceName(weapon, false);
//        eng->renderer->drawText(wpnName, renderArea_characterLines, xPos, yPos, clrWhite);
//        xPos += 1 + wpnName.size();

		//Firearm info
		eng->renderer->drawText("FIR:", renderArea_characterLines, xPos, yPos, clrGray);
		xPos += 4;

		if(itemDef->isRangedWeapon == true) {
			//Weapon damage and skill used
			const int multiplier = itemDef->isMachineGun == true ? NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST : 1;
			abilityUsed = itemDef->rangedAbilityUsed;
			dmgRolls = itemDef->rangedDmg.rolls * multiplier;
			dmgSides = itemDef->rangedDmg.sides;
			dmgPlus = itemDef->rangedDmg.plus * multiplier;

			//Damage
			const string dmgStrOverRide = itemDef->rangedDmgLabelOverRide;
			if(dmgStrOverRide == "") {
				wpnDmgStr = intToString(dmgRolls) + "d" + intToString(dmgSides);
				wpnDmgStr += dmgPlus > 0 ? "+" + intToString(dmgPlus) : (dmgPlus < 0 ? "-" + intToString(dmgPlus) : "");
			} else {
				wpnDmgStr = dmgStrOverRide;
			}

			//Total attack skill with weapon (base + actor skill)
			actorAttackSkill = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(abilityUsed, true);
			baseAttackSkill = itemDef->rangedBaseAttackSkill;
			totalAttackSkill = baseAttackSkill + actorAttackSkill;
			wpnSkillStr = intToString(totalAttackSkill) + "%";

			//Remaining/total ammo
			wpnAmmoStr = "";
			if(itemDef->rangedHasInfiniteAmmo == false) {
				wpnAmmoStr = intToString(weapon->ammoLoaded) + "/" + intToString(weapon->ammoCapacity);
			}

			eng->renderer->drawText(wpnDmgStr, renderArea_characterLines, xPos, yPos, clrWhite);
			xPos += 1 + wpnDmgStr.size();

			eng->renderer->drawText(wpnSkillStr, renderArea_characterLines, xPos, yPos, clrWhite);
			xPos += 1 + wpnSkillStr.size();

			eng->renderer->drawText(wpnAmmoStr, renderArea_characterLines, xPos, yPos, clrRedLight);
			xPos += 1 + wpnAmmoStr.size();
		} else {
			eng->renderer->drawText("N/A", renderArea_characterLines, xPos, yPos, clrWhite);
			xPos += 4;
		}

		//Melee info
		eng->renderer->drawText("MLE:", renderArea_characterLines, xPos, yPos, clrGray);
		xPos += 4;

		if(weapon->getInstanceDefinition().isMeleeWeapon == true) {
			//Weapon damage and skill used
			abilityUsed = itemDef->meleeAbilityUsed;
			dmgRolls = itemDef->meleeDmg.rolls;
			dmgSides = itemDef->meleeDmg.sides;
			dmgPlus = itemDef->meleeDmg.plus;

			//Damage
			wpnDmgStr = intToString(dmgRolls) + "d" + intToString(dmgSides);
			wpnDmgStr += dmgPlus > 0 ? "+" + intToString(dmgPlus) : (dmgPlus < 0 ? "-" + intToString(dmgPlus) : "");

			//Total attack skill with weapon (base + actor skill)
			actorAttackSkill = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(abilityUsed, true);
			baseAttackSkill = itemDef->meleeBaseAttackSkill;
			totalAttackSkill = baseAttackSkill + actorAttackSkill;
			wpnSkillStr = intToString(totalAttackSkill) + "%";

			//Weapon health?

			eng->renderer->drawText(wpnDmgStr, renderArea_characterLines, xPos, yPos, clrWhite);
			xPos += 1 + wpnDmgStr.size();

			eng->renderer->drawText(wpnSkillStr, renderArea_characterLines, xPos, yPos, clrWhite);
			xPos += 1 + wpnSkillStr.size();
		} else {
			eng->renderer->drawText("N/A", renderArea_characterLines, xPos, yPos, clrWhite);
			xPos += 4;
		}
	}

	//Turn
	//    const string turn = intToString( eng->gameTime->getTurn() );
	//    eng->renderer->drawText            ("TURN",       INTERFACE_X_POS_LEFT,               y_pos, clrGrayLight);
	//    eng->renderer->drawTextAlignRight  (turn,          MAINSCREEN_X_OFFSET - CELL_WIDTH,   y_pos, clrGrayLight);


	//Dungeon level
	xPos = CHARACTER_LINE_X0;
	yPos += 1;
	eng->renderer->drawText("DLVL:", renderArea_characterLines, xPos, yPos, clrGray);
	xPos += 5;
	const int DLVL = eng->map->getDungeonLevel();
	str = DLVL >= 0 ? intToString(DLVL) : "?";
	eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);

	DungeonMaster* const dm = eng->dungeonMaster;

	//Level and xp
	xPos += str.size() + 1;
	str = "LVL:" + intToString(dm->getLevel());
	eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrGreenLight);
	xPos += str.size() + 1;
	str = "NXT:" + intToString(dm->getXpToNextLvl() - dm->getXp());
	eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrGreenLight);
	xPos += str.size() + 1;

	//Armor
	eng->renderer->drawText("ARM:", renderArea_characterLines, xPos, yPos, clrGray);
	xPos += 4;
	const Item* const armor = eng->player->getInventory()->getItemInSlot(slot_armorBody);
	if(armor == NULL) {
		eng->renderer->drawText("N/A", renderArea_characterLines, xPos, yPos, clrWhite);
		xPos += 4;
	} else {
		str = dynamic_cast<const Armor*>(armor)->getArmorDataLine();
		eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);
		xPos += str.length() + 1;
	}

	//Missile weapon
	xPos = X_POS_MISSILE;
	eng->renderer->drawText("MIS:", renderArea_characterLines, xPos, yPos, clrGray);
	xPos += 4;

	Item* const item = eng->player->getInventory()->getItemInSlot(slot_missiles);
	if(item == NULL) {
		eng->renderer->drawText("N/A", renderArea_characterLines, xPos, yPos, clrWhite);
	} else {
		const ItemDefinition* itemDef = &(item->getInstanceDefinition());

		//Dmg
		DiceParam dmg = itemDef->missileDmg;

		string wpnDmgStr = intToString(dmg.rolls) + "d" + intToString(dmg.sides);
		wpnDmgStr += dmg.plus > 0 ? "+" + intToString(dmg.plus) : (dmg.plus < 0 ? "-" + intToString(dmg.plus) : "");

		//Total attack skill with item (base + actor skill)
		const int actorAttackSkill = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(ability_accuracyRanged, true);
		const int baseAttackSkill = itemDef->missileBaseAttackSkill;
		const int totalAttackSkill = baseAttackSkill + actorAttackSkill;
		const string wpnSkillStr = intToString(totalAttackSkill) + "%";

		eng->renderer->drawText(wpnDmgStr, renderArea_characterLines, xPos, yPos, clrWhite);
		xPos += 1 + wpnDmgStr.size();

		eng->renderer->drawText(wpnSkillStr, renderArea_characterLines, xPos, yPos, clrWhite);
		xPos += 1 + wpnSkillStr.size();

		eng->renderer->drawText(intToString(item->numberOfItems), renderArea_characterLines, xPos, yPos, clrRedLight);
	}

	yPos += 1;
	xPos = CHARACTER_LINE_X0;

	const string statusLine = eng->player->getStatusEffectsHandler()->getStatusLine();
	eng->renderer->drawText(statusLine, renderArea_characterLines, xPos, yPos, clrWhite);

	// Turn number
	str = "TRN:" + intToString(eng->gameTime->getTurn());
	xPos = MAP_X_CELLS - str.size() - 1;
	eng->renderer->drawText(str, renderArea_characterLines, xPos, yPos, clrWhite);
}

