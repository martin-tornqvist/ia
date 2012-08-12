#include "Popup.h"

#include "Engine.h"
#include "Render.h"
#include "TextFormatting.h"
#include "Log.h"
#include "Query.h"

Popup::BoxReturnData Popup::printBox(const int BOX_HALF_WIDTH) const {
	const int CELLS_FROM_MID_X = BOX_HALF_WIDTH;
	const int CELLS_FROM_MID_Y = 7;
	const int CELLS_Y_OFFSET = -1;
	const coord x0y0(MAP_X_CELLS_HALF - CELLS_FROM_MID_X, MAP_Y_CELLS_HALF - CELLS_FROM_MID_Y + CELLS_Y_OFFSET);
	const coord x1y1(MAP_X_CELLS_HALF + CELLS_FROM_MID_X, MAP_Y_CELLS_HALF + CELLS_FROM_MID_Y + CELLS_Y_OFFSET);
	const int CELL_W = eng->config->CELL_WIDTH_TEXT;
	const int CELL_H = eng->config->CELL_HEIGHT_TEXT;
	const int MAIN_Y_OFFS = eng->config->MAINSCREEN_Y_OFFSET;
	const int PIXEL_X0 = x0y0.x * CELL_W;
	const int PIXEL_Y0 = x0y0.y * CELL_H + MAIN_Y_OFFS;
	const int PIXEL_X1 = (x1y1.x + 1) * CELL_W - 1;
	const int PIXEL_Y1 = (x1y1.y + 1) * CELL_H + MAIN_Y_OFFS - 1;
	const int PIXEL_W = PIXEL_X1 - PIXEL_X0 + 1;
	const int PIXEL_H = PIXEL_Y1 - PIXEL_Y0 + 1;

	eng->renderer->clearAreaPixel(PIXEL_X0, PIXEL_Y0, PIXEL_W, PIXEL_H);

	const SDL_Color clrBox = clrWhite;

	eng->renderer->drawLineVertical(PIXEL_X0, PIXEL_Y0, PIXEL_H, clrBox);
	eng->renderer->drawLineVertical(PIXEL_X0 + CELL_W - 1 - 2, PIXEL_Y0, PIXEL_H, clrBox);

	eng->renderer->drawLineVertical(PIXEL_X1 - CELL_W + 1 + 2, PIXEL_Y0, PIXEL_H, clrBox);
	eng->renderer->drawLineVertical(PIXEL_X1, PIXEL_Y0, PIXEL_H, clrBox);

	eng->renderer->drawLineHorizontal(PIXEL_X0, PIXEL_Y0, PIXEL_W, clrBox);
	eng->renderer->drawLineHorizontal(PIXEL_X0, PIXEL_Y0 + CELL_H - 3, PIXEL_W, clrBox);

	eng->renderer->drawLineHorizontal(PIXEL_X0, PIXEL_Y1 - CELL_H + 1 + 2, PIXEL_W, clrBox);
	eng->renderer->drawLineHorizontal(PIXEL_X0, PIXEL_Y1, PIXEL_W, clrBox);

	const SDL_Color clr1 = clrRed;
	const SDL_Color clr2 = clrBlueLight;
	const SDL_Color clr3 = clrBlue;
	const SDL_Color clr4 = clrBrownDark;
	eng->renderer->drawRectangle(PIXEL_X0 + 1, PIXEL_Y0 + 1, CELL_W - 4, CELL_H - 4, clr1);
	eng->renderer->drawRectangle(PIXEL_X1 - CELL_W + 4, PIXEL_Y0 + 1, CELL_W - 4, CELL_H - 4, clr2);
	eng->renderer->drawRectangle(PIXEL_X0 + 1, PIXEL_Y1 - CELL_H + 4, CELL_W - 4, CELL_H - 4, clr3);
	eng->renderer->drawRectangle(PIXEL_X1 - CELL_W + 4, PIXEL_Y1 - CELL_H + 4, CELL_W - 4, CELL_H - 4, clr4);

	return BoxReturnData(x0y0 + coord(1, 1), x1y1 - coord(1, 1));
}

void Popup::showMessage(const string message) const {
	const int BOX_HALF_WIDTH = message.length() >= 250 ? 28 : 18;
	const BoxReturnData box = printBox(BOX_HALF_WIDTH);
	const int W = box.x1y1Text.x - box.x0y0Text.x + 1;
	vector<string> lines = eng->textFormatting->lineToLines(message, W);

	for(unsigned int i = 0; i < lines.size(); i++) {
		eng->renderer->drawText(lines.at(i), renderArea_screen, box.x0y0Text.x, (i + box.x0y0Text.y) + eng->config->MAINSCREEN_Y_CELLS_OFFSET, clrRedLight);
		//Copy the messages to the log history
		eng->log->addLineToHistory(lines.at(i));
	}

	eng->renderer->drawTextCentered("[Space/Esc] to close", renderArea_screen, box.x0y0Text.x + W / 2, box.x1y1Text.y
			+ eng->config->MAINSCREEN_Y_CELLS_OFFSET, clrWhiteHigh);

	eng->renderer->flip();
	eng->query->waitForEscOrSpace();
	eng->renderer->drawMapAndInterface();
}
