#ifndef INVENTORY_HANDLER
#define INVENTORY_HANDLER

#include "Inventory.h"

class Engine;

struct InventorySlotButton {
	InventorySlot* inventorySlot;
	char key;
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

	Engine* eng;
};

#endif
