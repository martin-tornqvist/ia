#ifndef MAPBUILD_SPAWN_ITEMS_H
#define MAPBUILD_SPAWN_ITEMS_H

#include <vector>

#include "ItemData.h"

class MapBuildSpawnItems
{
public:
   MapBuildSpawnItems(Engine* engine) : eng(engine) {}
   void spawnItems();
private:
   void buildCandidateList();

   ItemDevNames_t getFromCandidateList();

   vector<ItemDevNames_t> candidates;
   
   Engine* eng;
};


#endif
