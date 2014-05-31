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
  Room(Rect r)  : theme_(RoomThemeId::plain), descr_(""), r_(r) {}
  Room()        : theme_(RoomThemeId::plain), descr_(""), r_()  {}
  RoomThemeId theme_;
  std::string descr_;
  Rect        r_;
  std::vector<Room*> roomsConTo_;
};

struct Region {
public:
  Region(const Rect& r) : mainRoom_(nullptr), r_(r) {}
  Region()              : mainRoom_(nullptr), r_()  {}

  Rect  getRndRoomRect()  const;

  Room* mainRoom_;
  Rect  r_;
};

namespace MapGenUtils {

void getValidRoomCorrEntries(const Room& room, std::vector<Pos>& out);

void mk(const Rect& area, const FeatureId id);

void mkPathFindCor(Room& r0, Room& r1,
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

namespace Std               {bool run();} //Std

namespace IntroForest       {bool run();} //IntroForest

namespace EgyptTomb         {bool run();} //EgyptTomb

namespace CaveLvl           {bool run();} //CaveLvl

namespace TrapezohedronLvl  {bool run();} //TrapezohedronLvl

} //MapGen

#endif
