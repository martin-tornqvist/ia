#ifndef ITEM_EXPLOSIVE_H
#define ITEM_EXPLOSIVE_H

#include "Properties.h"
#include "Item.h"

class Engine;

class Explosive : public Item {
public:
  Explosive(ItemData* const itemData, Engine* engine) :
    Item(itemData, engine) {}
  virtual ~Explosive() {}

  virtual bool activateDefault(Actor* const actor) {
    (void)actor;
    setPlayerExplosive();
    return true;
  }
  virtual string getDefaultActivationLabel() const {
    return "Ignite";
  }

  virtual SDL_Color getInterfaceClr() const {return clrRedLgt;}

  virtual void setPlayerExplosive() const {}

protected:
};

class Dynamite: public Explosive {
public:
  Dynamite(ItemData* const itemData, Engine* engine) :
    Explosive(itemData, engine) {}
  ~Dynamite() {}
  void setPlayerExplosive() const;
private:
};

class Molotov: public Explosive {
public:
  Molotov(ItemData* const itemData, Engine* engine) :
    Explosive(itemData, engine) {}
  ~Molotov() {}
  void setPlayerExplosive() const;
private:
};

class Flare: public Explosive {
public:
  Flare(ItemData* const itemData, Engine* engine) :
    Explosive(itemData, engine) {}
  ~Flare() {}
  void setPlayerExplosive() const;
private:
};

#endif
