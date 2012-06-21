#ifndef RENDER_INVENTORY_H
#define RENDER_INVENTORY_H

#include "InventoryHandler.h"

#include "ItemData.h"
#include "MenuBrowser.h"

class Engine;

class RenderInventory {
public:
	RenderInventory(Engine* engine);

	void draw(const InventoryPurpose_t purpose, const MenuBrowser& browser, const bool DRAW_CMD_PROMPT, const bool DRAW_BROWSER);

	string getWeaponDataLine(Weapon* const weapon) const;

private:
	RenderInventory& operator=(const RenderInventory& other) {
	   (void)other; return *this;
   }

	void drawSlots(vector<InventorySlotButton>* slotButtons, const MenuBrowser& browser, const bool DRAW_BROWSER);
	void drawGeneralItems(const int xPosOffset, const InventoryPurpose_t purpose, const MenuBrowser& browser, const bool DRAW_BROWSER);

	Engine* eng;

	const int xPosListsLeft1;
	const int xPosListsLeft2;
	const int xPosListsLeft3;
	const int xPosListsRight1;
	const int xPosListsRight2;
	const int xPosListsRightStandardOffset;
	const int yPosLists;
};

#endif
