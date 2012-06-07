#include "ItemPickup.h"

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "ItemAmmoClip.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Query.h"
#include "ItemDrop.h"
#include "ItemFactory.h"

//--------------ITEM PICKUP EFFECTS--------------
void ItemPickup::pickupEffects(Actor* actor, Item* item) {
	(void)actor;
	(void)item;
}

//Picking up items from ground. This function can always be called to check if
//something is there to be picked up.
//---------------------------------OBS------------
//beh�ver anpassas s� att AI:n kan k�ra funktionen �t monstren ocks�***
void ItemPickup::tryPick() {
	Item* const item = eng->map->items[eng->player->pos.x][eng->player->pos.y];

	if(item != NULL) {
		Inventory* const playerInventory = eng->player->getInventory();

		const string ITEM_NAME = eng->itemData->itemInterfaceName(item, true);

		//If picked up item is missile weapon, try to add it to carried stack.
		if(item->getInstanceDefinition().isMissileWeapon == true) {
			Item* const carriedMissile = playerInventory->getItemInSlot(slot_missiles);
			if(carriedMissile != NULL) {
				if(item->getInstanceDefinition().devName == carriedMissile->getInstanceDefinition().devName) {
					eng->log->addMessage("You add " + ITEM_NAME + " to your missile stack.");
					carriedMissile->numberOfItems += item->numberOfItems;
					delete item;
					eng->map->items[eng->player->pos.x][eng->player->pos.y] = NULL;
					eng->gameTime->letNextAct();
					return;
				}
			} else {
				eng->log->addMessage("Use " + ITEM_NAME + " as thrown weapon (y/n)?");
				eng->renderer->flip();
				if(eng->query->yesOrNo() == true) {
					eng->log->clearLog();
					eng->log->addMessage("You pick up " + ITEM_NAME + ".");
					eng->renderer->flip();
					playerInventory->putItemInSlot(slot_missiles, item, true, true);
					eng->map->items[eng->player->pos.x][eng->player->pos.y] = NULL;
					eng->gameTime->letNextAct();
					return;
				}
			}
		}

		if(isInventoryFull(playerInventory, item) == false) {
			eng->log->clearLog();
			eng->log->addMessage("You pick up " + ITEM_NAME + ".");

			playerInventory->putItemInGeneral(item);

			eng->map->items[eng->player->pos.x][eng->player->pos.y] = NULL;

			eng->gameTime->letNextAct();
		} else {
			eng->log->clearLog();
			eng->log->addMessage("You can not carry more.");
		}
	} else {
		eng->log->clearLog();
		eng->log->addMessage("You see nothing to pick up here.");
	}
}

bool ItemPickup::isInventoryFull(Inventory* inventory, Item* item) const {
	//If item can be stacked, the inventory is not considered full.
	if(inventory->getElementToStackItem(item) != -1)
		return false;

	//Else, check if the size of the general slots is small enough.
	return static_cast<char> (inventory->getSlots()->size() + inventory->getGeneral()->size()) + 'a' - 1 >= 'z';
}

void ItemPickup::tryUnloadWeaponFromGround() {
	Item* item = eng->map->items[eng->player->pos.x][eng->player->pos.y];
	bool unloadedSomething = false;
	if(item != NULL) {
		if(item->getInstanceDefinition().isRangedWeapon == true) {
			Weapon* const weapon = dynamic_cast<Weapon*> (item);
			const int ammoLoaded = weapon->ammoLoaded;

			if(ammoLoaded > 0 && weapon->getInstanceDefinition().rangedHasInfiniteAmmo == false) {
				Inventory* const playerInventory = eng->player->getInventory();
				const ItemDevNames_t ammoType = weapon->getInstanceDefinition().rangedAmmoTypeUsed;

				ItemDefinition* const ammoDef = eng->itemData->itemDefinitions[ammoType];

				Item* spawnedAmmo = eng->itemFactory->spawnItem(ammoType);

				if(ammoDef->isAmmoClip == true) {
					//Unload a clip
					dynamic_cast<ItemAmmoClip*> (spawnedAmmo)->ammo = ammoLoaded;
				} else {
					//Unload loose ammo
					spawnedAmmo->numberOfItems = ammoLoaded;
				}
				const string WEAPON_REF_A = weapon->getInstanceDefinition().name.name_a;
				eng->log->addMessage("You unload " + WEAPON_REF_A);

				if(isInventoryFull(playerInventory, spawnedAmmo) == false) {
					playerInventory->putItemInGeneral(spawnedAmmo);
				} else {
					eng->itemDrop->dropItemOnMap(eng->player->pos, &spawnedAmmo);
					eng->log->addMessage("You have no room to keep the unloaded ammunition, item dropped on floor.");
				}

				dynamic_cast<Weapon*> (item)->ammoLoaded = 0;
				dynamic_cast<Weapon*> (item)->setColorForAmmoStatus();
				unloadedSomething = true;

				eng->gameTime->letNextAct();
			}
		}
	}
	if(unloadedSomething == false) {
		eng->log->addMessage("You see nothing to unload here.");
	}
}

