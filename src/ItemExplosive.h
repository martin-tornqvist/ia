#ifndef ITEM_EXPLOSIVE_H
#define ITEM_EXPLOSIVE_H

#include "StatusEffects.h"
#include "Item.h"

class Engine;

class Explosive : public Item {
public:
  Explosive(ItemDef* const itemDefinition) : Item(itemDefinition) {}
  virtual ~Explosive();

  virtual bool activateDefault(Actor* const actor, Engine* const engine) {
    (void)actor;
    setPlayerExplosive(engine);
    return true;
  }
  virtual string getDefaultActivationLabel() const {
    return "Ignite";
  }

  virtual SDL_Color getInterfaceClr() const {return clrRedLgt;}

  virtual void setPlayerExplosive(Engine* const engine) const;

protected:
};

class Dynamite: public Explosive {
public:
  Dynamite(ItemDef* const itemDefinition) : Explosive(itemDefinition) {
  }
  ~Dynamite() {
  }

  void setPlayerExplosive(Engine* const engine) const;

private:
};

class Molotov: public Explosive {
public:
  Molotov(ItemDef* const itemDefinition) : Explosive(itemDefinition) {
  }
  ~Molotov() {
  }

  void setPlayerExplosive(Engine* const engine) const;

private:
};

class Flare: public Explosive {
public:
  Flare(ItemDef* const itemDefinition) : Explosive(itemDefinition) {
  }
  ~Flare() {
  }

  void setPlayerExplosive(Engine* const engine) const;

private:
};

#endif
