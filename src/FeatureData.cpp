
#include "FeatureData.h"

#include "Colors.h"
#include "Actor.h"

using namespace std;

bool MoveRules::canMove(const vector<PropId>& actorsProps) const {
  if(canMoveCmn_) return true;

  //If not allowing normal move, check if any property overrides this
  for(PropId id : actorsProps) {if(canMoveIfHaveProp_[id]) return true;}

  return false;
}

namespace FeatureData {

FeatureDataT data[int(FeatureId::endOfFeatureId)];

namespace {

void resetData(FeatureDataT& d) {
  d.id = FeatureId::empty;
  d.spawnType = featureSpawnType_static;
  d.glyph = ' ';
  d.tile = TileId::empty;
  d.color = clrYellow;
  d.colorBg = clrBlack;
  d.moveRules.reset();
  d.isSoundPassable = true;
  d.isProjectilePassable = true;
  d.isVisionPassable = true;
  d.isSmokePassable = true;
  d.canHaveBlood = true;
  d.canHaveGore = true;
  d.canHaveCorpse = true;
  d.canHaveStaticFeature = true;
  d.canHaveItem = true;
  d.isBottomless = false;
  d.materialType = materialType_hard;
  d.name_a = "";
  d.name_the = "";
  d.messageOnPlayerBlocked = "The way is blocked.";
  d.messageOnPlayerBlockedBlind = "I bump into something.";
  d.dodgeModifier = 0;
  d.shockWhenAdjacent = 0;
  d.featureThemeSpawnRules.reset();
  d.featuresOnDestroyed.resize(0);
}

void addToListAndReset(FeatureDataT& d) {
  data[int(d.id)] = d;
  resetData(d);
}

void initDataList() {
  FeatureDataT d;
  resetData(d);
  addToListAndReset(d);

  /*---------------------------------------------*/
  d.id = FeatureId::floor;
  d.name_a = "stone floor";
  d.name_the = "the stone floor";
  d.glyph = '.';
  d.color = clrGray;
  d.tile = TileId::floor;
  d.moveRules.setCanMoveCmn();
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::wall;
  d.spawnType = featureSpawnType_other;
  d.name_a = "a stone wall";
  d.name_the = "the stone wall";
  d.glyph = Config::isAsciiWallFullSquare() ? 10 : '#';
  d.color = clrGray;
  d.tile = TileId::wallTop;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propBurrowing);
  d.isSoundPassable = false;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.isSmokePassable = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featuresOnDestroyed.push_back(FeatureId::rubbleHigh);
  d.featuresOnDestroyed.push_back(FeatureId::rubbleLow);
  d.featuresOnDestroyed.push_back(FeatureId::floor);
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::tree;
  d.name_a = "a tree";
  d.name_the = "the tree";
  d.glyph = '|';
  d.color = clrBrownDrk;
  d.tile = TileId::tree;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.isSoundPassable = false;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 1;
  d.messageOnPlayerBlocked = "There is a tree in the way.";
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::grass;
  d.name_a = "grass";
  d.name_the = "the grass";
  d.glyph = '.';
  d.tile = TileId::floor;
  d.color = clrGreen;
  d.moveRules.setCanMoveCmn();
  d.materialType = materialType_soft;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::grassWithered;
  d.name_a = "withered grass";
  d.name_the = "the withered grass";
  d.glyph = '.';
  d.tile = TileId::floor;
  d.color = clrBrownDrk;
  d.moveRules.setCanMoveCmn();
  d.materialType = materialType_soft;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::bush;
  d.name_a = "a shrub";
  d.name_the = "the shrub";
  d.glyph = '"';
  d.color = clrGreen;
  d.tile = TileId::bush;
  d.moveRules.setCanMoveCmn();
  d.materialType = materialType_soft;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::bushWithered;
  d.name_a = "a withered shrub";
  d.name_the = "the withered shrub";
  d.glyph = '"';
  d.color = clrBrownDrk;
  d.tile = TileId::bush;
  d.moveRules.setCanMoveCmn();
  d.materialType = materialType_soft;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::forestPath;
  d.name_a = "a stony path";
  d.name_the = "the stony path";
  d.glyph = '.';
  d.tile = TileId::floor;
  d.color = clrGray;
  d.moveRules.setCanMoveCmn();
  d.canHaveStaticFeature = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::stairs;
  d.spawnType = featureSpawnType_other;
  d.name_a = "a downward staircase";
  d.name_the = "the downward staircase";
  d.glyph = '>';
  d.color = clrWhiteHigh;
  d.tile = TileId::stairsDown;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::lever;
  d.spawnType = featureSpawnType_other;
  d.name_a = "a lever";
  d.name_the = "the lever";
  d.glyph = '%';
  d.color = clrWhite;
  d.tile = TileId::leverLeft;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::brazierGolden;
  d.name_a = "a golden brazier";
  d.name_the = "the golden brazier";
  d.glyph = '0';
  d.color = clrYellow;
  d.tile = TileId::brazier;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featureThemeSpawnRules.set(3, PlacementRule::nextToWallsOrAwayFromWalls,
  {RoomThemeId::ritual});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::shallowWater;
  d.spawnType = featureSpawnType_other;
  d.name_a = "shallow water";
  d.name_the = "the shallow water";
  d.glyph = '~';
  d.color = clrBlueLgt;
  d.tile = TileId::water1;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveStaticFeature = false;
  d.dodgeModifier = -10;
  d.materialType = materialType_fluid;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::deepWater;
  d.spawnType = featureSpawnType_other;
  d.name_a = "deep water";
  d.name_the = "the deep water";
  d.glyph = '~';
  d.color = clrBlue;
  d.tile = TileId::water1;
  d.moveRules.setPropCanMove(propFlying);
  d.moveRules.setPropCanMove(propOoze);
  d.moveRules.setPropCanMove(propEthereal);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveStaticFeature = false;
  d.dodgeModifier = -10;
  d.shockWhenAdjacent = 8;
  d.materialType = materialType_fluid;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::shallowMud;
  d.spawnType = featureSpawnType_other;
  d.name_a = "shallow mud";
  d.name_the = "the shallow mud";
  d.glyph = '~';
  d.color = clrBrownDrk;
  d.tile = TileId::water1;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveStaticFeature = false;
  d.dodgeModifier = -20;
  d.materialType = materialType_fluid;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::poolBlood;
  d.spawnType = featureSpawnType_other;
  d.name_a = "a pool of blood";
  d.name_the = "the pool of blood";
  d.glyph = '~';
  d.color = clrRed;
  d.tile = TileId::water1;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveStaticFeature = false;
  d.dodgeModifier = -10;
  d.shockWhenAdjacent = 3;
  d.materialType = materialType_fluid;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::chasm;
  d.name_a = "a chasm";
  d.name_the = "the chasm";
  d.glyph = ' ';
  d.color = clrBlack;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.isBottomless = true;
  d.messageOnPlayerBlocked = "A chasm lies in my way.";
  d.messageOnPlayerBlockedBlind =
    "I realize I am standing on the edge of a chasm.";
  d.shockWhenAdjacent = 3;
  d.materialType = materialType_empty;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::caveFloor;
  d.name_a = "cavern floor";
  d.name_the = "the cavern floor";
  d.glyph = '.';
  d.color = clrGray;
  d.tile = TileId::floor;
  d.moveRules.setCanMoveCmn();
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::gravestone;
  d.name_a = "a gravestone";
  d.name_the = "the gravestone";
  d.spawnType = featureSpawnType_other;
  d.glyph = '&';
  d.color = clrWhite;
  d.tile = TileId::graveStone;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 2;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::churchBench;
  d.name_a = "a church bench";
  d.name_the = "the church bench";
  d.glyph = '[';
  d.color = clrBrown;
  d.tile = TileId::churchBench;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.moveRules.setPropCanMove(propOoze);
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::churchCarpet;
  d.name_a = "a red carpet";
  d.name_the = "the red carpet";
  d.glyph = '.';
  d.color = clrRed;
  d.tile = TileId::floor;
  d.canHaveStaticFeature = false;
  d.moveRules.setCanMoveCmn();
  d.materialType = materialType_soft;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::rubbleHigh;
  d.name_a = "a big pile of debris";
  d.name_the = "the big pile of debris";
  d.glyph = 8;
  d.color = data[int(FeatureId::wall)].color;
  d.tile = TileId::rubbleHigh;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propOoze);
  d.moveRules.setPropCanMove(propBurrowing);
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.isSmokePassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featuresOnDestroyed.push_back(FeatureId::rubbleLow);
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::rubbleLow;
  d.name_a = "rubble";
  d.name_the = "the rubble";
  d.glyph = ',';
  d.color = data[int(FeatureId::wall)].color;
  d.tile = TileId::rubbleLow;
  d.moveRules.setCanMoveCmn();
  addToListAndReset(d);
  d.featureThemeSpawnRules.set(4, PlacementRule::nextToWallsOrAwayFromWalls,
  {RoomThemeId::plain, RoomThemeId::crypt, RoomThemeId::monster});
  /*---------------------------------------------*/
  d.id = FeatureId::statue;
  d.name_a = "a statue";
  d.name_the = "the statue";
  d.glyph = 5;
  d.color = clrWhite;
  d.tile = TileId::witchOrWarlock;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featureThemeSpawnRules.set(3, PlacementRule::nextToWallsOrAwayFromWalls,
  {RoomThemeId::plain, RoomThemeId::human});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::ghoulStatue;
  d.name_a = "a statue of a ghoulish creature";
  d.name_the = "the statue of a ghoulish creature";
  d.glyph = 'M';
  d.color = clrWhite;
  d.tile = TileId::ghoul;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 8;
  d.featureThemeSpawnRules.set(3, PlacementRule::nextToWallsOrAwayFromWalls,
  {RoomThemeId::plain, RoomThemeId::crypt});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::cocoon;
  d.name_a = "a cocoon";
  d.name_the = "the cocoon";
  d.spawnType = featureSpawnType_other;
  d.glyph = '8';
  d.color = clrWhite;
  d.tile = TileId::cocoon;
  d.isProjectilePassable = true;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 3;
  d.featureThemeSpawnRules.set(3, PlacementRule::nextToWallsOrAwayFromWalls,
  {RoomThemeId::spider});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::chest;
  d.name_a = "a chest";
  d.name_the = "the chest";
  d.spawnType = featureSpawnType_other;
  d.glyph = '+';
  d.color = clrBrownDrk;
  d.tile = TileId::chestClosed;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featureThemeSpawnRules.set(1, PlacementRule::nextToWalls,
  {RoomThemeId::human});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::cabinet;
  d.name_a = "a cabinet";
  d.name_the = "the cabinet";
  d.spawnType = featureSpawnType_other;
  d.glyph = '7';
  d.color = clrBrownDrk;
  d.tile = TileId::cabinetClosd;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featureThemeSpawnRules.set(1, PlacementRule::nextToWalls,
  {RoomThemeId::human});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::fountain;
  d.name_a = "a fountain";
  d.name_the = "the fountain";
  d.spawnType = featureSpawnType_other;
  d.glyph = '%';
  d.color = clrWhiteHigh;
  d.tile = TileId::fountain;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featureThemeSpawnRules.set(1, PlacementRule::awayFromWalls,
  {RoomThemeId::plain, RoomThemeId::human});
  addToListAndReset(d);
  /*---------------------------------------------*/
//  d.id = FeatureId::pillarCarved;
//  d.name_a = "a carved pillar";
//  d.name_the = "the carved pillar";
//  d.spawnType = featureSpawnType_other;
//  d.glyph = '1';
//  d.color = clrGray;
//  d.tile = TileId::pillarCarved;
////  d.canBodyTypePass[bodyType_ooze] = false;
//  d.isProjectilePassable = false;
//  d.isVisionPassable = false;
//  d.canHaveBlood = false;
//  d.canHaveGore = false;
//  d.canHaveCorpse = false;
//  d.canHaveStaticFeature = false;
//  d.canHaveItem = false;
////  d.featureThemeSpawnRules.set(1, PlacementRule::awayFromWalls, RoomThemeId::crypt, RoomThemeId::ritual, RoomThemeId::monster);
//  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::pillar;
  d.name_a = "a pillar";
  d.name_the = "the pillar";
  d.glyph = '|';
  d.color = clrGray;
  d.tile = TileId::pillar;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featureThemeSpawnRules.set(3, PlacementRule::awayFromWalls, {
    RoomThemeId::plain, RoomThemeId::crypt, RoomThemeId::ritual,
    RoomThemeId::monster
  });
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::pillarBroken;
  d.name_a = "a broken pillar";
  d.name_the = "the broken pillar";
  d.glyph = '|';
  d.color = clrGray;
  d.tile = TileId::pillarBroken;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featureThemeSpawnRules.set(4, PlacementRule::awayFromWalls, {
    RoomThemeId::plain, RoomThemeId::crypt, RoomThemeId::ritual,
    RoomThemeId::monster
  });
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::altar;
  d.name_a = "an altar";
  d.name_the = "the altar";
  d.spawnType = featureSpawnType_static;
  d.glyph = '_';
  d.color = clrWhiteHigh;
  d.tile = TileId::altar;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 10;
  d.featureThemeSpawnRules.set(1, PlacementRule::nextToWallsOrAwayFromWalls,
  {RoomThemeId::ritual});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::tomb;
  d.name_a = "a tomb";
  d.name_the = "the tomb";
  d.spawnType = featureSpawnType_other;
  d.glyph = '&';
  d.color = clrGray;
  d.tile = TileId::tomb;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 10;
  d.featureThemeSpawnRules.set(2, PlacementRule::nextToWallsOrAwayFromWalls,
  {RoomThemeId::crypt});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::pit;
  d.name_a = "a pit";
  d.name_the = "the pit";
  d.glyph = '^';
  d.color = clrGray;
  d.tile = TileId::pit;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.isBottomless = true;
  d.canHaveItem = false;
  d.messageOnPlayerBlocked = "A pit lies in my way.";
  d.messageOnPlayerBlockedBlind =
    "I realize I am standing on the edge of a pit.";
  d.shockWhenAdjacent = 5;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::door;
  d.spawnType = featureSpawnType_other;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.featuresOnDestroyed.push_back(FeatureId::rubbleLow);
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::trap;
  d.spawnType = featureSpawnType_other;
  d.moveRules.setCanMoveCmn();
  d.canHaveStaticFeature = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::litDynamite;
  d.spawnType = featureSpawnType_other;
  d.name_a = "a lit stick of dynamite";
  d.name_the = "the lit stick of dynamite";
  d.glyph = '/';
  d.color = clrRedLgt;
  d.tile = TileId::dynamiteLit;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveItem = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::litFlare;
  d.spawnType = featureSpawnType_other;
  d.name_a = "a lit flare";
  d.name_the = "the lit flare";
  d.glyph = '/';
  d.color = clrYellow;
  d.tile = TileId::flareLit;
  d.moveRules.setCanMoveCmn();
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::smoke;
  d.spawnType = featureSpawnType_other;
  d.name_a = "smoke";
  d.name_the = "the smoke";
  d.glyph = '*';
  d.color = clrGray;
  d.tile = TileId::smoke;
  d.moveRules.setCanMoveCmn();
  d.isVisionPassable = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::proxEventWallCrumble;
  d.spawnType = featureSpawnType_other;
  d.moveRules.setCanMoveCmn();
  addToListAndReset(d);
  /*---------------------------------------------*/
}

} //namespace

void init() {
  initDataList();
}

const FeatureDataT* getData(const FeatureId id) {
  return &(data[int(id)]);
}

} //FeatureData
