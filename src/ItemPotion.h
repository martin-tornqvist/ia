#ifndef ITEM_POTION_H
#define ITEM_POTION_H

#include "Item.h"
#include "CommonData.h"

class Engine;

class Potion: public Item {
public:
  Potion(ItemData* const itemData, Engine* engine) :
    Item(itemData, engine) {}

  virtual ~Potion() {}

  virtual bool activateDefault(Actor* const actor, Engine* const engine) {
    quaff(actor, engine);
    return true;
  }
  virtual string getDefaultActivationLabel() const {return "Drink";}

  SDL_Color getInterfaceClr() const {return clrBlueLgt;}

  void quaff(Actor* const actor, Engine* const engine);

  void collide(const Pos& pos, Actor* actor, Engine* const engine);

  void identify(const bool IS_SILENT_IDENTIFY, Engine* const engine);

protected:
  virtual void specificCollide(const Pos& pos, Actor* const actor,
                               Engine* const engine) = 0;

  virtual void specificQuaff(Actor* const actor, Engine* const engine) {
    (void) actor;
    (void) engine;
  }

  void failedToLearnRealName(Engine* const engine,
                             const string overrideFailString = "");

  virtual const string getRealTypeName() {return "[ERROR]";}
};

class PotionOfHealing: public Potion {
public:
  PotionOfHealing(ItemData* const itemData, Engine* engine) :
    Potion(itemData, engine) {}
  ~PotionOfHealing() {}
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const Pos& pos, Actor* const actor,
                       Engine* const engine);
  const string getRealTypeName() {return "Healing";}
};

class PotionOfSpirit: public Potion {
public:
  PotionOfSpirit(ItemData* const itemData, Engine* engine) :
    Potion(itemData, engine) {}
  ~PotionOfSpirit() {}
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const Pos& pos, Actor* const actor,
                       Engine* const engine);
  const string getRealTypeName() {return "Spirit";}
};

//class PotionOfSorcery: public Potion {
//public:
//  PotionOfSorcery(ItemData* const itemData, Engine* engine) :
//    Potion(itemData, engine) {}
//  ~PotionOfSorcery() {}
//  void specificQuaff(Actor* const actor, Engine* const engine);
//private:
//  void specificCollide(const Pos& pos, Actor* const actor,
//                       Engine* const engine) {
//    (void)pos;
//    (void)actor;
//    (void)engine;
//  }
//  const string getRealTypeName() {return "Sorcery";}
//};

class PotionOfBlindness: public Potion {
public:
  PotionOfBlindness(ItemData* const itemData, Engine* engine) :
    Potion(itemData, engine) {}
  ~PotionOfBlindness() {}
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const Pos& pos, Actor* const actor,
                       Engine* const engine);
  const string getRealTypeName() {return "Blindness";}
};

class PotionOfParalyzation: public Potion {
public:
  PotionOfParalyzation(ItemData* const itemData, Engine* engine) :
    Potion(itemData, engine) {}
  ~PotionOfParalyzation() {}
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const Pos& pos, Actor* const actor,
                       Engine* const engine);
  const string getRealTypeName() {return "Paralyzation";}
};

class PotionOfDisease: public Potion {
public:
  PotionOfDisease(ItemData* const itemData, Engine* engine) :
    Potion(itemData, engine) {}
  ~PotionOfDisease() {}
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const Pos& pos, Actor* const actor,
                       Engine* const engine) {
    (void)pos;
    (void)actor;
    (void)engine;
  }
  const string getRealTypeName() {return "Disease";}
};

class PotionOfConfusion: public Potion {
public:
  PotionOfConfusion(ItemData* const itemData, Engine* engine) :
    Potion(itemData, engine) {}
  ~PotionOfConfusion() {}
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const Pos& pos, Actor* const actor,
                       Engine* const engine);
  const string getRealTypeName() {return "Confusion";}
};

//class PotionOfCorruption: public Potion {
//public:
//  PotionOfCorruption(ItemData* const itemData, Engine* engine) : Potion(itemData, engine) {}
//  ~PotionOfCorruption() {}
//  void specificQuaff(Actor* const actor, Engine* const engine);
//private:
//  void specificCollide(const Pos& pos, Actor* const actor, Engine* const engine);
//  const string getRealTypeName() {return "Corruption";}
//};

