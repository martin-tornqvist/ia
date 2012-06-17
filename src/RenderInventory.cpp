#include "RenderInventory.h"

#include "Engine.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "InventoryIndexes.h"
#include "Log.h"
#include "Render.h"

RenderInventory::RenderInventory(Engine* engine) :
	eng(engine), xPosListsLeft1(1), xPosListsLeft2(xPosListsLeft1 + 11), xPosListsLeft3(xPosListsLeft2 + 2), xPosListsRight1(1), xPosListsRight2(
	   xPosListsRight1 + 3), xPosListsRightStandardOffset(xPosListsLeft3 + 27), yPosLists(0) {
}

string RenderInventory::getWeaponDataLine(Weapon* const weapon) const {
	string line = "";

	string wpnDmgStr, wpnSkillStr;
	Abilities_t abilityUsed;
	int dmgRolls, dmgSides, dmgPlus, actorAttackSkill, baseAttackSkill, totalAttackSkill;

	const ItemDefinition* itemDef = &(weapon->getInstanceDefinition());

	//Firearm info
	line = "F: ";

	if(itemDef->isRangedWeapon == true) {
		//Weapon damage and skill used
		const int multiplier = itemDef->isMachineGun == true ? NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST : 1;
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

		line += wpnDmgStr + " ";

		//Total attack skill with weapon (base + actor skill)
		abilityUsed = itemDef->rangedAbilityUsed;
		actorAttackSkill = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(abilityUsed, true);
		baseAttackSkill = itemDef->rangedBaseAttackSkill;
		totalAttackSkill = baseAttackSkill + actorAttackSkill;
		wpnSkillStr = intToString(totalAttackSkill) + "%";

		line += wpnSkillStr + " ";
	} else {
		line += "N/A ";
	}

	for(unsigned int i = line.size(); i < 16; i++) {
		line += " ";
	}

	//Melee info
	line += "M: ";

	if(itemDef->isMeleeWeapon == true) {
		//Weapon damage and skill used

		dmgRolls = itemDef->meleeDmg.rolls;
		dmgSides = itemDef->meleeDmg.sides;
		dmgPlus = itemDef->meleeDmg.plus;

		//Damage
		wpnDmgStr = intToString(dmgRolls) + "d" + intToString(dmgSides);
		wpnDmgStr += dmgPlus > 0 ? "+" + intToString(dmgPlus) : (dmgPlus < 0 ? "-" + intToString(dmgPlus) : "");

		line += wpnDmgStr + " ";

		//Total attack skill with weapon (base + actor skill)
		abilityUsed = itemDef->meleeAbilityUsed;
		actorAttackSkill = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(abilityUsed, true);
		baseAttackSkill = itemDef->meleeBaseAttackSkill;
		totalAttackSkill = baseAttackSkill + actorAttackSkill;
		wpnSkillStr = intToString(totalAttackSkill) + "%";

		line += wpnSkillStr + " ";
	} else {
		line += "N/A ";
	}

	return line;
}

