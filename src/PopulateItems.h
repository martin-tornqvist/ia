#ifndef POPULATE_ITEMS_H
#define POPULATE_ITEMS_H

#include <vector>

#include "ItemData.h"

class PopulateItems {
public:
  PopulateItems(Engine* engine) : eng(engine) {}

  void spawnItems();

private:
  void buildCandidateList();

  ItemDevNames_t getFromCandidateList();

  vector<ItemDevNames_t> candidates;

  Engine* eng;
};


#endif
