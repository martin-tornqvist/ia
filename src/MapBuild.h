#ifndef MAPBUILD_H
#define MAPBUILD_H

#include <algorithm>

#include "MapArea.h"
#include "MapTemplates.h"
#include "GameTime.h"
#include "ConstTypes.h"
#include "ConstDungeonSettings.h"

using namespace std;

class Engine;
class Door;

class MapBuild {
public:
  MapBuild(Engine* engine) :
    eng(engine) {
    m_mapAreas.resize(0);
  }

  //void buildStartArea1();

  void buildForest();

  void buildDungeonLevel();

  void buildCavern();

  void buildKingsTomb();

  void buildTrapezohedronLevel();

  //	MapJunction* getJunctionAt(const int x, const int y) {
  //		MapJunction* junction;
  //
  //		for(unsigned int i = 0; i < m_mapAreas.size(); i++) {
  //			junction = m_mapAreas.at(i).getJunctionAt(x, y);
  //			if(junction != NULL) {
  //				return junction;
  //			}
  //		}
  //		return NULL;
  //	}

  void makePathByRandomWalk(
    int originX, int originY, int len, Feature_t featureToMake,
    const bool TUNNEL_THROUGH_ANY_FEATURE, const bool ONLY_STRAIGHT = true, const coord x0y0Lim =
      coord(1, 1), const coord x1y1Lim = coord(MAP_X_CELLS - 2, MAP_Y_CELLS - 2));

private:
  void makeStraightPathByPathfinder(
    const coord origin, const coord target, Feature_t feature, const bool SMOOTH,
    const bool TUNNEL_THROUGH_ANY_FEATURE);

  void buildForestLimit();
  void buildForestOuterTreeline();
  void buildForestTreePatch();
  void buildForestTrees(const coord& stairsCoord);

  bool isAreaFree(const int x0, const int y0, const int x1, const int y1);

  vector<MapArea> m_mapAreas;

  coord
  getRectangularAreaDimensions(const Directions_t direction, const MapAreaType_t areaType, const int forcedCorridorLength = -1);

  bool buildRectangularArea(MapJunction* const junction, const MapAreaType_t areaType, const int forcedCorridorLength = -1);

  void coverAreaWithFeaturecoverAreaWithFeature(const Rectangle area, const Feature_t feature);

  void addJunction(MapArea& area, const coord upperLeft, const coord lowerRight, const int width, const int height,
                   const Directions_t junctionDirection);

  void addJunctionsToArea(
    MapArea& area, coord upperLeft, coord lowerRight, const int width, const int height,
    const Directions_t areaDirection);

  bool buildGeneralAreaFromTemplate(MapJunction* junction);
  bool buildStartRoom(int x0, int y0, int x1, int y1);

  void buildCorridorsAndRooms();

  void connectCorridorsAndRooms();

  void removeCorridorStumps();

  bool tryConnectPoints(const coord& c, const coord& offset);

  void buildRoomsAtCorridorEnds();

  int getRandomExpandableArea();

  void placeDoors();
  bool isPointGoodForDoor(coord testDoorCoord);
  bool isPointPassable(const int x, const int y);
  void placeDoorIfSingleEntrance(const vector<coord>& doorCandidates);

  void placeStairs();
  bool forbiddenStairCells[MAP_X_CELLS][MAP_Y_CELLS];

  Directions_t getRandomDirection() const;
  int getRandomRoomSize() const;
  int getRandomRoomSizeSmall() const;
  int getRandomCorridorLength() const;

  void buildFromTemplate(const coord pos, MapTemplate* t);
  void buildFromTemplate(const coord pos, TemplateDevName_t devName, const bool generalTemplate);

  Feature_t backup[MAP_X_CELLS][MAP_Y_CELLS];
  void backupMap();
  void restoreMap();

  bool doors[MAP_X_CELLS][MAP_Y_CELLS];

  Engine* eng;
};

#endif
