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

vector<MapData> mapList;

namespace
{

void mkLvl(const MapType& mapType)
{
    TRACE_FUNC_BEGIN;

    bool isLvlBuilt = false;

#ifndef NDEBUG
    int   nrAttempts  = 0;
    auto  startTime   = chrono::steady_clock::now();
#endif

    //TODO: When the map is invalid, any unique items spawned are lost forever.
    //Currently, the only effect of this should be that slightly fewever unique items
    //are found by the player.
    //It is bad design and should be fixed (but "good enough" for v17.0).

    while (!isLvlBuilt)
    {
#ifndef NDEBUG
        ++nrAttempts;
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

    mapList = vector<MapData>(NR_LVL_TOT, {MapType::std, IsMainDungeon::yes});

    //Forest intro level
    mapList[0] = {MapType::intro, IsMainDungeon::yes};

    //Occasionally set rats-in-the-walls level as intro to first late game level
    if (Rnd::oneIn(3))
    {
        mapList[DLVL_FIRST_LATE_GAME - 1] =
        {MapType::ratsInTheWalls, IsMainDungeon::yes};
    }

    //"Pharaoh chamber" is the first late game level
    mapList[DLVL_FIRST_LATE_GAME] = {MapType::egypt, IsMainDungeon::yes};

    mapList[DLVL_LAST + 1] = {MapType::boss,           IsMainDungeon::yes};
    mapList[DLVL_LAST + 2] = {MapType::trapezohedron,  IsMainDungeon::yes};
}

void storeToSaveLines(std::vector<std::string>& lines)
{
    lines.push_back(toStr(mapList.size()));

    for (const auto& mapData : mapList)
    {
        lines.push_back(toStr(int(mapData.type)));
        lines.push_back(mapData.isMainDungeon == IsMainDungeon::yes ? "1" : "0");
    }
}

void setupFromSaveLines(std::vector<std::string>& lines)
{
    const int NR_MAPS = toInt(lines.front());
    lines.erase(begin(lines));

    mapList.resize(size_t(NR_MAPS));

    for (auto& mapData : mapList)
    {
        mapData.type = MapType(toInt(lines.front()));
        lines.erase(begin(lines));

        mapData.isMainDungeon = lines.front() == "1" ?
                                IsMainDungeon::yes : IsMainDungeon::no;
        lines.erase(begin(lines));
    }
}

void goToNxt()
{
    TRACE_FUNC_BEGIN;

    mapList.erase(mapList.begin());
    const auto& mapData = mapList.front();

    if (mapData.isMainDungeon == IsMainDungeon::yes)
    {
        ++Map::dlvl;
    }

    mkLvl(mapData.type);

    Map::player->restoreShock(999, true);

    Map::player->tgt_ = nullptr;
    GameTime::updateLightMap();
    Map::player->updateFov();
    Map::player->updateClr();
    Render::drawMapAndInterface();

    if (mapData.isMainDungeon == IsMainDungeon::yes && Map::dlvl == DLVL_LAST - 1)
    {
        Log::addMsg("An ominous voice thunders in my ears.", clrWhite, false, true);
        Audio::play(SfxId::bossVoice2);
    }

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
    return mapList.front().type;
}

} //MapTravel
