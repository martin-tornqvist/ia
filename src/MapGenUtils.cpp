#include "MapGen.h"

#include "Engine.h"
#include "Map.h"
#include "FeatureFactory.h"
#include "MapParsing.h"

void MapGenUtilCorridorBuilder::buildZCorridorBetweenRooms(
  const Room& room1, const Room& room2, Dir_t cardinalDirToTravel,
  bool doorPosCandidates[MAP_X_CELLS][MAP_Y_CELLS]) {

  //Find all floor in both rooms
  vector<Pos> floorInR1Vector;
  floorInR1Vector.resize(0);
  bool floorInR1Grid[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetArray(floorInR1Grid, false);

  for(int y = room1.getY0(); y <= room1.getY1(); y++) {
    for(int x = room1.getX0(); x <= room1.getX1(); x++) {
      const Pos c = Pos(x, y);
      if(eng->map->cells[c.x][c.y].featureStatic->getId() ==
          feature_stoneFloor) {
        floorInR1Vector.push_back(c);
        floorInR1Grid[x][y] = true;
      }
    }
  }

  //Determine direction to travel from room 1
//  vector<Pos> travelDirCandidates;
//  travelDirCandidates.resize(0);
//  if(room2.getY1() < room1.getY0()) {
//    travelDirCandidates.push_back(Pos(0, -1));  //Up
//  }
//  if(room2.getY0() > room1.getY1()) {
//    travelDirCandidates.push_back(Pos(0, 1));   //Down
//  }
//  if(room2.getX1() < room1.getX0()) {
//    travelDirCandidates.push_back(Pos(-1, 0));  //Left
//  }
//  if(room2.getX0() > room1.getX1()) {
//    travelDirCandidates.push_back(Pos(1, 0));   //Right
//  }
//
//  const int ELEMENT = eng->dice.range(0, travelDirCandidates.size() - 1);
//  const Pos travelStartDir(travelDirCandidates.at(ELEMENT));

  const Pos roomDeltaSigns(
    DirConverter(eng).getOffset(cardinalDirToTravel));

  vector<Pos> PossInR1closeToR2;
  PossInR1closeToR2.resize(0);
  vector<Pos> floorInR2Vector;
  floorInR2Vector.resize(0);
  bool floorInR2Grid[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetArray(floorInR2Grid, false);
  for(int y = room2.getY0(); y <= room2.getY1(); y++) {
    for(int x = room2.getX0(); x <= room2.getX1(); x++) {
      Pos c = Pos(x, y);
      const FeatureStatic* const f =
        eng->map->cells[c.x][c.y].featureStatic;
      if(f->getId() == feature_stoneFloor) {
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
              PossInR1closeToR2.push_back(c);
            }
          }
          if(eng->basicUtils->isPosInsideMap(c) == false) {
            doneTravelling = true;
          }
        }
      }
    }
  }

  if(PossInR1closeToR2.size() > 0) {
    Pos c = PossInR1closeToR2.at(eng->dice(1, PossInR1closeToR2.size()) - 1);
    while(floorInR2Grid[c.x][c.y] == false) {
      c += roomDeltaSigns;
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
      if(doorPosCandidates != NULL) {
        doorPosCandidates[c.x][c.y] = true;
      }
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

    (2) While still in the x0y0,x1y1-boundaries of room 1, move straight out
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
                     eng->dice(1, floorInR1Vector.size()) - 1);
    const Pos c2 = floorInR2Vector.at(
                     eng->dice(1, floorInR2Vector.size()) - 1);

    Pos c = c1;

    // (2)
    while(floorInR1Grid[c.x][c.y] == true) {
      c += roomDeltaSigns;
    }
    eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
    if(doorPosCandidates != NULL) {
      doorPosCandidates[c.x][c.y] = true;
    }
    c += roomDeltaSigns;

    // (3)
    Pos cTemp(c - roomDeltaSigns);
    while(floorInR1Grid[cTemp.x][cTemp.y] == false) {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, cTemp, NULL);
      cTemp -= roomDeltaSigns;
    }

    const Pos deltaCur(c2 - c);
    const Pos deltaCurAbs(abs(deltaCur.x), abs(deltaCur.y));
    const Pos deltaCurSign(
      deltaCurAbs.x == 0 ? 0 : deltaCur.x / (deltaCurAbs.x),
      deltaCurAbs.y == 0 ? 0 : deltaCur.y / (deltaCurAbs.y));

    // (4)
    while(c.x != c2.x && c.y != c2.y && floorInR2Grid[c.x][c.y] == false) {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
      c += deltaCurSign - roomDeltaSigns;
    }

    // (5)
    while(c != c2 && floorInR2Grid[c.x][c.y] == false) {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
      c += roomDeltaSigns;
    }
    c -= roomDeltaSigns;
    if(floorInR2Vector.size() > 1) {
      if(doorPosCandidates != NULL) {
        doorPosCandidates[c.x][c.y] = true;
      }
    }
  }
}

