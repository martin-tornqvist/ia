#ifndef AI_HANDLE_INVENTORY_H
#define AI_HANDLE_INVENTORY_H

#include "Inventory.h"

/*
 * Purpose:
 * Change equipment, quaff potions, etc
 *
*/

class AI_handleInventory {
public:
  static bool action(Monster* monster, Engine& engine) {
    (void)monster;
    (void)engine;
    return false;
  }
private:

//  bool attemptSwapToMeleeWeapon(Inventory& inv) {
//    vector<Item*>* general = inv.getGeneral();
//  }
//  bool attemptSwapToRangedWeapon(Inventory& inv) {
//    vector<Item*>* general = inv.getGeneral();
//  }

};

#endif

