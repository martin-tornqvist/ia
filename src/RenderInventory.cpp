#include "RenderInventory.h"

#include "Engine.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "InventoryIndexes.h"
#include "Log.h"
#include "Render.h"

RenderInventory::RenderInventory(Engine* engine) :
  eng(engine), X_POS_LEFT(1), X_POS_WEIGHT(X_POS_LEFT + 60) {
}

void RenderInventory::drawDots(const int X_PREV, const int W_PREV, const int X_NEW, const int Y, const sf::Color& clr) {
  const int X_DOTS = X_PREV + W_PREV;
  const int W_DOTS = X_NEW - X_DOTS;
  const string dots(W_DOTS, '.');
  eng->renderer->drawText(dots, renderArea_screen, X_DOTS, Y, clr);
}

void RenderInventory::drawBrowseSlotsMode(const MenuBrowser& browser,
    const vector<InventorySlotButton>& invSlotButtons,
    const sf::Texture& bgTexture) {
  int yPos = 1;
  int xPos = X_POS_LEFT;

  eng->renderer->clearWindow();
  eng->renderer->drawScreenSizedTexture(bgTexture);
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng->renderer->coverArea(renderArea_screen, 0, 1, MAP_X_CELLS, NR_ITEMS + 2);

  string str = "Select slot to equip/unequip  [shift+select] drop  [space/esc] exit";
  eng->renderer->drawText(str, renderArea_screen, xPos, yPos, clrWhiteHigh);

  const int X_POS_ITEM_NAME = X_POS_LEFT + 14;

  yPos++;

  for(unsigned int i = 0; i < invSlotButtons.size(); i++) {
    const bool IS_CUR_POS = browser.getPos().y == static_cast<int>(i);
    str = "x) ";
    str.at(0) = 'a' + i;
    InventorySlot* const slot = invSlotButtons.at(i).inventorySlot;
    str += slot->interfaceName;
    xPos = X_POS_LEFT;
    eng->renderer->drawText(str, renderArea_screen, xPos, yPos, IS_CUR_POS ? clrWhiteHigh : clrRedLight);
    xPos = X_POS_ITEM_NAME;
    str = ": ";
    Item* const item = slot->item;
    if(item == NULL) {
      str += "<empty>";
      eng->renderer->drawText(str, renderArea_screen, xPos, yPos, IS_CUR_POS ? clrWhite : clrRedLight);
    } else {
      const sf::Color itemInterfClr = IS_CUR_POS ?
                                      clrWhiteHigh :
                                      item->getInterfaceClr();

      const ItemDefinition& d = item->getDef();
      PrimaryAttackMode_t attackMode = primaryAttackMode_none;
      if(slot->devName == slot_wielded || slot->devName == slot_wieldedAlt) {
        attackMode = d.primaryAttackMode == primaryAttackMode_missile ? primaryAttackMode_melee : d.primaryAttackMode;
      }
      else if(slot->devName == slot_missiles) {
        attackMode = primaryAttackMode_missile;
      }

      str += eng->itemData->getItemInterfaceRef(item, false, attackMode);
      eng->renderer->drawText(str, renderArea_screen, xPos, yPos, itemInterfClr);
      drawDots(xPos, static_cast<int>(str.size()), X_POS_WEIGHT, yPos, itemInterfClr);
      eng->renderer->drawText(item->getWeightLabel(), renderArea_screen, X_POS_WEIGHT, yPos, clrGray);
    }


    yPos++;
  }

  str = "x) Browse inventory";
  str.at(0) = invSlotButtons.back().key + 1;
  xPos = X_POS_LEFT;
  yPos += 1;
  const bool IS_CUR_POS = browser.getPos().y == static_cast<int>(invSlotButtons.size());
  eng->renderer->drawText(str, renderArea_screen, xPos, yPos, IS_CUR_POS ? clrWhiteHigh : clrRedLight);

  eng->renderer->updateWindow();
}

