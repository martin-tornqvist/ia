#ifndef ITEM_DEVICE_H
#define ITEM_DEVICE_H

#include "Item.h"

class Device: public Item {
public:
  Device(ItemDefinition* const itemDefinition) :
    Item(itemDefinition), isActivated_(false), nrTurnsToNextEffect_(-1) {
  }

  virtual ~Device() {
  }

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

  int getRandomNrTurnsToNextEffect(Engine* const engine) const;

  void runEffect(Engine* const engine);

  virtual void specificEffect(Engine* const engine) = 0;

  virtual void specificToggle(Engine* const engine) {
    (void)engine;
  }

  bool isActivated_;
  int nrTurnsToNextEffect_;
};

class DeviceSentry: public Device {
public:
  DeviceSentry(ItemDefinition* const itemDefinition) :
    Device(itemDefinition) {
  }

  ~DeviceSentry() {
  }

private:
  void specificEffect(Engine* const engine);
};

class DeviceElectricLantern: public Device {
public:
  DeviceElectricLantern(ItemDefinition* const itemDefinition) :
    Device(itemDefinition) {
  }

  ~DeviceElectricLantern() {
  }

  bool isGivingLight() const;

private:
  void specificEffect(Engine* const engine);

  void specificToggle(Engine* const engine);
};

#endif
