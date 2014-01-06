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
  void itemSpecificSetParamsFromSaveLines(vector<string>& lines);

  void identify(const bool IS_SILENT_IDENTIFY);

protected:
  virtual void deviceSpecificAddSaveLines(vector<string>& lines) {(void)lines;}
  virtual void deviceSpecificSetParamsFromSaveLines(vector<string>& lines) {
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

  ~DeviceSentry() override {}

private:
  void runGoodEffect() override;
  string getSpecificActivateMessage() override;
};

class DeviceRepeller: public Device {
public:
  DeviceRepeller(ItemData* const itemData, Engine& engine) :
    Device(itemData, engine) {}

  ~DeviceRepeller() override {}

private:
  void runGoodEffect() override;
  int getRandomNrTurnsToNextGoodEffect() const override;
  string getSpecificActivateMessage() override;
};

class DeviceRejuvenator: public Device {
public:
  DeviceRejuvenator(ItemData* const itemData, Engine& engine) :
    Device(itemData, engine) {}

  ~DeviceRejuvenator() override {}

private:
  void runGoodEffect() override;
  string getSpecificActivateMessage() override;
};

class DeviceTranslocator: public Device {
public:
  DeviceTranslocator(ItemData* const itemData, Engine& engine) :
    Device(itemData, engine) {}

  ~DeviceTranslocator() override {}

private:
  void runGoodEffect() override;
  string getSpecificActivateMessage() override;
};

class DeviceElectricLantern: public Device {
public:
  DeviceElectricLantern(ItemData* const itemData, Engine& engine) :
    Device(itemData, engine), malfunctCooldown_(-1) {}

  ~DeviceElectricLantern() override {}

  bool isGivingLight() const;

private:
  void printToggleMessage() override;

  void specificnewTurnInInventory() override;

  void runBadEffect() override;

  void specificToggle() override;

  int malfunctCooldown_;
};


#endif
