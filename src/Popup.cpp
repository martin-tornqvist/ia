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
//	const int CELL_W = eng->config->CELL_WIDTH_TEXT;
//	const int CELL_H = eng->config->CELL_HEIGHT_TEXT;
//	const int MAIN_Y_OFFS = eng->config->MAINSCREEN_Y_OFFSET;
//	const int PIXEL_X0 = x0y0.x * CELL_W;
//	const int PIXEL_Y0 = x0y0.y * CELL_H + MAIN_Y_OFFS;
//	const int PIXEL_X1 = (x1y1.x + 1) * CELL_W - 1;
//	const int PIXEL_Y1 = (x1y1.y + 1) * CELL_H + MAIN_Y_OFFS - 1;
//	const int PIXEL_W = PIXEL_X1 - PIXEL_X0 + 1;
//	const int PIXEL_H = PIXEL_Y1 - PIXEL_Y0 + 1;

//	eng->renderer->clearAreaPixel(PIXEL_X0, PIXEL_Y0, PIXEL_W, PIXEL_H);

	eng->renderer->clearAreaWithTextDimensions(renderArea_mainScreen, x0y0.x, x0y0.y, x1y1.x - x0y0.x + 1, x1y1.y - x0y0.y + 1);

	const SDL_Color clrBox = clrGray;

	for(int y = x0y0.y; y <= x1y1.y; y++) {
		for(int x = x0y0.x; x <= x1y1.x; x++) {
			if(x == x0y0.x || x == x1y1.x) {
				if(y == x0y0.y || y == x1y1.y) {
					eng->renderer->drawCharacter('#', renderArea_mainScreen, x, y, clrBox);
				} else {
					eng->renderer->drawCharacter('|', renderArea_mainScreen, x, y, clrBox);
				}
			} else {
				if(y == x0y0.y || y == x1y1.y) {
					eng->renderer->drawCharacter('=', renderArea_mainScreen, x, y, clrBox);
				}
			}
		}
	}

	return BoxReturnData(x0y0 + coord(1, 1), x1y1 - coord(1, 1));
}

void Popup::showMessage(const string message, const bool DRAW_MAP_AND_INTERFACE_AFTER) const {
	const int BOX_HALF_WIDTH = message.length() >= 250 ? 23 : 18;
	const BoxReturnData box = printBox(BOX_HALF_WIDTH);
	const int W = box.x1y1Text.x - box.x0y0Text.x + 1;
	vector<string> lines = eng->textFormatting->lineToLines(message, W + 1);

	for(unsigned int i = 0; i < lines.size(); i++) {
		eng->renderer->drawText(
		   lines.at(i), renderArea_screen, box.x0y0Text.x,
		   (i + box.x0y0Text.y) + eng->config->MAINSCREEN_Y_CELLS_OFFSET, clrRedLight);

		eng->log->addLineToHistory(lines.at(i));
	}

	eng->renderer->drawTextCentered(
	   "[Space/Esc] to close", renderArea_screen, box.x0y0Text.x + W / 2,
	   box.x1y1Text.y + eng->config->MAINSCREEN_Y_CELLS_OFFSET, clrWhiteHigh);

	eng->renderer->flip();
	eng->query->waitForEscOrSpace();
	if(DRAW_MAP_AND_INTERFACE_AFTER) {
	  eng->renderer->drawMapAndInterface(true);
	}
}
