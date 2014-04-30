#ifndef ITEM_POTION_H
#define ITEM_POTION_H

#include "Item.h"
#include "CmnData.h"

class Potion: public Item {
public:
  Potion(ItemDataT* const itemData) :
    Item(itemData) {}

  virtual ~Potion() {}

  ConsumeItem activateDefault(Actor* const actor) override {
    quaff(actor);
    return ConsumeItem::yes;
  }

  std::string getDefaultActivationLabel() const {return "Drink";}

  SDL_Color getInterfaceClr() const {return clrBlueLgt;}

  void quaff(Actor* const actor);

  void collide(const Pos& pos, Actor* actor);

  void identify(const bool IS_SILENT_IDENTIFY) override;

protected:
  virtual void collide_(const Pos& pos, Actor* const actor) = 0;

  virtual void quaff_(Actor* const actor) {(void) actor;}

  virtual const std::string getRealTypeName() {return "[WARNING]";}
};

class PotionOfVitality: public Potion {
public:
  PotionOfVitality(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfVitality() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Vitality";}
};

class PotionOfSpirit: public Potion {
public:
  PotionOfSpirit(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfSpirit() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Spirit";}
};

class PotionOfBlindness: public Potion {
public:
  PotionOfBlindness(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfBlindness() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Blindness";}
};

class PotionOfParal: public Potion {
public:
  PotionOfParal(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfParal() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Paralyzation";}
};

class PotionOfDisease: public Potion {
public:
  PotionOfDisease(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfDisease() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const std::string getRealTypeName() {return "Disease";}
};

class PotionOfConf: public Potion {
public:
  PotionOfConf(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfConf() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Confusion";}
};

class PotionOfFrenzy: public Potion {
public:
  PotionOfFrenzy(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfFrenzy() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Frenzy";}
};

class PotionOfFortitude: public Potion {
public:
  PotionOfFortitude(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfFortitude() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {
    return "Fortitude";
  }
};

class PotionOfPoison: public Potion {
public:
  PotionOfPoison(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfPoison() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Poison";}
};

class PotionOfInsight: public Potion {
public:
  PotionOfInsight(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfInsight() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const std::string getRealTypeName() {return "Insight";}
};

class PotionOfClairv: public Potion {
public:
  PotionOfClairv(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfClairv() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const std::string getRealTypeName() {return "Clairvoyance";}
};

class PotionOfRFire: public Potion {
public:
  PotionOfRFire(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfRFire() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Fire Resistance";}
};

class PotionOfAntidote: public Potion {
public:
  PotionOfAntidote(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfAntidote() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Antidote";}
};

class PotionOfRElec: public Potion {
public:
  PotionOfRElec(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfRElec() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Insulation";}
};

class PotionOfRAcid: public Potion {
public:
  PotionOfRAcid(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfRAcid() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Acid Resistance";}
};

class PotionOfDescent: public Potion {
public:
  PotionOfDescent(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionOfDescent() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const std::string getRealTypeName() {return "Descent";}
};

struct PotionLook {
  std::string namePlain;
  std::string nameA;
  SDL_Color clr;
};

namespace PotionNameHandling {

void init();

void setClrAndFalseName(ItemDataT* d);

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

} //PotionNameHandling

#endif
