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

  bool activateDefault(Actor* const actor);

  string getDefaultActivationLabel() const {return "Read";}

  SDL_Color getInterfaceClr() const {return clrMagenta;}

  bool read();

  const string getRealTypeName();

  void identify(const bool IS_SILENT_IDENTIFY);

  Spell* getSpell();
protected:
  void tryLearn();

  void failedToLearnRealName(const string overrideFailString = "");
};

class ScrollNameHandler {
public:
  ScrollNameHandler(Engine& engine);
  ~ScrollNameHandler() {}

  void setFalseScrollName(ItemData& d);

  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(vector<string>& lines);

private:
  vector<string> falseNames_;

  Engine& eng;
};

#endif
