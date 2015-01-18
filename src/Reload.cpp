#include "Reload.h"

#include <string>

#include "Converters.h"
#include "Item.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "DungeonMaster.h"
#include "PlayerBon.h"
#include "GameTime.h"
#include "Audio.h"
#include "Render.h"
#include "Utils.h"

using namespace std;

namespace Reload
{

namespace
{

void printMsgAndPlaySfx(Actor& actorReloading, Wpn* const wpn,
                        Item* const ammo, const ReloadResult result,
                        const bool IS_SWIFT_RELOAD)
{
  string ammoName = "";
  bool isClip = false;

  if (ammo)
  {
    ammoName  = ammo->getName(ItemRefType::a);
    isClip    = ammo->getData().type == ItemType::ammoClip;
  }

  const bool IS_PLAYER    = actorReloading.isPlayer();
  const string actorName  = actorReloading.getNameThe();

  switch (result)
  {
    case ReloadResult::notCarryingWpn:
      if (IS_PLAYER)
      {
        Log::addMsg("I am not wielding a weapon.");
      }
      break;

    case ReloadResult::wpnNotUsingAmmo:
      if (IS_PLAYER)
      {
        Log::addMsg("This weapon does not use ammo.");
      }
      break;

    case ReloadResult::alreadyFull:
      if (IS_PLAYER)
      {
        Log::addMsg("Weapon already loaded.");
      }
      break;

    case ReloadResult::noAmmo:
      if (IS_PLAYER)
      {
        Log::addMsg("I carry no ammunition for this weapon.");
      }
      break;

    case ReloadResult::success:
    {
      const string swiftStr = IS_SWIFT_RELOAD ? " swiftly" : "";
      if (IS_PLAYER)
      {
        Audio::play(wpn->getData().ranged.reloadSfx);

        if (isClip)
        {
          const string wpnName = wpn->getName(ItemRefType::plain, ItemRefInf::none);
          Log::addMsg(
            "I" + swiftStr + " reload the " + wpnName +
            " (" + toStr(wpn->nrAmmoLoaded) + "/" + toStr(wpn->AMMO_CAP) + ").");
        }
        else
        {
          Log::addMsg(
            "I" + swiftStr + " load " + ammoName + " (" + toStr(wpn->nrAmmoLoaded) +
            "/" + toStr(wpn->AMMO_CAP) + ").");
        }
        Render::drawMapAndInterface();
      }
      else
      {
        if (Map::player->isSeeingActor(actorReloading, nullptr))
        {
          Log::addMsg(actorName + swiftStr + " reloads.");
        }
      }
    } break;

    case ReloadResult::fumble:
      if (IS_PLAYER)
      {
        Log::addMsg("I fumble with " + ammoName + ".");
      }
      else
      {
        if (Map::player->isSeeingActor(actorReloading, nullptr))
        {
          Log::addMsg(actorName + " fumbles with " + ammoName + ".");
        }
      }
      break;
  }
}

} //namespace

bool reloadWieldedWpn(Actor& actorReloading)
{
  bool didAct = false;

  Inventory& inv      = actorReloading.getInv();
  Item* const wpnItem = inv.getItemInSlot(SlotId::wielded);

  if (!wpnItem)
  {
    printMsgAndPlaySfx(actorReloading, nullptr, nullptr,
                       ReloadResult::notCarryingWpn, false);
    return didAct;
  }

  Wpn* const    wpn           = static_cast<Wpn*>(wpnItem);
  ReloadResult  result        = ReloadResult::noAmmo;
  bool          isSwiftReload = false;

  if (actorReloading.isPlayer())
  {
    isSwiftReload = PlayerBon::traitsPicked[int(Trait::expertMarksman)] &&
                    Rnd::coinToss();
  }

  const int wpnAmmoCapacity = wpn->AMMO_CAP;

  if (wpnAmmoCapacity == 0)
  {
    printMsgAndPlaySfx(actorReloading, wpn, nullptr,
                       ReloadResult::wpnNotUsingAmmo, false);
  }
  else
  {
    const ItemId ammoType = wpn->getData().ranged.ammoItemId;
    Item* item            = nullptr;

    if (wpn->nrAmmoLoaded < wpnAmmoCapacity)
    {
      for (size_t i = 0; i < inv.general_.size(); ++i)
      {
        item = inv.general_[i];

        if (item->getId() == ammoType)
        {
          PropHandler& propHlr = actorReloading.getPropHandler();

          bool props[size_t(PropId::END)];
          propHlr.getPropIds(props);

          const bool IS_RELOADER_BLIND      = !actorReloading.getPropHandler().allowSee();
          const bool IS_REALOADER_TERRIFIED = props[int(PropId::terrified)];

          const int CHANCE_TO_FUMBLE = (IS_RELOADER_BLIND      ? 48 : 0) +
                                       (IS_REALOADER_TERRIFIED ? 48 : 0);

          if (Rnd::percentile() < CHANCE_TO_FUMBLE)
          {
            isSwiftReload = false;
            result        = ReloadResult::fumble;

            printMsgAndPlaySfx(actorReloading, nullptr, item, ReloadResult::fumble,
                               false);
          }
          else //Not fumbling
          {
            result      = ReloadResult::success;
            bool isClip = item->getData().type == ItemType::ammoClip;

            if (isClip)
            {
              const int previousAmmoCount = wpn->nrAmmoLoaded;
              AmmoClip* clipItem          = static_cast<AmmoClip*>(item);
              wpn->nrAmmoLoaded           = clipItem->ammo_;

              printMsgAndPlaySfx(actorReloading, wpn, item, result, isSwiftReload);

              //Destroy loaded clip
              inv.removeItemInBackpackWithIdx(i, true);

              //If weapon previously contained ammo, create a new clip item
              if (previousAmmoCount > 0)
              {
                item = ItemFactory::mk(ammoType);
                clipItem = static_cast<AmmoClip*>(item);
                clipItem->ammo_ = previousAmmoCount;
                inv.putInGeneral(clipItem);
              }
            }
            else //Ammo is stackable (e.g. shotgun shells)
            {
              wpn->nrAmmoLoaded += 1;

              printMsgAndPlaySfx(actorReloading, wpn, item, result, isSwiftReload);

              //Decrease ammo item number
              inv.decrItemInGeneral(i);
            }
          }
          break;
        }
      }
      if (result == ReloadResult::noAmmo)
      {
        printMsgAndPlaySfx(actorReloading, wpn, item, result, isSwiftReload);
      }
    }
    else //Weapon is full
    {
      result = ReloadResult::alreadyFull;
      printMsgAndPlaySfx(actorReloading, wpn, item, result, isSwiftReload);
    }
  }

  if (result == ReloadResult::success || result == ReloadResult::fumble)
  {
    didAct = true;
    GameTime::tick(isSwiftReload);
  }

  return didAct;
}

} //Reload
