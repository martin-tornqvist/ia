#ifndef LOG_H
#define LOG_H

#include <SDL/SDL.h>

#include <string>
#include <vector>
#include <iostream>

#include "Colors.h"
#include "Converters.h"

using namespace std;

class Engine;

class MessageLog {
public:
	MessageLog(Engine* engine) :
		eng(engine) {
		clearLog();
	}

	void addMessage(const string& text, const SDL_Color = clrWhite, bool queryInterruptPlayerAction = true);

	void drawLog() const;

	void displayHistory();

	void clearLog();

	void addLineToHistory(const string line) {
		vector<Message> historyLine;
		historyLine.push_back(Message(line, clrWhite));
		history.push_back(historyLine);
	}

private:
	struct Message {
		Message(const string& text, const SDL_Color color) :
			str(text), clr(color), repeats(1), strRepeats("") {
		}

		Message() {
		}

		void addRepeat() {
			repeats++;
			strRepeats = "(x";
			strRepeats += intToString(repeats);
			strRepeats += ")";
		}

		string str;
		SDL_Color clr;
		int repeats;
		string strRepeats;
	};

	void drawHistoryInterface(const int topLine, const int bottomLine) const;

	//Used by normal log, and history viewer
	void drawLine(const vector<Message>& line, const int yCell) const;

	vector<Message> line;

	//Returns the x cell position that a message should start on.
	//May be one higher than highest message index.
	int findCurXpos(const vector<Message>& line, const unsigned int messageNr) const;

	const Engine* eng;

	friend class Postmortem;
	vector<vector<Message> > history;
};

#endif
