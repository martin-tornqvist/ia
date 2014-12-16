#include "ItemDrop.h"

#include <algorithm>
#include <string>
#include <assert.h>

#include "CmnTypes.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "MapParsing.h"
#include "Utils.h"
#include "FeatureRigid.h"
#include "Render.h"

using namespace std;

namespace ItemDrop
{

void dropAllCharactersItems(Actor& actor)
{
  actor.getInv().dropAllNonIntrinsic(actor.pos);
}

void dropItemFromInv(Actor& actor, const InvList invList, const size_t ELEMENT,
                     const int NR_ITEMS_TO_DROP)
{
  Inventory& inv    = actor.getInv();
  Item* itemToDrop  = nullptr;

  if (invList == InvList::slots)
  {
    assert(ELEMENT != int(SlotId::END));
    itemToDrop = inv.slots_[ELEMENT].item;
  }
  else
  {
    assert(ELEMENT < inv.general_.size());
    itemToDrop = inv.general_[ELEMENT];
  }

  if (itemToDrop)
  {
    const bool  IS_STACKABLE            = itemToDrop->getData().isStackable;
    const int   NR_ITEMS_BEFORE_DROP    = itemToDrop->nrItems_;
    const bool  IS_WHOLE_STACK_DROPPED  = !IS_STACKABLE || NR_ITEMS_TO_DROP == -1 ||
                                          (NR_ITEMS_TO_DROP >= NR_ITEMS_BEFORE_DROP);

    string itemRef = "";

    if (invList == InvList::slots && IS_WHOLE_STACK_DROPPED)
    {
      //TODO This should be called from the Inventory instead.
      itemToDrop->onUnequip();
    }

    if (IS_WHOLE_STACK_DROPPED)
    {
      itemRef = itemToDrop->getName(ItemRefType::plural);
      inv.removeWithoutDestroying(invList, ELEMENT);
      dropItemOnMap(actor.pos, *itemToDrop);
    }
    else
    {
      Item* itemToKeep      = itemToDrop;
      itemToDrop            = ItemFactory::copyItem(itemToKeep);
      itemToDrop->nrItems_  = NR_ITEMS_TO_DROP;
      itemRef               = itemToDrop->getName(ItemRefType::plural);
      itemToKeep->nrItems_  = NR_ITEMS_BEFORE_DROP - NR_ITEMS_TO_DROP;
      dropItemOnMap(actor.pos, *itemToDrop);
    }

    //Messages
    if (&actor == Map::player)
    {
      Log::clearLog();
      Render::drawMapAndInterface();
      Log::addMsg("I drop " + itemRef + ".", clrWhite, false, true);
    }
    else
    {
      bool blocked[MAP_W][MAP_H];
      MapParse::run(CellCheck::BlocksLos(), blocked);
      if (Map::player->isSeeingActor(actor, blocked))
      {
        Log::addMsg(actor.getNameThe() + " drops " + itemRef + ".");
      }
    }

    //End turn
    GameTime::tick();
  }
}

//TODO This function is really weirdly written, and seems to even be doing
//wrong things. It should be refactored.
Item* dropItemOnMap(const Pos& intendedPos, Item& item)
{
  //If target cell is bottomless, just destroy the item
  const auto* const tgtRigid = Map::cells[intendedPos.x][intendedPos.y].rigid;
  if (tgtRigid->isBottomless())
  {
    delete &item;
    return nullptr;
  }

  //Make a vector of all cells on map with no blocking feature
  bool freeCellArray[MAP_W][MAP_H];
  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      Rigid* const f = Map::cells[x][y].rigid;
      freeCellArray[x][y] = f->canHaveItem() && !f->isBottomless();
    }
  }
  vector<Pos> freeCells;
  Utils::mkVectorFromBoolMap(true, freeCellArray, freeCells);

  //Sort the vector according to distance to origin
  IsCloserToPos isCloserToOrigin(intendedPos);
  sort(begin(freeCells), end(freeCells), isCloserToOrigin);

  Pos curPos;
  Pos stackPos;
  const bool IS_STACKABLE_TYPE = item.getData().isStackable;

  int ii = 0;
  const int VEC_SIZE = freeCells.size();
  for (int i = 0; i < VEC_SIZE; ++i)
  {
    //First look in all cells that has distance to origin equal to cell i to try and
    //merge the item if it stacks
    if (IS_STACKABLE_TYPE)
    {
      //While ii cell is not further away than i cell
      while (!isCloserToOrigin(freeCells[i], freeCells[ii]))
      {
        stackPos = freeCells[ii];
        Item* itemFoundOnFloor = Map::cells[stackPos.x][stackPos.y].item;
        if (itemFoundOnFloor)
        {
          if (itemFoundOnFloor->getData().id == item.getData().id)
          {
            item.nrItems_ += itemFoundOnFloor->nrItems_;
            delete itemFoundOnFloor;
            Map::cells[stackPos.x][stackPos.y].item = &item;
            return &item;
          }
        }
        ii++;
      }
    }
    else
    {
      //TODO Why is this called here? It doesn't seem right.
      item.appplyDropEffects();
    }

    curPos = freeCells[i];
    if (!Map::cells[curPos.x][curPos.y].item)
    {
      Map::cells[curPos.x][curPos.y].item = &item;

//      const bool IS_PLAYER_POS    = Map::player->pos == curPos;
//      const bool IS_INTENDED_POS  = curPos == intendedPos;
//      if (IS_PLAYER_POS && !IS_INTENDED_POS)
//      {
//        Log::addMsg("I feel something by my feet.");
//      }

      //TODO Won't this cause nullptr to be returned?
      //Shouldn't a pointer to the item be returned?
      break;
    }

    if (i == VEC_SIZE - 1)
    {
      delete &item;
      return nullptr;
    }
  }
  return nullptr;
}

} //ItemDrop
