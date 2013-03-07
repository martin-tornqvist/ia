#ifndef ITEM_DEVICE_H
#define ITEM_DEVICE_H

#include "Item.h"

class Device: public Item {
public:
  Device(ItemDefinition* const itemDefinition) :
    Item(itemDefinition), isActivated_(false) {
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

protected:
  bool toggle(Engine* const engine) {
    (void)engine;
    isActivated_ = !isActivated_;
    return false;
  }

  bool isActivated_;
};

class DeviceSentry: public Device {
public:
  DeviceSentry(ItemDefinition* const itemDefinition) :
    Device(itemDefinition) {
  }

  ~DeviceSentry() {
  }

private:
};

#endif
