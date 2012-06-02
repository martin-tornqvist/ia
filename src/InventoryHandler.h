#ifndef INVENTORY_HANDLER
#define INVENTORY_HANDLER

#include "Inventory.h"

class Engine;

struct InventorySlotButton {
	InventorySlot* inventorySlot;
	char key;
};

enum InventoryPurpose_t {
	inventoryPurpose_look,
	inventoryPurpose_wieldWear,
	inventoryPurpose_wieldAlt,
	inventoryPurpose_missileSelect,
	inventoryPurpose_use,
//	inventoryPurpose_read,
	inventoryPurpose_quaff,
	inventoryPurpose_eat,
	inventoryPurpose_selectDrop,
	inventoryPurpose_readyExplosive
};

enum GeneralInventoryFilters_t {
   generalInventoryFilter_wieldable,
	generalInventoryFilter_wieldableAndWearable,
	generalInventoryFilter_missile,
	generalInventoryFilter_usable,
//	generalInventoryFilter_readable,
	generalInventoryFilter_quaffable,
	generalInventoryFilter_eatable,
	generalInventoryFilter_explosives
};

class InventoryHandler {
public:
	InventoryHandler(Engine* engine) :
		eng(engine) {
	}

	void runPlayerInventory(InventoryPurpose_t purpose);

	vector<InventorySlotButton> playerSlotButtons;

	void updatePlayerGeneralSlotButtons(const InventoryPurpose_t purpose);

	InventorySlot* getSlotPressed(const char charIndex);

	//The int's in this vector are general inventory indexes.
	vector<unsigned int> generalItemsToShow;

	void initPlayerSlotButtons();

	//Call item's default activation.
	//This function is public because of forced eating before dieing of starvation (and perhaps more)
	void activateItem(const InventoryPurpose_t purpose, const unsigned int genSlot);

private:
	void showGeneralItemsFiltered(const GeneralInventoryFilters_t filter);
	void showAllGeneralItems();

	void swapItems(Item** item1, Item** item2);

	SDL_Event m_event;
	Uint8* m_keystates;

	Engine* eng;
};

#endif
