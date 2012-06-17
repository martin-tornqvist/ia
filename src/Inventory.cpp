#include "Inventory.h"

#include "ItemWeapon.h"
#include "Engine.h"
#include "ItemDrop.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "GameTime.h"
#include "Render.h"
#include "ItemFactory.h"

Inventory::Inventory(bool humanoid) {
	InventorySlot invSlot;

	if(humanoid == true) {
		invSlot.devName = slot_wielded;
		invSlot.interfaceName = "Wielding";
		invSlot.allowWieldedWeapon = true;
		m_slots.push_back(invSlot);
		invSlot.allowWieldedWeapon = false;

		invSlot.devName = slot_wieldedAlt;
		invSlot.interfaceName = "Prepared";
		invSlot.allowWieldedWeapon = true;
		m_slots.push_back(invSlot);
		invSlot.allowWieldedWeapon = false;

		invSlot.devName = slot_missiles;
		invSlot.interfaceName = "Missiles";
		invSlot.allowMissile = true;
		m_slots.push_back(invSlot);
		invSlot.allowMissile = false;

		invSlot.devName = slot_armorBody;
		invSlot.interfaceName = "On body";
		invSlot.allowArmor = true;
		m_slots.push_back(invSlot);
		invSlot.allowArmor = false;

		//		invSlot.devName = slot_cloak;
		//		invSlot.interfaceName = "Cloak";
		//		invSlot.allowCloak = true;
		//		m_slots.push_back(invSlot);
		//		invSlot.allowCloak = false;

		//		invSlot.devName = slot_amulet;
		//		invSlot.interfaceName = "Amulet";
		//		invSlot.allowAmulet = true;
		//		m_slots.push_back(invSlot);
		//		invSlot.allowAmulet = false;

		//		invSlot.devName = slot_ringLeft;
		//		invSlot.interfaceName = "Left ring";
		//		invSlot.allowRing = true;
		//		m_slots.push_back(invSlot);
		//		invSlot.devName = slot_ringRight;
		//		invSlot.interfaceName = "Right ring";
		//		m_slots.push_back(invSlot);
	}
}

void Inventory::addSaveLines(vector<string>& lines) const {
	for(unsigned int i = 0; i < m_slots.size(); i++) {
		Item* const item = m_slots.at(i).item;
		if(item == NULL) {
			lines.push_back("0");
		} else {
			lines.push_back(intToString(item->getInstanceDefinition().devName));
			lines.push_back(intToString(item->numberOfItems));
			item->itemSpecificAddSaveLines(lines);
		}
	}

	lines.push_back(intToString(m_general.size()));
	for(unsigned int i = 0; i < m_general.size(); i++) {
		Item* const item = m_general.at(i);
		lines.push_back(intToString(item->getInstanceDefinition().devName));
		lines.push_back(intToString(item->numberOfItems));
		item->itemSpecificAddSaveLines(lines);
	}
}

void Inventory::setParametersFromSaveLines(vector<string>& lines, Engine* const engine) {
	for(unsigned int i = 0; i < m_slots.size(); i++) {
		//Previous item is destroyed
		Item* item = m_slots.at(i).item;
		if(item != NULL) {
			delete item;
			m_slots.at(i).item = NULL;
		}

		const ItemDevNames_t devName = static_cast<ItemDevNames_t>(stringToInt(lines.front()));
		lines.erase(lines.begin());
		if(devName != item_empty) {
			item = engine->itemFactory->spawnItem(devName);
			item->numberOfItems = stringToInt(lines.front());
			lines.erase(lines.begin());
			item->itemSpecificSetParametersFromSaveLines(lines);

			m_slots.at(i).item = item;
		}
	}

	while(m_general.size() != 0) {
		deleteItemInGeneral(0);
	}

	const unsigned int NR_OF_GENERAL = stringToInt(lines.front());
	lines.erase(lines.begin());
	for(unsigned int i = 0; i < NR_OF_GENERAL; i++) {
		const ItemDevNames_t devName = static_cast<ItemDevNames_t>(stringToInt(lines.front()));
		lines.erase(lines.begin());
		Item* item = engine->itemFactory->spawnItem(devName);
		item->numberOfItems = stringToInt(lines.front());
		lines.erase(lines.begin());
		item->itemSpecificSetParametersFromSaveLines(lines);
		m_general.push_back(item);
	}
}

