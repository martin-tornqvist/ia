#include "Room.h"

#include <algorithm>

#include "Init.h"
#include "Utils.h"
#include "Map.h"
#include "MapParsing.h"
#include "FeatureRigid.h"
#include "MapGen.h"
#include "Gods.h"

#ifdef DEMO_MODE
#include "Render.h"
#include "SdlWrapper.h"
#endif // DEMO_MODE

using namespace std;

namespace
{

vector<RoomType> roomBucket_;

void addToRoomBucket(const RoomType type, const size_t NR)
{
  if (NR > 0)
  {
    roomBucket_.reserve(roomBucket_.size() + NR);

    for (size_t i = 0; i < NR; ++i) {roomBucket_.push_back(type);}
  }
}

} //namespace

//------------------------------------------------------------------- ROOM FACTORY
namespace RoomFactory
{

void initRoomBucket()
{
  TRACE_FUNC_BEGIN;

  roomBucket_.clear();

  const int DLVL = Map::dlvl;

  if (DLVL <= DLVL_LAST_EARLY_GAME)
  {
    addToRoomBucket(RoomType::human,    Rnd::range(3, 6));
    addToRoomBucket(RoomType::ritual,   Rnd::range(0, 1));
    addToRoomBucket(RoomType::spider,   Rnd::range(0, 3));
    addToRoomBucket(RoomType::crypt,    Rnd::range(1, 4));
    addToRoomBucket(RoomType::monster,  Rnd::range(0, 3));
    addToRoomBucket(RoomType::flooded,  Rnd::range(0, 1));
    addToRoomBucket(RoomType::muddy,    Rnd::range(0, 1));

    const size_t NR_PLAIN_ROOM_PER_THEMED = 1;

    addToRoomBucket(RoomType::plain, roomBucket_.size() * NR_PLAIN_ROOM_PER_THEMED);
  }
  else if (DLVL <= DLVL_LAST_MID_GAME)
  {
    addToRoomBucket(RoomType::human,    Rnd::range(1, 3));
    addToRoomBucket(RoomType::ritual,   Rnd::range(0, 1));
    addToRoomBucket(RoomType::spider,   Rnd::range(0, 3));
    addToRoomBucket(RoomType::crypt,    Rnd::range(0, 4));
    addToRoomBucket(RoomType::monster,  Rnd::range(0, 3));
    addToRoomBucket(RoomType::flooded,  Rnd::range(0, 2));
    addToRoomBucket(RoomType::muddy,    Rnd::range(0, 2));
    addToRoomBucket(RoomType::cave,     Rnd::range(1, 4));
    addToRoomBucket(RoomType::chasm,    Rnd::range(1, 3));
    addToRoomBucket(RoomType::forest,   Rnd::range(1, 4));

    const size_t NR_PLAIN_ROOM_PER_THEMED = 1;

    addToRoomBucket(RoomType::plain, roomBucket_.size() * NR_PLAIN_ROOM_PER_THEMED);
  }
  else
  {
    addToRoomBucket(RoomType::monster,  Rnd::range(0, 3));
    addToRoomBucket(RoomType::spider,   Rnd::range(0, 3));
    addToRoomBucket(RoomType::flooded,  Rnd::range(0, 2));
    addToRoomBucket(RoomType::muddy,    Rnd::range(0, 2));
    addToRoomBucket(RoomType::cave,     Rnd::range(3, 4));
    addToRoomBucket(RoomType::chasm,    Rnd::range(2, 4));
    addToRoomBucket(RoomType::forest,   Rnd::range(1, 4));
  }

  std::random_shuffle(begin(roomBucket_), end(roomBucket_));

  TRACE_FUNC_END;
}

Room* mk(const RoomType type, const Rect& r)
{
  switch (type)
  {
    case RoomType::cave:          return new CaveRoom(r);
    case RoomType::chasm:         return new ChasmRoom(r);
    case RoomType::crypt:         return new CryptRoom(r);
    case RoomType::flooded:       return new FloodedRoom(r);
    case RoomType::human:         return new HumanRoom(r);
    case RoomType::monster:       return new MonsterRoom(r);
    case RoomType::muddy:         return new MuddyRoom(r);
    case RoomType::plain:         return new PlainRoom(r);
    case RoomType::ritual:        return new RitualRoom(r);
    case RoomType::spider:        return new SpiderRoom(r);
    case RoomType::forest:        return new ForestRoom(r);
    case RoomType::END_OF_STD_ROOMS:
      TRACE << "Illegal room type id: " << int (type) << endl;
      assert(false);
      return nullptr;
    case RoomType::corrLink:      return new CorrLinkRoom(r);
    case RoomType::crumbleRoom:   return new CrumbleRoom(r);
    case RoomType::river:         return new RiverRoom(r);
  }
  TRACE << "Unhandled room type id: " << int (type) << endl;
  assert(false);
  return nullptr;
}

Room* mkRandomAllowedStdRoom(const Rect& r, const bool IS_SUBROOM)
{
  TRACE_FUNC_BEGIN_VERBOSE;

  auto roomBucketIt = begin(roomBucket_);

  Room* room = nullptr;

  while (true)
  {
    if (roomBucketIt == end(roomBucket_))
    {
      //No more rooms to pick from, generate a new room bucket
      initRoomBucket();
      roomBucketIt = begin(roomBucket_);
    }
    else
    {
      const RoomType    roomType  = *roomBucketIt;
      room                        = mk(roomType, r);
      room->isSubRoom_            = IS_SUBROOM;
      StdRoom* const    stdRoom   = static_cast<StdRoom*>(room);

      if (stdRoom->isAllowed())
      {
        roomBucket_.erase(roomBucketIt);

        TRACE_FUNC_END_VERBOSE << "Made room type: " << int (roomType) << endl;
        break;
      }
      else //Room not allowed (e.g. wrong dimensions)
      {
        delete room;
        ++roomBucketIt; //Try next room type in the bucket
      }
    }
  }

  TRACE_FUNC_END_VERBOSE;
  return room;
}

} //RoomFactory

