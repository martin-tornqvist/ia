#ifndef POPULATE_ITEMS_H
#define POPULATE_ITEMS_H

#include <vector>

#include "ItemData.h"

class PopulateItems {
public:
  PopulateItems(Engine& engine) : eng(engine) {}

  void spawnItems();

private:
  Engine& eng;
};


#endif
