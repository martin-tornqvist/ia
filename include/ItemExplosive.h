#ifndef ITEM_EXPLOSIVE_H
#define ITEM_EXPLOSIVE_H

#include "Properties.h"
#include "Item.h"

class Explosive : public Item {
public:
  Explosive(ItemData* const itemData) :
    Item(itemData) {}
  virtual ~Explosive() {}

  virtual ConsumeItem activateDefault(Actor* const actor) override {
    (void)actor;
    setPlayerExplosive();
    return ConsumeItem::yes;
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
  Dynamite(ItemData* const itemData) :
    Explosive(itemData) {}
  ~Dynamite() {}
  void setPlayerExplosive() const;
private:
};

class Molotov: public Explosive {
public:
  Molotov(ItemData* const itemData) :
    Explosive(itemData) {}
  ~Molotov() {}
  void setPlayerExplosive() const;
private:
};

class Flare: public Explosive {
public:
  Flare(ItemData* const itemData) :
    Explosive(itemData) {}
  ~Flare() {}
  void setPlayerExplosive() const;
private:
};

#endif
