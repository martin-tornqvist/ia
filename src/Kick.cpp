#include "Kick.h"

#include "Init.h"
#include "GameTime.h"
#include "FeatureRigid.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Render.h"
#include "MapParsing.h"
#include "Utils.h"
#include "TextFormatting.h"

using namespace std;

namespace Kick
{

void playerKick()
{
  TRACE_FUNC_BEGIN;

  Log::clearLog();
  Log::addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  Render::drawMapAndInterface();
  Pos kickPos(Map::player->pos + DirUtils::getOffset(Query::dir()));
  Log::clearLog();

  if (kickPos != Map::player->pos)
  {
    //Kick living actor?
    Actor* livingActor = Utils::getActorAtPos(kickPos, ActorState::alive);
    if (livingActor)
    {
      TRACE << "Actor found at kick pos, attempting to kick actor" << endl;
      if (Map::player->getPropHandler().allowAttackMelee(true))
      {
        TRACE << "Player is allowed to do melee attack" << endl;
        bool blocked[MAP_W][MAP_H];
        MapParse::run(CellCheck::BlocksLos(), blocked);

        TRACE << "Player can see actor" << endl;
        Map::player->kickMon(*livingActor);
      }
      TRACE_FUNC_END;
      return;
    }

    //Kick corpse?
    Actor* deadActor = Utils::getActorAtPos(kickPos, ActorState::corpse);
    if (deadActor)
    {
      const bool    IS_SEEING_CELL  = Map::cells[kickPos.x][kickPos.y].isSeenByPlayer;
      const string  corpseName      = IS_SEEING_CELL ? deadActor->getCorpseNameA() :
                                      "a corpse";

      Log::addMsg("I bash " + TextFormatting::firstToLower(corpseName) + ".");

      pair<int, int> kickDmg = ItemData::data[int(ItemId::playerKick)]->melee.dmg;
      deadActor->hit(kickDmg.first * kickDmg.second, DmgType::physical, DmgMethod::kick);

      GameTime::tick();
      TRACE_FUNC_END;
      return;
    }

    //Kick feature
    TRACE << "No actor at kick pos, attempting to kick feature instead" << endl;
    auto* const f = Map::cells[kickPos.x][kickPos.y].rigid;
    f->hit(DmgType::physical, DmgMethod::kick, Map::player);
  }
  TRACE_FUNC_END;
}

} //Kick
