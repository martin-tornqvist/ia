#include "InventoryHandler.h"

#include "Engine.h"

#include "ItemUsable.h"
#include "ItemScroll.h"
#include "ItemExplosive.h"
#include "ActorPlayer.h"
#include "ItemPotion.h"
#include "InventoryIndexes.h"
#include "MenuBrowser.h"
#include "Log.h"
#include "RenderInventory.h"
#include "MenuInputHandler.h"
#include "Render.h"
#include "ItemDrop.h"

void InventoryHandler::activateItem(const InventoryPurpose_t purpose, const unsigned int genSlot) {
	Inventory* const playerInv = eng->player->getInventory();
	Item* item = playerInv->getGeneral()->at(genSlot);

	bool decrease = true;

	switch(purpose) {
	case inventoryPurpose_use: {
		const ItemActivateReturn_t activateReturnValue = dynamic_cast<ItemUsable*>(item)->use(eng->player, eng);
		decrease = activateReturnValue == itemActivate_destroyed;
	}
	break;
	case inventoryPurpose_readyExplosive: {
		dynamic_cast<Explosive*>(item)->setPlayerExplosive(eng);
	}
	case inventoryPurpose_eat: {
	}
	break;
	case inventoryPurpose_quaff: {
		dynamic_cast<Potion*>(item)->quaff(eng->player, eng);
	}
	break;
	default: {
		cout << "[ERROR] Bad purpose in InventoryHandler::activateItem()" << endl;
	}
	break;
	}

	if(decrease == true) {
		playerInv->decreaseItemInGeneral(genSlot);
	}
}

void InventoryHandler::showGeneralItemsFiltered(const GeneralInventoryFilters_t filter) {
	Inventory* inv = eng->player->getInventory();
	vector<Item*>* general = inv->getGeneral();

	//generalItemsToShow is a vector for storing element numbers corresponding to the players general items vector
	generalItemsToShow.resize(0);

	const ItemDefinition* curDef = NULL;

	for(unsigned int i = 0; i < general->size(); i++) {
		curDef = &(general->at(i)->getInstanceDefinition());
		switch(filter) {
		case generalInventoryFilter_wieldableAndWearable: {
			if(curDef->isMeleeWeapon || curDef->isRangedWeapon || curDef->isAmulet || curDef->isArmor || curDef->isCloak || curDef->isRing)
				generalItemsToShow.push_back(i);
		}
		break;

		case generalInventoryFilter_wieldable: {
			if(curDef->isMeleeWeapon || curDef->isRangedWeapon)
				generalItemsToShow.push_back(i);
		}
		break;

		case generalInventoryFilter_explosives: {
			if(curDef->isExplosive)
				generalItemsToShow.push_back(i);
		}
		break;

		case generalInventoryFilter_missile: {
			if(curDef->isMissileWeapon)
				generalItemsToShow.push_back(i);
		}
		break;

		case generalInventoryFilter_usable: {
			if(curDef->isUsable)
				generalItemsToShow.push_back(i);
		}
		break;

		case generalInventoryFilter_quaffable: {
			if(curDef->isQuaffable)
				generalItemsToShow.push_back(i);
		}
		break;

		case generalInventoryFilter_eatable: {
			if(curDef->isEatable)
				generalItemsToShow.push_back(i);
		}
		break;
		}
	}
}

void InventoryHandler::showAllGeneralItems() {
	Inventory* inv = eng->player->getInventory();
	vector<Item*>* general = inv->getGeneral();
	int generalSize = general->size();

	generalItemsToShow.resize(0);

	//Store all vector elements
	for(int i = 0; i < generalSize; i++) {
		generalItemsToShow.push_back(i);
	}
}

InventorySlot* InventoryHandler::getSlotPressed(const char charIndex) {
	if(charIndex < 'a' || charIndex > playerSlotButtons.back().key) {
		return NULL;
	}
	return playerSlotButtons.at(charIndex - 'a').inventorySlot;
}

void InventoryHandler::updatePlayerGeneralSlotButtons(const InventoryPurpose_t purpose) {
	bool areSlotsShown = false;

	if(purpose == inventoryPurpose_look || purpose == inventoryPurpose_selectDrop) {
		showAllGeneralItems();
		areSlotsShown = true;
	}

	if(purpose == inventoryPurpose_wieldWear) {
		showGeneralItemsFiltered(generalInventoryFilter_wieldableAndWearable);
		areSlotsShown = false;
	}

	if(purpose == inventoryPurpose_wieldAlt) {
		showGeneralItemsFiltered(generalInventoryFilter_wieldable);
		areSlotsShown = false;
	}

	if(purpose == inventoryPurpose_readyExplosive) {
		showGeneralItemsFiltered(generalInventoryFilter_explosives);
		areSlotsShown = false;
	}

	if(purpose == inventoryPurpose_missileSelect) {
		showGeneralItemsFiltered(generalInventoryFilter_missile);
		areSlotsShown = false;
	}

	if(purpose == inventoryPurpose_use) {
		showGeneralItemsFiltered(generalInventoryFilter_usable);
		areSlotsShown = false;
	}

	if(purpose == inventoryPurpose_eat) {
		showGeneralItemsFiltered(generalInventoryFilter_eatable);
		areSlotsShown = false;
	}

	if(purpose == inventoryPurpose_quaff) {
		showGeneralItemsFiltered(generalInventoryFilter_quaffable);
	}

	eng->inventoryIndexes->setIndexes(areSlotsShown, playerSlotButtons.size(), generalItemsToShow);
}