void RenderInventory::drawBrowseInventoryMode(const MenuBrowser& browser,
    const vector<unsigned int>& genInvIndexes,
    const sf::Texture& bgTexture) {
  int xPos = X_POS_LEFT;
  int yPos = 1;

  eng->renderer->clearWindow();
  eng->renderer->drawScreenSizedTexture(bgTexture);
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng->renderer->coverArea(renderArea_screen, 0, 1, MAP_X_CELLS, NR_ITEMS + 1);

  string str = "Inventory  [shift+select] drop  [space/esc] done";
  eng->renderer->drawText(str, renderArea_screen, xPos, yPos, clrWhiteHigh);
  yPos++;

  Inventory* const inv = eng->player->getInventory();

  for(unsigned int i = 0; i < genInvIndexes.size(); i++) {
    const bool IS_CUR_POS = browser.getPos().y == static_cast<int>(i);
    Item* const item = inv->getGeneral()->at(genInvIndexes.at(i));

    const sf::Color itemInterfClr = IS_CUR_POS ?
                                    clrWhiteHigh :
                                    item->getInterfaceClr();
    str = "x) ";
    str.at(0) = 'a' + i;
    xPos = X_POS_LEFT;
    eng->renderer->drawText(str, renderArea_screen, xPos, yPos, IS_CUR_POS ? clrWhiteHigh : clrRedLight);
    xPos += 2;

    str = eng->itemData->getItemInterfaceRef(item, false);
    eng->renderer->drawText(str, renderArea_screen, xPos, yPos, itemInterfClr);
    drawDots(xPos, static_cast<int>(str.size()), X_POS_WEIGHT, yPos, itemInterfClr);
    eng->renderer->drawText(item->getWeightLabel(), renderArea_screen, X_POS_WEIGHT, yPos, clrGray);
    yPos++;
  }

  eng->renderer->updateWindow();
}

void RenderInventory::drawEquipMode(const MenuBrowser& browser, const SlotTypes_t slotToEquip,
                                    const vector<unsigned int>& genInvIndexes,
                                    const sf::Texture& bgTexture) {
  int xPos = X_POS_LEFT;
  int yPos = 1;

  eng->renderer->clearWindow();
  eng->renderer->drawScreenSizedTexture(bgTexture);
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng->renderer->coverArea(renderArea_screen, 0, 1, MAP_X_CELLS, NR_ITEMS + 1);

  string str = "";
  switch(slotToEquip) {
  case slot_armorBody: str = "Wear which armor?"; break;
  case slot_missiles: str = "Use which item as missiles?"; break;
  case slot_wielded: str = "Wield which item?"; break;
  case slot_wieldedAlt: str = "Use which item as prepared weapon?"; break;
  }
  str += "  [shift+select] drop  [space/esc] cancel";
  eng->renderer->drawText(str, renderArea_screen, xPos, yPos, clrWhiteHigh);
  yPos++;

  Inventory* const inv = eng->player->getInventory();

  for(unsigned int i = 0; i < genInvIndexes.size(); i++) {
    const bool IS_CUR_POS = browser.getPos().y == static_cast<int>(i);
    str = "x) ";
    str.at(0) = 'a' + i;
    xPos = X_POS_LEFT;
    eng->renderer->drawText(str, renderArea_screen, xPos, yPos, IS_CUR_POS ? clrWhiteHigh : clrRedLight);
    xPos += 2;

    Item* const item = inv->getGeneral()->at(genInvIndexes.at(i));

    const sf::Color itemInterfClr = IS_CUR_POS ?
                                    clrWhiteHigh :
                                    item->getInterfaceClr();

    const ItemDefinition& d = item->getDef();
    PrimaryAttackMode_t attackMode = primaryAttackMode_none;
    if(slotToEquip == slot_wielded || slotToEquip == slot_wieldedAlt) {
      attackMode = d.primaryAttackMode == primaryAttackMode_missile ? primaryAttackMode_melee : d.primaryAttackMode;
    }
    else if(slotToEquip == slot_missiles) {
      attackMode = primaryAttackMode_missile;
    }

    str = eng->itemData->getItemInterfaceRef(item, false, attackMode);
    eng->renderer->drawText(str, renderArea_screen, xPos, yPos, itemInterfClr);
    drawDots(xPos, static_cast<int>(str.size()), X_POS_WEIGHT, yPos, itemInterfClr);
    eng->renderer->drawText(item->getWeightLabel(), renderArea_screen, X_POS_WEIGHT, yPos, clrGray);
    yPos++;
  }

  eng->renderer->updateWindow();
}

