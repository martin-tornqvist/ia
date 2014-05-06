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

class PotionVitality: public Potion {
public:
  PotionVitality(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionVitality() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Vitality";}
};

class PotionSpirit: public Potion {
public:
  PotionSpirit(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionSpirit() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Spirit";}
};

class PotionBlindness: public Potion {
public:
  PotionBlindness(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionBlindness() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Blindness";}
};

class PotionParal: public Potion {
public:
  PotionParal(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionParal() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Paralyzation";}
};

class PotionDisease: public Potion {
public:
  PotionDisease(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionDisease() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const std::string getRealTypeName() {return "Disease";}
};

class PotionConf: public Potion {
public:
  PotionConf(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionConf() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Confusion";}
};

class PotionFrenzy: public Potion {
public:
  PotionFrenzy(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionFrenzy() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Frenzy";}
};

class PotionFortitude: public Potion {
public:
  PotionFortitude(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionFortitude() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {
    return "Fortitude";
  }
};

class PotionPoison: public Potion {
public:
  PotionPoison(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionPoison() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Poison";}
};

class PotionInsight: public Potion {
public:
  PotionInsight(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionInsight() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const std::string getRealTypeName() {return "Insight";}
};

class PotionClairv: public Potion {
public:
  PotionClairv(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionClairv() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor) {
    (void)pos;
    (void)actor;
  }
  const std::string getRealTypeName() {return "Clairvoyance";}
};

class PotionRFire: public Potion {
public:
  PotionRFire(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionRFire() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Fire Resistance";}
};

class PotionAntidote: public Potion {
public:
  PotionAntidote(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionAntidote() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Antidote";}
};

class PotionRElec: public Potion {
public:
  PotionRElec(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionRElec() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Insulation";}
};

class PotionRAcid: public Potion {
public:
  PotionRAcid(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionRAcid() {}
  void quaff_(Actor* const actor);
private:
  void collide_(const Pos& pos, Actor* const actor);
  const std::string getRealTypeName() {return "Acid Resistance";}
};

class PotionDescent: public Potion {
public:
  PotionDescent(ItemDataT* const itemData) :
    Potion(itemData) {}
  ~PotionDescent() {}
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

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

void setClrAndFalseName(ItemDataT* d);

} //PotionNameHandling

#endif
