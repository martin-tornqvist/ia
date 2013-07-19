#ifndef ITEM_MEDICAL_BAG_H
#define ITEM_MEDICAL_BAG_H

#include "Item.h"

enum MedicalBagAction_t {
  medicalBagAction_treatWound,
  medicalBagAction_sanitizeInfection,
  endOfMedicalBagActions
};

class MedicalBag: public Item {
public:
  MedicalBag(ItemDefinition* const itemDefinition) :
    Item(itemDefinition), nrSupplies_(30), nrTurnsLeft_(-1) {}

  virtual ~MedicalBag() {}

  bool activateDefault(Actor* const actor, Engine* const engine);

  void continueAction(Engine* const engine);
  void interrupted(Engine* const engine);
  void finishCurAction(Engine* const engine);

  virtual string getDefaultActivationLabel() const {return "Apply";}

  virtual SDL_Color getInterfaceClr() const {return clrGreen;}

  void itemSpecificAddSaveLines(vector<string>& lines) {
    lines.push_back(intToString(nrSupplies_));
  }
  void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
    nrSupplies_ = stringToInt(lines.front());
    lines.erase(lines.begin());
  }

  inline int getNrSupplies() const {return nrSupplies_;}

protected:
  MedicalBagAction_t playerChooseAction(Engine* const engine) const;

  int nrSupplies_;
  int nrTurnsLeft_;
  MedicalBagAction_t curAction_;

  int getTotTurnsForAction(const MedicalBagAction_t action,
                           Engine* const engine) const;

  int getNrSuppliesNeededForAction(const MedicalBagAction_t action,
                                   Engine* const engine) const;
};

#endif
