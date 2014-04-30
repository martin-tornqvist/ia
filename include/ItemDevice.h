#ifndef ITEM_DEVICE_H
#define ITEM_DEVICE_H

#include "Item.h"

class Device: public Item {
public:
  Device(ItemDataT* const itemData);

  virtual ~Device() {}

  virtual ConsumeItem activateDefault(Actor* const actor) override = 0;

  virtual SDL_Color getInterfaceClr() const {return clrCyan;}

  virtual void newTurnInInventory() override {}

  virtual std::string getDefaultActivationLabel() const override = 0;

  virtual void storeToSaveLines(std::vector<std::string>& lines)    override;
  virtual void setupFromSaveLines(std::vector<std::string>& lines)  override;

  void identify(const bool IS_SILENT_IDENTIFY) override;

  Condition condition_;
};

class StrangeDevice : public Device {
public:
  StrangeDevice(ItemDataT* const itemData);

  ConsumeItem activateDefault(Actor* const actor) override;

  std::string getDefaultActivationLabel() const override {return "Use";}

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

  std::string getDefaultActivationLabel() const override {
    return isActivated_ ? "Turn off" : "Turn on";
  }

  void storeToSaveLines(std::vector<std::string>& lines)    override;
  void setupFromSaveLines(std::vector<std::string>& lines)  override;

private:
  void toggle();

  int getRandomNrTurnsToNextBadEffect() const;

  int malfunctCooldown_;
  LanternMalfState malfState_;
  bool isActivated_;
  int nrTurnsToNextBadEffect_;
};

#endif
