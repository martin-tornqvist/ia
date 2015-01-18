#include "Log.h"

#include <algorithm>
#include <vector>
#include <string>
#include <iostream>

#include "Init.h"
#include "Input.h"
#include "Converters.h"
#include "Render.h"
#include "Query.h"
#include "ActorPlayer.h"
#include "Map.h"

using namespace std;

namespace Log
{

namespace
{

vector<Msg>           lines_[2];
vector< vector<Msg> > history_;
const string          moreStr = "-More-";

int getXAfterMsg(const Msg* const msg)
{
  if (!msg) {return 0;}

  string str = "";
  msg->getStrWithRepeats(str);
  return msg->xPos_ + str.size() + 1;
}

void drawHistoryInterface(const int TOP_LINE_NR, const int BTM_LINE_NR)
{
  const string decorationLine(MAP_W, '-');

  Render::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  const int X_LABEL = 3;

  if (history_.empty())
  {
    Render::drawText(" No message history ", Panel::screen,
                     Pos(X_LABEL, 0), clrGray);
  }
  else
  {
    Render::drawText(
      " Displaying messages " + toStr(TOP_LINE_NR + 1) + "-" +
      toStr(BTM_LINE_NR + 1) + " of " +
      toStr(history_.size()) + " ", Panel::screen, Pos(X_LABEL, 0), clrGray);
  }

  Render::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1), clrGray);

  Render::drawText(infoScrCmdInfo, Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrGray);
}

//Used by normal log and history viewer
void drawLine(const vector<Msg>& lineToDraw, const int Y_POS)
{
  for (const Msg& msg : lineToDraw)
  {
    string str = "";
    msg.getStrWithRepeats(str);
    Render::drawText(str, Panel::log, Pos(msg.xPos_, Y_POS), msg.clr_);
  }
}

} //namespace

void init()
{
  for (vector<Msg>& line : lines_)
  {
    line.clear();
  }

  history_.clear();
}

void clearLog()
{
  for (vector<Msg>& line : lines_)
  {
    if (!line.empty())
    {
      history_.push_back(line);

      while (history_.size() > 300)
      {
        history_.erase(history_.begin());
      }

      line.clear();
    }
  }
}

void drawLog(const bool SHOULD_UPDATE_SCREEN)
{
  const int NR_LINES_WITH_CONTENT = lines_[0].empty() ? 0 :
                                    lines_[1].empty() ? 1 : 2;

  if (NR_LINES_WITH_CONTENT > 0)
  {
    Render::coverArea(Panel::log, Pos(0, 0), Pos(MAP_W, NR_LINES_WITH_CONTENT));

    for (int i = 0; i < NR_LINES_WITH_CONTENT; ++i)
    {
      drawLine(lines_[i], i);
    }
  }

  if (SHOULD_UPDATE_SCREEN)
  {
    Render::updateScreen();
  }
}