void MapGen::backupMap() {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      backup[x][y] = eng->map->cells[x][y].featureStatic->getId();
    }
  }
}

void MapGen::restoreMap() {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      eng->featureFactory->spawnFeatureAt(backup[x][y], Pos(x, y));
    }
  }
}

void MapGen::makeStraightPathByPathfinder(
  const Pos origin, const Pos target, Feature_t feature, const bool SMOOTH,
  const bool TUNNEL_THROUGH_ANY_FEATURE) {

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetArray(blockers, false);
  vector<Pos> path;
  eng->pathFinder->run(origin, target, blockers, path);
  const int PATH_SIZE = path.size();
  for(int i = 0; i < PATH_SIZE; i++) {
    const Pos c = path.at(i);
    const FeatureStatic* const f =
      eng->map->cells[c.x][c.y].featureStatic;
    if(f->canHaveStaticFeature() || TUNNEL_THROUGH_ANY_FEATURE) {
      eng->featureFactory->spawnFeatureAt(feature, c);
      if(SMOOTH == false && eng->dice.percentile() < 33) {
        makePathByRandomWalk(c.x, c.y, eng->dice(1, 6), feature, true);
      }
    }
  }
}

void MapGen::makePathByRandomWalk(
  int originX, int originY, int len, Feature_t featureToMake,
  const bool TUNNEL_THROUGH_ANY_FEATURE, const bool ONLY_STRAIGHT,
  const Pos x0y0Lim, const Pos x1y1Lim) {

  int dx = 0;
  int dy = 0;
  int xPos = originX;
  int yPos = originY;

  vector<Pos> positionsToFill;

  bool dirOk = false;
  while(len > 0) {
    while(dirOk == false) {
      dx = eng->dice(1, 3) - 2;
      dy = eng->dice(1, 3) - 2;
      //TODO This is really ugly!
      dirOk =
        !(
          (dx == 0 && dy == 0) || xPos + dx < x0y0Lim.x ||
          yPos + dy < x0y0Lim.y || xPos + dx > x1y1Lim.x ||
          yPos + dy > x1y1Lim.y ||
          (ONLY_STRAIGHT == true && dx != 0 && dy != 0)
        );
    }
    const FeatureStatic* const f =
      eng->map->cells[xPos + dx][yPos + dy].featureStatic;
    if(f->canHaveStaticFeature() || TUNNEL_THROUGH_ANY_FEATURE) {
      positionsToFill.push_back(Pos(xPos + dx, yPos + dy));
      xPos += dx;
      yPos += dy;
      len--;
    }
    dirOk = false;
  }
  for(unsigned int i = 0; i < positionsToFill.size(); i++) {
    eng->featureFactory->spawnFeatureAt(featureToMake, positionsToFill.at(i));
  }
}

void MapGen::buildFromTemplate(const Pos pos, MapTemplate* t) {
  for(int dy = 0; dy < t->height; dy++) {
    for(int dx = 0; dx < t->width; dx++) {
      const Feature_t featureId = t->featureVector[dy][dx];
      if(featureId != feature_empty) {
        eng->featureFactory->spawnFeatureAt(featureId, pos + Pos(dx, dy));
      }
    }
  }
}

void MapGen::buildFromTemplate(const Pos pos, MapTemplateId_t templateId) {
  MapTemplate* t = eng->mapTemplateHandler->getTemplate(templateId);
  buildFromTemplate(pos, t);
}
