#include "ItemDrop.h"

#include "CmnTypes.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "MapParsing.h"
#include "Utils.h"

#include <algorithm>

namespace ItemDrop {

void dropAllCharactersItems(Actor* actor, bool died) {
  (void)died;
  actor->getInv().dropAllNonIntrinsic(actor->pos, true);
}

void dropItemFromInv(Actor* actorDropping, const int ELEMENT,
                     const int NR_ITEMS_TO_DROP) {
  Inventory& inv = actorDropping->getInv();
  Item* itemToDrop = inv.getItemInElement(ELEMENT);

  if(itemToDrop != NULL) {
    const bool IS_STACKABLE = itemToDrop->getData().isStackable;
    const int NR_ITEMS_BEFORE_DROP = itemToDrop->nrItems;
    const bool IS_WHOLE_STACK_DROPPED =
      IS_STACKABLE == false ||
      NR_ITEMS_TO_DROP == -1 ||
      (NR_ITEMS_TO_DROP >= NR_ITEMS_BEFORE_DROP);

    string itemRef = "";

    if(IS_WHOLE_STACK_DROPPED) {
      itemRef = ItemData::getItemRef(*itemToDrop, ItemRefType::plural);
      inv.removeInElementWithoutDeletingInstance(ELEMENT);
      dropItemOnMap(actorDropping->pos, *itemToDrop);
    } else {
      Item* itemToKeep = itemToDrop;
      itemToDrop = ItemFactory::copyItem(itemToKeep);
      itemToDrop->nrItems = NR_ITEMS_TO_DROP;
      itemRef = ItemData::getItemRef(*itemToDrop, ItemRefType::plural);
      itemToKeep->nrItems = NR_ITEMS_BEFORE_DROP - NR_ITEMS_TO_DROP;
      dropItemOnMap(actorDropping->pos, *itemToDrop);
    }

    //Messages
    const Actor* const curActor = GameTime::getCurrentActor();
    if(curActor == Map::player) {
      Log::clearLog();
      Log::addMsg(
        "I drop " + itemRef + ".", clrWhite, false, true);
    } else {
      bool blockers[MAP_W][MAP_H];
      MapParse::parse(CellPred::BlocksVision(), blockers);
      if(Map::player->isSeeingActor(*curActor, blockers)) {
        Log::addMsg(
          "I see " + curActor->getNameThe() + " drop " + itemRef + ".");
      }
    }

    //End turn
    GameTime::actorDidAct();
  }
}

//TODO This function is really weirdly written, and seems to even be doing
//wrong things. It should be refactored.
Item* dropItemOnMap(const Pos& intendedPos, Item& item) {
  //If target cell is bottomless, just destroy the item
  const Feature* const targetFeature =
    Map::cells[intendedPos.x][intendedPos.y].featureStatic;
  if(targetFeature->isBottomless()) {
    delete &item;
    return NULL;
  }

  //Make a vector of all cells on map with no blocking feature
  bool freeCellArray[MAP_W][MAP_H];
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      FeatureStatic* const f = Map::cells[x][y].featureStatic;
      freeCellArray[x][y] = f->canHaveItem() && f->isBottomless() == false;
    }
  }
  vector<Pos> freeCells;
  Utils::makeVectorFromBoolMap(true, freeCellArray, freeCells);

  //Sort the vector according to distance to origin
  IsCloserToOrigin isCloserToOrigin(intendedPos);
  sort(freeCells.begin(), freeCells.end(), isCloserToOrigin);

  Pos curPos;
  Pos stackPos;
  const bool IS_STACKABLE_TYPE = item.getData().isStackable;

  int ii = 0;
  const int VEC_SIZE = freeCells.size();
  for(int i = 0; i < VEC_SIZE; i++) {
    //First look in all cells that has distance to origin equal to cell i
    //to try and merge the item if it stacks
    if(IS_STACKABLE_TYPE) {
      //While ii cell is not further away than i cell
      while(isCloserToOrigin(freeCells.at(i), freeCells.at(ii)) == false) {
        stackPos = freeCells.at(ii);
        Item* itemFoundOnFloor = Map::cells[stackPos.x][stackPos.y].item;
        if(itemFoundOnFloor != NULL) {
          if(itemFoundOnFloor->getData().id == item.getData().id) {
            item.nrItems += itemFoundOnFloor->nrItems;
            delete itemFoundOnFloor;
            Map::cells[stackPos.x][stackPos.y].item = &item;
            return &item;
          }
        }
        ii++;
      }
    } else {
      //TODO Why is this called here? It doesn't seem right.
      item.appplyDropEffects();
    }

    if(&item == NULL) {break;}

    curPos = freeCells.at(i);
    if(Map::cells[curPos.x][curPos.y].item == NULL) {

      Map::cells[curPos.x][curPos.y].item = &item;

      const bool IS_PLAYER_POS    = Map::player->pos == curPos;
      const bool IS_INTENDED_POS  = curPos == intendedPos;
      if(IS_PLAYER_POS && IS_INTENDED_POS == false) {
        Log::addMsg("I feel something by my feet.");
      }

      //TODO Won't this cause NULL to be returned?
      //Shouldn't a pointer to the item be returned?
      break;
    }

    if(i == VEC_SIZE - 1) {
      delete &item;
      return NULL;
    }
  }
  return NULL;
}

} //ItemDrop
