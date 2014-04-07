#ifndef ITEM_SCROLL_H
#define ITEM_SCROLL_H

#include "Item.h"
#include "Spells.h"

class Engine;
class Actor;

class Scroll: public Item {
public:
  Scroll(ItemData* const itemData, Engine& engine) : Item(itemData, engine) {}

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

class ScrollNameHandler {
public:
  ScrollNameHandler(Engine& engine);
  ~ScrollNameHandler() {}

  void setFalseScrollName(ItemData& d);

  void addSaveLines(vector<string>& lines) const;
  void setParamsFromSaveLines(vector<string>& lines);

private:
  vector<string> falseNames_;

  Engine& eng;
};

#endif