void RenderInventory::drawUseMode(const MenuBrowser& browser,
                                  const vector<unsigned int>& genInvIndexes,
                                  const sf::Texture& bgTexture) {
  const int X_POS_CMD = X_POS_LEFT + 7;

  int xPos = X_POS_LEFT;
  int yPos = 1;

  eng->renderer->clearWindow();
  eng->renderer->drawScreenSizedTexture(bgTexture);
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng->renderer->coverArea(renderArea_screen, 0, 1, MAP_X_CELLS, NR_ITEMS + 1);

  string str = "Use which item?  [ctrl+select] drop  [space/esc] cancel";
  eng->renderer->drawText(str, renderArea_screen, xPos, yPos, clrWhiteHigh);
  yPos++;

  Inventory* const inv = eng->player->getInventory();

  for(unsigned int i = 0; i < genInvIndexes.size(); i++) {
    const bool IS_CUR_POS = browser.getPos().y == static_cast<int>(i);
    Item* const item = inv->getGeneral()->at(genInvIndexes.at(i));

    const sf::Color itemInterfClr = IS_CUR_POS ?
                                    clrWhiteHigh :
                                    item->getInterfaceClr();

    //Draw label
    const string& label = item->getDefaultActivationLabel();
    bool isNewLabel = false;
    if(i == 0) {
      isNewLabel = true;
    } else {
      Item* const itemPrev = inv->getGeneral()->at(genInvIndexes.at(i - 1));
      const string& labelPrev = itemPrev->getDefaultActivationLabel();
      isNewLabel = label != labelPrev;
    }
    if(isNewLabel) {
      xPos = X_POS_LEFT;
      eng->renderer->drawText(label, renderArea_screen, xPos, yPos, clrYellow);
    }

    xPos = X_POS_CMD;
    str = "x) ";
    str.at(0) = 'a' + i;
    xPos = X_POS_CMD;
    eng->renderer->drawText(str, renderArea_screen, xPos, yPos, IS_CUR_POS ? clrWhiteHigh : clrRedLight);
    xPos += 2;

    str = eng->itemData->getItemRef(item, itemRef_plain, false);
    if(item->numberOfItems > 1 && item->getDef().isStackable) {
      str += " (" + intToString(item->numberOfItems) + ")";
    }

    eng->renderer->drawText(str, renderArea_screen, xPos, yPos, itemInterfClr);
    drawDots(xPos, static_cast<int>(str.size()), X_POS_WEIGHT, yPos, itemInterfClr);
    eng->renderer->drawText(item->getWeightLabel(), renderArea_screen, X_POS_WEIGHT, yPos, clrGray);
    yPos++;
  }

  eng->renderer->updateWindow();
}

//void RenderInventory::drawDropMode(const Item* const itemToDrop) {
//
//}

//string RenderInventory::getWeaponDataLine(Weapon* const weapon) const {
//  string line = "";
//
//  string wpnDmgStr, wpnSkillStr;
//  Abilities_t abilityUsed;
//  int dmgRolls, dmgSides, dmgPlus, actorAttackSkill, baseAttackSkill, totalAttackSkill;
//
//  const ItemDefinition* itemDef = &(weapon->getDef());
//
//  //Firearm info
//  line = "F: ";
//
//  if(itemDef->isRangedWeapon) {
//    //Weapon damage and skill used
//    const int multiplier = itemDef->isMachineGun == true ? NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST : 1;
//    dmgRolls = itemDef->rangedDmg.rolls * multiplier;
//    dmgSides = itemDef->rangedDmg.sides;
//    dmgPlus = itemDef->rangedDmg.plus * multiplier;
//
//    //Damage
//    const string dmgStrOverRide = itemDef->rangedDmgLabelOverRide;
//    if(dmgStrOverRide == "") {
//      wpnDmgStr = intToString(dmgRolls) + "d" + intToString(dmgSides);
//      wpnDmgStr += dmgPlus > 0 ? "+" + intToString(dmgPlus) : (dmgPlus < 0 ? "-" + intToString(dmgPlus) : "");
//    } else {
//      wpnDmgStr = dmgStrOverRide;
//    }
//
//    line += wpnDmgStr + " ";
//
//    //Total attack skill with weapon (base + actor skill)
//    abilityUsed = itemDef->rangedAbilityUsed;
//    actorAttackSkill = eng->player->getDef()->abilityValues.getAbilityValue(abilityUsed, true, *(eng->player));
//    baseAttackSkill = itemDef->rangedBaseAttackSkill;
//    totalAttackSkill = min(100, baseAttackSkill + actorAttackSkill);
//    wpnSkillStr = intToString(totalAttackSkill) + "%";
//
//    line += wpnSkillStr + " ";
//  } else {
//    line += "N/A ";
//  }
//
//  for(unsigned int i = line.size(); i < 16; i++) {
//    line += " ";
//  }
//
//  //Melee info
//  line += "M: ";
//
//  if(itemDef->isMeleeWeapon) {
//    //Weapon damage and skill used
//
//    dmgRolls = itemDef->meleeDmg.first;
//    dmgSides = itemDef->meleeDmg.second;
//    dmgPlus = weapon->meleeDmgPlus;
//
//    //Damage
//    wpnDmgStr = intToString(dmgRolls) + "d" + intToString(dmgSides);
//    wpnDmgStr += dmgPlus > 0 ? "+" + intToString(dmgPlus) : (dmgPlus < 0 ? "-" + intToString(dmgPlus) : "");
//
//    line += wpnDmgStr + " ";
//
//    //Total attack skill with weapon (base + actor skill)
//    abilityUsed = itemDef->meleeAbilityUsed;
//    actorAttackSkill = eng->player->getDef()->abilityValues.getAbilityValue(abilityUsed, true, *(eng->player));
//    baseAttackSkill = itemDef->meleeBaseAttackSkill;
//    totalAttackSkill = min(100, baseAttackSkill + actorAttackSkill);
//    wpnSkillStr = intToString(totalAttackSkill) + "%";
//
//    line += wpnSkillStr + " ";
//  } else {
//    line += "N/A ";
//  }
//
//  return line;
//}

