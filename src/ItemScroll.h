#ifndef ITEM_SCROLL_H
#define ITEM_SCROLL_H

#include "Item.h"
#include "StatusEffects.h"

class Engine;
class Actor;

class Scroll: public Item {
public:
  Scroll(ItemDefinition* const itemDefinition) :
    Item(itemDefinition) {
  }

  virtual ~Scroll();

  int getChanceToCastFromMemory(Engine* const engine) const;

  bool activateDefault(Actor* const actor, Engine* const engine) {
    (void)actor;
    return read(false, engine);
  }
  string getDefaultActivationLabel() const {
    return "Read";
  }

  virtual sf::Color getInterfaceClr() const {
    return clrMagenta;
  }

  bool read(const bool IS_FROM_MEMORY, Engine* const engine);

  virtual const string getRealTypeName() {
    return "[ERROR]";
  }

  void setRealDefinitionNames(Engine* const engine, const bool IS_SILENT_IDENTIFY);

  void setCastFromMemoryChance(const int VAL);

protected:
  virtual void specificRead(const bool FROM_MEMORY, Engine* const engine) {
    (void)engine;
    (void)FROM_MEMORY;
  }

  bool attemptReadFromMemory(Engine* const engine);
  bool attemptReadFromScroll(Engine* const engine);

  void attemptMemorizeIfLearnable(Engine* const engine);

  void failedToLearnRealName(Engine* const engine, const string overrideFailString = "");
};

//class scrollOfSummoning: public Scroll {
//public:
//	scrollOfSummoning(ItemDefinition* const itemDefinition) :
//		Scroll(itemDefinition) {
//	}
//	~scrollOfSummoning() {
//	}
//	void specificRead(const bool FROM_MEMORY, Engine* const engine);
//
//	const string getRealTypeName() {
//		return "Summoning";
//	}
//private:
//};
//
//class scrollOfGreaterSummoning: public Scroll {
//public:
//	scrollOfGreaterSummoning(ItemDefinition* const itemDefinition) :
//		Scroll(itemDefinition) {
//	}
//	~scrollOfGreaterSummoning() {
//	}
//	void specificRead(const bool FROM_MEMORY, Engine* const engine);
//
//	const string getRealTypeName() {
//		return "Greater Summoning";
//	}
//private:
//};

class ScrollOfMayhem: public Scroll {
public:
  ScrollOfMayhem(ItemDefinition* const itemDefinition) :
    Scroll(itemDefinition) {
  }
  ~ScrollOfMayhem() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);

  const string getRealTypeName() {
    return "Mayhem";
  }
private:
};

class ScrollOfPestilence: public Scroll {
public:
  ScrollOfPestilence(ItemDefinition* const itemDefinition) :
    Scroll(itemDefinition) {
  }
  ~ScrollOfPestilence() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);

  const string getRealTypeName() {
    return "Pestilence";
  }
private:
};

class ScrollOfTeleportation: public Scroll {
public:
  ScrollOfTeleportation(ItemDefinition* const itemDefinition) :
    Scroll(itemDefinition) {
  }
  ~ScrollOfTeleportation() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);

  const string getRealTypeName() {
    return "Teleportation";
  }
private:
};

class ScrollOfDescent: public Scroll {
public:
  ScrollOfDescent(ItemDefinition* const itemDefinition) :
    Scroll(itemDefinition) {
  }
  ~ScrollOfDescent() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);

  const string getRealTypeName() {
    return "Descent";
  }
private:
};

class ScrollOfStatusOnAllVisibleMonsters: public Scroll {
public:
  ScrollOfStatusOnAllVisibleMonsters(ItemDefinition* const ItemDefinition) :
    Scroll(ItemDefinition) {
  }
  ~ScrollOfStatusOnAllVisibleMonsters() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);
protected:
  virtual StatusEffect* getStatusEffect(Engine* const engine) = 0;
};

class ScrollOfConfuseEnemies: public ScrollOfStatusOnAllVisibleMonsters {
public:
  ScrollOfConfuseEnemies(ItemDefinition* const ItemDefinition) :
    ScrollOfStatusOnAllVisibleMonsters(ItemDefinition) {
  }
  ~ScrollOfConfuseEnemies() {
  }

  const string getRealTypeName() {
    return "Confuse Enemies";
  }
private:
  StatusEffect* getStatusEffect(Engine* const engine);
};

//class ScrollOfBlindEnemies: public ScrollOfStatusOnAllVisibleMonsters {
//public:
//	ScrollOfBlindEnemies(ItemDefinition* const ItemDefinition) :
//		ScrollOfStatusOnAllVisibleMonsters(ItemDefinition) {
//	}
//	~ScrollOfBlindEnemies() {
//	}
//
//	const string getRealTypeName() {
//		return "Blind Enemies";
//	}
//private:
//	StatusEffect* getStatusEffect(Engine* const engine);
//};

class ScrollOfParalyzeEnemies: public ScrollOfStatusOnAllVisibleMonsters {
public:
  ScrollOfParalyzeEnemies(ItemDefinition* const ItemDefinition) :
    ScrollOfStatusOnAllVisibleMonsters(ItemDefinition) {
  }
  ~ScrollOfParalyzeEnemies() {
  }

  const string getRealTypeName() {
    return "Paralyze Enemies";
  }
private:
  StatusEffect* getStatusEffect(Engine* const engine);
};

class ScrollOfSlowEnemies: public ScrollOfStatusOnAllVisibleMonsters {
public:
  ScrollOfSlowEnemies(ItemDefinition* const ItemDefinition) :
    ScrollOfStatusOnAllVisibleMonsters(ItemDefinition) {
  }
  ~ScrollOfSlowEnemies() {
  }

