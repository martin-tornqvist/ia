#ifndef ITEM_DEVICE_H
#define ITEM_DEVICE_H

#include "Item.h"

class Device: public Item {
public:
  Device(ItemData* const itemData, Engine& engine);

  virtual ~Device() {}

  bool activateDefault(Actor* const actor);

  virtual string getDefaultActivationLabel() const {
    return isActivated_ ? "Deactivate" : "Activate";
  }

  virtual SDL_Color getInterfaceClr() const {return clrCyan;}

  void newTurnInInventory();

  void itemSpecificAddSaveLines(vector<string>& lines);
  void itemSpecificSetParametersFromSaveLines(vector<string>& lines);

  void identify(const bool IS_SILENT_IDENTIFY);

protected:
  virtual void deviceSpecificAddSaveLines(vector<string>& lines) {(void)lines;}
  virtual void deviceSpecificSetParametersFromSaveLines(vector<string>& lines) {
    (void)lines;
  }

  bool toggle();

  virtual int getRandomNrTurnsToNextGoodEffect() const;
  virtual int getRandomNrTurnsToNextBadEffect() const;

  virtual string getSpecificActivateMessage() {return "";}

  virtual void runGoodEffect() {}
  virtual void runBadEffect();

  virtual void specificToggle() {}

  virtual void specificnewTurnInInventory() {}

  virtual void printToggleMessage();

  bool isActivated_;
  int nrTurnsToNextGoodEffect_;
  int nrTurnsToNextBadEffect_;
};

class DeviceSentry: public Device {
public:
  DeviceSentry(ItemData* const itemData, Engine& engine) :
    Device(itemData, engine) {}

  ~DeviceSentry() {}

private:
  void runGoodEffect();
  string getSpecificActivateMessage();
};

class DeviceRepeller: public Device {
public:
  DeviceRepeller(ItemData* const itemData, Engine& engine) :
    Device(itemData, engine) {}

  ~DeviceRepeller() {}

private:
  void runGoodEffect();
  int getRandomNrTurnsToNextGoodEffect() const;
  string getSpecificActivateMessage();
};

class DeviceRejuvenator: public Device {
public:
  DeviceRejuvenator(ItemData* const itemData, Engine& engine) :
    Device(itemData, engine) {}

  ~DeviceRejuvenator() {}

private:
  void runGoodEffect();
  string getSpecificActivateMessage();
};

class DeviceTranslocator: public Device {
public:
  DeviceTranslocator(ItemData* const itemData, Engine& engine) :
    Device(itemData, engine) {}

  ~DeviceTranslocator() {}

private:
  void runGoodEffect();
  string getSpecificActivateMessage();
};

class DeviceElectricLantern: public Device {
public:
  DeviceElectricLantern(ItemData* const itemData, Engine& engine) :
    Device(itemData, engine), malfunctCooldown_(-1) {}

  ~DeviceElectricLantern() {}

  bool isGivingLight() const;

private:
  void printToggleMessage();

  void specificnewTurnInInventory();

  void runBadEffect();

  void specificToggle();

  int malfunctCooldown_;
};


#endif