//void RenderInventory::draw(const InventoryPurpose_t purpose, const MenuBrowser& browser, const bool DRAW_CMD_PROMPT, const bool DRAW_BROWSER) {
//  vector<InventorySlotButton>* slotButtons = &eng->inventoryHandler->playerSlotButtons;
//
//  vector<unsigned int>* generalItemsToShow = &eng->inventoryHandler->generalItemsToShow;
//
//  string doneString, captionLeft, captionRight;
//  string cmdString = "[a-";
//  const string END_CHAR_INDEX(1, eng->inventoryIndexes->getLastCharIndex());
//  cmdString += END_CHAR_INDEX + "]?";
//
//  switch(purpose) {
//  case inventoryPurpose_look: {
//    const int INVENTORY_HEIGHT = max(generalItemsToShow->size(), slotButtons->size());
//
//    if(DRAW_CMD_PROMPT == true) {
//      eng->log->addMessage("Displaying inventory.", clrWhiteHigh);
//    }
//
//    eng->renderer->coverArea(renderArea_mainScreen, 0, yPosLists, MAP_X_CELLS, INVENTORY_HEIGHT);
//
//    drawSlots(slotButtons, browser, DRAW_BROWSER);
//    drawGeneralItems(xPosListsRightStandardOffset, purpose, browser, DRAW_BROWSER);
//  }
//  break;
//
//  case inventoryPurpose_selectDrop: {
//    const int INVENTORY_HEIGHT = max(generalItemsToShow->size(), slotButtons->size());
//
//    if(DRAW_CMD_PROMPT == true) {
//      eng->log->addMessage("Drop which item", clrWhiteHigh);
//      eng->log->addMessage(cmdString, clrWhiteHigh);
//    }
//
//    eng->renderer->coverArea(renderArea_mainScreen, 0, yPosLists, MAP_X_CELLS, INVENTORY_HEIGHT);
//
//    drawSlots(slotButtons, browser, DRAW_BROWSER);
//    drawGeneralItems(xPosListsRightStandardOffset, purpose, browser, DRAW_BROWSER);
//  }
//  break;
//
//  case inventoryPurpose_wieldWear: {
//    const int INVENTORY_HEIGHT = generalItemsToShow->size();
//
//    if(DRAW_CMD_PROMPT == true) {
//      eng->log->addMessage("Wield or wear which item", clrWhiteHigh);
//      eng->log->addMessage(cmdString, clrWhiteHigh);
//    }
//
//    eng->renderer->coverArea(renderArea_mainScreen, 0, yPosLists, MAP_X_CELLS, INVENTORY_HEIGHT);
//
//    drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
//  }
//  break;
//
//  case inventoryPurpose_wieldAlt: {
//    const int INVENTORY_HEIGHT = generalItemsToShow->size();
//
//    if(DRAW_CMD_PROMPT == true) {
//      eng->log->addMessage("Choose weapon to keep ready", clrWhiteHigh);
//      eng->log->addMessage(cmdString, clrWhiteHigh);
//    }
//
//    eng->renderer->coverArea(renderArea_mainScreen, 0, yPosLists, MAP_X_CELLS, INVENTORY_HEIGHT);
//
//    drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
//  }
//  break;
//
//  case inventoryPurpose_missileSelect: {
//    const int INVENTORY_HEIGHT = generalItemsToShow->size();
//
//    if(DRAW_CMD_PROMPT == true) {
//      eng->log->addMessage("Use which item as thrown weapon", clrWhiteHigh);
//      eng->log->addMessage(cmdString, clrWhiteHigh);
//    }
//
//    eng->renderer->coverArea(renderArea_mainScreen, 0, yPosLists, MAP_X_CELLS, INVENTORY_HEIGHT);
//
//    drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
//  }
//  break;
//
//  case inventoryPurpose_use: {
//    const int INVENTORY_HEIGHT = generalItemsToShow->size();
//
//    if(DRAW_CMD_PROMPT == true) {
//      eng->log->addMessage("Use which item", clrWhiteHigh);
//      string endLetter = "a";
//      endLetter[0] += static_cast<char>(generalItemsToShow->size()) - 1;
//      eng->log->addMessage("[a-" + endLetter + "]?", clrWhiteHigh);
//    }
//
//    eng->renderer->coverArea(renderArea_mainScreen, 0, yPosLists, MAP_X_CELLS, INVENTORY_HEIGHT);
//
//    drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
//  }
//  break;
//
//  case inventoryPurpose_readyExplosive: {
//    const int INVENTORY_HEIGHT = generalItemsToShow->size();
//
//    if(DRAW_CMD_PROMPT == true) {
//      eng->log->addMessage("Ready what explosive", clrWhiteHigh);
//      string endLetter = "a";
//      endLetter[0] += static_cast<char>(generalItemsToShow->size()) - 1;
//      eng->log->addMessage("[a-" + endLetter + "]?", clrWhiteHigh);
//    }
//
//    eng->renderer->coverArea(renderArea_mainScreen, 0, yPosLists, MAP_X_CELLS, INVENTORY_HEIGHT);
//
//    drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
//  }
//  break;
//
//  case inventoryPurpose_eat: {
//    const int INVENTORY_HEIGHT = generalItemsToShow->size();
//
//    if(DRAW_CMD_PROMPT == true) {
//      eng->log->addMessage("Eat what", clrWhiteHigh);
//      string endLetter = "a";
//      endLetter[0] += static_cast<char>(generalItemsToShow->size()) - 1;
//      eng->log->addMessage("[a-" + endLetter + "]?", clrWhiteHigh);
//    }
//
//    eng->renderer->coverArea(renderArea_mainScreen, 0, yPosLists, MAP_X_CELLS, INVENTORY_HEIGHT);
//
//    drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
//  }
//  break;
//
//  case inventoryPurpose_quaff: {
//    const int INVENTORY_HEIGHT = generalItemsToShow->size();
//
//    if(DRAW_CMD_PROMPT == true) {
//      eng->log->addMessage("Drink what", clrWhiteHigh);
//      string endLetter = "a";
//      endLetter[0] += static_cast<char>(generalItemsToShow->size()) - 1;
//      eng->log->addMessage("[a-" + endLetter + "]?", clrWhiteHigh);
//    }
//
//    eng->renderer->coverArea(renderArea_mainScreen, 0, yPosLists, MAP_X_CELLS, INVENTORY_HEIGHT);
//
//    drawGeneralItems(0, purpose, browser, DRAW_BROWSER);
//  }
//  break;
//
//  default:
//  {} break;
//  }
//
//  if(DRAW_CMD_PROMPT == true) {
//    eng->log->addMessage("[Space/esc] Exit", clrWhiteHigh);
//  }
//
//  eng->renderer->updateWindow();
//}

