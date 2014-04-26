#ifndef ITEM_SCROLL_H
#define ITEM_SCROLL_H

#include "Item.h"
#include "Spells.h"

class Actor;

class Scroll: public Item {
public:
  Scroll(ItemData* const itemData) : Item(itemData) {}

  ~Scroll() {}

  ConsumeItem activateDefault(Actor* const actor) override;

  string getDefaultActivationLabel() const {return "Read";}

  SDL_Color getInterfaceClr() const {return clrMagenta;}

  ConsumeItem read();

  const string getRealTypeName();

  void identify(const bool IS_SILENT_IDENTIFY) override;

  Spell* getSpell();
protected:
  void tryLearn();
};

namespace ScrollNameHandling {

void init();

void setFalseScrollName(ItemData& d);

void storeToSaveLines(vector<string>& lines);
void setupFromSaveLines(vector<string>& lines);

} //ScrollNameHandling

#endif
