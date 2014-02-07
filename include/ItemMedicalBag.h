#ifndef ITEM_MEDICAL_BAG_H
#define ITEM_MEDICAL_BAG_H

#include "Item.h"

enum MedicalBagAction {
  medicalBagAction_sanitizeInfection,
  medicalBagAction_takeMorphine,
  medicalBagAction_treatWound,
  endOfMedicalBagActions
};

class MedicalBag: public Item {
public:
  MedicalBag(ItemData* const itemData, Engine& engine) :
    Item(itemData, engine), nrSupplies_(60), nrTurnsLeft_(-1) {}

  virtual ~MedicalBag() {}

  bool activateDefault(Actor* const actor);

  void continueAction();
  void interrupted();
  void finishCurAction();

  virtual string getDefaultActivationLabel() const {return "Apply";}

  virtual SDL_Color getInterfaceClr() const {return clrGreen;}

  void addSaveLines_(vector<string>& lines) {
    lines.push_back(toString(nrSupplies_));
  }
  void setParamsFromSaveLines_(vector<string>& lines) {
    nrSupplies_ = toInt(lines.front());
    lines.erase(lines.begin());
  }

  inline int getNrSupplies() const {return nrSupplies_;}

protected:
  MedicalBagAction playerChooseAction() const;

  int nrSupplies_;
  int nrTurnsLeft_;
  MedicalBagAction curAction_;

  int getTotTurnsForAction(const MedicalBagAction action) const;

  int getNrSuppliesNeededForAction(const MedicalBagAction action) const;
};

#endif
