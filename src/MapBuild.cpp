#include "MapBuild.h"

#include "Engine.h"
#include "Pathfinding.h"
#include "FeatureFactory.h"

bool MapBuild::isAreaFree(const int x0, const int y0, const int x1, const int y1) {
  if(x0 <= 0 || x1 >= MAP_X_CELLS - 1 || y0 <= 0 || y1 >= MAP_Y_CELLS - 1) {
    return false;
  }
  for(int y = y0; y <= y1; y++) {
    for(int x = x0; x <= x1; x++) {
      if(eng->map->featuresStatic[x][y]->getId() != feature_stoneWall) {
	return false;
      }
    }
  }
  return true;
}

void MapBuild::makeStraightPathByPathfinder(const coord origin, const coord target, Feature_t feature, 
					    const bool SMOOTH, const bool TUNNEL_THROUGH_ANY_FEATURE) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetBoolArray(blockers, false);
  vector<coord> path = eng->pathfinder->findPath(origin, blockers, target);
  for(unsigned int i = 0; i < path.size(); i++) {
    const coord c = path.at(i);
    if(eng->map->featuresStatic[c.x][c.y]->canHaveStaticFeature() || TUNNEL_THROUGH_ANY_FEATURE) {
      eng->featureFactory->spawnFeatureAt(feature, c);
      if(SMOOTH == false && eng->dice(1, 100) < 33) {
	makePathByRandomWalk(c.x, c.y, eng->dice(1, 6), feature, true);
      }
    }
  }
}

void MapBuild::makePathByRandomWalk(int originX, int originY, int len, Feature_t featureToMake, const bool TUNNEL_THROUGH_ANY_FEATURE,
				    const bool ONLY_STRAIGHT, const coord x0y0Lim, const coord x1y1Lim) {
  int dx = 0;
  int dy = 0;
  int xPos = originX;
  int yPos = originY;

  vector<coord> positionsToFill;

  bool directionOk = false;
  while(len > 0) {
    while(directionOk == false) {
      dx = eng->dice(1, 3) - 2;
      dy = eng->dice(1, 3) - 2;
      directionOk = !((dx == 0 && dy == 0) || xPos + dx < x0y0Lim.x || yPos + dy < x0y0Lim.y || xPos + dx > x1y1Lim.x || yPos + dy > x1y1Lim.y
		      || (ONLY_STRAIGHT == true && dx != 0 && dy != 0));
    }
    if(eng->map->featuresStatic[xPos + dx][yPos + dy]->canHaveStaticFeature() || TUNNEL_THROUGH_ANY_FEATURE) {
      positionsToFill.push_back(coord(xPos + dx, yPos + dy));
      xPos += dx;
      yPos += dy;
      len--;
    }
    directionOk = false;
  }
  for(unsigned int i = 0; i < positionsToFill.size(); i++) {
    eng->featureFactory->spawnFeatureAt(featureToMake, positionsToFill.at(i));
  }
}

void MapBuild::buildFromTemplate(const coord pos, MapTemplate* t) {
  for(int dy = 0; dy < t->height; dy++) {
    for(int dx = 0; dx < t->width; dx++) {
      const Feature_t featureId = t->featureVector[dy][dx];
      if(featureId != feature_empty) {
	eng->featureFactory->spawnFeatureAt(featureId, pos + coord(dx, dy));
      }
    }
  }

  //	MapArea mapArea;
  //	mapArea.setArea(originX, originY, originX + t->width - 1, originY + t->height - 1, /*mapArea_room*/mapArea_template);

  //	MapJunction junction;
  //	for(unsigned int i = 0; i < t->junctionPositions.size(); i++) {
  //		const int jX = originX + t->junctionPositions.at(i).x;
  //		const int jY = originY + t->junctionPositions.at(i).y;
  //
  //		const coord relativeOrigin(jX < originX ? 2 : (jX < originX + t->width ? 0 : -2), jY < originY ? 2 : (jY < originY + t->height ? 0 : -2));
  //
  //		mapArea.addJunction(jX, jY, relativeOrigin, eng);
  //	}

  //	m_mapAreas.push_back(mapArea);
}

void MapBuild::buildFromTemplate(const coord pos, TemplateDevName_t devName, const bool generalTemplate) {
  MapTemplate* t = eng->mapTemplateHandler->getTemplate(devName, generalTemplate);
  buildFromTemplate(pos, t);
}

void MapBuild::backupMap() {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      backup[x][y] = eng->map->featuresStatic[x][y]->getId();
    }
  }
}

void MapBuild::restoreMap() {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      eng->featureFactory->spawnFeatureAt(backup[x][y], coord(x, y));
    }
  }
}

