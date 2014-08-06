#ifndef ITEM_MEDICAL_BAG_H
#define ITEM_MEDICAL_BAG_H

#include "Item.h"

enum class MedBagAction {
  sanitizeInfection,
//  takeMorphine,
  treatWounds,
  END
};

class MedicalBag: public Item {
public:
  MedicalBag(ItemDataT* const itemData) :
    Item(itemData),
    nrSupplies_(60),
    nrTurnsUntilHealWounds_(-1),
    nrTurnsLeftSanitize_(-1) {}

  ~MedicalBag() {}

  ConsumeItem activateDefault(Actor* const actor) override;

  void continueAction();
  void interrupted();
  void finishCurAction();

  std::string getDefaultActivationLabel() const override {return "Apply";}

  Clr getInterfaceClr() const override {return clrGreen;}

  void storeToSaveLines(std::vector<std::string>& lines) override {
    lines.push_back(toStr(nrSupplies_));
  }
  void setupFromSaveLines(std::vector<std::string>& lines) override {
    nrSupplies_ = toInt(lines.front());
    lines.erase(begin(lines));
  }

  inline int getNrSupplies() const {return nrSupplies_;}

protected:
  MedBagAction playerChooseAction() const;

  int getTotTurnsForSanitize() const;
  int getTotSupplForSanitize() const;

  int nrSupplies_;

  int nrTurnsUntilHealWounds_;
  int nrTurnsLeftSanitize_;

  MedBagAction curAction_;
};

#endif