//void RenderInventory::drawSlots(vector<InventorySlotButton>* slotButtons, const MenuBrowser& browser, const bool DRAW_BROWSER) {
//  InventorySlotButton* slotButton = NULL;
//  InventorySlot* slot = NULL;
//  char key = ' ';
//  string slotString = "";
//
//  int yPos = yPosLists;
//
//  for(unsigned int i = 0; i < slotButtons->size(); i++) {
//    slotButton = &(slotButtons->at(i));
//    key = slotButton->key;
//    slotString = key;
//    slotString += ") ";
//    slot = slotButton->inventorySlot;
//    slotString += slot->interfaceName;
//
//    const sf::Color clr = DRAW_BROWSER && browser.getPos().x == 0 && browser.isPosAtKey('a' + i) ? clrWhite : clrRedLight;
//    eng->renderer->drawText(slotString, renderArea_mainScreen, xPosListsLeft1, yPos, clr);
//    slotString = ": ";
//    eng->renderer->drawText(slotString, renderArea_mainScreen, xPosListsLeft2, yPos, clr);
//    slotString = slot->item == NULL ? "(empty)" : eng->itemData->itemInterfaceName(slot->item, false);
//    eng->renderer->drawText(slotString, renderArea_mainScreen, xPosListsLeft3, yPos, clr);
//
//    if(slot->item != NULL) {
//      const string itemWeightLabel = slot->item->getWeightLabel();
//      if(itemWeightLabel != "") {
//        eng->renderer->drawText(itemWeightLabel + "  ", renderArea_mainScreen, xPosListsRightStandardOffset - 4, yPos, clrGray);
//      }
//    }
//
//    yPos += 1;
//  }
//}

