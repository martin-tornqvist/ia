#include "ItemDrop.h"

#include "Engine.h"
#include "ConstDungeonSettings.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Inventory.h"
#include "ItemFactory.h"

#include <algorithm>

void ItemDrop::dropAllCharactersItems(Actor* actor, bool died) {
  (void)died;
  actor->getInventory()->dropAllNonIntrinsic(actor->pos, true, eng);
}

void ItemDrop::dropItemFromInventory(Actor* actorDropping, const int ELEMENT, const int NR_ITEMS_TO_DROP) {
  Inventory* inventory = actorDropping->getInventory();
  Item* itemToDrop = inventory->getItemInElement(ELEMENT);

  const bool IS_STACKABLE = itemToDrop->getDef().isStackable;
  const int NR_ITEMS_BEFORE_DROP = itemToDrop->numberOfItems;
  const bool IS_WHOLE_STACK_DROPPED =
    IS_STACKABLE == false ||
    NR_ITEMS_TO_DROP == -1 ||
    (NR_ITEMS_TO_DROP >= NR_ITEMS_BEFORE_DROP);

  if(itemToDrop != NULL) {
    if(IS_WHOLE_STACK_DROPPED) {
      inventory->removeItemInElementWithoutDeletingInstance(ELEMENT);
      eng->itemDrop->dropItemOnMap(actorDropping->pos, &itemToDrop);
    } else {
      Item* itemToKeep = itemToDrop;
      itemToDrop = eng->itemFactory->copyItem(itemToKeep);
      itemToDrop->numberOfItems = NR_ITEMS_TO_DROP;
      itemToKeep->numberOfItems = NR_ITEMS_BEFORE_DROP - NR_ITEMS_TO_DROP;
      eng->itemDrop->dropItemOnMap(actorDropping->pos, &itemToDrop);
    }

    //Messages
    const string itemRef = eng->itemData->getItemRef(itemToDrop, itemRef_plural);
    const Actor* const curActor = eng->gameTime->getCurrentActor();
    if(curActor == eng->player) {
      eng->log->clearLog();
      eng->log->addMessage("I drop " + itemRef + ".");
    } else {
      bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);
      if(eng->player->checkIfSeeActor(*curActor, blockers)) {
        eng->log->addMessage("I see " + curActor->getNameThe() + " drop " + itemRef + ".");
      }
    }

    //End turn
    eng->gameTime->letNextAct();

  }
}

void ItemDrop::dropItemOnMap(const coord pos, Item** item) {
  //If target cell is bottomless, just destroy the item
  if(eng->map->featuresStatic[pos.x][pos.y]->isBottomless()) {
    delete *item;
    return;
  }

  //Make a vector of all cells on map with no blocking feature
  bool freeCellArray[MAP_X_CELLS][MAP_Y_CELLS];
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      FeatureStatic* const f = eng->map->featuresStatic[x][y];
      freeCellArray[x][y] = f->canHaveItem() && f->isBottomless() == false;
    }
  }
  vector<coord> freeCells;
  eng->mapTests->makeMapVectorFromArray(freeCellArray, freeCells);

  //Sort the vector according to distance to origin
  IsCloserToOrigin isCloserToOrigin(pos, eng);
  sort(freeCells.begin(), freeCells.end(), isCloserToOrigin);

  int curX, curY, stackX, stackY;
  const bool ITEM_STACKS = (*item)->getDef().isStackable;
  int ii = 0;
  Item* stackItem;
  const unsigned int vectorSize = freeCells.size();
  for(unsigned int i = 0; i < vectorSize; i++) {
    //First look in all cells that has distance to origin equal to cell i
    //to try and merge the item if it stacks
    if(ITEM_STACKS) {
      //While ii cell is not further away than i cell
      while(isCloserToOrigin(freeCells.at(i), freeCells.at(ii)) == false) {
        stackX = freeCells.at(ii).x;
        stackY = freeCells.at(ii).y;
        stackItem = eng->map->items[stackX][stackY];
        if(stackItem != NULL) {
          if(stackItem->getDef().devName == (*item)->getDef().devName) {
            stackItem->numberOfItems += (*item)->numberOfItems;
            delete(*item);
            *item = NULL;
            i = 999999;
            break;
          }
        }
        ii++;
      }
    } else {
      (*item)->appplyDropEffects();
    }

    if(*item == NULL) {
      break;
    }

    curX = freeCells.at(i).x;
    curY = freeCells.at(i).y;
    if(eng->map->items[curX][curY] == NULL) {
      eng->map->items[curX][curY] = *item;
      if(eng->player->pos == coord(curX, curY)) {
        if(curX != pos.x || curY != pos.y) {
          eng->log->addMessage("I feel something by my feet.");
        }
      }

      i = 999999;
    }

    if(i == vectorSize - 1) {
      delete *item;
      *item = NULL;
    }
  }
}