//class PotionOfTheCobra: public Potion {
//public:
//  PotionOfTheCobra(ItemData* const itemData, Engine* engine) :
//    Potion(itemData, engine) {}
//  ~PotionOfTheCobra() {}
//  void specificQuaff(Actor* const actor, Engine* const engine);
//private:
//  void specificCollide(const Pos& pos, Actor* const actor,
//                       Engine* const engine);
//  const string getRealTypeName() {return "the Cobra";}
//};

//class PotionOfStealth: public Potion {
//public:
//  PotionOfStealth(ItemData* const itemData, Engine* engine) :
//    Potion(itemData, engine) {
//  }
//  ~PotionOfStealth() {
//  }
//  void specificQuaff(Actor* const actor, Engine* const engine);
//private:
//  void specificCollide(const Pos& pos, Actor* const actor, Engine* const engine) {
//    (void)pos;
//    (void)actor;
//    (void)engine;
//  }
//  const string getRealTypeName() {
//    return "Ghostly Stealth";
//  }
//};

//class PotionOfFortitude: public Potion {
//public:
//  PotionOfFortitude(ItemData* const itemData, Engine* engine) :
//    Potion(itemData, engine) {}
//  ~PotionOfFortitude() {}
//  void specificQuaff(Actor* const actor, Engine* const engine);
//private:
//  void specificCollide(const Pos& pos, Actor* const actor,
//                       Engine* const engine);
//  const string getRealTypeName() {
//    return "Fortitude";
//  }
//};

//class PotionOfToughness: public Potion {
//public:
//  PotionOfToughness(ItemData* const itemData, Engine* engine) :
//    Potion(itemData, engine) {}
//  ~PotionOfToughness() {}
//  void specificQuaff(Actor* const actor, Engine* const engine);
//private:
//  void specificCollide(const Pos& pos, Actor* const actor,
//                       Engine* const engine);
//  const string getRealTypeName() {return "Toughness";}
//};

class PotionOfPoison: public Potion {
public:
  PotionOfPoison(ItemData* const itemData, Engine* engine) :
    Potion(itemData, engine) {}
  ~PotionOfPoison() {}
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const Pos& pos, Actor* const actor,
                       Engine* const engine);
  const string getRealTypeName() {return "Poison";}
};

class PotionOfKnowledge: public Potion {
public:
  PotionOfKnowledge(ItemData* const itemData, Engine* engine) :
    Potion(itemData, engine) {}
  ~PotionOfKnowledge() {}
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const Pos& pos, Actor* const actor,
                       Engine* const engine) {
    (void)pos;
    (void)actor;
    (void)engine;
  }
  const string getRealTypeName() {return "Arcane Knowledge";}
};

class PotionNameHandler {
public:
  PotionNameHandler(Engine* engine) :
    eng(engine) {
    m_falseNames.resize(0);

    addFalse("Golden", clrYellow);
    addFalse("Yellow", clrYellow);
    addFalse("Dark", clrGray);
    addFalse("Black", clrGray);
    addFalse("Oily", clrGray);
    addFalse("Smoky", clrWhite);
    addFalse("Slimy green", clrGreen);
    addFalse("Green", clrGreenLgt);
    addFalse("Fiery", clrRedLgt);
    addFalse("Murky", clrBrownDark);
    addFalse("Muddy", clrBrown);
    addFalse("Pink", clrMagentaLgt);
    addFalse("Watery", clrBlueLgt);
    addFalse("Metallic", clrGray);
    addFalse("Clear", clrWhiteHigh);
    addFalse("Misty", clrWhiteHigh);
    addFalse("Bloody", clrRed);
    addFalse("Magenta", clrMagenta);
    addFalse("Clotted", clrGreen);
    addFalse("Moldy", clrBrown);
    addFalse("Frothy", clrWhite);
  }

  ~PotionNameHandler() {m_falseNames.resize(0);}

  void setColorAndFalseName(ItemData* d);

  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(vector<string>& lines);

private:
  vector<StringAndClr> m_falseNames;

  void addFalse(const string& str, const SDL_Color clr) {
    m_falseNames.push_back(StringAndClr(str, clr));
  }

  Engine* eng;
};

#endif
