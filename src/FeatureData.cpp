#include "FeatureData.h"

#include "Init.h"
#include "Colors.h"
#include "Actor.h"
#include "FeatureStatic.h"
#include "FeatureMob.h"
#include "FeatureExaminable.h"
#include "FeatureTrap.h"
#include "FeatureProxEvent.h"

using namespace std;

bool MoveRules::canMove(const vector<PropId>& actorsProps) const {
  if(canMoveCmn_) return true;

  //If not allowing normal move, check if any property overrides this
  for(PropId id : actorsProps) {if(canMoveIfHaveProp_[id]) return true;}

  return false;
}

namespace FeatureData {

FeatureDataT data[int(FeatureId::END)];

namespace {

void resetData(FeatureDataT& d) {
  d.mkObj = [](const Pos& p) {(void)p; return nullptr;};
  d.id = FeatureId::empty;
  d.glyph = ' ';
  d.tile = TileId::empty;
  d.clr = clrYellow;
  d.clrBg = clrBlack;
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
  d.matlType = Matl::stone;
  d.nameA = "";
  d.nameThe = "";
  d.messageOnPlayerBlocked = "The way is blocked.";
  d.messageOnPlayerBlockedBlind = "I bump into something.";
  d.dodgeModifier = 0;
  d.shockWhenAdjacent = 0;
  d.themeSpawnRules.reset();
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
  d.mkObj = [](const Pos& p) {return new Floor(p);};
  d.nameA = "stone floor";
  d.nameThe = "the stone floor";
  d.glyph = '.';
  d.clr = clrGray;
  d.tile = TileId::floor;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::stone;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::bridge;
  d.mkObj = [](const Pos& p) {return new Bridge(p);};
  d.nameA = "a bridge";
  d.nameThe = "the bridge";
  d.clr = clrBrownDrk;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::wood;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::wall;
  d.mkObj = [](const Pos& p) {return new Wall(p);};
  d.nameA = "a stone wall";
  d.nameThe = "the stone wall";
  d.glyph = Config::isAsciiWallFullSquare() ? 10 : '#';
  d.clr = clrGray;
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
  d.matlType = Matl::stone;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::tree;
  d.mkObj = [](const Pos& p) {return new Tree(p);};
  d.nameA = "a tree";
  d.nameThe = "the tree";
  d.glyph = '|';
  d.clr = clrBrownDrk;
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
  d.matlType = Matl::wood;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::grass;
  d.mkObj = [](const Pos& p) {return new Grass(p);};
  d.nameA = "grass";
  d.nameThe = "the grass";
  d.glyph = '.';
  d.tile = TileId::floor;
  d.clr = clrGreen;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::plant;
  addToListAndReset(d);
  /*---------------------------------------------*/
//  d.id = FeatureId::grassWithered;
//  d.nameA = "withered grass";
//  d.nameThe = "the withered grass";
//  d.glyph = '.';
//  d.tile = TileId::floor;
//  d.clr = clrBrownDrk;
//  d.moveRules.setCanMoveCmn();
//  d.matlType = Matl::plant;
//  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::bush;
  d.mkObj = [](const Pos& p) {return new Bush(p);};
  d.nameA = "a shrub";
  d.nameThe = "the shrub";
  d.glyph = '"';
  d.clr = clrGreen;
  d.tile = TileId::bush;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::plant;
  addToListAndReset(d);
  /*---------------------------------------------*/
//  d.id = FeatureId::bushWithered;
//  d.nameA = "a withered shrub";
//  d.nameThe = "the withered shrub";
//  d.glyph = '"';
//  d.clr = clrBrownDrk;
//  d.tile = TileId::bush;
//  d.moveRules.setCanMoveCmn();
//  d.matlType = Matl::plant;
//  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::stairs;
  d.mkObj = [](const Pos& p) {return new Stairs(p);};
  d.nameA = "a downward staircase";
  d.nameThe = "the downward staircase";
  d.glyph = '>';
  d.clr = clrWhiteHigh;
  d.tile = TileId::stairsDown;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::lever;
  d.mkObj = [](const Pos& p) {return new Lever(p);};
  d.nameA = "a lever";
  d.nameThe = "the lever";
  d.glyph = '%';
  d.clr = clrWhite;
  d.tile = TileId::leverLeft;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.matlType = Matl::metal;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::brazier;
  d.mkObj = [](const Pos& p) {return new Brazier(p);};
  d.nameA = "a golden brazier";
  d.nameThe = "the golden brazier";
  d.glyph = '0';
  d.clr = clrYellow;
  d.tile = TileId::brazier;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.matlType = Matl::metal;
  d.themeSpawnRules.set(3, PlacementRule::either, {RoomType::ritual});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::liquidShallow;
  d.mkObj = [](const Pos& p) {return new LiquidShallow(p);};
  d.glyph = '~';
  d.tile = TileId::water1;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveStaticFeature = false;
  d.dodgeModifier = -10;
  d.matlType = Matl::fluid;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::liquidDeep;
  d.mkObj = [](const Pos& p) {return new LiquidDeep(p);};
  d.glyph = '~';
  d.tile = TileId::water1;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveStaticFeature = false;
  d.matlType = Matl::fluid;
  addToListAndReset(d);
  /*---------------------------------------------*/
//  d.id = FeatureId::shallowWater;
//  d.nameA = "shallow water";
//  d.nameThe = "the shallow water";
//  d.glyph = '~';
//  d.clr = clrBlueLgt;
//  d.tile = TileId::water1;
//  d.moveRules.setCanMoveCmn();
//  d.canHaveBlood = false;
//  d.canHaveGore = false;
//  d.canHaveStaticFeature = false;
//  d.dodgeModifier = -10;
//  d.matlType = Matl::fluid;
//  addToListAndReset(d);
  /*---------------------------------------------*/
//  d.id = FeatureId::deepWater;
//  d.nameA = "deep water";
//  d.nameThe = "the deep water";
//  d.glyph = '~';
//  d.clr = clrBlue;
//  d.tile = TileId::water1;
//  d.moveRules.setPropCanMove(propFlying);
//  d.moveRules.setPropCanMove(propOoze);
//  d.moveRules.setPropCanMove(propEthereal);
//  d.canHaveBlood = false;
//  d.canHaveGore = false;
//  d.canHaveStaticFeature = false;
//  d.dodgeModifier = -10;
//  d.shockWhenAdjacent = 8;
//  d.matlType = Matl::fluid;
//  addToListAndReset(d);
  /*---------------------------------------------*/
//  d.id = FeatureId::shallowMud;
//  d.nameA = "shallow mud";
//  d.nameThe = "the shallow mud";
//  d.glyph = '~';
//  d.clr = clrBrownDrk;
//  d.tile = TileId::water1;
//  d.moveRules.setCanMoveCmn();
//  d.canHaveBlood = false;
//  d.canHaveGore = false;
//  d.canHaveStaticFeature = false;
//  d.dodgeModifier = -20;
//  d.matlType = Matl::fluid;
//  addToListAndReset(d);
  /*---------------------------------------------*/
//  d.id = FeatureId::poolBlood;
//  d.nameA = "a pool of blood";
//  d.nameThe = "the pool of blood";
//  d.glyph = '~';
//  d.clr = clrRed;
//  d.tile = TileId::water1;
//  d.moveRules.setCanMoveCmn();
//  d.canHaveBlood = false;
//  d.canHaveGore = false;
//  d.canHaveStaticFeature = false;
//  d.dodgeModifier = -10;
//  d.shockWhenAdjacent = 3;
//  d.matlType = Matl::fluid;
//  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::chasm;
  d.mkObj = [](const Pos& p) {return new Chasm(p);};
  d.nameA = "a chasm";
  d.nameThe = "the chasm";
  d.glyph = ' ';
  d.clr = clrBlack;
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
  d.matlType = Matl::empty;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::gravestone;
  d.mkObj = [](const Pos& p) {return new GraveStone(p);};
  d.nameA = "a gravestone";
  d.nameThe = "the gravestone";
  d.glyph = '&';
  d.clr = clrWhite;
  d.tile = TileId::graveStone;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 2;
  d.matlType = Matl::stone;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::churchBench;
  d.mkObj = [](const Pos& p) {return new ChurchBench(p);};
  d.nameA = "a church bench";
  d.nameThe = "the church bench";
  d.glyph = '[';
  d.clr = clrBrown;
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
  d.matlType = Matl::wood;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::carpet;
  d.mkObj = [](const Pos& p) {return new Carpet(p);};
  d.nameA = "a red carpet";
  d.nameThe = "the red carpet";
  d.glyph = '.';
  d.clr = clrRed;
  d.tile = TileId::floor;
  d.canHaveStaticFeature = false;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::cloth;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::rubbleHigh;
  d.mkObj = [](const Pos& p) {return new RubbleHigh(p);};
  d.nameA = "a big pile of debris";
  d.nameThe = "the big pile of debris";
  d.glyph = 8;
  d.clr = data[int(FeatureId::wall)].clr;
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
  d.matlType = Matl::stone;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::rubbleLow;
  d.mkObj = [](const Pos& p) {return new RubbleLow(p);};
  d.nameA = "rubble";
  d.nameThe = "the rubble";
  d.glyph = ',';
  d.clr = data[int(FeatureId::wall)].clr;
  d.tile = TileId::rubbleLow;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(4, PlacementRule::either, {
    RoomType::plain, RoomType::crypt, RoomType::monster
  });
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::statue;
  d.mkObj = [](const Pos& p) {return new Statue(p);};
  d.nameA = "a statue";
  d.nameThe = "the statue";
  d.glyph = 5;
  d.clr = clrWhite;
  d.tile = TileId::witchOrWarlock;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(3, PlacementRule::either, {
    RoomType::plain, RoomType::human
  });
  addToListAndReset(d);
  /*---------------------------------------------*/
//  d.id = FeatureId::ghoulStatue;
//  d.mkObj = [](const Pos& p) {return new Statue(p);};
//  d.nameA = "a statue of a ghoulish creature";
//  d.nameThe = "the statue of a ghoulish creature";
//  d.glyph = 'M';
//  d.clr = clrWhite;
//  d.tile = TileId::ghoul;
//  d.isProjectilePassable = false;
//  d.isVisionPassable = false;
//  d.canHaveBlood = false;
//  d.canHaveGore = false;
//  d.canHaveCorpse = false;
//  d.canHaveStaticFeature = false;
//  d.canHaveItem = false;
//  d.shockWhenAdjacent = 8;
//  d.themeSpawnRules.set(3, PlacementRule::either, {
//    RoomType::plain, RoomType::crypt
//  });
//  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::cocoon;
  d.mkObj = [](const Pos& p) {return new Cocoon(p);};
  d.nameA = "a cocoon";
  d.nameThe = "the cocoon";
  d.glyph = '8';
  d.clr = clrWhite;
  d.tile = TileId::cocoon;
  d.isProjectilePassable = true;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 3;
  d.matlType = Matl::cloth;
  d.themeSpawnRules.set(3, PlacementRule::either, {RoomType::spider});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::chest;
  d.mkObj = [](const Pos& p) {return new Chest(p);};
  d.nameA = "a chest";
  d.nameThe = "the chest";
  d.glyph = '+';
  d.clr = clrBrownDrk;
  d.tile = TileId::chestClosed;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.themeSpawnRules.set(1, PlacementRule::nextToWalls, {RoomType::human});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::cabinet;
  d.mkObj = [](const Pos& p) {return new Cabinet(p);};
  d.nameA = "a cabinet";
  d.nameThe = "the cabinet";
  d.glyph = '7';
  d.clr = clrBrownDrk;
  d.tile = TileId::cabinetClosd;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.matlType = Matl::wood;
  d.themeSpawnRules.set(1, PlacementRule::nextToWalls, {RoomType::human});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::fountain;
  d.mkObj = [](const Pos& p) {return new Fountain(p);};
  d.nameA = "a fountain";
  d.nameThe = "the fountain";
  d.glyph = '%';
  d.clr = clrWhiteHigh;
  d.tile = TileId::fountain;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(1, PlacementRule::awayFromWalls, {
    RoomType::plain, RoomType::human
  });
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::pillar;
  d.mkObj = [](const Pos& p) {return new Pillar(p);};
  d.nameA = "a pillar";
  d.nameThe = "the pillar";
  d.glyph = '|';
  d.clr = clrGray;
  d.tile = TileId::pillar;
  d.isProjectilePassable = false;
  d.isVisionPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(3, PlacementRule::awayFromWalls, {
    RoomType::plain, RoomType::crypt, RoomType::ritual, RoomType::monster
  });
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::altar;
  d.mkObj = [](const Pos& p) {return new Altar(p);};
  d.nameA = "an altar";
  d.nameThe = "the altar";
  d.glyph = '_';
  d.clr = clrWhiteHigh;
  d.tile = TileId::altar;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 10;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(1, PlacementRule::either, {RoomType::ritual});
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::tomb;
  d.mkObj = [](const Pos& p) {return new Tomb(p);};
  d.nameA = "a tomb";
  d.nameThe = "the tomb";
  d.glyph = '&';
  d.clr = clrGray;
  d.tile = TileId::tomb;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 10;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(2, PlacementRule::either, {RoomType::crypt});
  addToListAndReset(d);
  /*---------------------------------------------*/
//  d.id = FeatureId::pit;
//  d.mkObj = [](const Pos& p) {return new Pit(p);};
//  d.nameA = "a pit";
//  d.nameThe = "the pit";
//  d.glyph = '^';
//  d.clr = clrGray;
//  d.tile = TileId::pit;
//  d.moveRules.setPropCanMove(propEthereal);
//  d.moveRules.setPropCanMove(propFlying);
//  d.canHaveBlood = false;
//  d.canHaveGore = false;
//  d.canHaveCorpse = false;
//  d.canHaveStaticFeature = false;
//  d.isBottomless = true;
//  d.canHaveItem = false;
//  d.messageOnPlayerBlocked = "A pit lies in my way.";
//  d.messageOnPlayerBlockedBlind =
//    "I realize I am standing on the edge of a pit.";
//  d.shockWhenAdjacent = 5;
//  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::door;
  d.mkObj = [](const Pos& p) {return new Door(p);};
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveStaticFeature = false;
  d.canHaveItem = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::trap;
  d.mkObj = [](const Pos& p) {return new Trap(p);};
  d.moveRules.setCanMoveCmn();
  d.canHaveStaticFeature = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::litDynamite;
  d.mkObj = [](const Pos& p) {return new LitDynamite(p);};
  d.nameA = "a lit stick of dynamite";
  d.nameThe = "the lit stick of dynamite";
  d.glyph = '/';
  d.clr = clrRedLgt;
  d.tile = TileId::dynamiteLit;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveItem = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::litFlare;
  d.mkObj = [](const Pos& p) {return new LitFlare(p);};
  d.nameA = "a lit flare";
  d.nameThe = "the lit flare";
  d.glyph = '/';
  d.clr = clrYellow;
  d.tile = TileId::flareLit;
  d.moveRules.setCanMoveCmn();
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::smoke;
  d.mkObj = [](const Pos& p) {return new Smoke(p);};
  d.nameA = "smoke";
  d.nameThe = "the smoke";
  d.glyph = '*';
  d.clr = clrGrayDrk;
  d.tile = TileId::smoke;
  d.moveRules.setCanMoveCmn();
  d.isVisionPassable = false;
  addToListAndReset(d);
  /*---------------------------------------------*/
  d.id = FeatureId::proxEventWallCrumble;
  d.mkObj = [](const Pos& p) {return new ProxEventWallCrumble(p);};
  d.moveRules.setCanMoveCmn();
  addToListAndReset(d);
  /*---------------------------------------------*/
}

} //namespace

void init() {
  initDataList();
}

const FeatureDataT& getData(const FeatureId id) {
  assert(id != FeatureId::empty);
  assert(id != FeatureId::END);
  return data[int(id)];
}

} //FeatureData