  const string getRealTypeName() {
    return "Slow Enemies";
  }
private:
  StatusEffect* getStatusEffect(Engine* const engine);
};

class ScrollOfBlessing: public Scroll {
public:
  ScrollOfBlessing(ItemDefinition* const itemDefinition) :
    Scroll(itemDefinition) {
  }
  ~ScrollOfBlessing() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);

  const string getRealTypeName() {
    return "Blessing";
  }
private:
};

class ScrollOfDetectItems: public Scroll {
public:
  ScrollOfDetectItems(ItemDefinition* const itemDefinition) :
    Scroll(itemDefinition) {
  }
  ~ScrollOfDetectItems() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);

  const string getRealTypeName() {
    return "Detect Items";
  }
private:
};

class ScrollOfDetectTraps: public Scroll {
public:
  ScrollOfDetectTraps(ItemDefinition* const itemDefinition) :
    Scroll(itemDefinition) {
  }
  ~ScrollOfDetectTraps() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);

  const string getRealTypeName() {
    return "Detect Traps";
  }
private:
};

class ScrollOfClairvoyance: public Scroll {
public:
  ScrollOfClairvoyance(ItemDefinition* const itemDefinition) :
    Scroll(itemDefinition) {
  }
  ~ScrollOfClairvoyance() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);

  const string getRealTypeName() {
    return "Clairvoyance";
  }
private:
};

class ScrollOfAzathothsBlast: public Scroll {
public:
  ScrollOfAzathothsBlast(ItemDefinition* const itemDefinition) :
    Scroll(itemDefinition) {
  }
  ~ScrollOfAzathothsBlast() {
  }
  void specificRead(const bool FROM_MEMORY, Engine* const engine);

  static void castAt(const coord& pos, Engine* const engine);

  const string getRealTypeName() {
    return "Azathoths Blast";
  }
private:
};

//class ScrollOfVoidChain: public Scroll {
//public:
//  ScrollOfVoidChain(ItemDefinition* const itemDefinition) :
//    Scroll(itemDefinition) {
//  }
//  ~ScrollOfVoidChain() {
//  }
//  void specificRead(const bool FROM_MEMORY, Engine* const engine);
//
//  static void castAt(const coord& pos, Engine* const engine);
//
//  const string getRealTypeName() {
//    return "Void Chain";
//  }
//private:
//};

//class ScrollOfIbnGhazisPowder: public Scroll {
//public:
//  ScrollOfIbnGhazisPowder(ItemDefinition* const itemDefinition) :
//    Scroll(itemDefinition) {
//  }
//  ~ScrollOfIbnGhazisPowder() {
//  }
//  void specificRead(const bool FROM_MEMORY, Engine* const engine);
//
//  const string getRealTypeName() {
//    return "Ibn Ghazis Powder";
//  }
//private:
//};

class ScrollNameHandler {
public:
  ScrollNameHandler(Engine* engine) :
    eng(engine) {
    m_falseNames.resize(0);
    m_falseNames.push_back("Cruensseasrjit");
    m_falseNames.push_back("Rudsceleratus");
    m_falseNames.push_back("Rudminuox");
    m_falseNames.push_back("Cruo-stragaraNa");
    m_falseNames.push_back("PrayaNavita");
    m_falseNames.push_back("Pretiacruento");
    m_falseNames.push_back("Pestis cruento");
    m_falseNames.push_back("Cruento pestis");
    m_falseNames.push_back("Domus-bhaava");
    m_falseNames.push_back("Acerbus-shatruex");
    m_falseNames.push_back("Pretaanluxis");
    m_falseNames.push_back("PraaNsilenux");
    m_falseNames.push_back("Quodpipax");
    m_falseNames.push_back("Lokemundux");
    m_falseNames.push_back("Profanuxes");
    m_falseNames.push_back("Shaantitus");
    m_falseNames.push_back("Geropayati");
    m_falseNames.push_back("Vilomaxus");
    m_falseNames.push_back("Bhuudesco");
    m_falseNames.push_back("Durbentia");
    m_falseNames.push_back("Bhuuesco");
    m_falseNames.push_back("Maravita");
    m_falseNames.push_back("Infirmux");

    vector<string> cmb;
    cmb.resize(0);
    cmb.push_back("Cruo");
    cmb.push_back("Cruonit");
    cmb.push_back("Cruentu");
    cmb.push_back("Marana");
    cmb.push_back("Domus");
    cmb.push_back("Malax");
    cmb.push_back("Caecux");
    cmb.push_back("Eximha");
    cmb.push_back("Vorox");
    cmb.push_back("Bibox");
    cmb.push_back("Pallex");
    cmb.push_back("Profanx");
    cmb.push_back("Invisuu");
    cmb.push_back("Invisux");
    cmb.push_back("Odiosuu");
    cmb.push_back("Odiosux");
    cmb.push_back("Vigra");
    cmb.push_back("Crudux");
    cmb.push_back("Desco");
    cmb.push_back("Esco");
    cmb.push_back("Gero");
    cmb.push_back("Klaatu");
    cmb.push_back("Barada");
    cmb.push_back("Nikto");

    const unsigned int CMB_SIZ = cmb.size();
    for(unsigned int i = 0; i < CMB_SIZ; i++) {
      for(unsigned int ii = 0; ii < CMB_SIZ; ii++) {
        if(i != ii) {
          m_falseNames.push_back(cmb.at(i) + " " + cmb.at(ii));
        }
      }
    }

  }
  ~ScrollNameHandler() {
    m_falseNames.resize(0);
  }

  void setFalseScrollName(ItemDefinition* d);

  void addSaveLines(vector<string>& lines) const;
  void setParametersFromSaveLines(vector<string>& lines);

private:
  vector<string> m_falseNames;

  Engine* eng;
};

#endif