//------------------------------------------------------------------- ROOM
Room::Room(Rect r, RoomType type) :
  r_          (r),
  type_       (type),
  isSubRoom_  (false) {}

void Room::mkDrk() const
{
  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      if (Map::roomMap[x][y] == this) {Map::cells[x][y].isDark = true;}
    }
  }

  //Also make sub rooms dark
  for (Room* const subRoom : subRooms_) {subRoom->mkDrk();}
}

//------------------------------------------------------------------- STANDARD ROOM
void StdRoom::onPreConnect(bool doorProposals[MAP_W][MAP_H])
{
  onPreConnect_(doorProposals);
}

void StdRoom::onPostConnect(bool doorProposals[MAP_W][MAP_H])
{
  placeAutoFeatures();

  onPostConnect_(doorProposals);

  //Make dark?
  int pctChanceDark = getBasePctChanceDrk() - 15;
  pctChanceDark += Map::dlvl; //Increase with higher dungeon level
  constrInRange(0, pctChanceDark, 100);
  if (Rnd::percentile() < pctChanceDark) {mkDrk();}
}

size_t StdRoom::tryGetAutoFeaturePlacement(
  const vector<Pos>& adjToWalls, const vector<Pos>& awayFromWalls,
  const vector<const FeatureDataT*>& featureDataBucket, Pos& posRef) const
{
  TRACE_FUNC_BEGIN_VERBOSE;

  if (featureDataBucket.empty())
  {
    posRef = Pos(-1, -1);
    return 0;
  }

  const bool IS_ADJ_TO_WALLS_AVAIL    = !adjToWalls.empty();
  const bool IS_AWAY_FROM_WALLS_AVAIL = !awayFromWalls.empty();

  if (!IS_ADJ_TO_WALLS_AVAIL && !IS_AWAY_FROM_WALLS_AVAIL)
  {
    TRACE_FUNC_END_VERBOSE << "No eligible cells found" << endl;
    posRef = Pos(-1, -1);
    return 0;
  }

  //TODO Use bucket instead

  const int NR_ATTEMPTS_TO_FIND_POS = 100;
  for (int i = 0; i < NR_ATTEMPTS_TO_FIND_POS; ++i)
  {
    const size_t      NR_DATA = featureDataBucket.size();
    const size_t      ELEMENT = Rnd::range(0, NR_DATA - 1);
    const auto* const data    = featureDataBucket[ELEMENT];

    if (IS_ADJ_TO_WALLS_AVAIL &&
        data->roomSpawnRules.getPlacementRule() == PlacementRule::adjToWalls)
    {
      posRef = adjToWalls[Rnd::range(0, adjToWalls.size() - 1)];
      TRACE_FUNC_END_VERBOSE;
      return ELEMENT;
    }

    if (IS_AWAY_FROM_WALLS_AVAIL &&
        data->roomSpawnRules.getPlacementRule() == PlacementRule::awayFromWalls)
    {
      posRef = awayFromWalls[Rnd::range(0, awayFromWalls.size() - 1)];
      TRACE_FUNC_END_VERBOSE;
      return ELEMENT;
    }

    if (data->roomSpawnRules.getPlacementRule() == PlacementRule::either)
    {
      if (Rnd::coinToss())
      {
        if (IS_ADJ_TO_WALLS_AVAIL)
        {
          posRef = adjToWalls[Rnd::range(0, adjToWalls.size() - 1)];
          TRACE_FUNC_END_VERBOSE;
          return ELEMENT;

        }
      }
      else
      {
        if (IS_AWAY_FROM_WALLS_AVAIL)
        {
          posRef = awayFromWalls[Rnd::range(0, awayFromWalls.size() - 1)];
          TRACE_FUNC_END_VERBOSE;
          return ELEMENT;
        }
      }
    }
  }
  TRACE_FUNC_END_VERBOSE;
  return 0;
}

