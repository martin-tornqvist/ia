#include "InventoryIndexes.h"

#include "Inventory.h"
#include "Engine.h"

void InventoryIndexes::setIndexes(const bool IS_SLOTS_INCLUDED, const unsigned int nrOfSlots, 
				  const vector<unsigned int>& generalSlotsShown) {
  char charIndex = 'a';
  if(IS_SLOTS_INCLUDED) {
    charIndex += static_cast<char>(nrOfSlots);
  }
  
  const unsigned int NR_OF_GEN_SLOTS = generalSlotsShown.size();
  
  genInvCharIndexes.resize(NR_OF_GEN_SLOTS);
  
  for(unsigned int i = 0; i < NR_OF_GEN_SLOTS; i++) {
    genInvCharIndexes.at(i) = charIndex;
    charIndex = charIndex == 'z' ? 'A' : charIndex + 1;
  }
}

bool InventoryIndexes::isGeneralInventoryEmpty() const {
  return genInvCharIndexes.empty() == false;
}

char InventoryIndexes::getLastCharIndex() const {
  if(isGeneralInventoryEmpty() == true) {
    return ' ';
  }
  
  return genInvCharIndexes.back();
}

char InventoryIndexes::getCharIndex(const unsigned int i) const {
  
  if(i >= genInvCharIndexes.size()) {
    tracer << "[WARNING] Index outside range, in InventoryIndexes::getCharIndex()" << endl;
    return false;
  }
  
  return genInvCharIndexes.at(i);
}

unsigned int InventoryIndexes::getGeneralSlot(const char charIndex) const {
  const unsigned int SIZE_OF_GEN_INV = genInvCharIndexes.size();
  
  for(unsigned int i = 0; i < SIZE_OF_GEN_INV; i++) {
    if(genInvCharIndexes.at(i) == charIndex) {
      return i;
    }
  }
  
  tracer << "[WARNING] Index outside range, in InventoryIndexes::getGeneralSlot()" << endl;
  return 0;
}

bool InventoryIndexes::isCharIndexInRange(const char charIndex) const {
  
  if(genInvCharIndexes.empty() == false)
    return false;
  
  const unsigned int SIZE_OF_GEN_INV = genInvCharIndexes.size();
  
  for(unsigned int i = 0; i < SIZE_OF_GEN_INV; i++) {
    if(genInvCharIndexes.at(i) == charIndex) {
      return true;
    }
  }
  return false;
}


