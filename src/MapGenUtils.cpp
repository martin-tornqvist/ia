#include "MapGen.h"

#include <vector>

#include "Map.h"
#include "FeatureFactory.h"
#include "MapParsing.h"
#include "Utils.h"
#include "MapTemplates.h"

#ifdef DEMO_MODE
#include "SdlWrapper.h"
#endif // DEMO_MODE

using namespace std;

namespace MapGenUtils {

namespace {

FeatureId backup[MAP_W][MAP_H];

} //namespace

//Note: The parameter rectangle does not have to go up-left to bottom-right,
//the method adjusts the order
void build(const Rect& area, const FeatureId id) {
  const Pos p0 = Pos(min(area.p0.x, area.p1.x),
                       min(area.p0.y, area.p1.y));

  const Pos p1 = Pos(max(area.p0.x, area.p1.x),
                       max(area.p0.y, area.p1.y));

  for(int x = p0.x; x <= p1.x; x++) {
    for(int y = p0.y; y <= p1.y; y++) {
      FeatureFactory::spawn(id, Pos(x, y), nullptr);
    }
  }
}

void build(const vector<Pos>& posList, const FeatureId id) {
  for(const Pos & pos : posList) {FeatureFactory::spawn(id, pos);}
}

void buildZCorridorBetweenRooms(const Room& r1, const Room& r2,
                                Dir cardinalDirToTravel,
                                bool doorPosBucket[MAP_W][MAP_H]) {
  //Find all floor in both rooms
  vector<Pos> floorInR1Vector;
  floorInR1Vector.resize(0);
  bool floorInR1Grid[MAP_W][MAP_H];
  Utils::resetArray(floorInR1Grid, false);

  for(int y = r1.getY0(); y <= r1.getY1(); y++) {
    for(int x = r1.getX0(); x <= r1.getX1(); x++) {
      const Pos c = Pos(x, y);
      if(Map::cells[c.x][c.y].featureStatic->getId() ==
          FeatureId::floor) {
        floorInR1Vector.push_back(c);
        floorInR1Grid[x][y] = true;
      }
    }
  }

  const Pos roomDeltaSigns(DirUtils::getOffset(cardinalDirToTravel));

  vector<Pos> possInR1closeToR2;
  possInR1closeToR2.resize(0);
  vector<Pos> floorInR2Vector;
  floorInR2Vector.resize(0);
  bool floorInR2Grid[MAP_W][MAP_H];
  Utils::resetArray(floorInR2Grid, false);
  for(int y = r2.getY0(); y <= r2.getY1(); y++) {
    for(int x = r2.getX0(); x <= r2.getX1(); x++) {
      Pos c = Pos(x, y);
      const FeatureStatic* const f =
        Map::cells[c.x][c.y].featureStatic;
      if(f->getId() == FeatureId::floor) {
        floorInR2Vector.push_back(c);
        floorInR2Grid[x][y] = true;

        //Try a simple corridor
        bool doneTravelling = false;
        int nrOfJumps = 0;
        while(doneTravelling == false) {
          c += roomDeltaSigns * -1;
          nrOfJumps++;
          if(floorInR1Grid[c.x][c.y] == true) {
            //floor -> wall -> (wall) -> floor
            if(nrOfJumps <= 3) {
              possInR1closeToR2.push_back(c);
            }
          }
          if(Utils::isPosInsideMap(c) == false) {
            doneTravelling = true;
          }
        }
      }
    }
  }

  if(possInR1closeToR2.size() > 0) {
    Pos c = possInR1closeToR2.at(Rnd::dice(1, possInR1closeToR2.size()) - 1);
    while(floorInR2Grid[c.x][c.y] == false) {
      c += roomDeltaSigns;
      FeatureFactory::spawn(FeatureId::floor, c, nullptr);
      if(doorPosBucket != nullptr) {doorPosBucket[c.x][c.y] = true;}
    }
  } else {

    /*
    Description of the following algorithm:
    ======================================================================
    What has been done so far is to pick one position in each room with floor
    in it (c1 and c2). There is also a vector of all floor in c1 and one of all
    floor in c2 (floorInR1Vector, floorInR2Vector), as well as boolean arrays
    of the floor (floorInR1Grid, floorInR2Grid).

    First, we attempted to find two points with only one or two wall cells
    between them, so a simple connection could be made. Given that this point
    was reached, no two such position were found.

    A constant value is stored that is used heavily for corridor navigation:
    "roomDeltaSigns". It tells the direction from room 1 to room 2.
    (e.g. 1,0 means room 2 lies to the east of room 1)

    The following will happen below:

    (1) One position in each room is picked randomly (c1 and c2).

    (2) While still in the p0,p1-boundaries of room 1, move straight out
    from room 1 ("straight out" means in the direction of roomDeltaSigns).
    When we stand in the first cell outside room 1, we turn that into floor,
    and store that as a suggestion for a door. Then we move yet another step
    straight away from the room.

    (3) A secondary corridor is tunnelled backwards from where we stood
    after (2), until floor is reached in room 1.

    (4) We stand once again in the cell where (2) finished. Now a new constant
    value is stored called "deltaCurSign". It tells the direction from the
    position we stand in at the start of this phase to the position we are
    aiming for in room 2 (c2). If roomDeltaSigns is subtracted from deltaCurSign,
    a position is had which tells the direction 90 degrees from the tunnel dug
    in (2).
    Example:
    Room 1 lies east of room 2, roomDeltaSigns = (1,0). c2 lies to the east and
    below the current position, deltaCurSign = (1,1). So
    deltaCurSign - roomDeltaSigns = (1,1) - (1,0) = (0,1).
    We now tunnel in this direction. First make floor, then move, until we hit
    either the x- or y-axis of c2 --or-- if we stand on floor in room 2.

    (5) Now we move again in the direction of roomDeltaSigns (straight east in
    the above example). First make floor, then move, until we stand on floor
    in room 2. Afterwards we move one step backwards and suggest that for door
    position.

    #######
    ...33324  ######
    ....###4  #.....
    ......#4  #.....
    ......#45555....
    #######   #.....
              ###### */

    // (1)
    const Pos c1 = floorInR1Vector.at(
                     Rnd::dice(1, floorInR1Vector.size()) - 1);
    const Pos c2 = floorInR2Vector.at(
                     Rnd::dice(1, floorInR2Vector.size()) - 1);

    Pos c = c1;

    // (2)
    while(floorInR1Grid[c.x][c.y] == true) {
      c += roomDeltaSigns;
    }
    FeatureFactory::spawn(FeatureId::floor, c, nullptr);
    if(doorPosBucket != nullptr) {doorPosBucket[c.x][c.y] = true;}
    c += roomDeltaSigns;

    // (3)
    Pos cTemp(c - roomDeltaSigns);
    while(floorInR1Grid[cTemp.x][cTemp.y] == false) {
      FeatureFactory::spawn(FeatureId::floor, cTemp, nullptr);
      cTemp -= roomDeltaSigns;
    }

    const Pos deltaCur(c2 - c);
    const Pos deltaCurAbs(abs(deltaCur.x), abs(deltaCur.y));
    const Pos deltaCurSign(
      deltaCurAbs.x == 0 ? 0 : deltaCur.x / (deltaCurAbs.x),
      deltaCurAbs.y == 0 ? 0 : deltaCur.y / (deltaCurAbs.y));

    // (4)
    while(c.x != c2.x && c.y != c2.y && floorInR2Grid[c.x][c.y] == false) {
      FeatureFactory::spawn(FeatureId::floor, c, nullptr);
      c += deltaCurSign - roomDeltaSigns;
    }

    // (5)
    while(c != c2 && floorInR2Grid[c.x][c.y] == false) {
      FeatureFactory::spawn(FeatureId::floor, c, nullptr);
      c += roomDeltaSigns;
    }
    c -= roomDeltaSigns;
    if(floorInR2Vector.size() > 1) {
      if(doorPosBucket != nullptr) {doorPosBucket[c.x][c.y] = true;}
    }
  }
}

void backupMap() {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      backup[x][y] = Map::cells[x][y].featureStatic->getId();
    }
  }
}

