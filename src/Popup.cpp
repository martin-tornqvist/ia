#include "Popup.h"

#include "Engine.h"
#include "Render.h"
#include "TextFormatting.h"
#include "Log.h"
#include "Query.h"

Popup::BoxReturnData Popup::printBox(const int BOX_HALF_WIDTH) const {
  const int CELLS_FROM_MID_X = BOX_HALF_WIDTH;
  const int CELLS_FROM_MID_Y = 7;
  const int CELLS_Y_OFFSET = -2;
  const coord x0y0(MAP_X_CELLS_HALF - CELLS_FROM_MID_X, MAP_Y_CELLS_HALF - CELLS_FROM_MID_Y + CELLS_Y_OFFSET);
  const coord x1y1(MAP_X_CELLS_HALF + CELLS_FROM_MID_X, MAP_Y_CELLS_HALF + CELLS_FROM_MID_Y + CELLS_Y_OFFSET);

  eng->renderer->coverArea(renderArea_mainScreen, x0y0.x, x0y0.y, x1y1.x - x0y0.x + 1, x1y1.y - x0y0.y + 1);

  const bool& USE_TILE_SET = eng->config->USE_TILE_SET;

  const sf::Color clrBox = clrGray;

  for(int y = x0y0.y; y <= x1y1.y; y++) {
    for(int x = x0y0.x; x <= x1y1.x; x++) {
      if(x == x0y0.x || x == x1y1.x) {
        if(y == x0y0.y || y == x1y1.y) {
          if(USE_TILE_SET) {
            if(x == x0y0.x && y == x0y0.y) {
              eng->renderer->drawTileInMap(tile_popupCornerTopLeft, x, y, clrBox, false, clrBlack);
            } else if(x == x1y1.x && y == x0y0.y) {
              eng->renderer->drawTileInMap(tile_popupCornerTopRight, x, y, clrBox, false, clrBlack);
            } else if(x == x0y0.x && y == x1y1.y) {
              eng->renderer->drawTileInMap(tile_popupCornerBottomLeft, x, y, clrBox, false, clrBlack);
            } else {
              eng->renderer->drawTileInMap(tile_popupCornerBottomRight, x, y, clrBox, false, clrBlack);
            }
          } else {
            eng->renderer->drawCharacter('#', renderArea_mainScreen, x, y, clrBox);
          }
        } else {
          if(USE_TILE_SET) {
            eng->renderer->drawTileInMap(tile_popupVerticalBar, x, y, clrBox, false, clrBlack);
          } else {
            eng->renderer->drawCharacter('|', renderArea_mainScreen, x, y, clrBox);
          }
        }
      } else {
        if(y == x0y0.y || y == x1y1.y) {
          if(USE_TILE_SET) {
            eng->renderer->drawTileInMap(tile_popupHorizontalBar, x, y, clrBox, false, clrBlack);
          } else {
            eng->renderer->drawCharacter('=', renderArea_mainScreen, x, y, clrBox);
          }
        }
      }
    }
  }

  return BoxReturnData(x0y0 + coord(1, 1), x1y1 - coord(1, 1));
}

void Popup::showMessage(const string message, const bool DRAW_MAP_AND_INTERFACE, const string title) const {
  if(DRAW_MAP_AND_INTERFACE) {
    eng->renderer->drawMapAndInterface(false);
  }

  const int BOX_HALF_WIDTH = message.length() >= 250 ? 23 : 18;
  const BoxReturnData box = printBox(BOX_HALF_WIDTH);
  const int W = box.x1y1Text.x - box.x0y0Text.x + 1;
  vector<string> lines = eng->textFormatting->lineToLines(message, W + 1);

  const int MAP_Y_OFFSET = eng->config->MAINSCREEN_Y_CELLS_OFFSET;
  const int TEXT_POS_TOP = box.x0y0Text.y + MAP_Y_OFFSET;
  if(title != "") {
    eng->renderer->drawTextCentered(title, renderArea_screen, MAP_X_CELLS_HALF, TEXT_POS_TOP, clrCyanLight, true);
  }
  for(unsigned int i = 0; i < lines.size(); i++) {
    const int Y = TEXT_POS_TOP + i + (title != "");
    eng->renderer->drawText(lines.at(i), renderArea_screen, box.x0y0Text.x, Y, clrRedLight);
    eng->log->addLineToHistory(lines.at(i));
  }

  eng->renderer->drawTextCentered("[Space/Esc] to close", renderArea_screen, box.x0y0Text.x + W / 2, box.x1y1Text.y + MAP_Y_OFFSET, clrWhiteHigh);

  eng->renderer->updateWindow();

  eng->query->waitForEscOrSpace();

  if(DRAW_MAP_AND_INTERFACE) {
    eng->renderer->drawMapAndInterface();
  }
}
