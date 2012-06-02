#ifndef INVENTORY_INDEXES_H
#define INVENTOTY_INDEXES_H

#include "Inventory.h"

#include <vector>
#include <iostream>

using namespace std;

class InventoryIndexes {
public:

	void setIndexes(const bool includeSlots, const unsigned int nrOfSlots, const vector<unsigned int>& generalSlotsShown) {
		char charIndex = 'a';
		if(includeSlots == true)
			charIndex += static_cast<char> (nrOfSlots);

		const unsigned int NR_OF_GEN_SLOTS = generalSlotsShown.size();

		genInvCharIndexes.resize(NR_OF_GEN_SLOTS);

		for(unsigned int i = 0; i < NR_OF_GEN_SLOTS; i++) {
			genInvCharIndexes.at(i) = charIndex;
			charIndex = charIndex == 'z' ? '0' : charIndex + 1;
		}
	}

	bool isGeneralInventoryEmpty() const {
		return genInvCharIndexes.size() == 0;
	}

	char getLastCharIndex() const {
		if(isGeneralInventoryEmpty() == true) {
			cout << "[ERROR] index outside range, in InventoryIndexes::getLastIndexChar()" << endl;
			return ' ';
		}

		return genInvCharIndexes.back();
	}

	char getCharIndex(const unsigned int i) {

		if(i >= genInvCharIndexes.size()) {
			cout << "[ERROR] index outside range, in InventoryIndexes::getCharIndex()" << endl;
			return false;
		}

		return genInvCharIndexes.at(i);
	}

	unsigned int getGeneralSlot(const char charIndex) const {
		const unsigned int SIZE_OF_GEN_INV = genInvCharIndexes.size();

		for(unsigned int i = 0; i < SIZE_OF_GEN_INV; i++)
			if(genInvCharIndexes.at(i) == charIndex)
				return i;

		cout << "[ERROR] charIndex outside range, in InventoryIndexes::getGeneralSlot()" << endl;
		return 0;
	}

	bool isCharIndexInRange(const char charIndex) const {

		if(genInvCharIndexes.size() == 0)
			return false;

		const unsigned int SIZE_OF_GEN_INV = genInvCharIndexes.size();

		for(unsigned int i = 0; i < SIZE_OF_GEN_INV; i++) {
			if(genInvCharIndexes.at(i) == charIndex) {
				return true;
			}
		}
		return false;
	}

private:
	vector<char> genInvCharIndexes;
};

#endif