void restoreMap() {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      FeatureFactory::spawn(backup[x][y], Pos(x, y));
    }
  }
}

void digWithPathfinder(const Pos& origin, const Pos& target, FeatureId feature,
                       const bool SMOOTH, const bool DIG_THROUGH_ANY_FEATURE) {

  bool blockers[MAP_W][MAP_H];
  Utils::resetArray(blockers, false);
  vector<Pos> path;
  PathFind::run(origin, target, blockers, path);
  const int PATH_SIZE = path.size();
  for(int i = 0; i < PATH_SIZE; i++) {
    const Pos c = path.at(i);
    const FeatureStatic* const f =
      Map::cells[c.x][c.y].featureStatic;
    if(f->canHaveStaticFeature() || DIG_THROUGH_ANY_FEATURE) {
      FeatureFactory::spawn(feature, c);
      if(SMOOTH == false && Rnd::percentile() < 33) {
        digByRandomWalk(c, Rnd::dice(1, 6), feature, true);
      }
    }
  }
}

void digByRandomWalk(const Pos& origin, int len, FeatureId featureToMake,
                     const bool DIG_THROUGH_ANY_FEATURE,
                     const bool ONLY_STRAIGHT, const Pos& p0Lim,
                     const Pos& p1Lim) {
  int dx = 0;
  int dy = 0;
  int xPos = origin.x;
  int yPos = origin.y;

  vector<Pos> positionsToSet;

  bool dirOk = false;
  while(len > 0) {
    while(dirOk == false) {
      dx = Rnd::dice(1, 3) - 2;
      dy = Rnd::dice(1, 3) - 2;
      //TODO This is really ugly!
      dirOk =
        !(
          (dx == 0 && dy == 0) || xPos + dx < p0Lim.x ||
          yPos + dy < p0Lim.y || xPos + dx > p1Lim.x ||
          yPos + dy > p1Lim.y ||
          (ONLY_STRAIGHT == true && dx != 0 && dy != 0)
        );
    }
    const FeatureStatic* const f =
      Map::cells[xPos + dx][yPos + dy].featureStatic;
    if(f->canHaveStaticFeature() || DIG_THROUGH_ANY_FEATURE) {
      positionsToSet.push_back(Pos(xPos + dx, yPos + dy));
      xPos += dx;
      yPos += dy;
      len--;
    }
    dirOk = false;
  }
  for(unsigned int i = 0; i < positionsToSet.size(); i++) {
    FeatureFactory::spawn(featureToMake, positionsToSet.at(i));
  }
}

void buildFromTempl(const Pos& pos, const MapTempl& t) {
  for(int dy = 0; dy < t.h; dy++) {
    for(int dx = 0; dx < t.w; dx++) {
      const FeatureId featureId = t.featureVector[dy][dx];
      if(featureId != FeatureId::empty) {
        FeatureFactory::spawn(featureId, pos + Pos(dx, dy));
      }
    }
  }
}

void buildFromTempl(const Pos& pos, MapTemplId templateId) {
  buildFromTempl(pos, MapTemplHandling::getTempl(templateId));
}

} //MapGenUtils
