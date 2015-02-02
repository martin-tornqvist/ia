#include "MapGen.h"

#include <vector>

#include "ActorPlayer.h"
#include "ActorFactory.h"
#include "ActorMon.h"
#include "Map.h"
#include "Utils.h"
#include "ActorFactory.h"
#include "FeatureRigid.h"
#include "MapParsing.h"
#include "Highscore.h"
#include "Fov.h"
#include "TextFormat.h"
#include "PopulateMonsters.h"
#include "PopulateItems.h"
#include "ItemFactory.h"
#include "FeatureDoor.h"
#include "FeatureEvent.h"
#include "GameTime.h"

using namespace std;

namespace MapGen
{

//------------------------------------------------------------------- FOREST
namespace
{

void mkForestLimit()
{
    auto putTree = [](const int X, const int Y) {Map::put(new Tree(Pos(X, Y)));};

    for (int y = 0; y < MAP_H; ++y) {putTree(0,          y);}
    for (int x = 0; x < MAP_W; ++x) {putTree(x,          0);}
    for (int y = 0; y < MAP_H; ++y) {putTree(MAP_W - 1,  y);}
    for (int x = 0; x < MAP_W; ++x) {putTree(x,          MAP_H - 1);}
}

void mkForestOuterTreeline()
{
    const int MAX_LEN = 2;

    for (int y = 0; y < MAP_H; ++y)
    {
        for (int x = 0; x <= MAX_LEN; ++x)
        {
            if (Rnd::range(1, 4) > 1 || x == 0)
            {
                Map::put(new Tree(Pos(x, y)));
            }
            else
            {
                break;
            }
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAX_LEN; ++y)
        {
            if (Rnd::range(1, 4) > 1 || y == 0)
            {
                Map::put(new Tree(Pos(x, y)));
            }
            else
            {
                break;
            }
        }
    }

    for (int y = 0; y < MAP_H; ++y)
    {
        for (int x = MAP_W - 1; x >= MAP_W - MAX_LEN; x--)
        {
            if (Rnd::range(1, 4) > 1 || x == MAP_W - 1)
            {
                Map::put(new Tree(Pos(x, y)));
            }
            else
            {
                break;
            }
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = MAP_H - 1; y >= MAP_H - MAX_LEN; y--)
        {
            if (Rnd::range(1, 4) > 1 || y == MAP_H - 1)
            {
                Map::put(new Tree(Pos(x, y)));
            }
            else
            {
                break;
            }
        }
    }
}

void mkForestTreePatch()
{
    const int NR_TREES_TO_PUT = Rnd::range(5, 17);

    Pos curPos(Rnd::range(1, MAP_W - 2), Rnd::range(1, MAP_H - 2));

    int nrTreesCreated = 0;

    while (nrTreesCreated < NR_TREES_TO_PUT)
    {
        if (
            !Utils::isPosInsideMap(curPos) ||
            Utils::kingDist(curPos, Map::player->pos) <= 1)
        {
            return;
        }

        Map::put(new Tree(curPos));

        ++nrTreesCreated;

        //Find next pos
        while (Map::cells[curPos.x][curPos.y].rigid->getId() == FeatureId::tree ||
                Utils::kingDist(curPos, Map::player->pos) <= 2)
        {
            if (Rnd::coinToss())
            {
                curPos.x += Rnd::coinToss() ? -1 : 1;
            }
            else
            {
                curPos.y += Rnd::coinToss() ? -1 : 1;
            }

            if (!Utils::isPosInsideMap(curPos)) {return;}
        }
    }
}

void mkForestTrees()
{
    MapGenUtils::backupMap();

    const Pos churchPos(MAP_W - 33, 2);

    int nrForestPatches = Rnd::range(40, 55);

    vector<Pos> path;

    bool proceed = false;
    while (!proceed)
    {
        for (int i = 0; i < nrForestPatches; ++i) {mkForestTreePatch();}

        const MapTempl& templ     = MapTemplHandling::getTempl(MapTemplId::church);
        const Pos       templDims = templ.getDims();

        for (int x = 0; x < templDims.x; ++x)
        {
            for (int y = 0; y < templDims.y; ++y)
            {
                const auto& templCell = templ.getCell(x, y);
                const auto  fId       = templCell.featureId;

                const Pos p(churchPos + Pos(x, y));

                if (fId != FeatureId::END)
                {
                    Rigid* const f =
                        Map::put(static_cast<Rigid*>(FeatureData::getData(fId).mkObj(p)));

                    if (fId == FeatureId::grass)
                    {
                        //All grass around the church is withered
                        static_cast<Grass*>(f)->type_ = GrassType::withered;
                    }
                }
                if (templCell.val == 1)
                {
                    Map::put(new Door(p, new Wall(p), DoorSpawnState::closed));
                }
            }
        }

        bool blocked[MAP_W][MAP_H];
        MapParse::run(CellCheck::BlocksMoveCmn(false), blocked);

        Pos stairsPos;
        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                const auto id = Map::cells[x][y].rigid->getId();

                if (id == FeatureId::stairs)
                {
                    stairsPos.set(x, y);
                    blocked[x][y] = false;
                }
                else if (id == FeatureId::door)
                {
                    blocked[x][y] = false;
                }
            }
        }

