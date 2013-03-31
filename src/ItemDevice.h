#ifndef ITEM_DEVICE_H
#define ITEM_DEVICE_H

#include "Item.h"

class Device: public Item {
public:
  Device(ItemDefinition* const itemDefinition) :
    Item(itemDefinition), isActivated_(false), nrTurnsToNextGoodEffect_(-1), nrTurnsToNextBadEffect_(-1) {
  }

  virtual ~Device() {}

  bool activateDefault(Actor* const actor, Engine* const engine) {
    (void)actor;
    return toggle(engine);
  }
  virtual string getDefaultActivationLabel() const {
    return isActivated_ ? "Deactivate" : "Activate";
  }

  virtual sf::Color getInterfaceClr() const {
    return clrCyan;
  }

  void newTurn(Engine* const engine);

protected:
  bool toggle(Engine* const engine);

  virtual int getRandomNrTurnsToNextGoodEffect(Engine* const engine) const;
  virtual int getRandomNrTurnsToNextBadEffect(Engine* const engine) const;

  virtual void runGoodEffect(Engine* const engine) = 0;

  virtual void runBadEffect(Engine* const engine);

  virtual void specificToggle(Engine* const engine) {
    (void)engine;
  }

  virtual void specificNewTurn(Engine* const engine) {
    (void)engine;
  }

  bool isActivated_;
  int nrTurnsToNextGoodEffect_;
  int nrTurnsToNextBadEffect_;
};

class DeviceSentry: public Device {
public:
  DeviceSentry(ItemDefinition* const itemDefinition) :
    Device(itemDefinition) {}

  ~DeviceSentry() {}

private:
  void runGoodEffect(Engine* const engine);
};

class DeviceRepeller: public Device {
public:
  DeviceRepeller(ItemDefinition* const itemDefinition) :
    Device(itemDefinition) {}

  ~DeviceRepeller() {}

private:
  void runGoodEffect(Engine* const engine);
  int getRandomNrTurnsToNextGoodEffect(Engine* const engine) const;
};

class DeviceRejuvenator: public Device {
public:
  DeviceRejuvenator(ItemDefinition* const itemDefinition) :
    Device(itemDefinition) {}

  ~DeviceRejuvenator() {}

private:
  void runGoodEffect(Engine* const engine);
};

class DeviceTranslocator: public Device {
public:
  DeviceTranslocator(ItemDefinition* const itemDefinition) :
    Device(itemDefinition) {}

  ~DeviceTranslocator() {}

private:
  void runGoodEffect(Engine* const engine);
};

//class DeviceSpellReflector: public Device {
//public:
//  DeviceSpellReflector(ItemDefinition* const itemDefinition) :
//    Device(itemDefinition) {}
//
//  ~DeviceSpellReflector() {}
//
//private:
//  void runGoodEffect(Engine* const engine);
//};

class DeviceElectricLantern: public Device {
public:
  DeviceElectricLantern(ItemDefinition* const itemDefinition) :
    Device(itemDefinition) {}

  ~DeviceElectricLantern() {}

  bool isGivingLight() const;

private:
  void specificNewTurn(Engine* const engine);

  void runGoodEffect(Engine* const engine);

  void runBadEffect(Engine* const engine);

  void specificToggle(Engine* const engine);
};


#endif