bool Inventory::hasDynamiteInGeneral() const {
	return hasItemInGeneral(item_dynamite);
}

bool Inventory::hasItemInGeneral(const ItemDevNames_t devName) const {
	for(unsigned int i = 0; i < m_general.size(); i++) {
		if(m_general.at(i)->getInstanceDefinition().devName == devName)
			return true;
	}

	return false;
}

int Inventory::getItemStackSizeInGeneral(const ItemDevNames_t devName) const {
	for(unsigned int i = 0; i < m_general.size(); i++) {
		if(m_general.at(i)->getInstanceDefinition().devName == devName) {
			if(m_general.at(i)->getInstanceDefinition().isStackable == false) {
				return 1;
			} else {
				return m_general.at(i)->numberOfItems;
			}
		}
	}

	return 0;
}

void Inventory::decreaseDynamiteInGeneral() {
	for(unsigned int i = 0; i < m_general.size(); i++) {
		if(m_general.at(i)->getInstanceDefinition().devName == item_dynamite) {
			decreaseItemInGeneral(i);
			break;
		}
	}
}

/*
 bool Inventory::hasFirstAidInGeneral()
 {
 for(unsigned int i = 0; i < m_general.size(); i++) {
 if(m_general.at(i)->getInstanceDefinition().devName == item_firstAidKit)
 return true;
 }

 return false;
 }

 void Inventory::decreaseFirstAidInGeneral()
 {
 for(unsigned int i = 0; i < m_general.size(); i++) {
 if(m_general.at(i)->getInstanceDefinition().devName == item_firstAidKit) {
 decreaseItemInGeneral(i);
 break;
 }
 }
 }
 */

void Inventory::putItemInGeneral(Item* item) {
	bool stackedItem = false;

	//If item stacks, see if there is other items of same type
	if(item->getInstanceDefinition().isStackable == true) {

		const int stackIndex = getElementToStackItem(item);

		if(stackIndex != -1) {
			Item* compareItem = m_general.at(stackIndex);

			//Keeping picked up item and destroying the one in the inventory,
			//to keep the parameter pointer valid.
			item->numberOfItems += compareItem->numberOfItems;
			delete compareItem;
			m_general.at(stackIndex) = item;
			stackedItem = true;
		}
	}

	if(stackedItem == false) {
		m_general.push_back(item);
	}
}

int Inventory::getElementToStackItem(Item* item) const {
	if(item->getInstanceDefinition().isStackable == true) {
		for(unsigned int i = 0; i < m_general.size(); i++) {
			Item* compare = m_general.at(i);

			if(compare->getInstanceDefinition().devName == item->getInstanceDefinition().devName) {
				return i;
			}
		}
	}

	return -1;
}

void Inventory::dropAllNonIntrinsic(const coord pos, const bool rollForDestruction, Engine* engine) {
	Item* item;

	//Drop from slots
	for(unsigned int i = 0; i < m_slots.size(); i++) {
		item = m_slots.at(i).item;
		if(item != NULL) {
			if(rollForDestruction && engine->dice(1, 100) < CHANCE_TO_DESTROY_COMMON_ITEMS_ON_DROP) {
				delete m_slots.at(i).item;
			} else {
				engine->itemDrop->dropItemOnMap(pos, &item);
			}

			m_slots.at(i).item = NULL;
		}
	}

	//Drop from general
	unsigned int i = 0;
	while(i < m_general.size()) {
		item = m_general.at(i);
		if(item != NULL) {
			if(rollForDestruction && engine->dice(1, 100) < CHANCE_TO_DESTROY_COMMON_ITEMS_ON_DROP) {
				delete m_general.at(i);
			} else {
				engine->itemDrop->dropItemOnMap(pos, &item);
			}

			m_general.erase(m_general.begin() + i);
		}
		i++;
	}
}