        PathFind::run(Map::player->pos, stairsPos, blocked, path);

        size_t minPathLength = 1;
        size_t maxPathLength = 999;

        if (path.size() >= minPathLength && path.size() <= maxPathLength)
        {
            proceed = true;
        }
        else
        {
            MapGenUtils::restoreMap();
        }

        maxPathLength++;
    }

    //Build path
    for (const Pos& pathPos : path)
    {
        for (int dx = -1; dx < 1; ++dx)
        {
            for (int dy = -1; dy < 1; ++dy)
            {
                const Pos p(pathPos + Pos(dx, dy));

                if (Map::cells[p.x][p.y].rigid->canHaveRigid() && Utils::isPosInsideMap(p))
                {
                    Floor* const floor = new Floor(p);
                    floor->type_ = FloorType::stonePath;
                    Map::put(floor);
                }
            }
        }
    }

    //Place graves
    vector<HighScoreEntry> entries = HighScore::getEntriesSorted();

    const int NR_NON_WIN =
        count_if(begin(entries), end(entries), [](const HighScoreEntry & e)
    {
        return !e.isWin();
    });

    const int MAX_NR_TO_PLACE     = 7;
    const int NR_GRAVES_TO_PLACE  = min(MAX_NR_TO_PLACE, NR_NON_WIN);

