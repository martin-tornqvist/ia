#ifndef ITEM_POTION_H
#define ITEM_POTION_H

#include "Item.h"
#include "ConstTypes.h"

class Engine;

class Potion: public Item {
public:
  Potion(ItemDefinition* const itemDefinition) :
    Item(itemDefinition) {
  }

  virtual ~Potion() {
  }

  virtual bool activateDefault(Actor* const actor, Engine* const engine) {
    quaff(actor, engine);
    return true;
  }
  virtual string getDefaultActivationLabel() const {
    return "Drink";
  }

  sf::Color getInterfaceClr() const {
    return clrBlueLight;
  }


  void quaff(Actor* const actor, Engine* const engine);

  void collide(const coord& pos, Actor* actor, const ItemDefinition& itemDef, Engine* const engine);

  void setRealDefinitionNames(Engine* const engine, const bool IS_SILENT_IDENTIFY);

protected:
  virtual void specificCollide(const coord& pos, Actor* const actor, Engine* const engine) = 0;

  virtual void specificQuaff(Actor* const actor, Engine* const engine) {
    (void) actor;
    (void) engine;
  }

  void failedToLearnRealName(Engine* const engine, const string overrideFailString = "");

  virtual const string getRealTypeName() {
    return "[ERROR]";
  }
};

class PotionOfHealing: public Potion {
public:
  PotionOfHealing(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfHealing() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine);
  const string getRealTypeName() {
    return "Healing";
  }
};

class PotionOfSorcery: public Potion {
public:
  PotionOfSorcery(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfSorcery() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
    (void)pos;
    (void)actor;
    (void)engine;
  }
  const string getRealTypeName() {
    return "Sorcery";
  }
};

class PotionOfBlindness: public Potion {
public:
  PotionOfBlindness(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfBlindness() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine);
  const string getRealTypeName() {
    return "Blindness";
  }
};

class PotionOfParalyzation: public Potion {
public:
  PotionOfParalyzation(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfParalyzation() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine);
  const string getRealTypeName() {
    return "Paralyzation";
  }
};

class PotionOfDisease: public Potion {
public:
  PotionOfDisease(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfDisease() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
    (void)pos;
    (void)actor;
    (void)engine;
  }
  const string getRealTypeName() {
    return "Disease";
  }
};

class PotionOfConfusion: public Potion {
public:
  PotionOfConfusion(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfConfusion() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine);
  const string getRealTypeName() {
    return "Confusion";
  }
};

class PotionOfCorruption: public Potion {
public:
  PotionOfCorruption(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfCorruption() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine);
  const string getRealTypeName() {
    return "Corruption";
  }
};

class PotionOfTheCobra: public Potion {
public:
  PotionOfTheCobra(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfTheCobra() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine);
  const string getRealTypeName() {
    return "the Cobra";
  }
};

class PotionOfStealth: public Potion {
public:
  PotionOfStealth(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfStealth() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine) {
    (void)pos;
    (void)actor;
    (void)engine;
  }
  const string getRealTypeName() {
    return "Ghostly Stealth";
  }
};

class PotionOfFortitude: public Potion {
public:
  PotionOfFortitude(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfFortitude() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine);
  const string getRealTypeName() {
    return "Fortitude";
  }
};

class PotionOfToughness: public Potion {
public:
  PotionOfToughness(ItemDefinition* const itemDefinition) :
    Potion(itemDefinition) {
  }
  ~PotionOfToughness() {
  }
  void specificQuaff(Actor* const actor, Engine* const engine);
private:
  void specificCollide(const coord& pos, Actor* const actor, Engine* const engine);
  const string getRealTypeName() {
    return "Toughness";
  }
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
    addFalse("Green", clrGreenLight);
    addFalse("Fiery", clrRedLight);
    addFalse("Murky", clrBrownDark);
    addFalse("Muddy", clrBrown);
    addFalse("Pink", clrMagentaLight);
    addFalse("Watery", clrBlueLight);
    addFalse("Metallic", clrGray);
    addFalse("Clear", clrWhiteHigh);
    addFalse("Misty", clrWhiteHigh);
    addFalse("Bloody", clrRed);
    addFalse("Magenta", clrMagenta);
    addFalse("Clotted", clrGreen);
    addFalse("Moldy", clrBrown);
    addFalse("Frothy", clrWhite);
  }

  ~PotionNameHandler() {
    m_falseNames.resize(0);
  }

  void setColorAndFalseName(ItemDefinition* d);

  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(vector<string>& lines);

private:
  vector<StringAndColor> m_falseNames;

  void addFalse(const string& str, const sf::Color clr) {
    m_falseNames.push_back(StringAndColor(str, clr));
  }

  Engine* eng;
};

#endif