int StdRoom::placeAutoFeatures()
{
  TRACE_FUNC_BEGIN;

  vector<const FeatureDataT*> featureBucket;

  for (int i = 0; i < int (FeatureId::END); ++i)
  {
    const auto& d           = FeatureData::getData((FeatureId)(i));
    const auto& spawnRules  = d.roomSpawnRules;

    if (spawnRules.isBelongingToRoomType(type_) &&
        Utils::isValInRange(Map::dlvl, spawnRules.getDlvlsAllowed()))
    {
      featureBucket.push_back(&d);
    }
  }

  vector<Pos> adjToWalls;
  vector<Pos> awayFromWalls;

  MapPatterns::getCellsInRoom(*this, adjToWalls, awayFromWalls);

  vector<int> spawnCount(featureBucket.size(), 0);

  int nrFeaturesLeftToPlace = Rnd::range(getNrAutoFeaturesAllowed());
  int nrFeaturesPlaced      = 0;

  while (true)
  {
    if (nrFeaturesLeftToPlace == 0)
    {
      TRACE_FUNC_END << "Placed enough features" << endl;
      return nrFeaturesPlaced;
    }

    Pos pos(-1, -1);

    const size_t FEATURE_IDX =
      tryGetAutoFeaturePlacement(adjToWalls, awayFromWalls, featureBucket, pos);

    if (pos.x >= 0)
    {
      assert(FEATURE_IDX < featureBucket.size());

      const FeatureDataT* d = featureBucket[FEATURE_IDX];

      TRACE << "Placing feature" << endl;
      Map::put(static_cast<Rigid*>(d->mkObj(pos)));
      ++spawnCount[FEATURE_IDX];

      nrFeaturesLeftToPlace--;
      nrFeaturesPlaced++;

      //Check if more of this feature can be spawned. If not, erase it.
      if (spawnCount[FEATURE_IDX] >= d->roomSpawnRules.getMaxNrInRoom())
      {
        spawnCount   .erase(spawnCount   .begin() + FEATURE_IDX);
        featureBucket.erase(featureBucket.begin() + FEATURE_IDX);

        if (featureBucket.empty())
        {
          TRACE_FUNC_END << "No more features to place" << endl;
          return nrFeaturesPlaced;
        }
      }

      //Erase all adjacent positions
      auto isAdj = [&](const Pos & p) {return Utils::isPosAdj(p, pos, true);};
      adjToWalls   .erase(remove_if(begin(adjToWalls),    end(adjToWalls),    isAdj),
                          end(adjToWalls));
      awayFromWalls.erase(remove_if(begin(awayFromWalls), end(awayFromWalls), isAdj),
                          end(awayFromWalls));
    }
    else //No good feature placement found
    {
      TRACE_FUNC_END << "No remaining positions to place feature" << endl;
      return nrFeaturesPlaced;
    }
  }
}

//------------------------------------------------------------------- PLAIN ROOM
Range PlainRoom::getNrAutoFeaturesAllowed() const
{
  if (Rnd::oneIn(4))
  {
    return {1, 2};
  }
  else
  {
    return {0, 0};
  }
}

int PlainRoom::getBasePctChanceDrk() const
{
  return 5;
}

void PlainRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  MapGenUtils::cutRoomCorners (*this);
  if (Rnd::fraction(1, 3)) {MapGenUtils::mkPillarsInRoom(*this);}
}

void PlainRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

}

//------------------------------------------------------------------- HUMAN ROOM
Range HumanRoom::getNrAutoFeaturesAllowed() const
{
  return {3, 6};
}

int HumanRoom::getBasePctChanceDrk() const
{
  return 10;
}

bool HumanRoom::isAllowed() const
{
  return r_.getMinDim() >= 4 && r_.getMaxDim() <= 8;
}

void HumanRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  MapGenUtils::cutRoomCorners (*this);
  if (Rnd::fraction(1, 3)) {MapGenUtils::mkPillarsInRoom(*this);}
}

void HumanRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  if (Rnd::coinToss())
  {
    bool blocked[MAP_W][MAP_H];
    MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

    for (int x = r_.p0.x + 1; x <= r_.p1.x - 1; ++x)
    {
      for (int y = r_.p0.y + 1; y <= r_.p1.y - 1; ++y)
      {
        if (!blocked[x][y] && Map::roomMap[x][y] == this)
        {
          Carpet* const carpet = new Carpet(Pos(x, y));
          Map::put(carpet);
        }
      }
    }
  }
}

//------------------------------------------------------------------- RITUAL ROOM
Range RitualRoom::getNrAutoFeaturesAllowed() const
{
  return {1, 5};
}

int RitualRoom::getBasePctChanceDrk() const
{
  return 15;
}

bool RitualRoom::isAllowed() const
{
  return r_.getMinDim() >= 4 && r_.getMaxDim() <= 8;
}

void RitualRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  MapGenUtils::cutRoomCorners (*this);
  if (Rnd::fraction(1, 3)) {MapGenUtils::mkPillarsInRoom(*this);}
}

void RitualRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  Gods::setRandomGod();

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  const int BLOODY_CHAMBER_PCT = 60;
  if (Rnd::percentile() < BLOODY_CHAMBER_PCT)
  {
    Pos origin(-1, -1);
    vector<Pos> originBucket;
    for (int y = r_.p0.y; y <= r_.p1.y; ++y)
    {
      for (int x = r_.p0.x; x <= r_.p1.x; ++x)
      {
        if (Map::cells[x][y].rigid->getId() == FeatureId::altar)
        {
          origin = Pos(x, y);
          y = 999;
          x = 999;
        }
        else
        {
          if (!blocked[x][y]) {originBucket.push_back(Pos(x, y));}
        }
      }
    }
    if (!originBucket.empty())
    {
      if (origin.x == -1)
      {
        const int ELEMENT = Rnd::range(0, originBucket.size() - 1);
        origin = originBucket[ELEMENT];
      }
      for (int dx = -1; dx <= 1; ++dx)
      {
        for (int dy = -1; dy <= 1; ++dy)
        {
          if ((dx == 0 && dy == 0) || (Rnd::percentile() < BLOODY_CHAMBER_PCT / 2))
          {
            const Pos pos = origin + Pos(dx, dy);
            if (!blocked[pos.x][pos.y])
            {
              Map::mkGore(pos);
              Map::mkBlood(pos);
            }
          }
        }
      }
    }
  }
}

//------------------------------------------------------------------- SPIDER ROOM
Range SpiderRoom::getNrAutoFeaturesAllowed() const
{
  return {0, 3};
}

int SpiderRoom::getBasePctChanceDrk() const
{
  return 30;
}

bool SpiderRoom::isAllowed() const
{
  return r_.getMinDim() >= 3 && r_.getMaxDim() <= 8;
}

void SpiderRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  //Early game : Always reshape by cutting corners
  //Mid    -   : "Flip a coin"
  //Late   -   : Always reshape by cavifying
  const bool IS_EARLY = Map::dlvl <= DLVL_LAST_EARLY_GAME;
  const bool IS_MID   = !IS_EARLY && Map::dlvl <= DLVL_LAST_MID_GAME;

  if (IS_EARLY || (IS_MID && Rnd::coinToss()))
  {
    MapGenUtils::cutRoomCorners (*this);
  }
  else
  {
    MapGenUtils::cavifyRoom(*this);
  }

  if ((IS_EARLY || IS_MID) && Rnd::fraction(1, 3))
  {
    MapGenUtils::mkPillarsInRoom(*this);
  }
}

void SpiderRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;
}

//------------------------------------------------------------------- CRYPT ROOM
Range CryptRoom::getNrAutoFeaturesAllowed() const
{
  return {3, 6};
}

int CryptRoom::getBasePctChanceDrk() const
{
  return 60;
}

bool CryptRoom::isAllowed() const
{
  return r_.getMinDim() >= 3  && r_.getMaxDim() <= 12;
}

void CryptRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  MapGenUtils::cutRoomCorners (*this);
  if (Rnd::fraction(1, 3)) {MapGenUtils::mkPillarsInRoom(*this);}
}

void CryptRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;
}

//------------------------------------------------------------------- MONSTER ROOM
Range MonsterRoom::getNrAutoFeaturesAllowed() const
{
  return {0, 6};
}

int MonsterRoom::getBasePctChanceDrk() const
{
  return 75;
}

bool MonsterRoom::isAllowed() const
{
  return r_.getMinDim() >= 4 && r_.getMaxDim() <= 8;
}

void MonsterRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  const bool IS_EARLY = Map::dlvl <= DLVL_LAST_EARLY_GAME;
  const bool IS_MID   = !IS_EARLY && Map::dlvl <= DLVL_LAST_MID_GAME;

  if (IS_EARLY || IS_MID)
  {
    if (Rnd::fraction(3, 4))
    {
      MapGenUtils::cutRoomCorners (*this);
    }
    if (Rnd::fraction(1, 3))
    {
      MapGenUtils::mkPillarsInRoom(*this);
    }
  }
  else //Is late game
  {
    MapGenUtils::cavifyRoom(*this);
  }
}

void MonsterRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  int       nrBloodPut  = 0;
  const int NR_TRIES    = 1000; //TODO Hacky, needs improving

  for (int i = 0; i < NR_TRIES; ++i)
  {
    for (int x = r_.p0.x; x <= r_.p1.x; ++x)
    {
      for (int y = r_.p0.y; y <= r_.p1.y; ++y)
      {
        if (!blocked[x][y] && Map::roomMap[x][y] == this && Rnd::fraction(2, 5))
        {
          Map::mkGore(Pos(x, y));
          Map::mkBlood(Pos(x, y));
          nrBloodPut++;
        }
      }
    }
    if (nrBloodPut > 0)
    {
      break;
    }
  }
}

//------------------------------------------------------------------- FLOODED ROOM
Range FloodedRoom::getNrAutoFeaturesAllowed() const
{
  return {0, 0};
}

int FloodedRoom::getBasePctChanceDrk() const
{
  return 25;
}

bool FloodedRoom::isAllowed() const
{
  return true;
}

void FloodedRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  //Early game : Always reshape by cutting corners
  //Mid    -   : "Flip a coin"
  //Late   -   : Always reshape by cavifying
  const bool IS_EARLY = Map::dlvl <= DLVL_LAST_EARLY_GAME;
  const bool IS_MID   = !IS_EARLY && Map::dlvl <= DLVL_LAST_MID_GAME;

  if (IS_EARLY || (IS_MID && Rnd::coinToss()))
  {
    MapGenUtils::cutRoomCorners (*this);
  }
  else
  {
    MapGenUtils::cavifyRoom(*this);
  }

  if ((IS_EARLY || IS_MID) && Rnd::fraction(1, 3))
  {
    MapGenUtils::mkPillarsInRoom(*this);
  }
}

void FloodedRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

#ifndef NDEBUG
  //Sanity check (look for some features that should not exist in this room)
  for (int x = r_.p0.x; x <= r_.p1.x; ++x)
  {
    for (int y = r_.p0.y; y <= r_.p1.y; ++y)
    {
      if (Map::roomMap[x][y] == this)
      {
        const auto id = Map::cells[x][y].rigid->getId();
        if (id == FeatureId::chest    ||
            id == FeatureId::tomb     ||
            id == FeatureId::cabinet  ||
            id == FeatureId::fountain)
        {
          TRACE << "Illegal feature found in room" << endl;
          assert(false);
        }
      }
    }
  }
#endif // NDEBUG

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  const int LIQUID_ONE_IN_N = Rnd::range(2, 4);

  for (int x = r_.p0.x; x <= r_.p1.x; ++x)
  {
    for (int y = r_.p0.y; y <= r_.p1.y; ++y)
    {
      if (!blocked[x][y] && Map::roomMap[x][y] == this && Rnd::oneIn(LIQUID_ONE_IN_N))
      {
        LiquidShallow* const liquid = new LiquidShallow(Pos(x, y));
        liquid->type_ = LiquidType::water;
        Map::put(liquid);
      }
    }
  }
}

//------------------------------------------------------------------- MUDDY ROOM
Range MuddyRoom::getNrAutoFeaturesAllowed() const
{
  return {0, 0};
}

int MuddyRoom::getBasePctChanceDrk() const
{
  return 25;
}

bool MuddyRoom::isAllowed() const
{
  return true;
}

void MuddyRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

#ifndef NDEBUG
  //Sanity check (look for some features that should not exist in this room)
  for (int x = r_.p0.x; x <= r_.p1.x; ++x)
  {
    for (int y = r_.p0.y; y <= r_.p1.y; ++y)
    {
      if (Map::roomMap[x][y] == this)
      {
        const auto id = Map::cells[x][y].rigid->getId();
        if (id == FeatureId::chest    ||
            id == FeatureId::tomb     ||
            id == FeatureId::cabinet  ||
            id == FeatureId::fountain)
        {
          TRACE << "Illegal feature found in room" << endl;
          assert(false);
        }
      }
    }
  }
#endif // NDEBUG

  //Early game : Always reshape by cutting corners
  //Mid    -   : "Flip a coin"
  //Late   -   : Always reshape by cavifying
  const bool IS_EARLY = Map::dlvl <= DLVL_LAST_EARLY_GAME;
  const bool IS_MID   = !IS_EARLY && Map::dlvl <= DLVL_LAST_MID_GAME;

  if (IS_EARLY || (IS_MID && Rnd::coinToss()))
  {
    MapGenUtils::cutRoomCorners (*this);
  }
  else
  {
    MapGenUtils::cavifyRoom(*this);
  }

  if ((IS_EARLY || IS_MID) && Rnd::fraction(1, 3))
  {
    MapGenUtils::mkPillarsInRoom(*this);
  }
}

void MuddyRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  const int LIQUID_ONE_IN_N = Rnd::range(2, 4);

  for (int x = r_.p0.x; x <= r_.p1.x; ++x)
  {
    for (int y = r_.p0.y; y <= r_.p1.y; ++y)
    {
      if (!blocked[x][y] && Map::roomMap[x][y] == this && Rnd::oneIn(LIQUID_ONE_IN_N))
      {
        LiquidShallow* const liquid = new LiquidShallow(Pos(x, y));
        liquid->type_ = LiquidType::mud;
        Map::put(liquid);
      }
    }
  }
}

//------------------------------------------------------------------- CAVE ROOM
Range CaveRoom::getNrAutoFeaturesAllowed() const
{
  return {0, 0};
}

int CaveRoom::getBasePctChanceDrk() const
{
  return 30;
}

bool CaveRoom::isAllowed() const
{
  return !isSubRoom_;
}

void CaveRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  MapGenUtils::cavifyRoom(*this);
}

void CaveRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;
}

//------------------------------------------------------------------- FOREST ROOM
Range ForestRoom::getNrAutoFeaturesAllowed() const
{
  if (Rnd::oneIn(3))
  {
    return {2, 6};
  }
  else
  {
    return {0, 1};
  }
}

int ForestRoom::getBasePctChanceDrk() const
{
  return 10;
}

bool ForestRoom::isAllowed() const
{
  return !isSubRoom_;
}

void ForestRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  MapGenUtils::cavifyRoom(*this);
}

void ForestRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  vector<Pos> treePosBucket;

  for (int x = r_.p0.x; x <= r_.p1.x; ++x)
  {
    for (int y = r_.p0.y; y <= r_.p1.y; ++y)
    {
      if (!blocked[x][y] && Map::roomMap[x][y] == this)
      {
        const Pos p(x, y);
        treePosBucket.push_back(p);

        if (Rnd::oneIn(6))
        {
          Map::put(new Bush(p));
        }
        else
        {
          Map::put(new Grass(p));
        }
      }
    }
  }

  random_shuffle(begin(treePosBucket), end(treePosBucket));

  int nrTreesPlaced = 0;

  const int TREE_ONE_IN_N = Rnd::range(2, 5);

  while (!treePosBucket.empty())
  {
    const Pos p = treePosBucket.back();
    treePosBucket.pop_back();

    if (Rnd::oneIn(TREE_ONE_IN_N))
    {
      blocked[p.x][p.y] = true;

      if (MapParse::isMapConnected(blocked))
      {
        Map::put(new Tree(p));
        ++nrTreesPlaced;
      }
      else
      {
        blocked[p.x][p.y] = false;
      }
    }
  }
}

//------------------------------------------------------------------- CHASM ROOM
Range ChasmRoom::getNrAutoFeaturesAllowed() const
{
  return {0, 0};
}

int ChasmRoom::getBasePctChanceDrk() const
{
  return 25;
}

bool ChasmRoom::isAllowed() const
{
  return r_.getMinDim() >= 5 &&
         r_.getMaxDim() <= 9;
}

void ChasmRoom::onPreConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  MapGenUtils::cavifyRoom(*this);
}

void ChasmRoom::onPostConnect_(bool doorProposals[MAP_W][MAP_H])
{
  (void)doorProposals;

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  bool blockedExpanded[MAP_W][MAP_H];
  MapParse::expand(blocked, blockedExpanded);

  vector<Pos> originBucket;

  Pos origin;

  for (int x = r_.p0.x; x <= r_.p1.x; ++x)
  {
    for (int y = r_.p0.y; y <= r_.p1.y; ++y)
    {
      if (!blockedExpanded[x][y] && Map::roomMap[x][y] == this)
      {
        origin.set(x, y);
      }
    }
  }

  int flood[MAP_W][MAP_H];

  FloodFill::run(origin, blockedExpanded, flood, 10000, { -1,  -1}, false);

  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      const Pos p(x, y);

      if (p == origin || flood[x][y] != 0)
      {
        Map::put(new Chasm(p));
      }
    }
  }
}

