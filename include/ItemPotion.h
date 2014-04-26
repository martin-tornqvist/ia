#ifndef ITEM_POTION_H
#define ITEM_POTION_H

#include "Item.h"
#include "CmnData.h"

class Potion: public Item {
public:
  Potion(ItemData* const itemData) :
    Item(itemData) {}

  virtual ~Potion() {}

  ConsumeItem activateDefault(Actor* const actor) override {
    quaff(actor);
    return ConsumeItem::yes;
  }

  string getDefaultActivationLabel() const {return "Drink";}

  SDL_Color getInterfaceClr() const {return clrBlueLgt;}

  void quaff(Actor* const actor);

  void collide(const Pos& pos, Actor* actor);

  void identify(const bool IS_SILENT_IDENTIFY) override;

protected:
  virtual void collide_(const Pos& pos, Actor* const actor) = 0;

  virtual void quaff_(Actor* const actor) {(void) actor;}

  virtual const string getRealTypeName() {return "[WARNING]";}
};

class PotionOfVitality: public Potion {
public:
  PotionOfVitality(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfVitality() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Vitality";}
};

class PotionOfSpirit: public Potion {
public:
  PotionOfSpirit(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfSpirit() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Spirit";}
};

class PotionOfBlindness: public Potion {
public:
  PotionOfBlindness(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfBlindness() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Blindness";}
};

class PotionOfParal: public Potion {
public:
  PotionOfParal(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfParal() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Paralyzation";}
};

class PotionOfDisease: public Potion {
public:
  PotionOfDisease(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfDisease() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const string getRealTypeName() {return "Disease";}
};

class PotionOfConf: public Potion {
public:
  PotionOfConf(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfConf() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Confusion";}
};

class PotionOfFrenzy: public Potion {
public:
  PotionOfFrenzy(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfFrenzy() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Frenzy";}
};

class PotionOfFortitude: public Potion {
public:
  PotionOfFortitude(ItemData* const itemData) :
    Potion(itemData) {}
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
  PotionOfPoison(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfPoison() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Poison";}
};

class PotionOfInsight: public Potion {
public:
  PotionOfInsight(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfInsight() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const string getRealTypeName() {return "Insight";}
};

class PotionOfClairv: public Potion {
public:
  PotionOfClairv(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfClairv() {}
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
  PotionOfRFire(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfRFire() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Fire Resistance";}
};

class PotionOfAntidote: public Potion {
public:
  PotionOfAntidote(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfAntidote() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Antidote";}
};

class PotionOfRElec: public Potion {
public:
  PotionOfRElec(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfRElec() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Insulation";}
};

class PotionOfRAcid: public Potion {
public:
  PotionOfRAcid(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfRAcid() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const string getRealTypeName() {return "Acid Resistance";}
};

class PotionOfDescent: public Potion {
public:
  PotionOfDescent(ItemData* const itemData) :
    Potion(itemData) {}
  ~PotionOfDescent() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const string getRealTypeName() {return "Descent";}
};

struct PotionLook {
  string namePlain;
  string nameA;
  SDL_Color clr;
};

namespace PotionNameHandling {

void init();

void setClrAndFalseName(ItemData* d);

void storeToSaveLines(vector<string>& lines);
void setupFromSaveLines(vector<string>& lines);

} //PotionNameHandling

#endif
