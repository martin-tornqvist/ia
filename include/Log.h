#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H

#include <string>
#include <vector>
#include <iostream>

#include "Colors.h"
#include "Converters.h"
#include "CmnTypes.h"

class Msg {
public:
  Msg(const std::string& text, const Clr& clr, const int X_POS) :
    clr_(clr), xPos_(X_POS), str_(text), repeatsStr_(""),
    nr_(1) {}

  Msg() : Msg("", clrWhite, 0) {}

  inline void getStrWithRepeats(std::string& strRef) const {
    strRef = str_ + (nr_ > 1 ? repeatsStr_ : "");
  }

  inline void getStrRaw(std::string& strRef) const {strRef = str_;}

  inline void incrRepeat() {
    nr_++;
    repeatsStr_ = "(x" + toStr(nr_) + ")";
  }

  Clr clr_;
  int xPos_;

private:
  std::string str_;
  std::string repeatsStr_;
  int nr_;
};

namespace Log {

void init();

void addMsg(const std::string& text, const Clr& clr = clrWhite,
            const bool INTERRUPT_PLAYER_ACTIONS = false,
            const bool ADD_MORE_PROMPT_AFTER_MSG = false);

void drawLog(const bool SHOULD_UPDATE_SCREEN);

void displayHistory();

void clearLog();

void addLineToHistory(const std::string& lineToAdd);

const std::vector< std::vector<Msg> >& getHistory();

} //Log

#endif