void RenderInventory::draw(const InventoryPurpose_t purpose, const MenuBrowser& browser, const bool DRAW_CMD_PROMPT, const bool DRAW_BROWSER) {
	vector<InventorySlotButton>* slotButtons = &eng->inventoryHandler->playerSlotButtons;

	vector<unsigned int>* generalItemsToShow = &eng->inventoryHandler->generalItemsToShow;

	string doneString, captionLeft, captionRight;
	string cmdString = "[a-";
	const string END_CHAR_INDEX(1, eng->inventoryIndexes->getLastCharIndex());
	cmdString += END_CHAR_INDEX + "]?";

	switch(purpose) {
	case inventoryPurpose_look: {
		const int INVENTORY_HEIGHT = max(generalItemsToShow->size(), slotButtons->size());

		if(DRAW_CMD_PROMPT == true) {
			eng->log->addMessage("Displaying inventory.", clrWhiteHigh);
		}

		eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsLeft1, yPosLists, MAP_X_CELLS - 1, INVENTORY_HEIGHT);

		drawSlots(slotButtons, browser, DRAW_BROWSER);
		drawGeneralItems(xPosListsRightStandardOffset, purpose, browser, DRAW_BROWSER);
	}
	break;

	case inventoryPurpose_selectDrop: {
		const int INVENTORY_HEIGHT = max(generalItemsToShow->size(), slotButtons->size());

		if(DRAW_CMD_PROMPT == true) {
			eng->log->addMessage("Drop which item", clrWhiteHigh);
			eng->log->addMessage(cmdString, clrWhiteHigh);
		}

		eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsLeft1, yPosLists, MAP_X_CELLS - 1, INVENTORY_HEIGHT);

		drawSlots(slotButtons, browser, DRAW_BROWSER);
		drawGeneralItems(xPosListsRightStandardOffset, purpose, browser, DRAW_BROWSER);
	}
	break;

	case inventoryPurpose_wieldWear: {
		const int INVENTORY_HEIGHT = generalItemsToShow->size();

		if(DRAW_CMD_PROMPT == true) {
			eng->log->addMessage("Wield or wear which item", clrWhiteHigh);
			eng->log->addMessage(cmdString, clrWhiteHigh);
		}

		eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsLeft1, yPosLists, xPosListsRightStandardOffset, INVENTORY_HEIGHT);

		drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
	}
	break;

	case inventoryPurpose_wieldAlt: {
		const int INVENTORY_HEIGHT = generalItemsToShow->size();

		if(DRAW_CMD_PROMPT == true) {
			eng->log->addMessage("Choose weapon to keep ready", clrWhiteHigh);
			eng->log->addMessage(cmdString, clrWhiteHigh);
		}

		eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsLeft1, yPosLists, xPosListsRightStandardOffset, INVENTORY_HEIGHT);

		drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
	}
	break;

	case inventoryPurpose_missileSelect: {
		const int INVENTORY_HEIGHT = generalItemsToShow->size();

		if(DRAW_CMD_PROMPT == true) {
			eng->log->addMessage("Use which item as thrown weapon", clrWhiteHigh);
			eng->log->addMessage(cmdString, clrWhiteHigh);
		}

		eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsLeft1, yPosLists, xPosListsRightStandardOffset, INVENTORY_HEIGHT);

		drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
	}
	break;

	case inventoryPurpose_use: {
		const int INVENTORY_HEIGHT = generalItemsToShow->size();

		if(DRAW_CMD_PROMPT == true) {
			eng->log->addMessage("Use which item", clrWhiteHigh);
			string endLetter = "a";
			endLetter[0] += static_cast<char>(generalItemsToShow->size()) - 1;
			eng->log->addMessage("[a-" + endLetter + "]?", clrWhiteHigh);
		}

		eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsRight1, yPosLists, xPosListsRightStandardOffset, INVENTORY_HEIGHT);

		drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
	}
	break;

//    case inventoryPurpose_read: {
//        const int INVENTORY_HEIGHT = generalItemsToShow->size();
//
//        if(DRAW_CMD_PROMPT == true) {
//            eng->log->addMessage("Study what text", clrWhiteHigh);
//            string endLetter = "a";
//            endLetter[0] += static_cast<char> (generalItemsToShow->size()) - 1;
//            eng->log->addMessage("[a-" + endLetter + "]?", clrWhiteHigh);
//        }
//
//        eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsRight1, yPosLists, xPosListsRightStandardOffset, INVENTORY_HEIGHT);
//
//        drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
//    }
//    break;

	case inventoryPurpose_readyExplosive: {
		const int INVENTORY_HEIGHT = generalItemsToShow->size();

		if(DRAW_CMD_PROMPT == true) {
			eng->log->addMessage("Ready what explosive", clrWhiteHigh);
			string endLetter = "a";
			endLetter[0] += static_cast<char>(generalItemsToShow->size()) - 1;
			eng->log->addMessage("[a-" + endLetter + "]?", clrWhiteHigh);
		}

		eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsRight1, yPosLists, xPosListsRightStandardOffset, INVENTORY_HEIGHT);

		drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
	}
	break;

	case inventoryPurpose_eat: {
		const int INVENTORY_HEIGHT = generalItemsToShow->size();

		if(DRAW_CMD_PROMPT == true) {
			eng->log->addMessage("Eat what", clrWhiteHigh);
			string endLetter = "a";
			endLetter[0] += static_cast<char>(generalItemsToShow->size()) - 1;
			eng->log->addMessage("[a-" + endLetter + "]?", clrWhiteHigh);
		}

		eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsRight1, yPosLists, xPosListsRightStandardOffset, INVENTORY_HEIGHT);

		drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
	}
	break;

	case inventoryPurpose_quaff: {
		const int INVENTORY_HEIGHT = generalItemsToShow->size();

		if(DRAW_CMD_PROMPT == true) {
			eng->log->addMessage("Drink what", clrWhiteHigh);
			string endLetter = "a";
			endLetter[0] += static_cast<char>(generalItemsToShow->size()) - 1;
			eng->log->addMessage("[a-" + endLetter + "]?", clrWhiteHigh);
		}

		eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, xPosListsRight1, yPosLists, xPosListsRightStandardOffset, INVENTORY_HEIGHT);

		drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
	}
	break;

	default:
		cout << "[ERROR] Wrong purpose in Interface::drawInventory()" << endl;
		break;

	}

	if(DRAW_CMD_PROMPT == true) {
		eng->log->addMessage("[Space/esc] Exit", clrWhiteHigh);
	}

	eng->renderer->flip();
}

