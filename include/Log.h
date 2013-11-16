#ifndef LOG_H
#define LOG_H

#include <string>
#include <vector>
#include <iostream>

#include "Colors.h"
#include "Converters.h"

using namespace std;

class Engine;

class Log {
public:
  Log(Engine* engine) :
    eng(engine) {clearLog();}

  void addMsg(const string& text, const SDL_Color = clrWhite,
              const bool INTERRUPT_PLAYER_ACTIONS = false,
              const bool FORCE_MORE_PROMPT = false);

  void drawLog() const;

  void displayHistory();

  void clearLog();

  void addLineToHistory(const string& lineToAdd) {
    vector<Message> historyLine;
    historyLine.push_back(Message(lineToAdd, clrWhite));
    history.push_back(historyLine);
  }

private:
  struct Message {
    Message(const string& text, const SDL_Color color) :
      str(text), clr(color), repeats(1), strRepeats("") {
    }

    Message() {}

    void addRepeat() {
      repeats++;
      strRepeats = "(x";
      strRepeats += toString(repeats);
      strRepeats += ")";
    }

    string str;
    SDL_Color clr;
    int repeats;
    string strRepeats;
  };

  void drawHistoryInterface(const int topLine, const int bottomLine) const;

  //Used by normal log, and history viewer
  void drawLine(const vector<Message>& lineToDraw, const int yCell) const;

  vector<Message> line;

  //Returns the x cell position that a message should start on.
  //May be one higher than highest message index.
  int findCurXpos(const vector<Message>& line,
                  const unsigned int messageNr) const;

  const Engine* eng;

  friend class Postmortem;
  vector<vector<Message> > history;
};

#endif