    if (NR_GRAVES_TO_PLACE > 0)
    {
        bool blocked[MAP_W][MAP_H];
        MapParse::run(CellCheck::BlocksMoveCmn(true), blocked);

        bool fov[MAP_W][MAP_H];

        const int SEARCH_RADI = FOV_STD_RADI_INT - 2;
        const int TRY_PLACE_EVERY_N_STEP = 2;

        vector<Pos> graveCells;

        int pathWalkCount = 0;
        for (size_t i = 0; i < path.size(); ++i)
        {
            if (pathWalkCount == TRY_PLACE_EVERY_N_STEP)
            {
                Fov::runFovOnArray(blocked, path[i], fov, false);

                for (int dy = -SEARCH_RADI; dy <= SEARCH_RADI; ++dy)
                {
                    for (int dx = -SEARCH_RADI; dx <= SEARCH_RADI; ++dx)
                    {
                        const int X = path[i].x + dx;
                        const int Y = path[i].y + dy;

                        const bool IS_LEFT_OF_CHURCH =
                            X < churchPos.x - (SEARCH_RADI) + 2;
                        const bool IS_ON_STONE_PATH =
                            Map::cells[X][Y].rigid->getId() == FeatureId::floor;

                        bool isLeftOfPrev = true;
                        if (!graveCells.empty())
                        {
                            isLeftOfPrev = X < graveCells.back().x;
                        }

                        bool isPosOk = fov[X][Y]         &&
                                       IS_LEFT_OF_CHURCH &&
                                       !IS_ON_STONE_PATH &&
                                       isLeftOfPrev;

                        if (isPosOk)
                        {
                            for (int dxx = -1; dxx <= 1; dxx++)
                            {
                                for (int dyy = -1; dyy <= 1; dyy++)
                                {
                                    if (blocked[X + dxx][Y + dyy])
                                    {
                                        isPosOk = false;
                                    }
                                }
                            }
                            if (isPosOk)
                            {
                                graveCells.push_back(Pos(X, Y));
                                blocked[X][Y] = true;
                                if (int(graveCells.size()) == NR_GRAVES_TO_PLACE)
                                {
                                    i = 9999;
                                }
                                dy = 99999;
                                dx = 99999;
                            }
                        }
                    }
                }
                pathWalkCount = 0;
            }
            pathWalkCount++;
        }

        size_t entryIdx = 0;

        for (size_t gravePosIdx = 0; gravePosIdx < graveCells.size(); ++gravePosIdx)
        {
            GraveStone*     grave = new GraveStone(graveCells[gravePosIdx]);
            HighScoreEntry  entry = entries[entryIdx];

            //Skip winning entries
            while (entry.isWin())
            {
                ++entryIdx;
                entry = entries[entryIdx];
            }

            const string    name      = entry.getName();
            vector<string>  dateStrVector;

            dateStrVector.clear();
            TextFormat::getSpaceSeparatedList(entry.getDateAndTime(), dateStrVector);
            const string  dateStr     = dateStrVector[0];
            const string  scoreStr    = toStr(entry.getScore());
            const string  dlvlStr     = toStr(entry.getDlvl());
            string        classStr    = "";
            PlayerBon::getBgTitle(entry.getBg(), classStr);

            grave->setInscription("RIP " + name + ", " + classStr + ", " + dateStr
                                  + ", Score: " + scoreStr);

            Map::put(grave);
            ++entryIdx;
        }
    }
}

} //namespace

