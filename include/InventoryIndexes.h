#ifndef INVENTORY_INDEXES_H
#define INVENTOTY_INDEXES_H

#include <vector>
#include <iostream>

using namespace std;

class InventoryIndexes {
public:

  void setIndexes(const bool IS_SLOTS_INCLUDED, const unsigned int nrOfSlots, const vector<unsigned int>& generalSlotsShown);

  bool isGeneralInventoryEmpty() const;

  char getLastCharIndex() const;

  char getCharIndex(const unsigned int i) const;

  unsigned int getGeneralSlot(const char charIndex) const;

  bool isCharIndexInRange(const char charIndex) const;

private:
  vector<char> genInvCharIndexes;
};

#endif
