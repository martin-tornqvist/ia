#ifndef ITEM_H
#define ITEM_H

#include <iostream>

#include "ItemData.h"
#include "Art.h"

enum ItemActivateReturn_t {
	itemActivate_keep, itemActivate_destroyed
};

class Item {
public:
	Item(ItemDefinition* const itemDefinition) :
		numberOfItems(1), m_instanceDefinition(*itemDefinition), m_archetypeDefinition(itemDefinition) {
		if(itemDefinition->meleeStatusEffect != NULL) {
			m_instanceDefinition.meleeStatusEffect = itemDefinition->meleeStatusEffect->copy();
		}
		if(itemDefinition->rangedStatusEffect != NULL) {
			m_instanceDefinition.rangedStatusEffect = itemDefinition->rangedStatusEffect->copy();
		}
	}

	virtual ~Item() {
	}

	virtual void reset() {
		numberOfItems = 1;
	}

	const ItemDefinition& getInstanceDefinition() {
		//Updating certain data to ensure synchronisation between archetype and local definition.
		m_instanceDefinition.name.name = m_archetypeDefinition->name.name;
		m_instanceDefinition.name.name_plural = m_archetypeDefinition->name.name_plural;
		m_instanceDefinition.name.name_a = m_archetypeDefinition->name.name_a;
		m_instanceDefinition.isIdentified = m_archetypeDefinition->isIdentified;
		m_instanceDefinition.isScrollLearned = m_archetypeDefinition->isScrollLearned;

		return m_instanceDefinition;
	}

	ItemDefinition& getArchetypeDefinition() {
      return *m_archetypeDefinition;
	}

	SDL_Color getColor() {
		return m_instanceDefinition.color;
	}
	char getGlyph() {
		return m_instanceDefinition.glyph;
	}
	Tile_t getTile() {
		return m_instanceDefinition.tile;
	}

	void resetGlyphAndColor() {
		m_instanceDefinition.glyph = m_archetypeDefinition->glyph;
		m_instanceDefinition.color = m_archetypeDefinition->color;
	}

	virtual ItemActivateReturn_t defaultActivation(Actor*, Engine*) {
		return itemActivate_keep;
	}

	virtual void setRealDefinitionNames(Engine* const engine, const bool IS_SILENT_IDENTIFY) {
		(void)engine;
		(void)IS_SILENT_IDENTIFY;
	}

	virtual void itemSpecificAddSaveLines(vector<string>& lines) {
		(void)lines;
	}
	virtual void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
		(void)lines;
	}

	int getWeight() const {
        return m_instanceDefinition.itemWeight * numberOfItems;
	}

	string getWeightLabel() const {
        const int WEIGHT = getWeight();
        if(WEIGHT <= (itemWeight_extraLight + itemWeight_light)/2) return " - ";
        if(WEIGHT <= (itemWeight_light + itemWeight_medium)/2) return "LGT";
        if(WEIGHT <= (itemWeight_medium + itemWeight_heavy)/2) return "MED";
        return "HVY";
	}

	int numberOfItems;

protected:
	ItemDefinition m_instanceDefinition;
	ItemDefinition* m_archetypeDefinition;

	//Called by the ItemDrop class to make noise etc
	friend class ItemDrop;
	virtual void appplyDropEffects() {
	}
};

#endif
