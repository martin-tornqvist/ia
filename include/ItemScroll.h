#ifndef ITEM_SCROLL_H
#define ITEM_SCROLL_H

#include "Item.h"

class Actor;
class Spell;

class Scroll: public Item
{
public:
  Scroll(ItemDataT* const itemData) :
    Item(itemData) {}

  ~Scroll() {}

  Clr getInterfaceClr() const override
  {
    return clrMagenta;
  }

  ConsumeItem activate(Actor* const actor) override;

  ConsumeItem read();

  const std::string getRealName() const;

  std::vector<std::string> getDescr() const override;

  void identify(const bool IS_SILENT_IDENTIFY) override;

  Spell* mkSpell() const;

protected:
  void tryLearn();

  std::string getNameInf() const override;
};

namespace ScrollNameHandling
{

void init();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

} //ScrollNameHandling

#endif