bool Inventory::hasAmmoForFirearmInInventory() {
	Weapon* weapon = dynamic_cast<Weapon*>(getItemInSlot(slot_wielded));

	//If weapon found
	if(weapon != NULL) {

		//If weapon has infinite ammo, return true but with a warning
		if(weapon->getInstanceDefinition().rangedHasInfiniteAmmo == true) {
			cout << "[WARNING] Inventory::hasAmmoForFirearm...()" << "ran on weapon with infinite ammo" << endl;

			return true;
		}

		//If weapon is a firearm
		if(weapon->getInstanceDefinition().isRangedWeapon == true) {

			//Get weapon ammo type
			const ItemDevNames_t ammoDevName = weapon->getInstanceDefinition().rangedAmmoTypeUsed;

			//Look for that ammo type in inventory
			for(unsigned int i = 0; i < m_general.size(); i++) {
				if(m_general.at(i)->getInstanceDefinition().devName == ammoDevName) {
					return true;
				}
			}
		}
	}
	return false;
}

void Inventory::decreaseItemInSlot(SlotTypes_t slotName) {
	Item* item = getItemInSlot(slotName);
	bool stack = item->getInstanceDefinition().isStackable;
	bool deleteItem = true;

	if(stack == true) {
		item->numberOfItems -= 1;

		if(item->numberOfItems > 0) {
			deleteItem = false;
		}
	}

	if(deleteItem == true) {
		getSlot(slotName)->item = NULL;
		delete item;
	}
}

void Inventory::deleteItemInGeneral(unsigned element) {
	if(m_general.size() > 0) {
		delete m_general.at(element);
		m_general.erase(m_general.begin() + element);
	}
}

void Inventory::decreaseItemInGeneral(unsigned element) {
	Item* item = m_general.at(element);
	bool stack = item->getInstanceDefinition().isStackable;
	bool deleteItem = true;

	if(stack == true) {
		item->numberOfItems -= 1;

		if(item->numberOfItems > 0) {
			deleteItem = false;
		}
	}

	if(deleteItem == true) {
		m_general.erase(m_general.begin() + element);

		delete item;
	}
}

void Inventory::decreaseItemTypeInGeneral(const ItemDevNames_t devName) {
	for(unsigned int i = 0; i < m_general.size(); i++) {
		if(m_general.at(i)->getInstanceDefinition().devName == devName) {
			decreaseItemInGeneral(i);
			return;
		}
	}
}

void Inventory::moveItemToSlot(InventorySlot* inventorySlot, const unsigned int generalInventoryElement) {
	bool generalSlotExists = generalInventoryElement < m_general.size();
	Item* item = NULL;
	Item* slotItem = inventorySlot->item;

	if(generalSlotExists == true) {
		item = m_general.at(generalInventoryElement);
	}

	if(generalSlotExists == true && item != NULL) {
		if(slotItem == NULL) {
			inventorySlot->item = item;
			m_general.erase(m_general.begin() + generalInventoryElement);
		} else {
			m_general.erase(m_general.begin() + generalInventoryElement);
			m_general.push_back(slotItem);
			inventorySlot->item = item;
		}
	}
}

