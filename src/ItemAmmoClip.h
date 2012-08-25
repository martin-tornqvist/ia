#ifndef ITEM_AMMO_CLIP_H
#define ITEM_AMMO_CLIP_H

#include "Item.h"

#include "Converters.h"

class ItemAmmoClip: public Item
{
public:
ItemAmmoClip(ItemDefinition* const itemDefinition) : Item(itemDefinition) {
      setFullAmmo();
   }

   ~ItemAmmoClip() {}

   int ammo;

   void setFullAmmo() {ammo = m_archetypeDefinition->ammoContainedInClip;}

	void itemSpecificAddSaveLines(vector<string>& lines) {
		lines.push_back(intToString(m_instanceDefinition.ammoContainedInClip));
	}

	void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
		m_instanceDefinition.ammoContainedInClip = stringToInt(lines.front());
		lines.erase(lines.begin());
	}

private:
};

#endif
