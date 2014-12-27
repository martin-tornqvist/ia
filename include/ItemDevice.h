#ifndef ITEM_DEVICE_H
#define ITEM_DEVICE_H

#include <vector>
#include <string>

#include "Item.h"

class Device: public Item
{
public:
  Device(ItemDataT* const itemData);

  virtual ~Device() {}

  virtual ConsumeItem activate(Actor* const actor) override = 0;

  virtual Clr getInterfaceClr() const override {return clrCyan;}

  virtual void onNewTurnInInventory() override {}

  void identify(const bool IS_SILENT_IDENTIFY) override;
};

class StrangeDevice : public Device
{
public:
  StrangeDevice(ItemDataT* const itemData);

  virtual std::vector<std::string> getDescr() const override final;

  ConsumeItem activate(Actor* const actor) override;

  virtual std::string getNameInf() const override;

  virtual void storeToSaveLines(std::vector<std::string>& lines)    override;
  virtual void setupFromSaveLines(std::vector<std::string>& lines)  override;

  Condition condition_;

private:
  virtual std::vector<std::string> getDescrIdentified() const = 0;

  virtual ConsumeItem triggerEffect() = 0;
};

class DeviceBlaster : public StrangeDevice
{
public:
  DeviceBlaster(ItemDataT* const itemData) :
    StrangeDevice(itemData) {}

  ~DeviceBlaster() override {}

private:
  std::vector<std::string> getDescrIdentified() const
  {
    return {"When activated, this strange device blasts all visible enemies with "
            "infernal power."
           };
  }

  ConsumeItem triggerEffect() override;
};

class DeviceShockwave : public StrangeDevice
{
public:
  DeviceShockwave(ItemDataT* const itemData) :
    StrangeDevice(itemData) {}

  ~DeviceShockwave() override {}

private:
  std::vector<std::string> getDescrIdentified() const
  {
    return {"When activated, this strange device generates a shock wave which violently "
            "pushes away any adjacent creatures and destroys structures."
           };
  }

  ConsumeItem triggerEffect() override;
};

class DeviceRejuvenator : public StrangeDevice
{
public:
  DeviceRejuvenator(ItemDataT* const itemData) :
    StrangeDevice(itemData) {}

  ~DeviceRejuvenator() override {}

private:
  std::vector<std::string> getDescrIdentified() const
  {
    return {"When activated, this strange device heals all wounds and physical "
            "maladies."
           };
  }

  ConsumeItem triggerEffect() override;
};

class DeviceTranslocator : public StrangeDevice
{
public:
  DeviceTranslocator(ItemDataT* const itemData) :
    StrangeDevice(itemData) {}

  ~DeviceTranslocator() override {}

private:
  std::vector<std::string> getDescrIdentified() const
  {
    return {"When activated, this strange device teleports all visible enemies to "
            "different locations."
           };
  }

  ConsumeItem triggerEffect() override;
};

class DeviceSentryDrone : public StrangeDevice
{
public:
  DeviceSentryDrone(ItemDataT* const itemData) :
    StrangeDevice(itemData) {}

  ~DeviceSentryDrone() override {}

private:
  std::vector<std::string> getDescrIdentified() const
  {
    return {"When activated, this strange device will \"come alive\" and guard the "
            "user."
           };
  }

  ConsumeItem triggerEffect() override;
};

enum class LanternMalfState {working, flicker, malfunction};
enum class LanternLightSize {none, small, normal};

class DeviceLantern : public Device
{
public:
  DeviceLantern(ItemDataT* const itemData);

  ~DeviceLantern() override {}

  ConsumeItem activate(Actor* const actor)          override;
  void          onNewTurnInInventory()              override;
  void          onPickupToBackpack(Inventory& inv)  override;

  LanternLightSize getCurLightSize() const;

  void storeToSaveLines  (std::vector<std::string>& lines) override;
  void setupFromSaveLines(std::vector<std::string>& lines) override;

  int               nrTurnsLeft_;
  int               nrMalfunctTurnsLeft_;
  LanternMalfState  malfState_;
  bool              isActivated_;

private:
  void toggle();

  int getRandomNrTurnsToNextBadEffect() const;

  std::string getNameInf() const override;
};

#endif
