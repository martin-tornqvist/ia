#ifndef MAPBUILD_H
#define MAPBUILD_H

#include <vector>

#include "CmnTypes.h"
#include "RoomTheme.h"
#include "Feature.h"
#include "MapTemplates.h"

struct Region;

enum HorizontalVertical {horizontal, vertical};

enum class RoomReshapeType {trimCorners, pillarsRandom};

struct Room {
public:
  Room(Rect r) : roomTheme(RoomThemeId::plain), roomDescr(""), r_(r) {}

  Room() : roomTheme(RoomThemeId::plain), roomDescr(""), r_() {}

  Rect getRect()  const {return r_;}
  int getX0()     const {return r_.p0.x;}
  int getY0()     const {return r_.p0.y;}
  int getX1()     const {return r_.p1.x;}
  int getY1()     const {return r_.p1.y;}
  Pos getP0()     const {return r_.p0;}
  Pos getP1()     const {return r_.p1;}

  RoomThemeId roomTheme;

  std::string roomDescr;

private:
  Rect r_;
};

struct Region {
public:
  Region(const Rect& r);
  Region();
  ~Region() {}

  Rect getRndRoomRect() const;

  bool isRegionNeighbour(const Region& other);

  Pos getCenterPos()    const {return (r_.p1 + r_.p0) / 2;}
  Rect getRect()        const {return r_;}
  Pos getP0()           const {return r_.p0;}
  Pos getP1()           const {return r_.p1;}

  int getNrConnections();

  bool regionsConnectedTo_[3][3];

  Room* mainRoom_;

  bool isConnected_;
  bool isBuilt_;

private:
  Rect r_;

};

namespace MapGenUtils {

void getValidRoomCorrEntries(Room& r, std::vector<Pos>& out);

void mk(const Rect& area, const FeatureId id);

void mkPathFindCorridor(const Room& r0, const Room& r1,
                        bool doorPosProposals[MAP_W][MAP_H] = nullptr);

void backupMap();
void restoreMap();

void mkFromTempl(const Pos& pos, const MapTempl& t);
void mkFromTempl(const Pos& pos, const MapTemplId templateId);

void mkByRandomWalk(const Pos& p0, int len, FeatureId featureToMk,
                    const bool DIG_ANY_FEATURE,
                    const bool ONLY_STRAIGHT = true,
                    const Pos& p0Lim = Pos(1, 1),
                    const Pos& p1Lim = Pos(MAP_W - 2, MAP_H - 2));

void mkWithPathfinder(const Pos& p0, const Pos& p1,
                      FeatureId feature, const bool SMOOTH,
                      const bool DIG_ANY_FEATURE);

} //MapGenUtils

namespace MapGen {

namespace Std {

bool run();

} //Std

namespace IntroForest {

bool run();

} //IntroForest

namespace EgyptTomb {

bool run();

} //EgyptTomb

namespace CaveLvl {

bool run();

} //CaveLvl

namespace TrapezohedronLvl {

bool run();

} //TrapezohedronLvl

} //MapGen

#endif
