#ifndef ITEM_SCROLL_H
#define ITEM_SCROLL_H

#include "Item.h"
#include "Spells.h"

class Actor;

class Scroll: public Item {
public:
  Scroll(ItemDataT* const itemData) : Item(itemData) {}

  ~Scroll() {}

  ConsumeItem activateDefault(Actor* const actor) override;

  std::string getDefaultActivationLabel() const {return "Read";}

  Clr getInterfaceClr() const {return clrMagenta;}

  ConsumeItem read();

  const std::string getRealTypeName();

  void identify(const bool IS_SILENT_IDENTIFY) override;

  Spell* getSpell();
protected:
  void tryLearn();
};

namespace ScrollNameHandling {

void init();

void setFalseScrollName(ItemDataT& d);

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

} //ScrollNameHandling

#endif