void InventoryHandler::initPlayerSlotButtons() {
	vector<InventorySlot>* invSlots = eng->player->getInventory()->getSlots();
	char key = 'a';

	InventorySlotButton inventorySlotButton;

	for(unsigned int i = 0; i < invSlots->size(); i++) {
		inventorySlotButton.inventorySlot = &invSlots->at(i);
		inventorySlotButton.key = key;
		key++;
		playerSlotButtons.push_back(inventorySlotButton);
	}
}

void InventoryHandler::runPlayerInventory(InventoryPurpose_t purpose) {
	updatePlayerGeneralSlotButtons(purpose);
	const unsigned int NR_OF_SLOTS = static_cast<int>(playerSlotButtons.size());

	bool done = false;

	const int NR_ITEMS_FIRST = purpose == inventoryPurpose_selectDrop ? NR_OF_SLOTS : generalItemsToShow.size();
	const int NR_ITEMS_SECOND = purpose == inventoryPurpose_selectDrop ? generalItemsToShow.size() : 0;
	MenuBrowser browser(NR_ITEMS_FIRST, NR_ITEMS_SECOND);

	const bool DRAW_BROWSER = purpose != inventoryPurpose_look;

	if(generalItemsToShow.size() == 0 && (
	         purpose == inventoryPurpose_use ||
	         purpose == inventoryPurpose_quaff ||
	         purpose == inventoryPurpose_eat ||
	         purpose == inventoryPurpose_wieldWear ||
	         purpose == inventoryPurpose_wieldAlt ||
	         purpose == inventoryPurpose_readyExplosive ||
	         purpose == inventoryPurpose_missileSelect)) {
		done = true;

		if(purpose == inventoryPurpose_use)
			eng->log->addMessage("I have nothing to use.");
		if(purpose == inventoryPurpose_quaff)
			eng->log->addMessage("I have nothing to drink.");
		if(purpose == inventoryPurpose_eat)
			eng->log->addMessage("I have nothing to eat.");
		if(purpose == inventoryPurpose_wieldWear || purpose == inventoryPurpose_wieldAlt)
			eng->log->addMessage("I have nothing to wield or wear.");
		if(purpose == inventoryPurpose_readyExplosive)
			eng->log->addMessage("I have no explosives.");
		if(purpose == inventoryPurpose_missileSelect)
			eng->log->addMessage("I have nothing to use as a thrown weapon.");
	} else {
		eng->renderInventory->draw(purpose, browser, true, DRAW_BROWSER);
	}

	while(done == false) {
		const MenuAction_t action = eng->menuInputHandler->getAction(browser);

		if(action == menuAction_browsed) {
			eng->renderInventory->draw(purpose, browser, false, DRAW_BROWSER);
		}

		if(action == menuAction_canceled) {
			eng->log->clearLog();
			eng->renderer->drawMapAndInterface();
			done = true;
			return;
		}

		if(action == menuAction_selected) {
			switch(purpose) {
			case inventoryPurpose_selectDrop: {
				//Equipment slot?
				if(browser.getPos().x == 0) {
					Item* item = eng->player->getInventory()->getSlots()->at(browser.getPos().y).item;
					if(item != NULL) {
						eng->log->clearLog();
						eng->itemDrop->dropItemFromInventory(eng->player, browser.getPos().y);
						eng->renderer->drawMapAndInterface();
						return;
					}
				}
				//General slot?
				if(browser.getPos().x == 1) {
					const unsigned int DROP_ELEMENT = NR_OF_SLOTS + browser.getPos().y;
					eng->log->clearLog();
					eng->itemDrop->dropItemFromInventory(eng->player, DROP_ELEMENT);
					eng->renderer->drawMapAndInterface();
					return;
				}
			}
			break;
			case inventoryPurpose_eat:
			case inventoryPurpose_use:
			case inventoryPurpose_quaff:
			case inventoryPurpose_readyExplosive: {
				eng->log->clearLog();
				activateItem(purpose, generalItemsToShow.at(browser.getPos().y));
				return;
			}
			break;
			case inventoryPurpose_wieldWear:
			case inventoryPurpose_missileSelect: {
				eng->log->clearLog();
				eng->player->getInventory()->equipGeneralItemAndEndTurn(generalItemsToShow.at(browser.getPos().y), purpose, eng);
				return;
			}
			break;
			case inventoryPurpose_wieldAlt: {
				eng->log->clearLog();
				eng->player->getInventory()->equipGeneralItemToAltAndEndTurn(generalItemsToShow.at(browser.getPos().y), eng);
				return;
			}
			break;
			default: {
			}
			break;
			}
		}
	}

	eng->renderer->drawMapAndInterface();
}

void InventoryHandler::swapItems(Item** item1, Item** item2) {
	Item* buffer = *item1;
	*item1 = *item2;
	*item2 = buffer;
}

