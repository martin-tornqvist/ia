#ifndef ITEM_H
#define ITEM_H

#include <iostream>

#include "ItemData.h"
#include "Art.h"

enum ItemActivateReturn_t {
  itemActivate_keep, itemActivate_destroyed
};

class Item {
public:
  Item(ItemDefinition* itemDefinition) :
    numberOfItems(1), def_(itemDefinition) {
  }

  virtual ~Item();

  virtual void reset();

  const ItemDefinition& getDef() const {
    return *def_;
  }

  virtual SDL_Color getColor() const {
    return def_->color;
  }

  char getGlyph() const {
    return def_->glyph;
  }

  Tile_t getTile() const {
    return def_->tile;
  }

  virtual ItemActivateReturn_t defaultActivation(Actor*, Engine*) {
    return itemActivate_keep;
  }

  virtual void setRealDefinitionNames(Engine* const engine, const bool IS_SILENT_IDENTIFY) {
    (void)engine;
    (void)IS_SILENT_IDENTIFY;
  }

  virtual void itemSpecificAddSaveLines(vector<string>& lines) {
    (void)lines;
  }

  virtual void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
    (void)lines;
  }

  int getWeight() const {
    return def_->itemWeight * numberOfItems;
  }

  string getWeightLabel() const {
    const int WEIGHT = getWeight();
    if(WEIGHT <= (itemWeight_extraLight + itemWeight_light) / 2) return " - ";
    if(WEIGHT <= (itemWeight_light + itemWeight_medium) / 2) return "LGT";
    if(WEIGHT <= (itemWeight_medium + itemWeight_heavy) / 2) return "MED";
    return "HVY";
  }

  int numberOfItems;

protected:
  ItemDefinition* def_;

  //Called by the ItemDrop class to make noise etc
  friend class ItemDrop;
  virtual void appplyDropEffects() {
  }
};

#endif
