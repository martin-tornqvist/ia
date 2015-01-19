#include "Disarm.h"

#include "GameTime.h"
#include "Log.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "Query.h"
#include "Map.h"
#include "FeatureTrap.h"
#include "Utils.h"
#include "Inventory.h"

namespace Disarm
{

void playerDisarm()
{
    //TODO: It would probably be more fun if examine were allowed while blind,
    //with some potentially horrible results

    //Abort if blind
    if (!Map::player->getPropHandler().allowSee())
    {
        Log::addMsg("Not while blind.");
        Render::drawMapAndInterface();
        return;
    }

    //Abort if held by spider web
    const Pos playerPos = Map::player->pos;
    const auto* const featureAtPlayer =
        Map::cells[playerPos.x][playerPos.y].rigid;
    if (featureAtPlayer->getId() == FeatureId::trap)
    {
        const Trap* const trap = static_cast<const Trap*>(featureAtPlayer);
        if (trap->getTrapType() == TrapId::web)
        {
            const auto* const web = static_cast<const TrapWeb*>(trap->getSpecificTrap());
            if (web->isHolding())
            {
                Log::addMsg("Not while entangled in a spider web.");
                Render::drawMapAndInterface();
                return;
            }
        }
    }

    //Abort if encumbered
    if (Map::player->getEncPercent() >= 100)
    {
        Log::addMsg("Not while encumbered.");
        Render::drawMapAndInterface();
        return;
    }

    Log::addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
    Render::drawMapAndInterface();

    const Pos pos(Map::player->pos + DirUtils::getOffset(Query::dir()));

    if (pos == Map::player->pos)
    {
        Log::clearLog();
        Render::drawMapAndInterface();
    }
    else
    {
        //Abort if cell is unseen
        if (!Map::cells[pos.x][pos.y].isSeenByPlayer)
        {
            Log::addMsg("I cannot see there.");
            Render::drawMapAndInterface();
            return;
        }

        Log::clearLog();

        Actor* actorOnTrap = Utils::getActorAtPos(pos);

        //Abort if trap blocked by monster
        if (actorOnTrap)
        {
            if (Map::player->isSeeingActor(*actorOnTrap, nullptr))
            {
                Log::addMsg("It's blocked.");
            }
            else
            {
                Log::addMsg("Something is blocking it.");
            }
        }
        else //No actor on the trap
        {
            Map::cells[pos.x][pos.y].rigid->disarm();
        }

        Render::drawMapAndInterface();
    }
}

} //Disarm