void RenderInventory::drawSlots(vector<InventorySlotButton>* slotButtons, const MenuBrowser& browser, const bool DRAW_BROWSER) {
	InventorySlotButton* slotButton = NULL;
	InventorySlot* slot = NULL;
	char key = ' ';
	string slotString = "";

	int yPos = yPosLists;

	for(unsigned int i = 0; i < slotButtons->size(); i++) {
		slotButton = &(slotButtons->at(i));
		key = slotButton->key;
		slotString = key;
		slotString += ") ";
		slot = slotButton->inventorySlot;
		slotString += slot->interfaceName;

		const SDL_Color clr = DRAW_BROWSER && browser.getPos().x == 0 && browser.isPosAtKey('a' + i) ? clrWhite : clrRedLight;
		eng->renderer->drawText(slotString, renderArea_mainScreen, xPosListsLeft1, yPos, clr);
		slotString = ": ";
		eng->renderer->drawText(slotString, renderArea_mainScreen, xPosListsLeft2, yPos, clr);
		slotString = slot->item == NULL ? "(empty)" : eng->itemData->itemInterfaceName(slot->item, false);
		eng->renderer->drawText(slotString, renderArea_mainScreen, xPosListsLeft3, yPos, clr);

        if(slot->item != NULL) {
            const string itemWeightLabel = slot->item->getWeightLabel();
            if(itemWeightLabel != "") {
                eng->renderer->drawText(itemWeightLabel + "  ", renderArea_mainScreen, xPosListsRightStandardOffset - 4, yPos, clrGray);
            }
        }

		yPos += 1;
	}
}

void RenderInventory::drawGeneralItems(const int xPosOffset, const InventoryPurpose_t purpose, const MenuBrowser& browser, const bool DRAW_BROWSER) {
	vector<Item*>* generalItems = eng->player->getInventory()->getGeneral();
	vector<unsigned int>* generalItemsToShow = &(eng->inventoryHandler->generalItemsToShow);
	int currentElement = 0;

	string slotString = "";

	int yPos = yPosLists;

	for(unsigned int i = 0; i < generalItemsToShow->size(); i++) {
		currentElement = generalItemsToShow->at(i);
		slotString = eng->inventoryIndexes->getCharIndex(i);
		slotString += ") ";

		const bool ELEMENT_IS_SELECTED = ((browser.getNrOfItemsInFirstList() > 0 && browser.getPos().x == 1) ||
		                                  browser.getNrOfItemsInSecondList() == 0) &&
		                                 browser.getPos().y == static_cast<int>(i);
		const SDL_Color clr = DRAW_BROWSER && ELEMENT_IS_SELECTED ? clrWhite : clrRedLight;
		eng->renderer->drawText(slotString, renderArea_mainScreen, xPosOffset + xPosListsRight1, yPos, clr);
		slotString = eng->itemData->itemInterfaceName(generalItems->at(currentElement), false);
		eng->renderer->drawText(slotString, renderArea_mainScreen, xPosOffset + xPosListsRight2, yPos, clr);

		if(purpose == inventoryPurpose_wieldWear || purpose == inventoryPurpose_wieldAlt) {
			const ItemDefinition& itemDef = generalItems->at(currentElement)->getInstanceDefinition();
			if(itemDef.isRangedWeapon || itemDef.isMissileWeapon || itemDef.isMeleeWeapon) {
				const string weaponDataLine = getWeaponDataLine(dynamic_cast<Weapon*>(generalItems->at(currentElement)));
				string fill;
				fill.resize(0);
				const unsigned int FILL_SIZE = 29 - slotString.size();
				for(unsigned int i = 0; i < FILL_SIZE; i++) {
					fill.push_back('.');
				}
				eng->renderer->drawText(fill, renderArea_mainScreen, xPosOffset + xPosListsRight2 + slotString.size(), yPos, clrGray);
				const int x = 29;
				eng->renderer->drawText(weaponDataLine, renderArea_mainScreen, x, yPos, clrWhite);
			}
		}

		const string itemWeightLabel = generalItems->at(currentElement)->getWeightLabel();
		if(itemWeightLabel != "") {
            eng->renderer->drawText(itemWeightLabel + "  ", renderArea_mainScreen, MAP_X_CELLS - 5, yPos, clrGray);
		}

		yPos += 1;
	}
}

