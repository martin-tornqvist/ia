#ifndef ITEM_EXPLOSIVE_H
#define ITEM_EXPLOSIVE_H

#include <string>

#include "Item.h"

class Explosive : public Item {
public:
  virtual ~Explosive() {}

  Explosive() = delete;

  ConsumeItem activateDefault(Actor* const actor) override final;
  std::string getDefaultActivationLabel()   const override final {return "Ignite";}
  Clr getInterfaceClr()                     const override final {return clrRedLgt;}

  virtual void        onStdTurnPlayerHoldIgnited()          = 0;
  virtual void        onThrownIgnitedLanding(const Pos& p)  = 0;
  virtual void        onPlayerParalyzed()                   = 0;
  virtual Clr         getIgnitedProjectileClr() const       = 0;
  virtual std::string getStrOnPlayerThrow()     const       = 0;

protected:
  Explosive(ItemDataT* const itemData) : Item(itemData), fuseTurns_(-1) {}

  virtual int   getStdFuseTurns() const = 0;
  virtual void  onPlayerIgnite()  const = 0;

  int fuseTurns_;
};

class Dynamite: public Explosive {
public:
  Dynamite(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr         getIgnitedProjectileClr()       const override {return clrRedLgt;}
  std::string getStrOnPlayerThrow()           const override {
    return "I throw a lit dynamite stick.";
  }

protected:
  int getStdFuseTurns() const override {return 6;}
  void onPlayerIgnite() const override;
};

class Molotov: public Explosive {
public:
  Molotov(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr         getIgnitedProjectileClr()       const override {return clrYellow;}
  std::string getStrOnPlayerThrow()           const override {
    return "I throw a lit Molotov Cocktail.";
  }

protected:
  int getStdFuseTurns() const override {return 12;}
  void onPlayerIgnite() const override;
};

class Flare: public Explosive {
public:
  Flare(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr getIgnitedProjectileClr()               const override {return clrYellow;}
  std::string getStrOnPlayerThrow()           const override {
    return "I throw a lit flare.";
  }

protected:
  int getStdFuseTurns() const override {return 200;}
  void onPlayerIgnite() const override;
};

class SmokeGrenade: public Explosive {
public:
  SmokeGrenade(ItemDataT* const itemData) : Explosive(itemData) {}

  void        onThrownIgnitedLanding(const Pos& p)  override;
  void        onStdTurnPlayerHoldIgnited()          override;
  void        onPlayerParalyzed()                   override;
  Clr         getIgnitedProjectileClr()       const override {return getData().clr;}
  std::string getStrOnPlayerThrow()           const override {
    return "I throw a smoke grenade.";
  }

protected:
  int getStdFuseTurns() const override {return 12;}
  void onPlayerIgnite() const override;
};

#endif
