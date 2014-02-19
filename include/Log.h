#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H

#include <string>
#include <vector>
#include <iostream>

#include "Colors.h"
#include "Converters.h"
#include "CommonTypes.h"

class Engine;

class Log {
public:
  Log(Engine& engine) :
    eng(engine) {clearLog();}

  void addMsg(const std::string& text, const SDL_Color& clr = clrWhite,
              const bool INTERRUPT_PLAYER_ACTIONS = false,
              const bool ADD_MORE_PROMPT_AFTER_MSG = false);

  void drawLog(const bool SHOULD_UPDATE_SCREEN) const;

  void displayHistory();

  void clearLog();

  void addLineToHistory(const std::string& lineToAdd) {
    std::vector<Msg> historyLine;
    historyLine.push_back(Msg(lineToAdd, clrWhite, 0));
    history.push_back(historyLine);
  }

private:
  class Msg {
  public:
    Msg(const std::string& text, const SDL_Color& clr, const int X_POS) :
      clr_(clr), xPos_(X_POS), str_(text), repeatsStr_(""),
      nr_(1) {}

    Msg() : Msg("", clrWhite, 0) {}

    inline void getStrWithRepeats(std::string& strRef) const {
      strRef = str_ + (nr_ > 1 ? repeatsStr_ : "");
    }

    inline void getStrRaw(std::string& strRef) const {strRef = str_;}

    void incrRepeat() {
      nr_++;
      repeatsStr_ = "(x" + toString(nr_) + ")";
    }

    SDL_Color clr_;
    int xPos_;

  private:
    std::string str_;
    std::string repeatsStr_;
    int nr_;
  };

  inline int getXAfterMsg(const Msg* const msg) {
    if(msg == NULL) {
      return 0;
    } else {
      std::string str = "";
      msg->getStrWithRepeats(str);
      return msg->xPos_ + str.size() + 1;
    }
  }

  void promptAndClearLog();

  void drawHistoryInterface(const int TOP_LINE_NR, const int BTM_LINE_NR) const;

  //Used by normal log and history viewer
  void drawLine(const std::vector<Msg>& lineToDraw, const int Y_POS) const;

  std::vector<Msg> lines[2];

  Engine& eng;

  friend class Postmortem;
  std::vector< std::vector<Msg> > history;
};

#endif
