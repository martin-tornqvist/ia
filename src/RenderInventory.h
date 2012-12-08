#ifndef RENDER_INVENTORY_H
#define RENDER_INVENTORY_H

#include "InventoryHandler.h"

#include "SFML/Graphics/Texture.hpp"

#include "ItemData.h"
#include "MenuBrowser.h"

class Engine;

class RenderInventory {
public:
  RenderInventory(Engine* engine);

  void drawBrowseSlotsMode(const MenuBrowser& browser,
                           const vector<InventorySlotButton>& invSlotButtons,
                           const sf::Texture& bgTexture);

  void drawBrowseInventoryMode(const MenuBrowser& browser,
                               const vector<unsigned int>& genInvIndexes,
                               const sf::Texture& bgTexture);

  void drawEquipMode(const MenuBrowser& browser, const SlotTypes_t slotToEquip,
                     const vector<unsigned int>& genInvIndexes,
                     const sf::Texture& bgTexture);

  void drawUseMode(const MenuBrowser& browser,
                   const vector<unsigned int>& genInvIndexes,
                   const sf::Texture& bgTexture);

//  void drawDropMode(const Item* const itemToDrop);

private:
  RenderInventory& operator=(const RenderInventory& other) {
    (void)other;
    return *this;
  }

  void drawDots(const int X_PREV, const int W_PREV, const int X_NEW, const int Y, const sf::Color& clr);

  Engine* eng;

  const int X_POS_LEFT;
  const int X_POS_WEIGHT;

  friend class Examine;
};

#endif
