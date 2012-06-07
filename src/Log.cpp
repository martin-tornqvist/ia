#include "Log.h"

#include <algorithm>

#include "Engine.h"
#include "Converters.h"
#include "Render.h"
#include "Query.h"
#include "ActorPlayer.h"

using namespace std;

void MessageLog::clearLog() {
	if(line.size() != 0) {

		history.push_back(line);

		while(history.size() > 300) {
			history.erase(history.begin());
		}

		line.resize(0);
		drawLog();
	}
}

void MessageLog::drawLine(const vector<Message>& line, const int yCell) const {
	SDL_Color clr;
	string str;
	int drawXpos;

	const unsigned int LINE_SIZE = line.size();

	for(unsigned int i = 0; i < LINE_SIZE; i++) {
		const Message& curMessage = line.at(i);
		clr = curMessage.clr;
		str = curMessage.str;
		if(curMessage.repeats > 1) {
			str += curMessage.strRepeats;
		}

		drawXpos = findCurXpos(line, i);

		eng->renderer->drawText(str, renderArea_log, drawXpos, yCell, clr);
	}
}

void MessageLog::drawLog() const {
	eng->renderer->clearRenderArea(renderArea_log);
	string str;
	drawLine(line, 0);
//	eng->renderer->flip();
}

void MessageLog::displayHistory() {
	clearLog();

	eng->renderer->clearRenderArea(renderArea_screen);

	string str;

	int topElement = max(0, static_cast<int> (history.size()) - static_cast<int> (MAP_Y_CELLS));
	int btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int> (history.size()) - 1);
	drawHistoryInterface(topElement, btmElement);
	int yCell = 1;
	for(int i = topElement; i <= btmElement; i++) {
		drawLine(history.at(static_cast<unsigned int> (i)), yCell);
		yCell++;
	}

	eng->renderer->flip();

	//Read keys
	SDL_Event event;
	bool done = false;
	while(done == false) {
		while(SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN: {
				int key = event.key.keysym.sym;

				switch (key) {
				case SDLK_2:
				case SDLK_KP2:
				case SDLK_DOWN: {
					topElement = max(0, min(topElement + static_cast<int> (MAP_Y_CELLS), static_cast<int> (history.size())
							- static_cast<int> (MAP_Y_CELLS)));
					int btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int> (history.size()) - 1);

					eng->renderer->clearAreaWithTextDimensions(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);

					drawHistoryInterface(topElement, btmElement);
					yCell = 1;
					for(int i = topElement; i <= btmElement; i++) {
						drawLine(history.at(static_cast<unsigned int> (i)), yCell);
						yCell++;
					}
					eng->renderer->flip();
				}
				break;
				case SDLK_8:
				case SDLK_KP8:
				case SDLK_UP: {
					topElement = max(0, min(topElement - static_cast<int> (MAP_Y_CELLS), static_cast<int> (history.size())
							- static_cast<int> (MAP_Y_CELLS)));
					int btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int> (history.size()) - 1);
					eng->renderer->clearAreaWithTextDimensions(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);
					drawHistoryInterface(topElement, btmElement);
					yCell = 1;
					for(int i = topElement; i <= btmElement; i++) {
						drawLine(history.at(static_cast<unsigned int> (i)), yCell);
						yCell++;
					}
					eng->renderer->flip();
				}
				break;
				case SDLK_SPACE:
				case SDLK_ESCAPE: {
					done = true;
				}
				break;

				}
			}
			break;
			default: {
			}
			break;
			}
		}
		SDL_Delay(1);
	}

	eng->renderer->clearRenderArea(renderArea_screen);
	eng->renderer->drawMapAndInterface();
}

void MessageLog::drawHistoryInterface(const int topLine, const int bottomLine) const {
	const string decorationLine(MAP_X_CELLS - 2, '-');

	eng->renderer->clearRenderArea(renderArea_log);
	eng->renderer->drawText(decorationLine, renderArea_screen, 1, 1, clrWhite);
	if(history.size() == 0) {
		eng->renderer->drawText(" No message history ", renderArea_screen, 3, 1, clrWhite);
	} else {
		eng ->renderer->drawText(" Displaying messages " + intToString(topLine) + "-" + intToString(bottomLine) + " of "
				+ intToString(history.size()) + " ", renderArea_screen, 3, 1, clrWhite);
	}

	eng->renderer->drawText(decorationLine, renderArea_characterLines, 1, 1, clrWhite);
	eng->renderer->drawText(" [2/8, Down/Up] to navigate  [Space/Esc] to exit. ", renderArea_characterLines, 3, 1, clrWhite);
}

int MessageLog::findCurXpos(const vector<Message>& line, const unsigned int messageNr) const {
	if(messageNr == 0)
		return 0;

	const unsigned int LINE_SIZE = line.size();

	if(LINE_SIZE == 0)
		return 0;

	int xPos = 0;

	for(unsigned int i = 0; i < messageNr; i++) {

		const Message& curMessage = line.at(i);

		xPos += static_cast<int> (curMessage.str.length());

		if(curMessage.repeats > 1)
			xPos += 4;

		xPos++;
	}

	return xPos;
}

void MessageLog::addMessage(const string& text, const SDL_Color color, bool queryInterruptPlayerAction) {
	bool repeated = false;

	//New message equal to previous?
	if(line.size() > 0) {
		if(line.back().str.compare(text) == 0) {
			line.back().addRepeat();
			repeated = true;
		}
	}

	if(repeated == false) {
		const int REPEAT_LABEL_LENGTH = 4;
		const int MORE_PROMPT_LENGTH = 7;

		const int CUR_X_POS = findCurXpos(line, line.size());

		const bool MESSAGE_FITS = CUR_X_POS + static_cast<int> (text.size()) + REPEAT_LABEL_LENGTH + MORE_PROMPT_LENGTH < MAP_X_CELLS;

		if(MESSAGE_FITS == false) {
			eng->renderer->drawText("[MORE]", renderArea_log, CUR_X_POS, 0, clrCyanLight);
			eng->renderer->flip();
			eng->query->waitForKeyPress();
			clearLog();
		}

		const Message m(text, color);
		line.push_back(m);
	}

	drawLog();

	//Messages may stop long actions like first aid and auto travel.
	if(queryInterruptPlayerAction == true) {
		eng->player->queryInterruptActions();
	}
}