bool mkIntroLvl()
{
    for (int x = 1; x < MAP_W - 1; ++x)
    {
        for (int y = 1; y < MAP_H - 1; ++y)
        {
            const Pos p(x, y);
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

    mkForestOuterTreeline();
    mkForestTrees();
    mkForestLimit();

    PopulateMon::populateIntroLvl();

    return true;
}

//------------------------------------------------------------------- EGYPT
bool mkEgyptLvl()
{
    Map::resetMap();

    const MapTempl& templ     = MapTemplHandling::getTempl(MapTemplId::egypt);
    const Pos       templDims = templ.getDims();
    const int       STAIR_VAL = Rnd::range(2, 3);

    for (int x = 0; x < templDims.x; ++x)
    {
        for (int y = 0; y < templDims.y; ++y)
        {
            const auto& templCell = templ.getCell(x, y);
            const Pos p(x, y);
            if (templCell.featureId != FeatureId::END)
            {
                if (templCell.val == STAIR_VAL)
                {
                    Map::put(new Stairs(p));
                }
                else
                {
                    const auto& d = FeatureData::getData(templCell.featureId);
                    Map::put(static_cast<Rigid*>(d.mkObj(p)));
                }
            }
            if (templCell.actorId != ActorId::END)
            {
                Actor* const actor = ActorFactory::mk(templCell.actorId, p);
                static_cast<Mon*>(actor)->isRoamingAllowed_ = false;
            }
            if (templCell.val == 1)
            {
                Map::player->pos = p;
            }
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Rigid* const f = Map::cells[x][y].rigid;
            if (f->getId() == FeatureId::wall)
            {
                static_cast<Wall*>(f)->type_ = WallType::egypt;
            }
        }
    }

    PopulateItems::mkItemsOnFloor();

    return true;
}

//------------------------------------------------------------------- LENG
bool mkLengLvl()
{
    Map::resetMap();

    const MapTempl& templ     = MapTemplHandling::getTempl(MapTemplId::leng);
    const Pos       templDims = templ.getDims();

    for (int x = 0; x < templDims.x; ++x)
    {
        for (int y = 0; y < templDims.y; ++y)
        {
            const auto& templCell = templ.getCell(x, y);
            const auto  fId       = templCell.featureId;
            const Pos p(x, y);
            if (fId != FeatureId::END)
            {
                const auto& d = FeatureData::getData(fId);
                auto* const f = Map::put(static_cast<Rigid*>(d.mkObj(p)));
                if (fId == FeatureId::grass)
                {
                    if (Rnd::oneIn(50))
                    {
                        Map::put(new Bush(p));
                    }
                }
                else if (fId == FeatureId::wall)
                {
                    auto* const wall = static_cast<Wall*>(f);
                    if (templCell.val == 2)
                    {
                        wall->type_ = WallType::cliff;
                    }
                    else if (templCell.val == 3 || templCell.val == 5)
                    {
                        wall->type_ = WallType::lengMonestary;
                    }
                }
            }

            if (templCell.actorId != ActorId::END)
            {
                ActorFactory::mk(templCell.actorId, p);
            }

            switch (templCell.val)
            {
            case 1:
                Map::player->pos = p;
                break;

            case 3:
                Map::cells[x][y].isDark = true;
                break;

            case 6:
            {
                Wall* mimic   = new Wall(p);
                mimic->type_  = WallType::lengMonestary;
                Map::put(new Door(p, mimic, DoorSpawnState::closed));
            }
            break;

            default: {}
                break;
            }
        }
    }
    return true;
}

//------------------------------------------------------------------- RATS IN THE WALLS
bool mkRatsInTheWallsLvl()
{
    Map::resetMap();

    const MapTempl& templ     = MapTemplHandling::getTempl(MapTemplId::ratsInTheWalls);
    const Pos       templDims = templ.getDims();

    const int       RAT_THING_ONE_IN_N_RAT = 3;
    const Fraction  bonesOneInN(1, 2);

    for (int x = 0; x < templDims.x; ++x)
    {
        for (int y = 0; y < templDims.y; ++y)
        {
            const auto& templCell = templ.getCell(x, y);
            const auto  fId       = templCell.featureId;
            const Pos p(x, y);

            if (fId != FeatureId::END)
            {
                const auto& d = FeatureData::getData(fId);
                auto* const f = Map::put(static_cast<Rigid*>(d.mkObj(p)));

                if (f->getId() == FeatureId::wall)
                {
                    if (templCell.val == 2) //Constructed walls
                    {
                        if (Rnd::oneIn(2))
                        {
                            Map::put(new RubbleLow(p));
                        }
                        else if (Rnd::oneIn(5))
                        {
                            Map::put(new RubbleHigh(p));
                        }
                        else
                        {
                            static_cast<Wall*>(f)->type_ = WallType::cmn;
                        }
                    }
                    else //Cave walls
                    {
                        static_cast<Wall*>(f)->type_ = WallType::cave;
                    }
                }
                else if (f->getId() == FeatureId::floor)
                {
                    if (templCell.val == 4 && Rnd::fraction(bonesOneInN))
                    {
                        Map::put(new Bones(p));
                    }
                    else
                    {
                        static_cast<Floor*>(f)->type_ = FloorType::cave;
                    }
                }

                if (templCell.actorId == ActorId::rat)
                {
                    if (Rnd::oneIn(RAT_THING_ONE_IN_N_RAT))
                    {
                        ActorFactory::mk(ActorId::ratThing, p);
                    }
                    else
                    {
                        ActorFactory::mk(ActorId::rat, p);
                    }
                }
            }

            switch (templCell.val)
            {
            case 1:
                Map::player->pos = p;
                break;

            case 3:
                GameTime::addMob(new EventRatsInTheWallsDiscovery(p));
                break;

            default: {}
                break;
            }
        }
    }

    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksMoveCmn(true), blocked);

    //Spawn extra rats in the rightmost part of the map
    for (int x = (MAP_W * 7) / 8; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (!blocked[x][y])
            {
                const Pos p(x, y);

                if (Rnd::oneIn(RAT_THING_ONE_IN_N_RAT))
                {
                    ActorFactory::mk(ActorId::ratThing, p);
                }
                else
                {
                    ActorFactory::mk(ActorId::rat, p);
                }
            }
        }
    }

    //Set all actors to non-roaming (will be set to roaming by the discovery event)
    for (Actor* const actor : GameTime::actors_)
    {
        if (!actor->isPlayer())
        {
            static_cast<Mon*>(actor)->isRoamingAllowed_ = false;
        }
    }

    PopulateItems::mkItemsOnFloor();

    return true;
}

