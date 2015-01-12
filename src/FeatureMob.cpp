#include "Init.h"

#include <algorithm>
#include <vector>

#include "FeatureMob.h"
#include "GameTime.h"
#include "PlayerBon.h"
#include "Explosion.h"
#include "Utils.h"
#include "Map.h"
#include "FeatureRigid.h"
#include "Fov.h"
#include "Inventory.h"
#include "Item.h"
#include "Log.h"

using namespace std;

//------------------------------------------------------------------- SMOKE
void Smoke::onNewTurn()
{
  auto* actor = Utils::getActorAtPos(pos_);

  if (actor)
  {
    const bool IS_PLAYER = actor == Map::player;

    //TODO: There needs to be some criteria here, so that e.g. a statue-monster or a
    //very alien monster can't get blinded by smoke (but do not use isHumanoid - rats,
    //wolves etc should definitely be blinded by smoke).
    //Perhaps add some property like "hasEyes"?

    bool isProtectedBlindness = false;

    if (IS_PLAYER)
    {
      auto&       inv             = Map::player->getInv();
      auto* const playerHeadItem  = inv.slots_[int(SlotId::head)].item;
      auto* const playerBodyItem  = inv.slots_[int(SlotId::body)].item;
      if (playerHeadItem)
      {
        if (playerHeadItem->getData().id == ItemId::gasMask)
        {
          isProtectedBlindness = true;

          //This may destroy the gasmask
          static_cast<GasMask*>(playerHeadItem)->decrTurnsLeft(inv);
        }
      }
      if (playerBodyItem)
      {
        if (playerBodyItem->getData().id == ItemId::armorAsbSuit)
        {
          isProtectedBlindness = true;
        }
      }
    }

    //Blinded by smoke?
    if (!isProtectedBlindness && Rnd::oneIn(4))
    {
      if (IS_PLAYER) {Log::addMsg("I am getting smoke in my eyes.");}
      actor->getPropHandler().tryApplyProp(
        new PropBlind(PropTurns::specific, Rnd::range(1, 3)));
    }

    //Choking (this is determined by rBreath)?
    if (Rnd::oneIn(4))
    {
      bool props[int(PropId::END)];
      actor->getPropHandler().getPropIds(props);

      if (!props[int(PropId::rBreath)])
      {
        string sndMsg = "";

        if (IS_PLAYER)
        {
          Log::addMsg("I am choking!", clrMsgBad);
        }
        else
        {
          if (actor->isHumanoid()) {sndMsg = "I hear choking.";}
        }

        const auto alerts = IS_PLAYER ? AlertsMon::yes : AlertsMon::no;

        SndEmit::emitSnd(Snd(sndMsg, SfxId::END, IgnoreMsgIfOriginSeen::yes, actor->pos,
                             actor, SndVol::low, alerts));
        actor->hit(1, DmgType::pure);
      }
    }
  }

  //If not permanent, count down turns left and possibly erase self
  if (nrTurnsLeft_ > -1)
  {
    if (--nrTurnsLeft_ <= 0) {GameTime::eraseMob(this, true);}
  }
}

string Smoke::getName(const Article article)  const
{
  string ret = "";
  if (article == Article::the) {ret += "the ";}
  return ret + "smoke";
}

Clr Smoke::getClr() const
{
  return clrGray;
}

//------------------------------------------------------------------- DYNAMITE
void LitDynamite::onNewTurn()
{
  nrTurnsLeft_--;
  if (nrTurnsLeft_ <= 0)
  {
    const int D = PlayerBon::traitsPicked[int(Trait::demExpert)] ? 1 : 0;

    const Pos pos(pos_);

    //Removing the dynamite before the explosion, so it won't be rendered after the
    //explosion (could happen for example if there are "more" prompts).
    GameTime::eraseMob(this, true);

    //Note: The dynamite is now deleted. Do not use member variable after this point.

    Explosion::runExplosionAt(pos, ExplType::expl, ExplSrc::misc, D);
  }
}

string LitDynamite::getName(const Article article)  const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "lit stick of dynamite";
}

Clr LitDynamite::getClr() const
{
  return clrRedLgt;
}

//------------------------------------------------------------------- FLARE
void LitFlare::onNewTurn()
{
  nrTurnsLeft_--;
  if (nrTurnsLeft_ <= 0) {GameTime::eraseMob(this, true);}
}

void LitFlare::addLight(bool light[MAP_W][MAP_H]) const
{
  bool myLight[MAP_W][MAP_H];
  Utils::resetArray(myLight, false);
  const int R = FOV_STD_RADI_INT; //getLightRadius();
  Pos p0(max(0,         pos_.x - R),  max(0,          pos_.y - R));
  Pos p1(min(MAP_W - 1, pos_.x + R),  min(MAP_H - 1,  pos_.y + R));
  bool blockedLos[MAP_W][MAP_H];
  for (int y = p0.y; y <= p1.y; ++y)
  {
    for (int x = p0.x; x <= p1.x; ++x)
    {
      blockedLos[x][y] = !Map::cells[x][y].rigid->isLosPassable();
    }
  }

  Fov::runFovOnArray(blockedLos, pos_, myLight, false);
  for (int y = p0.y; y <= p1.y; ++y)
  {
    for (int x = p0.x; x <= p1.x; ++x)
    {
      if (myLight[x][y]) {light[x][y] = true;}
    }
  }
}

string LitFlare::getName(const Article article)  const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "lit flare";
}

Clr LitFlare::getClr() const
{
  return clrYellow;
}
