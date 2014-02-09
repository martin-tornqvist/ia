#ifndef ITEM_POTION_H
#define ITEM_POTION_H

#include "Item.h"
#include "CommonData.h"

class Engine;

class Potion: public Item {
public:
  Potion(ItemData* const itemData, Engine& engine) :
    Item(itemData, engine) {}

  virtual ~Potion() {}

  virtual bool activateDefault(Actor* const actor) {
    quaff(actor);
    return true;
  }
  virtual string getDefaultActivationLabel() const {return "Drink";}

  SDL_Color getInterfaceClr() const {return clrBlueLgt;}

  void quaff(Actor* const actor);

  void collide(const Pos& pos, Actor* actor);

  void identify(const bool IS_SILENT_IDENTIFY) override;

protected:
  virtual void collide_(const Pos& pos, Actor* const actor) = 0;

  virtual void quaff_(Actor* const actor) {(void) actor;}

  virtual const string getRealTypeName() {return "[WARNING]";}
};

class PotionOfHealing: public Potion {
public:
  PotionOfHealing(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfHealing() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Healing";}
};

class PotionOfSpirit: public Potion {
public:
  PotionOfSpirit(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfSpirit() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Spirit";}
};

class PotionOfBlindness: public Potion {
public:
  PotionOfBlindness(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfBlindness() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Blindness";}
};

class PotionOfParalyzation: public Potion {
public:
  PotionOfParalyzation(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfParalyzation() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Paralyzation";}
};

class PotionOfDisease: public Potion {
public:
  PotionOfDisease(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfDisease() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const string getRealTypeName() {return "Disease";}
};

class PotionOfConfusion: public Potion {
public:
  PotionOfConfusion(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfConfusion() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Confusion";}
};

class PotionOfFrenzy: public Potion {
public:
  PotionOfFrenzy(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfFrenzy() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Frenzy";}
};

class PotionOfFortitude: public Potion {
public:
  PotionOfFortitude(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfFortitude() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {
    return "Fortitude";
  }
};

class PotionOfPoison: public Potion {
public:
  PotionOfPoison(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfPoison() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Poison";}
};

class PotionOfInsight: public Potion {
public:
  PotionOfInsight(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfInsight() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const string getRealTypeName() {return "Insight";}
};

class PotionOfClairvoyance: public Potion {
public:
  PotionOfClairvoyance(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfClairvoyance() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const string getRealTypeName() {return "Clairvoyance";}
};

class PotionOfRFire: public Potion {
public:
  PotionOfRFire(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfRFire() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Fire Resistance";}
};

class PotionOfAntidote: public Potion {
public:
  PotionOfAntidote(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfAntidote() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Antidote";}
};

class PotionOfRElec: public Potion {
public:
  PotionOfRElec(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfRElec() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Insulation";}
};

class PotionOfRAcid: public Potion {
public:
  PotionOfRAcid(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfRAcid() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Acid Resistance";}
};

class PotionOfDescent: public Potion {
public:
  PotionOfDescent(ItemData* const itemData, Engine& engine) :
    Potion(itemData, engine) {}
  ~PotionOfDescent() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Descent";}
};

struct PotionLook {
  string namePlain;
  string nameA;
  SDL_Color clr;
};

class PotionNameHandler {
public:
  PotionNameHandler(Engine& engine) :
    eng(engine) {
    potionLooks_.resize(0);

    potionLooks_.push_back(PotionLook {"Golden",   "a Golden",   clrYellow});
    potionLooks_.push_back(PotionLook {"Yellow",   "a Yellow",   clrYellow});
    potionLooks_.push_back(PotionLook {"Dark",     "a Dark",     clrGray});
    potionLooks_.push_back(PotionLook {"Black",    "a Black",    clrGray});
    potionLooks_.push_back(PotionLook {"Oily",     "an Oily",    clrGray});
    potionLooks_.push_back(PotionLook {"Smoky",    "a Smoky",    clrWhite});
    potionLooks_.push_back(PotionLook {"Slimy",    "a Slimy",    clrGreen});
    potionLooks_.push_back(PotionLook {"Green",    "a Green",    clrGreenLgt});
    potionLooks_.push_back(PotionLook {"Fiery",    "a Firey",    clrRedLgt});
    potionLooks_.push_back(PotionLook {"Murky",    "a Murky",    clrBrownDrk});
    potionLooks_.push_back(PotionLook {"Muddy",    "a Muddy",    clrBrown});
    potionLooks_.push_back(PotionLook {"Violet",   "a Violet",   clrViolet});
    potionLooks_.push_back(PotionLook {"Orange",   "an Orange",  clrOrange});
    potionLooks_.push_back(PotionLook {"Watery",   "a Watery",   clrBlueLgt});
    potionLooks_.push_back(PotionLook {"Metallic", "a Metallic", clrGray});
    potionLooks_.push_back(PotionLook {"Clear",    "a Clear",    clrWhiteHigh});
    potionLooks_.push_back(PotionLook {"Misty",    "a Misty",    clrWhiteHigh});
    potionLooks_.push_back(PotionLook {"Bloody",   "a Bloody",   clrRed});
    potionLooks_.push_back(PotionLook {"Magenta",  "a Magenta",  clrMagenta});
    potionLooks_.push_back(PotionLook {"Clotted",  "a Clotted",  clrGreen});
    potionLooks_.push_back(PotionLook {"Moldy",    "a Moldy",    clrBrown});
    potionLooks_.push_back(PotionLook {"Frothy",   "a Frothy",   clrWhite});
  }

  ~PotionNameHandler() {potionLooks_.resize(0);}

  void setClrAndFalseName(ItemData* d);

  void addSaveLines(vector<string>& lines) const;
  void setParamsFromSaveLines(vector<string>& lines);

private:
  vector<PotionLook> potionLooks_;

  Engine& eng;
};

#endif
