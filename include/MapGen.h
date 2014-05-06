#ifndef MAPBUILD_H
#define MAPBUILD_H

#include "CmnTypes.h"
#include "RoomTheme.h"
#include "Feature.h"
#include "MapTemplates.h"

struct Region;

enum HorizontalVertical {horizontal, vertical};

enum class RoomReshapeType {trimCorners, pillarsRandom};

struct Room {
public:
  Room(Rect dims) : roomTheme(RoomThemeId::plain), roomDescr(""), dims_(dims) {}

  Room() : roomTheme(RoomThemeId::plain), roomDescr(""),
    dims_(Rect(Pos(-1, -1), Pos(-1, -1))) {}

  inline Rect getDims()   const {return dims_;}
  inline int getX0()      const {return dims_.x0y0.x;}
  inline int getY0()      const {return dims_.x0y0.y;}
  inline int getX1()      const {return dims_.x1y1.x;}
  inline int getY1()      const {return dims_.x1y1.y;}
  inline Pos getX0Y0()    const {return dims_.x0y0;}
  inline Pos getX1Y1()    const {return dims_.x1y1;}

  RoomThemeId roomTheme;

  std::string roomDescr;

private:
  Rect dims_;
};

struct Region {
public:
  Region(const Pos& x0y0, const Pos& x1y1);
  Region();
  ~Region();

  Rect getRandomRectForRoom() const;
  Rect getRegionRect() const {return Rect(x0y0_, x1y1_);}

  bool isRegionNeighbour(const Region& other);

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

namespace MapGenUtils {

void buildZCorridorBetweenRooms(const Room& r1, const Room& r2,
                                Dir cardinalDirToTravel,
                                bool doorPosBucket[MAP_W][MAP_H] = NULL);

void backupMap();
void restoreMap();

void buildFromTempl(const Pos& pos, const MapTempl& t);
void buildFromTempl(const Pos& pos, const MapTemplId templateId);

void digByRandomWalk(const Pos& origin, int len, FeatureId featureToMake,
                     const bool DIG_THROUGH_ANY_FEATURE,
                     const bool ONLY_STRAIGHT = true,
                     const Pos& x0y0Lim = Pos(1, 1),
                     const Pos& x1y1Lim = Pos(MAP_W - 2, MAP_H - 2));

void digWithPathfinder(const Pos& origin, const Pos& target,
                       FeatureId feature, const bool SMOOTH,
                       const bool DIG_THROUGH_ANY_FEATURE);

} //MapGenUtils

namespace MapGen {

namespace Bsp {

bool run();

} //Bsp

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