//------------------------------------------------------------------- RIVER ROOM
void RiverRoom::onPreConnect(bool doorProposals[MAP_W][MAP_H])
{
  TRACE_FUNC_BEGIN;

  //The strategy here is to expand the the river on both sides until parallel
  //to the closest center cell of another room

  const bool IS_HOR = dir_ == hor;

  TRACE << "Finding room centers" << endl;
  bool centers[MAP_W][MAP_H];
  Utils::resetArray(centers, false);

  for (Room* const room : Map::roomList)
  {
    if (room != this)
    {
      const Pos cPos(room->r_.getCenterPos());
      centers[cPos.x][cPos.y] = true;
    }
  }

  TRACE << "Finding closest room center coordinates on both sides "
        << "(y coordinate if horizontal river, x if vertical)" << endl;
  int closestCenter0 = -1;
  int closestCenter1 = -1;

  //Using nestled scope to avoid declaring x and y at function scope
  {
    int x, y;

    //iOuter and iInner should be references to x or y.
    auto findClosestCenter0 =
      [&](const Range & rOuter, const Range & rInner, int& iOuter, int& iInner)
    {
      for (iOuter = rOuter.lower; iOuter >= rOuter.upper; --iOuter)
      {
        for (iInner = rInner.lower; iInner <= rInner.upper; ++iInner)
        {
          if (centers[x][y])
          {
            closestCenter0 = iOuter;
            break;
          }
        }
        if (closestCenter0 != -1)
        {
          break;
        }
      }
    };

    auto findClosestCenter1 =
      [&](const Range & rOuter, const Range & rInner, int& iOuter, int& iInner)
    {
      for (iOuter = rOuter.lower; iOuter <= rOuter.upper; ++iOuter)
      {
        for (iInner = rInner.lower; iInner <= rInner.upper; ++iInner)
        {
          if (centers[x][y])
          {
            closestCenter1 = iOuter;
            break;
          }
        }
        if (closestCenter1 != -1) {break;}
      }
    };

    if (IS_HOR)
    {
      const int RIVER_Y = r_.p0.y;
      findClosestCenter0(Range(RIVER_Y - 1, 1),         Range(1, MAP_W - 2),  y, x);
      findClosestCenter1(Range(RIVER_Y + 1, MAP_H - 2), Range(1, MAP_W - 2),  y, x);
    }
    else
    {
      const int RIVER_X = r_.p0.x;
      findClosestCenter0(Range(RIVER_X - 1, 1),         Range(1, MAP_H - 2),  x, y);
      findClosestCenter1(Range(RIVER_X + 1, MAP_W - 2), Range(1, MAP_H - 2),  x, y);
    }
  }

  TRACE << "Expanding and filling river" << endl;

  bool blocked[MAP_W][MAP_H];

  //Within the expansion limits, mark all cells not belonging to another room as free.
  //All other cells are considered as blocking.
  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      blocked[x][y] = true;
      if ((dir_ == hor && (y >= closestCenter0 && y <= closestCenter1)) ||
          (dir_ == ver && (x >= closestCenter0 && x <= closestCenter1))
         )
      {
        Room* r       = Map::roomMap[x][y];
        blocked[x][y] = r && r != this;
      }
    }
  }
  bool blockedExpanded[MAP_W][MAP_H];
  MapParse::expand(blocked, blockedExpanded);

  int flood[MAP_W][MAP_H];
  const Pos origin(r_.getCenterPos());
  FloodFill::run(origin, blockedExpanded, flood, INT_MAX, Pos(-1, -1), true);

  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      const Pos p(x, y);
      if (flood[x][y] > 0 || p == origin)
      {
        LiquidDeep* const liquid  = new LiquidDeep(p);
        liquid->type_             = LiquidType::water;
        Map::put(liquid);
        Map::roomMap[x][y] = this;
        r_.p0.x = min(r_.p0.x, x);
        r_.p0.y = min(r_.p0.y, y);
        r_.p1.x = max(r_.p1.x, x);
        r_.p1.y = max(r_.p1.y, y);
      }
    }
  }

  TRACE << "Making bridge(s)" << endl;

  //Mark which side each cell belongs to
  enum Side {inRiver, side0, side1};
  Side sides[MAP_W][MAP_H];

  //Using nestled scope to avoid declaring x and y at function scope
  {
    int x, y;

    //iOuter and iInner should be references to x or y.
    auto markSides =
      [&](const Range & rOuter, const Range & rInner, int& iOuter, int& iInner)
    {
      for (iOuter = rOuter.lower; iOuter <= rOuter.upper; ++iOuter)
      {
        bool isOnSide0 = true;
        for (iInner = rInner.lower; iInner <= rInner.upper; ++iInner)
        {
          if (Map::roomMap[x][y] == this)
          {
            isOnSide0 = false;
            sides[x][y] = inRiver;
          }
          else
          {
            sides[x][y] = isOnSide0 ? side0 : side1;
          }
        }
      }
    };

    if (dir_ == hor)
    {
      markSides(Range(1, MAP_W - 2), Range(1, MAP_H - 2), x, y);
    }
    else
    {
      markSides(Range(1, MAP_H - 2), Range(1, MAP_W - 2), y, x);
    }
  }

  bool validRoomEntries0[MAP_W][MAP_H];
  bool validRoomEntries1[MAP_W][MAP_H];
  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      validRoomEntries0[x][y] = validRoomEntries1[x][y] = false;
    }
  }

  const int EDGE_D = 4;
  for (int x = EDGE_D; x < MAP_W - EDGE_D; ++x)
  {
    for (int y = EDGE_D; y < MAP_H - EDGE_D; ++y)
    {
      const FeatureId featureId = Map::cells[x][y].rigid->getId();
      if (featureId == FeatureId::wall && !Map::roomMap[x][y])
      {
        const Pos p(x, y);
        int nrCardinalFloor  = 0;
        int nrCardinalRiver  = 0;
        for (const auto& d : DirUtils::cardinalList)
        {
          const auto pAdj(p + d);
          const auto* const f = Map::cells[pAdj.x][pAdj.y].rigid;
          if (f->getId() == FeatureId::floor)        {nrCardinalFloor++;}
          if (Map::roomMap[pAdj.x][pAdj.y] == this)  {nrCardinalRiver++;}
        }
        if (nrCardinalFloor == 1 && nrCardinalRiver == 1)
        {
          switch (sides[x][y])
          {
            case side0:   validRoomEntries0[x][y] = true; break;
            case side1:   validRoomEntries1[x][y] = true; break;
            case inRiver: {} break;
          }
        }
      }
    }
  }

#ifdef DEMO_MODE
  Render::drawMap();
  for (int y = 1; y < MAP_H - 1; ++y)
  {
    for (int x = 1; x < MAP_W - 1; ++x)
    {
      Pos p(x, y);
      if (validRoomEntries0[x][y])
      {
        Render::drawGlyph('0', Panel::map, p, clrRedLgt);
      }
      if (validRoomEntries1[x][y])
      {
        Render::drawGlyph('1', Panel::map, p, clrRedLgt);
      }
      if (validRoomEntries0[x][y] || validRoomEntries1[x][y])
      {
        Render::updateScreen();
        SdlWrapper::sleep(100);
      }
    }
  }
