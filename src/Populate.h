#ifndef POPULATE_H
#define POPULATE_H

#include <vector>

#include "ActorData.h"

class Engine;


class Populate
{
public:
	Populate(Engine* engine) : eng(engine) {}

	void populate() const;

	void spawnOneMonster(const bool IS_AFTER_MAP_CREATION) const;

	bool spawnGroupOfMonstersAtFreeCells(
	   vector<coord>& freeCells, const bool IS_AFTER_MAP_CREATION, const bool ALLOW_ROAM,
	   const SpecialRoom_t belongingToSpecialRoomType = endOfSpecialRooms) const;

private:
	int getOutOfDepthOffset() const;
	Engine* eng;
};


#endif
