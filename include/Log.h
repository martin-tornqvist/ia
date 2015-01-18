#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H

#include <string>
#include <vector>

#include "Colors.h"
#include "Converters.h"
#include "CmnTypes.h"

class Msg
{
public:
  Msg(const std::string& text, const Clr& clr, const int X_POS) :
    clr_        (clr),
    xPos_       (X_POS),
    str_        (text),
    repeatsStr_ (""),
    nr_         (1) {}

  Msg() :
    clr_        (clrWhite),
    xPos_       (0),
    str_        (""),
    repeatsStr_ (""),
    nr_         (1) {}

  void getStrWithRepeats(std::string& strRef) const
  {
    strRef = str_ + (nr_ > 1 ? repeatsStr_ : "");
  }

  void getStrRaw(std::string& strRef) const {strRef = str_;}

  void incrRepeat()
  {
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

namespace Log
{

void init();

void addMsg(const std::string& str, const Clr& clr = clrWhite,
            const bool INTERRUPT_PLAYER_ACTIONS = false,
            const bool ADD_MORE_PROMPT_AFTER_MSG = false);

//Note: This function can safely be called at any time. If there is content in the log,
//a "more" prompt will be done, and the log is cleared. If the log happens to be empty,
//nothing is done.
void morePrompt();

void drawLog(const bool SHOULD_UPDATE_SCREEN);

void displayHistory();

void clearLog();

void addLineToHistory(const std::string& lineToAdd);

const std::vector< std::vector<Msg> >& getHistory();

} //Log

#endif
