#include "FeatureData.h"

#include "Init.h"
#include "Colors.h"
#include "Actor.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"
#include "FeatureTrap.h"
#include "FeatureProxEvent.h"
#include "FeatureDoor.h"

using namespace std;

bool MoveRules::canMove(const bool actorPropIds[endOfPropIds]) const
{
  if(canMoveCmn_)
  {
    return true;
  }

  //If not allowing normal move, check if any property overrides this
  for(int i = 0; i < endOfPropIds; ++i)
  {
    if(actorPropIds[i] && canMoveIfHaveProp_[i])
    {
      return true;
    }
  }

  return false;
}

namespace FeatureData
{

FeatureDataT data[int(FeatureId::END)];

namespace
{

void resetData(FeatureDataT& d)
{
  d.mkObj = [](const Pos & p) {(void)p; return nullptr;};
  d.id = FeatureId::END;
  d.glyph = ' ';
  d.tile = TileId::empty;
  d.moveRules.reset();
  d.isSoundPassable = true;
  d.isProjectilePassable = true;
  d.isLosPassable = true;
  d.isSmokePassable = true;
  d.canHaveBlood = true;
  d.canHaveGore = true;
  d.canHaveCorpse = true;
  d.canHaveRigid = true;
  d.canHaveItem = true;
  d.isBottomless = false;
  d.matlType = Matl::stone;
  d.msgOnPlayerBlocked = "The way is blocked.";
  d.msgOnPlayerBlockedBlind = "I bump into something.";
  d.dodgeModifier = 0;
  d.shockWhenAdjacent = 0;
  d.themeSpawnRules.reset();
}

void addToListAndReset(FeatureDataT& d)
{
  data[int(d.id)] = d;
  resetData(d);
}

void initDataList()
{
  FeatureDataT d;
  resetData(d);

  //---------------------------------------------------------------------------
  d.id = FeatureId::floor;
  d.mkObj = [](const Pos & p) {return new Floor(p);};
  d.glyph = '.';
  d.tile = TileId::floor;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::stone;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::bridge;
  d.mkObj = [](const Pos & p) {return new Bridge(p);};
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::wood;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::wall;
  d.mkObj = [](const Pos & p) {return new Wall(p);};
  d.glyph = Config::isAsciiWallFullSquare() ? 10 : '#';
  d.tile = TileId::wallTop;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propBurrowing);
  d.isSoundPassable = false;
  d.isProjectilePassable = false;
  d.isLosPassable = false;
  d.isSmokePassable = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::tree;
  d.mkObj = [](const Pos & p) {return new Tree(p);};
  d.glyph = '|';
  d.tile = TileId::tree;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.isSoundPassable = false;
  d.isProjectilePassable = false;
  d.isLosPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 1;
  d.msgOnPlayerBlocked = "There is a tree in the way.";
  d.matlType = Matl::wood;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::grass;
  d.mkObj = [](const Pos & p) {return new Grass(p);};
  d.glyph = '.';
  d.tile = TileId::floor;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::plant;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::bush;
  d.mkObj = [](const Pos & p) {return new Bush(p);};
  d.glyph = '"';
  d.tile = TileId::bush;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::plant;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::stairs;
  d.mkObj = [](const Pos & p) {return new Stairs(p);};
  d.glyph = '>';
  d.tile = TileId::stairsDown;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::lever;
  d.mkObj = [](const Pos & p) {return new Lever(p);};
  d.glyph = '%';
  d.tile = TileId::leverLeft;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::metal;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::brazier;
  d.mkObj = [](const Pos & p) {return new Brazier(p);};
  d.glyph = '0';
  d.tile = TileId::brazier;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::metal;
  d.themeSpawnRules.set(3, PlacementRule::either, {RoomType::ritual});
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::liquidShallow;
  d.mkObj = [](const Pos & p) {return new LiquidShallow(p);};
  d.glyph = '~';
  d.tile = TileId::water1;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveRigid = false;
  d.dodgeModifier = -10;
  d.matlType = Matl::fluid;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::liquidDeep;
  d.mkObj = [](const Pos & p) {return new LiquidDeep(p);};
  d.glyph = '~';
  d.tile = TileId::water1;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveRigid = false;
  d.matlType = Matl::fluid;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::chasm;
  d.mkObj = [](const Pos & p) {return new Chasm(p);};
  d.glyph = ' ';
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.isBottomless = true;
  d.msgOnPlayerBlocked = "A chasm lies in my way.";
  d.msgOnPlayerBlockedBlind =
    "I realize I am standing on the edge of a chasm.";
  d.shockWhenAdjacent = 3;
  d.matlType = Matl::empty;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::gravestone;
  d.mkObj = [](const Pos & p) {return new GraveStone(p);};
  d.glyph = '&';
  d.tile = TileId::graveStone;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 2;
  d.matlType = Matl::stone;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::churchBench;
  d.mkObj = [](const Pos & p) {return new ChurchBench(p);};
  d.glyph = '[';
  d.tile = TileId::churchBench;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.moveRules.setPropCanMove(propOoze);
  d.isProjectilePassable = false;
  d.isLosPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::wood;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::carpet;
  d.mkObj = [](const Pos & p) {return new Carpet(p);};
  d.glyph = '.';
  d.tile = TileId::floor;
  d.canHaveRigid = false;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::cloth;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::rubbleHigh;
  d.mkObj = [](const Pos & p) {return new RubbleHigh(p);};
  d.glyph = 8;
  d.tile = TileId::rubbleHigh;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propOoze);
  d.moveRules.setPropCanMove(propBurrowing);
  d.isProjectilePassable = false;
  d.isLosPassable = false;
  d.isSmokePassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::rubbleLow;
  d.mkObj = [](const Pos & p) {return new RubbleLow(p);};
  d.glyph = ',';
  d.tile = TileId::rubbleLow;
  d.moveRules.setCanMoveCmn();
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(4, PlacementRule::either,
  {
    RoomType::plain, RoomType::crypt, RoomType::monster
  });
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::statue;
  d.mkObj = [](const Pos & p) {return new Statue(p);};
  d.glyph = 5;
  d.tile = TileId::witchOrWarlock;
  d.isProjectilePassable = false;
  d.isLosPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(3, PlacementRule::either,
  {
    RoomType::plain, RoomType::human, RoomType::forest
  });
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::cocoon;
  d.mkObj = [](const Pos & p) {return new Cocoon(p);};
  d.glyph = '8';
  d.tile = TileId::cocoon;
  d.isProjectilePassable = true;
  d.isLosPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 3;
  d.matlType = Matl::cloth;
  d.themeSpawnRules.set(3, PlacementRule::either, {RoomType::spider});
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::chest;
  d.mkObj = [](const Pos & p) {return new Chest(p);};
  d.glyph = '+';
  d.tile = TileId::chestClosed;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.themeSpawnRules.set(1, PlacementRule::adjToWalls, {RoomType::human});
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::cabinet;
  d.mkObj = [](const Pos & p) {return new Cabinet(p);};
  d.glyph = '7';
  d.tile = TileId::cabinetClosd;
  d.isProjectilePassable = false;
  d.isLosPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::wood;
  d.themeSpawnRules.set(1, PlacementRule::adjToWalls, {RoomType::human});
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::fountain;
  d.mkObj = [](const Pos & p) {return new Fountain(p);};
  d.glyph = '%';
  d.tile = TileId::fountain;
  d.isProjectilePassable = false;
  d.isLosPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(1, PlacementRule::awayFromWalls,
  {
    RoomType::plain, RoomType::human, RoomType::forest
  });
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::pillar;
  d.mkObj = [](const Pos & p) {return new Pillar(p);};
  d.glyph = '|';
  d.tile = TileId::pillar;
  d.isProjectilePassable = false;
  d.isLosPassable = false;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(3, PlacementRule::awayFromWalls,
  {
    RoomType::plain, RoomType::crypt, RoomType::ritual, RoomType::monster
  });
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::altar;
  d.mkObj = [](const Pos & p) {return new Altar(p);};
  d.glyph = '_';
  d.tile = TileId::altar;
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 10;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(1, PlacementRule::either, {RoomType::ritual, RoomType::forest});
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::tomb;
  d.mkObj = [](const Pos & p) {return new Tomb(p);};
  d.glyph = '&';
  d.tile = TileId::tomb;
  d.moveRules.setPropCanMove(propEthereal);
  d.moveRules.setPropCanMove(propFlying);
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  d.shockWhenAdjacent = 10;
  d.matlType = Matl::stone;
  d.themeSpawnRules.set(2, PlacementRule::either, {RoomType::crypt, RoomType::forest});
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::door;
  d.mkObj = [](const Pos & p) {return new Door(p);};
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveRigid = false;
  d.canHaveItem = false;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::trap;
  d.mkObj = [](const Pos & p) {return new Trap(p);};
  d.moveRules.setCanMoveCmn();
  d.canHaveRigid = false;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::litDynamite;
  d.mkObj = [](const Pos & p) {return new LitDynamite(p);};
  d.glyph = '/';
  d.tile = TileId::dynamiteLit;
  d.moveRules.setCanMoveCmn();
  d.canHaveBlood = false;
  d.canHaveGore = false;
  d.canHaveCorpse = false;
  d.canHaveItem = false;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::litFlare;
  d.mkObj = [](const Pos & p) {return new LitFlare(p);};
  d.glyph = '/';
  d.tile = TileId::flareLit;
  d.moveRules.setCanMoveCmn();
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::smoke;
  d.mkObj = [](const Pos & p) {return new Smoke(p);};
  d.glyph = '*';
  d.tile = TileId::smoke;
  d.moveRules.setCanMoveCmn();
  d.isLosPassable = false;
  addToListAndReset(d);
  //---------------------------------------------------------------------------
  d.id = FeatureId::proxEventWallCrumble;
  d.mkObj = [](const Pos & p) {return new ProxEventWallCrumble(p);};
  d.moveRules.setCanMoveCmn();
  addToListAndReset(d);
  //---------------------------------------------------------------------------
}

} //namespace

void init()
{
  TRACE_FUNC_BEGIN;
  initDataList();
  TRACE_FUNC_END;
}

const FeatureDataT& getData(const FeatureId id)
{
  assert(id != FeatureId::END);
  assert(id != FeatureId::END);
  return data[int(id)];
}

} //FeatureData
