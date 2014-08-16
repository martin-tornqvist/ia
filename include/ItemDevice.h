#ifndef ITEM_DEVICE_H
#define ITEM_DEVICE_H

#include "Item.h"

class Device: public Item {
public:
  Device(ItemDataT* const itemData);

  virtual ~Device() {}

  virtual ConsumeItem activateDefault(Actor* const actor) override = 0;

  virtual Clr getInterfaceClr() const override {return clrCyan;}

  virtual void newTurnInInventory() override {}

  virtual void storeToSaveLines(std::vector<std::string>& lines)    override;
  virtual void setupFromSaveLines(std::vector<std::string>& lines)  override;

  void identify(const bool IS_SILENT_IDENTIFY) override;

  Condition condition_;
};

class StrangeDevice : public Device {
public:
  StrangeDevice(ItemDataT* const itemData);

  ConsumeItem activateDefault(Actor* const actor) override;

  virtual std::string getNameInf() const override;

private:
  virtual void triggerEffect() = 0;
};

class DeviceSentry: public StrangeDevice {
public:
  DeviceSentry(ItemDataT* const itemData) : StrangeDevice(itemData) {}

  ~DeviceSentry() override {}

private:
  void triggerEffect() override;
};

class DeviceRepeller: public StrangeDevice {
public:
  DeviceRepeller(ItemDataT* const itemData) : StrangeDevice(itemData) {}

  ~DeviceRepeller() override {}

private:
  void triggerEffect() override;
};

class DeviceRejuvenator: public StrangeDevice {
public:
  DeviceRejuvenator(ItemDataT* const itemData) :
    StrangeDevice(itemData) {}

  ~DeviceRejuvenator() override {}

private:
  void triggerEffect() override;
};

class DeviceTranslocator: public StrangeDevice {
public:
  DeviceTranslocator(ItemDataT* const itemData) :
    StrangeDevice(itemData) {}

  ~DeviceTranslocator() override {}

private:
  void triggerEffect() override;
};

enum class LanternMalfState {working, flicker, malfunction, destroyed};
enum class LanternLightSize {none, small, normal};

class DeviceLantern: public Device {
public:
  DeviceLantern(ItemDataT* const itemData);

  ~DeviceLantern() override {}

  ConsumeItem activateDefault(Actor* const actor) override;

  void newTurnInInventory() override;

  LanternLightSize getCurLightSize() const;

  void storeToSaveLines(std::vector<std::string>& lines)    override;
  void setupFromSaveLines(std::vector<std::string>& lines)  override;

private:
  void toggle();

  int getRandomNrTurnsToNextBadEffect() const;

  std::string getNameInf() const override {return isActivated_ ? "{Lit}" : "";}

  int malfunctCooldown_;
  LanternMalfState malfState_;
  bool isActivated_;
  int nrTurnsToNextBadEffect_;
};

#endif
