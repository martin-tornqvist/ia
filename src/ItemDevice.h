#ifndef ITEM_DEVICE_H
#define ITEM_DEVICE_H

#include "Item.h"

class Device: public Item {
public:
  Device(ItemDefinition* const itemDefinition) :
    Item(itemDefinition), isActivated_(false), nrTurnsToNextGoodEffect_(-1),
    nrTurnsToNextBadEffect_(-1) {}

  virtual ~Device() {}

  bool activateDefault(Actor* const actor, Engine* const engine);

  virtual string getDefaultActivationLabel() const {
    return isActivated_ ? "Deactivate" : "Activate";
  }

  virtual SDL_Color getInterfaceClr() const {return clrCyan;}

  void newTurnInInventory(Engine* const engine);

  void itemSpecificAddSaveLines(vector<string>& lines);
  void itemSpecificSetParametersFromSaveLines(vector<string>& lines);

  void identify(const bool IS_SILENT_IDENTIFY);

protected:
  virtual void deviceSpecificAddSaveLines(vector<string>& lines) {(void)lines;}
  virtual void deviceSpecificSetParametersFromSaveLines(vector<string>& lines) {
    (void)lines;
  }

  bool toggle(Engine* const engine);

  virtual int getRandomNrTurnsToNextGoodEffect(Engine* const engine) const;
  virtual int getRandomNrTurnsToNextBadEffect(Engine* const engine) const;

  virtual string getSpecificActivateMessage() {return "";}

  virtual void runGoodEffect(Engine* const engine) {(void) engine;}
  virtual void runBadEffect(Engine* const engine);

  virtual void specificToggle(Engine* const engine) {(void)engine;}

  virtual void specificnewTurnInInventory(Engine* const engine) {(void)engine;}

  virtual void printToggleMessage(Engine* const engine);

  bool isActivated_;
  int nrTurnsToNextGoodEffect_;
  int nrTurnsToNextBadEffect_;
};

class DeviceSentry: public Device {
public:
  DeviceSentry(ItemDefinition* const itemDefinition) : Device(itemDefinition) {}

  ~DeviceSentry() {}

private:
  void runGoodEffect(Engine* const engine);
  string getSpecificActivateMessage();
};

class DeviceRepeller: public Device {
public:
  DeviceRepeller(ItemDefinition* const itemDefinition) : Device(itemDefinition) {}

  ~DeviceRepeller() {}

private:
  void runGoodEffect(Engine* const engine);
  int getRandomNrTurnsToNextGoodEffect(Engine* const engine) const;
  string getSpecificActivateMessage();
};

class DeviceRejuvenator: public Device {
public:
  DeviceRejuvenator(ItemDefinition* const itemDefinition) : Device(itemDefinition) {}

  ~DeviceRejuvenator() {}

private:
  void runGoodEffect(Engine* const engine);
  string getSpecificActivateMessage();
};

class DeviceTranslocator: public Device {
public:
  DeviceTranslocator(ItemDefinition* const itemDefinition) : Device(itemDefinition) {}

  ~DeviceTranslocator() {}

private:
  void runGoodEffect(Engine* const engine);
  string getSpecificActivateMessage();
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
    Device(itemDefinition), malfunctCooldown_(-1) {}

  ~DeviceElectricLantern() {}

  bool isGivingLight() const;

private:
  void printToggleMessage(Engine* const engine);

  void specificnewTurnInInventory(Engine* const engine);

  void runBadEffect(Engine* const engine);

  void specificToggle(Engine* const engine);

  int malfunctCooldown_;
};


#endif
