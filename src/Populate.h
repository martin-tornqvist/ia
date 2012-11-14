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

	void spawnOneMonster() const;

	bool spawnGroupOfMonstersAtFreeCells(vector<coord>& freeCells, const bool ALLOW_ROAM /*,
	   const RoomTheme_t belongingToSpecialRoomType = endOfRoomThemes */) const;

private:
	int getOutOfDepthOffset() const;
	Engine* eng;
};


#endif
