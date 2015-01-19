#include "PopulateTraps.h"

#include <algorithm>

#include "Init.h"
#include "Map.h"
#include "MapGen.h"
#include "MapParsing.h"
#include "Utils.h"
#include "FeatureData.h"
#include "FeatureTrap.h"

using namespace std;

namespace PopulateTraps
{

namespace
{

void mkTrapAt(const TrapId id, const Pos& pos)
{
    const auto* const f     = Map::cells[pos.x][pos.y].rigid;
    const auto&       d     = FeatureData::getData(f->getId());
    const auto* const mimic = static_cast<Rigid*>(d.mkObj(pos));
    Map::put(new Trap(pos, mimic, id));
}

} //namespace

void populateStdLvl()
{
    TRACE_FUNC_BEGIN;

    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksMoveCmn(false), blocked);

    //Put traps in non-plain rooms
    for (Room* const room : Map::roomList)
    {
        const RoomType type = room->type_;

        if (type != RoomType::plain)
        {
            Fraction chanceForTrappedRoom(0, 0);

            switch (type)
            {
            case RoomType::human:   chanceForTrappedRoom.set(1, 4); break;
            case RoomType::ritual:  chanceForTrappedRoom.set(1, 4); break;
            case RoomType::spider:  chanceForTrappedRoom.set(2, 3); break;
            case RoomType::crypt:   chanceForTrappedRoom.set(3, 4); break;
            case RoomType::monster: chanceForTrappedRoom.set(1, 4); break;
            case RoomType::chasm:   chanceForTrappedRoom.set(1, 4); break;
            case RoomType::forest:
            case RoomType::plain:
            case RoomType::flooded:
            case RoomType::muddy:
            case RoomType::cave:
            case RoomType::END_OF_STD_ROOMS:
            case RoomType::river:
            case RoomType::corrLink:
            case RoomType::crumbleRoom: {} break;
            }

            if (Rnd::fraction(chanceForTrappedRoom))
            {
                TRACE_VERBOSE << "Trapping non-plain room" << endl;

                vector<Pos> trapPosBucket;

                const Pos& p0 = room->r_.p0;
                const Pos& p1 = room->r_.p1;
                for (int y = p0.y; y <= p1.y; ++y)
                {
                    for (int x = p0.x; x <= p1.x; ++x)
                    {
                        if (!blocked[x][y] && Map::cells[x][y].rigid->canHaveRigid())
                        {
                            trapPosBucket.push_back(Pos(x, y));
                        }
                    }
                }

                int         nrPosCand       = int(trapPosBucket.size());
                const bool  IS_SPIDER_ROOM  = type == RoomType::spider;
                const int   NR_BASE_TRAPS   = min(nrPosCand / 2, IS_SPIDER_ROOM ? 3 : 1);

                for (int i = 0; i < NR_BASE_TRAPS; ++i)
                {
                    if (nrPosCand == 0)
                    {
                        break;
                    }

                    const TrapId trapType = IS_SPIDER_ROOM ?
                                            TrapId::web :
                                            TrapId(Rnd::range(0, int(TrapId::END) - 1));

                    const int ELEMENT = Rnd::range(0, trapPosBucket.size() - 1);
                    const Pos& pos    = trapPosBucket[ELEMENT];

                    TRACE_VERBOSE << "Placing base trap" << endl;
                    mkTrapAt(trapType, pos);
                    blocked[pos.x][pos.y] = true;
                    trapPosBucket.erase(trapPosBucket.begin() + ELEMENT);
                    nrPosCand--;

                    //Spawn up to N traps in nearest cells (not necessarily adjacent)
                    IsCloserToPos sorter(pos);
                    sort(trapPosBucket.begin(), trapPosBucket.end(), sorter);
                    const int NR_ADJ = trapType == TrapId::web ? 0 :
                                       min(Rnd::range(0, 2), nrPosCand);
                    TRACE_VERBOSE << "Placing adjacent traps" << endl;
                    for (int i_adj = 0; i_adj < NR_ADJ; i_adj++)
                    {
                        const Pos& adjPos = trapPosBucket.front();
                        mkTrapAt(trapType, adjPos);
                        blocked[adjPos.x][adjPos.y] = true;
                        trapPosBucket.erase(trapPosBucket.begin());
                        nrPosCand--;
                    }
                }
            }
        }
    }

    const int CHANCE_ALLOW_TRAPPED_PLAIN_AREAS = min(85, 30 + (Map::dlvl * 5));
    if (Rnd::percentile() < CHANCE_ALLOW_TRAPPED_PLAIN_AREAS)
    {
        TRACE_VERBOSE << "Trapping plain room" << endl;

        vector<Pos> trapPosBucket;
        for (int y = 1; y < MAP_H - 1; ++y)
        {
            for (int x = 1; x < MAP_W - 1; ++x)
            {
                if (Map::roomMap[x][y])
                {
                    if (
                        !blocked[x][y]                                &&
                        Map::roomMap[x][y]->type_ == RoomType::plain  &&
                        Map::cells[x][y].rigid->canHaveRigid())
                    {
                        trapPosBucket.push_back(Pos(x, y));
                    }
                }
            }
        }

        int nrPosCand = int(trapPosBucket.size());
        const int NR_BASE_TRAPS = min(nrPosCand / 2, Rnd::range(1, 3));
        for (int i = 0; i < NR_BASE_TRAPS; ++i)
        {
            if (nrPosCand == 0)
            {
                break;
            }

            const TrapId trapType = TrapId(Rnd::range(0, int(TrapId::END) - 1));

            const int ELEMENT = Rnd::range(0, trapPosBucket.size() - 1);
            const Pos& pos = trapPosBucket[ELEMENT];

            TRACE_VERBOSE << "Placing base trap" << endl;
            mkTrapAt(trapType, pos);
            trapPosBucket.erase(trapPosBucket.begin() + ELEMENT);
            nrPosCand--;

            //Spawn up to N traps in nearest cells (not necessarily adjacent)
            IsCloserToPos sorter(pos);
            sort(trapPosBucket.begin(), trapPosBucket.end(), sorter);
            const int NR_ADJ = trapType == TrapId::web ? 0 :
                               min(Rnd::range(0, 2), nrPosCand);
            TRACE_VERBOSE << "Placing adjacent traps" << endl;
            for (int i_adj = 0; i_adj < NR_ADJ; i_adj++)
            {
                const Pos& adjPos = trapPosBucket.front();
                mkTrapAt(trapType, adjPos);
                trapPosBucket.erase(trapPosBucket.begin());
                nrPosCand--;
            }
        }
    }
    TRACE_FUNC_END;
}

} //PopulateTraps
