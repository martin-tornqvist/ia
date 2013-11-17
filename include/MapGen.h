#ifndef MAPBUILD_H
#define MAPBUILD_H

#include "CommonTypes.h"
#include "RoomTheme.h"
#include "Feature.h"
#include "MapTemplates.h"

using namespace std;

class Engine;
struct Region;

enum HorizontalVertical_t {
  horizontal,
  vertical
};

enum RoomReshape_t {
  roomReshape_trimCorners,
  roomReshape_pillarsRandom
};

struct Room {
public:
Room(Rect dims) : roomTheme(roomTheme_plain), roomDescr(""), dims_(dims) {}

  Room() : roomTheme(roomTheme_plain), roomDescr(""),
    dims_(Rect(Pos(-1, -1), Pos(-1, -1))) {}

  inline Rect getDims()   const {return dims_;}
  inline int getX0()      const {return dims_.x0y0.x;}
  inline int getY0()      const {return dims_.x0y0.y;}
  inline int getX1()      const {return dims_.x1y1.x;}
  inline int getY1()      const {return dims_.x1y1.y;}
  inline Pos getX0Y0()    const {return dims_.x0y0;}
  inline Pos getX1Y1()    const {return dims_.x1y1;}

  RoomTheme_t roomTheme;

  string roomDescr;

private:
  Rect dims_;
};

class MapGen {
public:
  MapGen(Engine* engine) : eng(engine) {}
  ~MapGen() {}

  inline void run() {specificRun();}

protected:
  virtual void specificRun() = 0;

  void buildFromTemplate(const Pos pos, MapTemplate* t);
  void buildFromTemplate(const Pos pos, MapTemplateId_t templateId);

  Feature_t backup[MAP_X_CELLS][MAP_Y_CELLS];
  void backupMap();
  void restoreMap();

  void makePathByRandomWalk(
    int originX, int originY, int len, Feature_t featureToMake,
    const bool TUNNEL_THROUGH_ANY_FEATURE, const bool ONLY_STRAIGHT = true,
    const Pos x0y0Lim = Pos(1, 1),
    const Pos x1y1Lim = Pos(MAP_X_CELLS - 2, MAP_Y_CELLS - 2));

  void makeStraightPathByPathfinder(
    const Pos origin, const Pos target, Feature_t feature, const bool SMOOTH,
    const bool TUNNEL_THROUGH_ANY_FEATURE);

  Engine* const eng;
};

class MapGenBsp : public MapGen {
public:
  MapGenBsp(Engine* engine) : MapGen(engine) {}
  virtual ~MapGenBsp() {}

private:
  void specificRun();

  void coverAreaWithFeature(const Rect& area, const Feature_t feature);

  int getNrStepsInDirectionUntilWallFound(Pos c, const Direction_t dir) const;

  bool isAllRoomsConnected();

  Room* buildRoom(const Rect& roomPoss);

  bool roomCells[MAP_X_CELLS][MAP_Y_CELLS]; //Used for helping to build the map
  bool regionsToBuildCave[3][3];

  void makeCrumbleRoom(const Rect roomAreaIncludingWalls,
                       const Pos proxEventPos);

  void connectRegions(Region* regions[3][3]);
  void buildAuxRooms(Region* regions[3][3]);
  bool tryPlaceAuxRoom(
    const int X0, const int Y0, const int W, const int H,
    bool blockers[MAP_X_CELLS][MAP_Y_CELLS], const Pos doorPos);

  void buildMergedRegionsAndRooms(
    Region* regions[3][3], const int SPLIT_X1, const int SPLIT_X2,
    const int SPLIT_Y1, const int SPLIT_Y2);

  void buildCaves(Region* regions[3][3]);

  void buildCorridorBetweenRooms(const Region* const r1,
				 const Region* const r2);
  void placeDoorAtPosIfSuitable(const Pos pos);

  void reshapeRoom(const Room& room);

  void buildRoomsInRooms();

  void postProcessFillDeadEnds();

  bool globalDoorPositionCandidates[MAP_X_CELLS][MAP_Y_CELLS];

//  void findEdgesOfRoom(const Rect roomPoss, vector<Pos>& vectorToFill);

  bool isRegionFoundInCardinalDirection(
    const Pos pos, bool region[MAP_X_CELLS][MAP_Y_CELLS]) const;

  bool isAreaFree(
    const Rect& area, bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]);
  bool isAreaAndBorderFree(
    const Rect& areaWithBorder, bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]);
  bool isAreaFree(
    const int X0, const int Y0, const int X1, const int Y1,
    bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]);

  void decorate();

  Pos placeStairs();

//  void makeLevers();
//  void spawnLeverAdaptAndLinkDoor(const Pos& leverPos, Door& door);

  void revealAllDoorsBetweenPlayerAndStairs(const Pos& stairsPos);

//  void buildNaturalArea(Region* regions[3][3]);
//  void makeRiver(Region* regions[3][3]);

//  vector<Room*> rooms_;
  void deleteAndRemoveRoomFromList(Room* const room);

  bool forbiddenStairCellsGlobal[MAP_X_CELLS][MAP_Y_CELLS];
};

struct ConnectionPointsAndDistance {
public:
  ConnectionPointsAndDistance(Pos c1_, Pos c2_, int dist_) :
    c1(c1_), c2(c2_), dist(dist_) {}
  ConnectionPointsAndDistance() {}

  Pos c1;
  Pos c2;
  int dist;
};

struct Region {
public:
  Region(Pos x0y0, Pos x1y1);
  Region();
  ~Region();

  Rect getRandomPossForRoom(Engine* eng) const;
  Rect getRegionPoss() const {
    return Rect(x0y0_, x1y1_);
  }

  bool isRegionNeighbour(const Region& other, Engine* const engine);

  inline Pos getCenterPos() const {return (x1y1_ + x0y0_) / 2;}
  inline Pos getX0Y0()      const {return x0y0_;}
  inline Pos getX1Y1()      const {return x1y1_;}

  int getNrOfConnections();

  bool regionsConnectedTo[3][3];

  Room* mainRoom;

  bool isConnected;

private:
  Pos x0y0_, x1y1_;
};

class MapGenIntroForest : public MapGen {
public:
  MapGenIntroForest(Engine* engine) : MapGen(engine) {}
  ~MapGenIntroForest() {}

private:
  void specificRun();

  void buildForestLimit();
  void buildForestOuterTreeline();
  void buildForestTreePatch();
  void buildForestTrees(const Pos& stairsPos);
};

class MapGenEgyptTomb : public MapGen {
public:
  MapGenEgyptTomb(Engine* engine) : MapGen(engine) {}
  ~MapGenEgyptTomb() {}

private:
  void specificRun();
};

class MapGenCaveLvl : public MapGen {
public:
  MapGenCaveLvl(Engine* engine) : MapGen(engine) {}
  ~MapGenCaveLvl() {}

private:
  void specificRun();
};

class MapGenTrapezohedronLvl : public MapGen {
public:
  MapGenTrapezohedronLvl(Engine* engine) : MapGen(engine) {}
  ~MapGenTrapezohedronLvl() {}

private:
  void specificRun();
};

#endif