//void RenderInventory::drawGeneralItems(const int xPosOffset, const InventoryPurpose_t purpose, const MenuBrowser& browser, const bool DRAW_BROWSER) {
//  vector<Item*>* generalItems = eng->player->getInventory()->getGeneral();
//  vector<unsigned int>* generalItemsToShow = &(eng->inventoryHandler->generalItemsToShow);
//  int currentElement = 0;
//
//  string slotString = "";
//
//  int yPos = yPosLists;
//
//  for(unsigned int i = 0; i < generalItemsToShow->size(); i++) {
//    currentElement = generalItemsToShow->at(i);
//    slotString = eng->inventoryIndexes->getCharIndex(i);
//    slotString += ") ";
//
//    const bool ELEMENT_IS_SELECTED = ((browser.getNrOfItemsInFirstList() > 0 && browser.getPos().x == 1) ||
//                                      browser.getNrOfItemsInSecondList() == 0) &&
//                                     browser.getPos().y == static_cast<int>(i);
//    const sf::Color clr = DRAW_BROWSER && ELEMENT_IS_SELECTED ? clrWhite : clrRedLight;
//    eng->renderer->drawText(slotString, renderArea_mainScreen, xPosOffset + xPosListsRight1, yPos, clr);
//    slotString = eng->itemData->itemInterfaceName(generalItems->at(currentElement), false);
//    eng->renderer->drawText(slotString, renderArea_mainScreen, xPosOffset + xPosListsRight2, yPos, clr);
//
//    if(purpose == inventoryPurpose_wieldWear || purpose == inventoryPurpose_wieldAlt) {
//      const ItemDefinition& itemDef = generalItems->at(currentElement)->getDef();
//      if(itemDef.isRangedWeapon || itemDef.isMissileWeapon || itemDef.isMeleeWeapon) {
//        const string weaponDataLine = getWeaponDataLine(dynamic_cast<Weapon*>(generalItems->at(currentElement)));
//        string fill;
//        fill.resize(0);
//        const unsigned int FILL_SIZE = 29 - slotString.size();
//        for(unsigned int ii = 0; ii < FILL_SIZE; ii++) {
//          fill.push_back('.');
//        }
//        eng->renderer->drawText(fill, renderArea_mainScreen, xPosOffset + xPosListsRight2 + slotString.size(), yPos, clrGray);
//        const int x = 29;
//        eng->renderer->drawText(weaponDataLine, renderArea_mainScreen, x, yPos, clrWhite);
//      }
//    }
//
//    const string itemWeightLabel = generalItems->at(currentElement)->getWeightLabel();
//    if(itemWeightLabel != "") {
//      eng->renderer->drawText(itemWeightLabel + "  ", renderArea_mainScreen, MAP_X_CELLS - 5, yPos, clrGray);
//    }
//
//    yPos += 1;
//  }
//}

