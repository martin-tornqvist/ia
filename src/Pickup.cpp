#include "Pickup.h"

#include <string>

#include "Init.h"
#include "Item.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Query.h"
#include "Drop.h"
#include "ItemFactory.h"
#include "GameTime.h"
#include "Audio.h"

using namespace std;

namespace ItemPickup
{

namespace
{

void pickupEffects(Actor* actor, Item* item)
{
    (void)actor;
    (void)item;
}

} //namespace

//Can always be called, to check if something is there to be picked up.
void tryPick()
{
    const Pos&  pos   = Map::player->pos;
    Item* const item  = Map::cells[pos.x][pos.y].item;

    if (item)
    {
        Inventory& playerInv = Map::player->getInv();

        const string ITEM_NAME = item->getName(ItemRefType::plural);

        //If picked up item is missile weapon, try to add it to carried stack.
        if (item->getData().ranged.isThrowingWpn)
        {
            Item* const carriedMissile = playerInv.getItemInSlot(SlotId::thrown);
            if (carriedMissile)
            {
                if (item->getId() == carriedMissile->getData().id)
                {
                    Audio::play(SfxId::pickup);

                    Log::addMsg("I add " + ITEM_NAME + " to my missile stack.");
                    carriedMissile->nrItems_ += item->nrItems_;
                    delete item;
                    Map::cells[pos.x][pos.y].item = nullptr;
                    GameTime::tick();
                    return;
                }
            }
        }

        Audio::play(SfxId::pickup);

        Log::clearLog();
        Log::addMsg("I pick up " + ITEM_NAME + ".");

        playerInv.putInGeneral(item);

        Map::cells[pos.x][pos.y].item = nullptr;

        GameTime::tick();
    }
    else //No item in this cell
    {
        Log::clearLog();
        Log::addMsg("I see nothing to pick up here.");
    }
}

Ammo* unloadRangedWpn(Wpn& wpn)
{
    assert(!wpn.getData().ranged.hasInfiniteAmmo);

    const int NR_AMMO_LOADED = wpn.nrAmmoLoaded;

    if (NR_AMMO_LOADED == 0)
    {
        return nullptr;
    }

    const ItemId      ammoId      = wpn.getData().ranged.ammoItemId;
    ItemDataT* const  ammoData    = ItemData::data[int(ammoId)];
    Item*             spawnedAmmo = ItemFactory::mk(ammoId);

    if (ammoData->type == ItemType::ammoClip)
    {
        //Unload a clip
        static_cast<AmmoClip*>(spawnedAmmo)->ammo_ = NR_AMMO_LOADED;
    }
    else
    {
        //Unload loose ammo
        spawnedAmmo->nrItems_ = NR_AMMO_LOADED;
    }

    wpn.nrAmmoLoaded = 0;

    return static_cast<Ammo*>(spawnedAmmo);
}

void tryUnloadWpnOrPickupAmmo()
{
    Item* item = Map::cells[Map::player->pos.x][Map::player->pos.y].item;

    if (item)
    {
        if (item->getData().ranged.isRangedWpn)
        {
            Wpn* const wpn = static_cast<Wpn*>(item);

            if (!wpn->getData().ranged.hasInfiniteAmmo)
            {
                Ammo* const spawnedAmmo = unloadRangedWpn(*wpn);

                if (spawnedAmmo)
                {
                    Audio::play(SfxId::pickup);

                    const string name = wpn->getName(ItemRefType::a, ItemRefInf::yes);

                    Log::addMsg("I unload " + name + ".");

                    Map::player->getInv().putInGeneral(spawnedAmmo);

                    GameTime::tick();
                    return;
                }
            }
        }
        else //Not a ranged weapon
        {
            if (
                item->getData().type == ItemType::ammo ||
                item->getData().type == ItemType::ammoClip)
            {
                tryPick();
                return;
            }
        }
    }

    Log::addMsg("I see no ammo to unload or pick up here.");
}

} //ItemPickup