#endif // DEMO_MODE

  vector<int> coordinates(IS_HOR ? MAP_W : MAP_H);
  iota(begin(coordinates), end(coordinates), 0);
  random_shuffle(coordinates.begin(), coordinates.end());

  vector<int> cBuilt;

  const int MIN_EDGE_DIST   = 6;
  const int MAX_NR_BRIDGES  = Rnd::range(1, 3);

  for (const int BRIDGE_C : coordinates)
  {
    if (BRIDGE_C < MIN_EDGE_DIST ||
        (IS_HOR  && BRIDGE_C > MAP_W - 1 - MIN_EDGE_DIST) ||
        (!IS_HOR && BRIDGE_C > MAP_H - 1 - MIN_EDGE_DIST))
    {
      continue;
    }
    bool isTooCloseToOtherBridge = false;
    const int MIN_D = 2;
    for (int cOther : cBuilt)
    {
      if (Utils::isValInRange(BRIDGE_C, Range(cOther - MIN_D, cOther + MIN_D)))
      {
        isTooCloseToOtherBridge = true;
        break;
      }
    }
    if (isTooCloseToOtherBridge) {continue;}

    //Check if current bridge coord would connect matching room connections.
    //If so both roomCon0 and roomCon1 will be set.
    Pos roomCon0(-1, -1);
    Pos roomCon1(-1, -1);
    const int C0_0 = IS_HOR ? r_.p1.y : r_.p1.x;
    const int C1_0 = IS_HOR ? r_.p0.y : r_.p0.x;
    for (int c = C0_0; c != C1_0; --c)
    {
      if ((IS_HOR  && sides[BRIDGE_C][c] == side0) ||
          (!IS_HOR && sides[c][BRIDGE_C] == side0))
      {
        break;
      }
      const Pos pNxt = IS_HOR ? Pos(BRIDGE_C, c - 1) : Pos(c - 1, BRIDGE_C);
      if (validRoomEntries0[pNxt.x][pNxt.y])
      {
        roomCon0 = pNxt;
        break;
      }
    }
    const int C0_1 = IS_HOR ? r_.p0.y : r_.p0.x;
    const int C1_1 = IS_HOR ? r_.p1.y : r_.p1.x;
    for (int c = C0_1; c != C1_1; ++c)
    {
      if ((IS_HOR  && sides[BRIDGE_C][c] == side1) ||
          (!IS_HOR && sides[c][BRIDGE_C] == side1))
      {
        break;
      }
      const Pos pNxt = IS_HOR ? Pos(BRIDGE_C, c + 1) : Pos(c + 1, BRIDGE_C);
      if (validRoomEntries1[pNxt.x][pNxt.y])
      {
        roomCon1 = pNxt;
        break;
      }
    }

    //Make the bridge if valid connection pairs found
    if (roomCon0.x != -1 && roomCon1.x != -1)
    {
#ifdef DEMO_MODE
      Render::drawMap();
      Render::drawGlyph('0', Panel::map, roomCon0, clrGreenLgt);
      Render::drawGlyph('1', Panel::map, roomCon1, clrYellow);
      Render::updateScreen();
      SdlWrapper::sleep(2000);
#endif // DEMO_MODE

      TRACE << "Found valid connection pair at: "
            << roomCon0.x << "," << roomCon0.y << " / "
            << roomCon1.x << "," << roomCon1.y << endl
            << "Making bridge at coord: " << BRIDGE_C << endl;
      if (IS_HOR)
      {
        for (int y = roomCon0.y; y <= roomCon1.y; ++y)
        {
          if (Map::roomMap[BRIDGE_C][y] == this)
          {
            auto* const bridge = new Bridge(Pos(BRIDGE_C, y));
            bridge->setDir(ver);
            Map::put(bridge);
          }
        }
      }
      else
      {
        for (int x = roomCon0.x; x <= roomCon1.x; ++x)
        {
          if (Map::roomMap[x][BRIDGE_C] == this)
          {
            auto* const bridge = new Bridge(Pos(x, BRIDGE_C));
            bridge->setDir(hor);
            Map::put(bridge);
          }
        }
      }
      Map::put(new Floor(roomCon0));
      Map::put(new Floor(roomCon1));
      doorProposals[roomCon0.x][roomCon0.y] = true;
      doorProposals[roomCon1.x][roomCon1.y] = true;
      cBuilt.push_back(BRIDGE_C);
    }
    if (int (cBuilt.size()) >= MAX_NR_BRIDGES)
    {
      TRACE << "Enough bridges built" << endl;
      break;
    }
  }
  TRACE << "Bridges built/attempted: " << cBuilt.size() << "/"
        << MAX_NR_BRIDGES << endl;

  if (cBuilt.empty())
  {
    MapGen::isMapValid = false;
  }
  else
  {
    TRACE << "Converting some remaining valid room entries to floor" << endl;
    for (int x = 0; x < MAP_W; ++x)
    {
      for (int y = 0; y < MAP_H; ++y)
      {
        if (validRoomEntries0[x][y] || validRoomEntries1[x][y])
        {
          if (find(cBuilt.begin(), cBuilt.end(), x) == cBuilt.end())
          {
            if (Rnd::oneIn(4))
            {
              Map::put(new Floor(Pos(x, y)));
              Map::roomMap[x][y] = this;
            }
          }
        }
      }
    }
  }
  TRACE_FUNC_END;
}
