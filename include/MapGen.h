#ifndef MAPBUILD_H
#define MAPBUILD_H

#include <vector>

#include "CmnTypes.h"
#include "MapTemplates.h"

class Room;

struct Region {
public:
  Region(const Rect& r) : mainRoom_(nullptr), r_(r), isFree_(true) {}
  Region()              : mainRoom_(nullptr), r_(),  isFree_(true) {}

  Rect getRndRoomRect() const;

  Room* mainRoom_;
  Rect  r_;
  bool  isFree_;
};

namespace MapGenUtils {

void cutRoomCorners(const Room& room);

void mkPillarsInRoom(const Room& room);

void getValidRoomCorrEntries(const Room& room, std::vector<Pos>& out);

void mkPathFindCor(Room& r0, Room& r1,
                   bool doorProposals[MAP_W][MAP_H] = nullptr);

void backupMap();
void restoreMap();

void rndWalk(const Pos& p0, int len, std::vector<Pos>& posListRef,
             const bool ALLOW_DIAGONAL = true,
             Rect area = Rect(1, 1, MAP_W - 2, MAP_H - 2));

void pathfinderWalk(const Pos& p0, const Pos& p1, std::vector<Pos>& posListRef,
                    const bool IS_SMOOTH);

} //MapGenUtils

namespace MapGen {

//This variable is checked at certain points to see if the current map
//has been flagged as "failed". Setting isMapValid to false will generally
//stop map generation, discard the map, and trigger generation of a new map.
extern bool isMapValid;

namespace Std               {bool run();}
namespace IntroForest       {bool run();}
namespace Egypt             {bool run();}
namespace CaveLvl           {bool run();}
namespace TrapezohedronLvl  {bool run();}

} //MapGen

#endif
