#include "MapTravel.h"

#include "Init.h"

#include <list>

#ifndef NDEBUG
#include <chrono>
#endif // NDEBUG

#include "Map.h"
#include "MapGen.h"
#include "PopulateItems.h"
#include "Render.h"
#include "Log.h"
#include "FeatureRigid.h"
#include "Utils.h"

using namespace std;

namespace MapTravel
{

namespace
{

//Note: This includes forest intro level, rats in the walls level, etc. Basically every
//level that increments the DLVL number.
enum IsMainDungeon {no, yes};

struct MapData
{
    MapType       type;
    IsMainDungeon isMainDungeon;
};

vector<MapData> mapList_;

void mkLvl(const MapType& mapType)
{
    TRACE_FUNC_BEGIN;

    bool isLvlBuilt = false;

#ifndef NDEBUG
    int   nrAttempts  = 0;
    auto  startTime   = chrono::steady_clock::now();
#endif

    while (!isLvlBuilt)
    {
#ifndef NDEBUG
        nrAttempts++;
#endif
        switch (mapType)
        {
        case MapType::intro:          isLvlBuilt = MapGen::mkIntroLvl();          break;
        case MapType::std:            isLvlBuilt = MapGen::mkStdLvl();            break;
        case MapType::egypt:          isLvlBuilt = MapGen::mkEgyptLvl();          break;
        case MapType::leng:           isLvlBuilt = MapGen::mkLengLvl();           break;
        case MapType::ratsInTheWalls: isLvlBuilt = MapGen::mkRatsInTheWallsLvl(); break;
        case MapType::trapezohedron:  isLvlBuilt = MapGen::mkTrapezohedronLvl();  break;
        case MapType::boss:           isLvlBuilt = MapGen::mkBossLvl();           break;
        }
    }

#ifndef NDEBUG
    auto diffTime = chrono::steady_clock::now() - startTime;

    TRACE << "Map built after   " << nrAttempts << " attempt(s). " << endl
          << "Total time taken: "
          << chrono::duration <double, milli> (diffTime).count() << " ms" << endl;
#endif

    TRACE_FUNC_END;
}

} //namespace

void init()
{
    //Forest + dungeon + boss + trapezohedron
    const size_t NR_LVL_TOT = DLVL_LAST + 3;

    mapList_ = vector<MapData>(NR_LVL_TOT, {MapType::std, IsMainDungeon::yes});

    //Forest intro level
    mapList_[0] = {MapType::intro, IsMainDungeon::yes};

    //Occasionally set rats-in-the-walls level as first late game level
    if (Rnd::oneIn(3))
    {
        mapList_[DLVL_FIRST_LATE_GAME] = {MapType::ratsInTheWalls, IsMainDungeon::yes};
    }

    mapList_[DLVL_LAST + 1] = {MapType::boss,           IsMainDungeon::yes};
    mapList_[DLVL_LAST + 2] = {MapType::trapezohedron,  IsMainDungeon::yes};
}

void storeToSaveLines(std::vector<std::string>& lines)
{
    lines.push_back(toStr(mapList_.size()));
    for (const MapData& entry : mapList_)
    {
        lines.push_back(toStr(int(entry.type)));
        lines.push_back(entry.isMainDungeon == IsMainDungeon::yes ? "1" : "0");
    }
}

void setupFromSaveLines(std::vector<std::string>& lines)
{
    const int SIZE = toInt(lines.front());
    lines.erase(begin(lines));

    for (int i = 0; i < SIZE; ++i)
    {
        const MapType type = MapType(toInt(lines.front()));
        lines.erase(begin(lines));
        const IsMainDungeon isMainDungeon = lines.front() == "1" ?
                                            IsMainDungeon::yes : IsMainDungeon::no;
        lines.erase(begin(lines));
        mapList_.push_back({type, isMainDungeon});
    }
}

void goToNxt()
{
    TRACE_FUNC_BEGIN;

    Map::player->restoreShock(999, true);

    mapList_.erase(mapList_.begin());
    const auto& mapData = mapList_.front();

    if (mapData.isMainDungeon == IsMainDungeon::yes)
    {
        ++Map::dlvl;
    }

    mkLvl(mapData.type);

    Map::player->tgt_ = nullptr;
    Map::player->updateFov();
    Map::player->updateClr();
    Render::drawMapAndInterface();
    Audio::tryPlayAmb(1);

    if (Map::player->phobias[int(Phobia::deepPlaces)])
    {
        Log::addMsg("I am plagued by my phobia of deep places!");
        Map::player->getPropHandler().tryApplyProp(new PropTerrified(PropTurns::std));
        return;
    }

    TRACE_FUNC_END;
}

MapType getMapType()
{
    return mapList_.front().type;
}

} //MapTravel