//------------------------------------------------------------------- BOSS
bool mkBossLvl()
{
    Map::resetMap();

    const MapTempl& templ     = MapTemplHandling::getTempl(MapTemplId::bossLevel);
    const Pos       templDims = templ.getDims();

    for (int x = 0; x < templDims.x; ++x)
    {
        for (int y = 0; y < templDims.y; ++y)
        {
            Map::cells[x][y].isDark = true;

            const auto& templCell = templ.getCell(x, y);
            const auto  fId       = templCell.featureId;

            const Pos p(x, y);

            if (fId != FeatureId::END)
            {
                const auto& d = FeatureData::getData(fId);
                Map::put(static_cast<Rigid*>(d.mkObj(p)));
            }

            if (templCell.actorId != ActorId::END)
            {
                ActorFactory::mk(templCell.actorId, Pos(x, y));
            }

            switch (templCell.val)
            {
            case 1:
                Map::player->pos = p;
                break;

            default: {}
                break;
            }
        }
    }

    return true;
}

//------------------------------------------------------------------- TRAPEZOHEDRON
bool mkTrapezohedronLvl()
{
    Map::resetMap();

//  for (int x = 0; x < MAP_W; ++x)
//  {
//    for (int y = 0; y < MAP_H; ++y)
//    {
//      auto* const wall  = new Wall(Pos(x, y));
//      Map::put(wall);
//      wall->type_       = WallType::cave;
//      wall->isMossy_    = false;
//    }
//  }
//
//  const Pos& origin     = Map::player->pos;
//  const Pos  mapCenter  = Pos(MAP_W_HALF, MAP_H_HALF);
//
//  auto putCaveFloor = [](const vector<Pos>& positions)
//  {
//    for (const Pos& p : positions)
//    {
//      auto* const floor = new Floor(p);
//      Map::put(floor);
//      floor->type_      = FloorType::cave;
//    }
//  };
//
//  vector<Pos> floorCells;
//
//  MapGenUtils::rndWalk(origin, 150, floorCells, true);
//  putCaveFloor(floorCells);
//
//  MapGenUtils::rndWalk(mapCenter, 800, floorCells, true);
//  putCaveFloor(floorCells);
//
//  MapGenUtils::pathfinderWalk(origin, mapCenter, floorCells, false);
//  putCaveFloor(floorCells);
//
//  bool blocked[MAP_W][MAP_H];
//  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);
//  vector<Pos> itemPosBucket;
//
//  for (int x = 0; x < MAP_W; ++x)
//  {
//    for (int y = 0; y < MAP_H; ++y)
//    {
//      if (!blocked[x][y] && Pos(x, y) != origin)
//      {
//        itemPosBucket.push_back(Pos(x, y));
//      }
//    }
//  }
//
//  const int ELEMENT = Rnd::range(0, itemPosBucket.size() - 1);
//  ItemFactory::mkItemOnFloor(ItemId::trapezohedron, itemPosBucket[ELEMENT]);

    return true;
}

} //MapGen