void Inventory::equipGeneralItemAndEndTurn(const unsigned int generalInventoryElement, const InventoryPurpose_t purpose, Engine* engine) {
	const bool IS_PLAYER = this == engine->player->getInventory();

	bool isEndingTurn = true;
	if(IS_PLAYER) {
		const AbilityRollResult_t rollResult = engine->abilityRoll->roll(engine->player->getInstanceDefinition()->abilityValues.getAbilityValue(
		      ability_weaponHandling, true));
		isEndingTurn = rollResult < successSmall;
	}

	Item* item = m_general.at(generalInventoryElement);
    const ItemDefinition& d = item->getInstanceDefinition();

//	if(d.isMeleeWeapon || d.isRangedWeapon) {
    if(purpose == inventoryPurpose_wieldWear && (d.isMeleeWeapon || d.isRangedWeapon)) {
		Item* const itemBefore = getItemInSlot(slot_wielded);
		moveItemToSlot(getSlot(slot_wielded), generalInventoryElement);
		Item* const itemAfter = getItemInSlot(slot_wielded);
		if(IS_PLAYER == true) {
			if(itemBefore != NULL) {
				engine->log->addMessage("You were wielding " + engine->itemData->itemInterfaceName(itemBefore, true) + ".");
			}
			const SDL_Color clr = isEndingTurn ? clrWhite : clrMagenta;
			engine->log->addMessage("You are now wielding " + engine->itemData->itemInterfaceName(itemAfter, true) + ".", clr);
		}
	}

	if(purpose == inventoryPurpose_wieldWear && d.isArmor) {
		Item* const itemBefore = getItemInSlot(slot_armorBody);
		moveItemToSlot(getSlot(slot_armorBody), generalInventoryElement);
		Item* const itemAfter = getItemInSlot(slot_armorBody);
		if(IS_PLAYER == true) {
			if(itemBefore != NULL) {
				engine->log->addMessage("You wore " + engine->itemData->itemInterfaceName(itemBefore, true) + ".");
			}
			engine->log->addMessage("You are now wearing " + engine->itemData->itemInterfaceName(itemAfter, true) + ".");
		}
		isEndingTurn = true;
	}

	if(purpose == inventoryPurpose_missileSelect) {
		Item* const itemBefore = getItemInSlot(slot_missiles);
		moveItemToSlot(getSlot(slot_missiles), generalInventoryElement);
		Item* const itemAfter = getItemInSlot(slot_missiles);
		if(IS_PLAYER == true) {
			if(itemBefore != NULL) {
				engine->log->addMessage("You were using " + engine->itemData->itemInterfaceName(itemBefore, true) + " as missile weapon.");
			}
			const SDL_Color clr = isEndingTurn ? clrWhite : clrMagenta;
			engine->log->addMessage("You are now using " + engine->itemData->itemInterfaceName(itemAfter, true) + " as missile weapon.", clr);
		}
	}
	if(isEndingTurn) {
		engine->gameTime->letNextAct();
	}
}

void Inventory::equipGeneralItemToAltAndEndTurn(const unsigned int generalInventoryElement, Engine* engine) {
	const AbilityRollResult_t rollResult = engine->abilityRoll->roll(engine->player->getInstanceDefinition()->abilityValues.getAbilityValue(
	      ability_weaponHandling, true));
	const bool IS_FREE_TURN_DUE_TO_SKILL = rollResult >= successSmall;
	const SDL_Color clr = IS_FREE_TURN_DUE_TO_SKILL ? clrMagenta : clrWhite;

	Item* const itemBefore = getItemInSlot(slot_wieldedAlt);
	moveItemToSlot(getSlot(slot_wieldedAlt), generalInventoryElement);
	Item* const itemAfter = getItemInSlot(slot_wieldedAlt);

	engine->renderer->drawMapAndInterface();

	if(itemBefore != NULL) {
		engine->log->addMessage("You were using " + engine->itemData->itemInterfaceName(itemBefore, true) + " as a prepared weapon.", clr);
	}
	engine->log->addMessage("You are now using " + engine->itemData->itemInterfaceName(itemAfter, true) + " as a prepared weapon.", clr);

	if(IS_FREE_TURN_DUE_TO_SKILL == false) {
		engine->gameTime->letNextAct();
	}
}

void Inventory::swapWieldedAndPrepared(const bool END_TURN, Engine* engine) {
	InventorySlot* slot1 = getSlot(slot_wielded);
	InventorySlot* slot2 = getSlot(slot_wieldedAlt);
	Item* item1 = slot1->item;
	Item* item2 = slot2->item;
	slot1->item = item2;
	slot2->item = item1;

	if(END_TURN) {
		engine->gameTime->letNextAct();
	}
}

void Inventory::moveItemFromGeneralToIntrinsics(const unsigned int generalInventoryElement) {
	bool generalSlotExists = generalInventoryElement < m_general.size();

	if(generalSlotExists == true) {
		Item* item = m_general.at(generalInventoryElement);
		bool itemExistsInGeneralSlot = item != NULL;

		if(itemExistsInGeneralSlot == true) {
			m_intrinsics.push_back(item);
			m_general.erase(m_general.begin() + generalInventoryElement);
		}
	}
}

