#include "Knockback.h"

#include <algorithm>
#include <vector>

#include "Attack.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Config.h"
#include "GameTime.h"
#include "Render.h"
#include "MapParsing.h"
#include "SdlWrapper.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"

using namespace std;

namespace KnockBack
{

void tryKnockBack(Actor& defender, const Pos& attackedFromPos,
                  const bool IS_SPIKE_GUN, const bool IS_MSG_ALLOWED)
{
  const bool IS_DEF_MON = &defender != Map::player;

  if(IS_DEF_MON || !Config::isBotPlaying())
  {
    if(defender.getData().actorSize <= actorSize_giant)
    {
      vector<PropId> props;
      defender.getPropHandler().getAllActivePropIds(props);

      const bool DEF_CAN_BE_KNOCKED_BACK =
        find(begin(props), end(props), propEthereal)  == end(props) &&
        find(begin(props), end(props), propOoze)      == end(props);

      const Pos d = (defender.pos - attackedFromPos).getSigns();

      const int KNOCK_RANGE = 2;

      for(int i = 0; i < KNOCK_RANGE; ++i)
      {
        const Pos newPos = defender.pos + d;

        bool blocked[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksActor(defender, true), blocked);

        const bool IS_CELL_BOTTOMLESS =
          Map::cells[newPos.x][newPos.y].rigid->isBottomless();

        const bool IS_CELL_BLOCKED = blocked[newPos.x][newPos.y] &&
                                     !IS_CELL_BOTTOMLESS;

        if(DEF_CAN_BE_KNOCKED_BACK && !IS_CELL_BLOCKED)
        {
          const bool IS_PLAYER_SEE_DEF = IS_DEF_MON ?
                                         Map::player->isSeeingActor(defender, nullptr) :
                                         true;

          if(i == 0)
          {
            if(IS_MSG_ALLOWED)
            {
              if(IS_DEF_MON && IS_PLAYER_SEE_DEF)
              {
                Log::addMsg(defender.getNameThe() + " is knocked back!");
              }
              else
              {
                Log::addMsg("I am knocked back!");
              }
            }
            defender.getPropHandler().tryApplyProp(
              new PropParalyzed(PropTurns::specific, 1), false, false);
          }

          defender.pos = newPos;

          Render::drawMapAndInterface();

          SdlWrapper::sleep(Config::getDelayProjectileDraw());

          if(IS_CELL_BOTTOMLESS)
          {
            if(IS_DEF_MON && IS_PLAYER_SEE_DEF)
            {
              Log::addMsg(
                defender.getNameThe() + " plummets down the depths.",
                clrMsgGood);
            }
            else
            {
              Log::addMsg("I plummet down the depths!", clrMsgBad);
            }
            defender.die(true, false, false);
            return;
          }

          // Bump features (e.g. so monsters can be knocked back into traps)
          vector<Mob*> mobs;
          GameTime::getMobsAtPos(defender.pos, mobs);
          for(Mob* const mob : mobs)
          {
            mob->bump(defender);
          }

          if(!defender.isAlive())
          {
            return;
          }

          Rigid* const f = Map::cells[defender.pos.x][defender.pos.y].rigid;
          f->bump(defender);

          if(!defender.isAlive())
          {
            return;
          }
        }
        else //Cannot knock back (cell blocked or defender cannot be knocked)
        {
          //Defender nailed to a wall from a spike gun?
          if(IS_SPIKE_GUN)
          {
            Rigid* const f = Map::cells[newPos.x][newPos.y].rigid;
            if(!f->isLosPassable())
            {
              defender.getPropHandler().tryApplyProp(
                new PropNailed(PropTurns::indefinite));
            }
          }
          return;
        }
      }
    }
  }
}

} //KnockBack
