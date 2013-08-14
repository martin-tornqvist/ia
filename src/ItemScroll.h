#ifndef ITEM_SCROLL_H
#define ITEM_SCROLL_H

#include "Item.h"
#include "Spells.h"

class Engine;
class Actor;

class Scroll: public Item {
public:
  Scroll(ItemDef* const itemDefinition) : Item(itemDefinition) {}

  ~Scroll() {}

  bool activateDefault(Actor* const actor, Engine* const engine) {
    (void)actor;
    return read(engine);
  }
  string getDefaultActivationLabel() const {return "Read";}

  SDL_Color getInterfaceClr() const {return clrMagenta;}

  bool read(Engine* const engine);

  const string getRealTypeName();

  void identify(const bool IS_SILENT_IDENTIFY, Engine* const engine);

  Spell* getSpell(Engine* const engine);
protected:
  void tryLearn(Engine* const engine);

  void failedToLearnRealName(Engine* const engine,
                             const string overrideFailString = "");
};

class ScrollNameHandler {
public:
  ScrollNameHandler(Engine* engine);
  ~ScrollNameHandler() {m_falseNames.resize(0);}

  void setFalseScrollName(ItemDef* d);

  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(vector<string>& lines);

private:
  vector<string> m_falseNames;

  Engine* eng;
};

#endif