bool Inventory::moveItemToGeneral(InventorySlot* inventorySlot) {
	Item* const item = inventorySlot->item;
	if(item == NULL) {
		return false;
	} else {
		inventorySlot->item = NULL;
		putItemInGeneral(item);
		return true;
	}
}

bool Inventory::hasItemInSlot(SlotTypes_t slotName) {
	for(unsigned int i = 0; i < m_slots.size(); i++) {
		if(m_slots[i].devName == slotName) {
			if(m_slots[i].item != NULL) {
				return true;
			}
		}
	}

	return false;
}

void Inventory::removeItemInElementWithoutDeletingInstance(int number) {
	//If number corresponds to equiped slots, remove item in that slot
	if(number >= 0 && number < signed(m_slots.size())) {
		m_slots.at(number).item = NULL;
	} else {
		//If number corresponds to general slot, remove that slot
		number -= m_slots.size();
		if(number >= 0 && number < signed(m_general.size())) {
			m_general.erase(m_general.begin() + number);
		}
	}
}

int Inventory::getElementWithItemType(const ItemDevNames_t itemDevName) const {
	for(unsigned int i = 0; i < m_general.size(); i++) {
		if(m_general.at(i)->getInstanceDefinition().devName == itemDevName) {
			return i;
		}
	}
	return -1;
}

Item* Inventory::getItemInElement(int number) {
	//If number corresponds to equiped slots, return item in that slot
	if(number >= 0 && number < signed(m_slots.size())) {
		return m_slots.at(number).item;
	}

	//If number corresponds to general slot, return that item
	number -= m_slots.size();
	if(number >= 0 && number < signed(m_general.size())) {
		return m_general.at(number);
	}

	return NULL;
}

Item* Inventory::getItemInSlot(SlotTypes_t slotName) {
	if(hasItemInSlot(slotName) == true) {
		for(unsigned int i = 0; i < m_slots.size(); i++) {
			if(m_slots[i].devName == slotName) {
				return m_slots[i].item;
				break;
			}
		}
	}

	return NULL;
}

Item* Inventory::getIntrinsicInElement(int element) {
	if(getIntrinsicsSize() > element)
		return m_intrinsics[element];

	return NULL;
}

void Inventory::putItemInIntrinsics(Item* item) {
	if(item->getInstanceDefinition().isIntrinsicWeapon == true)
		m_intrinsics.push_back(item);
	else cout << "[ERROR] ITEM IS NOT AN INTRINSIC - " << "IN putItemInIntrinsics()" << endl;
}

Item* Inventory::getLastItemInGeneral() {
	int s = m_general.size();

	if(s != 0)
		return m_general.at(m_general.size() - 1);

	return NULL;
}

InventorySlot* Inventory::getSlot(SlotTypes_t slotName) {
	InventorySlot* slot = NULL;

	for(unsigned int i = 0; i < m_slots.size(); i++) {
		if(m_slots[i].devName == slotName) {
			slot = &m_slots[i];
		}
	}
	return slot;
}

void Inventory::putItemInSlot(SlotTypes_t slotName, Item* item, bool putInGeneral_ifOccupied, bool putInGeneral_ifSlotNotFound) {
	bool hasSlot = false;

	for(unsigned int i = 0; i < m_slots.size(); i++) {
		if(m_slots[i].devName == slotName) {
			hasSlot = true;
			if(m_slots[i].item == NULL)
				m_slots[i].item = item;
			else if(putInGeneral_ifOccupied == true)
				m_general.push_back(item);
		}
	}

	if(putInGeneral_ifSlotNotFound == true && hasSlot == false)
		m_general.push_back(item);

	//cout << "ID: " << item->id << endl;
}

int Inventory::getTotalItemWeight() const {
    int weight = 0;
    for(unsigned int i = 0; i < m_slots.size(); i++) {
        if(m_slots.at(i).item != NULL) {
            weight += m_slots.at(i).item->getWeight();
        }
    }
    for(unsigned int i = 0; i < m_general.size(); i++) {
        weight += m_general.at(i)->getWeight();
    }
    return weight;
}