void addMsg(const string& str, const Clr& clr, const bool INTERRUPT_PLAYER_ACTIONS,
            const bool ADD_MORE_PROMPT_AFTER_MSG)
{
  assert(!str.empty());

#ifndef NDEBUG
  if (str[0] == ' ')
  {
    TRACE << "Message starts with space: \"" << str << "\"" << endl;
    assert(false);
  }
#endif

  //If frenzied, change message
  bool props[size_t(PropId::END)];
  Map::player->getPropHandler().getPropIds(props);

  if (props[size_t(PropId::frenzied)])
  {
    string frenziedStr = str;

    bool hasLowerCase = false;

    for (auto c : frenziedStr)
    {
      if (c >= 'a' && c <= 'z')
      {
        hasLowerCase = true;
        break;
      }
    }

    const char LAST           = frenziedStr.back();
    bool isEndedByPunctuation = LAST == '.' || LAST == '!';

    if (hasLowerCase && isEndedByPunctuation)
    {
      //Convert to upper case
      transform(begin(frenziedStr), end(frenziedStr), begin(frenziedStr), ::toupper);

      //Do not put "!" if string contains "..."
      if (frenziedStr.find("...") == string::npos)
      {
        //Change "." to "!" at the end
        if (frenziedStr.back() == '.')
        {
          frenziedStr.back() = '!';
        }

        //Add some "!"
        frenziedStr += "!!";
      }

      addMsg(frenziedStr, clr, INTERRUPT_PLAYER_ACTIONS, ADD_MORE_PROMPT_AFTER_MSG);

      return;
    }
  }

  int curLineNr = lines_[1].empty() ? 0 : 1;

  Msg* prevMsg = nullptr;

  if (!lines_[curLineNr].empty())
  {
    prevMsg = &lines_[curLineNr].back();
  }

  bool isRepeated = false;

  //Check if message is identical to previous
  if (!ADD_MORE_PROMPT_AFTER_MSG && prevMsg)
  {
    string prevStr = "";
    prevMsg->getStrRaw(prevStr);
    if (prevStr.compare(str) == 0)
    {
      prevMsg->incrRepeat();
      isRepeated = true;
    }
  }

  if (!isRepeated)
  {
    const int REPEAT_STR_LEN = 4;

    const int PADDING_LEN = REPEAT_STR_LEN + (curLineNr == 0 ? 0 : (moreStr.size() + 1));

    int xPos = getXAfterMsg(prevMsg);

    const bool IS_MSG_FIT = xPos + int(str.size()) + PADDING_LEN - 1 < MAP_W;

    if (!IS_MSG_FIT)
    {
      if (curLineNr == 0)
      {
        curLineNr = 1;
      }
      else
      {
        morePrompt();
        curLineNr = 0;
      }
      xPos = 0;
    }

    lines_[curLineNr].push_back(Msg(str, clr, xPos));
  }

  if (ADD_MORE_PROMPT_AFTER_MSG)
  {
    morePrompt();
  }

  //Messages may stop long actions like first aid and quick walk
  if (INTERRUPT_PLAYER_ACTIONS)
  {
    Map::player->interruptActions();
  }

  Map::player->onLogMsgPrinted();
}

void morePrompt()
{
  //If the current log is empty, do nothing
  if (lines_[0].empty())
  {
    return;
  }

  Render::drawMapAndInterface(false);

  drawLog(false);

  int xPos    = 0;
  int lineNr = lines_[1].empty() ? 0 : 1;

  if (!lines_[lineNr].empty())
  {
    Msg* const lastMsg = &lines_[lineNr].back();
    xPos = getXAfterMsg(lastMsg);
    if (lineNr == 0)
    {
      if (xPos + int(moreStr.size()) - 1 >= MAP_W)
      {
        xPos    = 0;
        lineNr  = 1;
      }
    }
  }

  Render::drawText(moreStr, Panel::log, Pos(xPos, lineNr), clrBlack, clrGray);

  Render::updateScreen();
  Query::waitForConfirm();
  clearLog();
}

void displayHistory()
{
  clearLog();

  const int LINE_JUMP           = 3;
  const int NR_LINES_TOT        = history_.size();
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

  int topNr = max(0, NR_LINES_TOT - MAX_NR_LINES_ON_SCR);
  int btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

  while (true)
  {
    Render::clearScreen();
    drawHistoryInterface(topNr, btmNr);
    int yPos = 1;
    for (int i = topNr; i <= btmNr; ++i)
    {
      drawLine(history_[i], yPos++);
    }
    Render::updateScreen();

    const KeyData& d = Input::getInput();
    if (d.key == '2' || d.sdlKey == SDLK_DOWN || d.key == 'j')
    {
      topNr += LINE_JUMP;
      if (NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
      {
        topNr = 0;
      }
      else
      {
        topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
      }
    }
    else if (d.key == '8' || d.sdlKey == SDLK_UP || d.key == 'k')
    {
      topNr = max(0, topNr - LINE_JUMP);
    }
    else if (d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE)
    {
      break;
    }
    btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);
  }

  Render::drawMapAndInterface();
}

void addLineToHistory(const string& lineToAdd)
{
  vector<Msg> historyLine;
  historyLine.push_back(Msg(lineToAdd, clrWhite, 0));
  history_.push_back(historyLine);
}

const vector< vector<Msg> >& getHistory()
{
  return history_;
}

} //Log
