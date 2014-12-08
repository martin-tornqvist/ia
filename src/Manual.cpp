#include "Manual.h"

#include "Init.h"

#include <fstream>
#include <vector>

#include "Input.h"
#include "TextFormatting.h"
#include "Render.h"

using namespace std;

namespace Manual
{

namespace
{

vector<string> lines_;

void readFile()
{
  string curLine;
  ifstream file("manual.txt");

  vector<string> formatted;

  if (file.is_open())
  {
    while (getline(file, curLine))
    {
      if (curLine.empty())
      {
        lines_.push_back(curLine);
      }
      else
      {
        //Do not format lines that start with two spaces
        bool shouldFormatLine = true;
        if (curLine.size() > 1)
        {
          if (curLine[0] == ' ' && curLine[1] == ' ')
          {
            shouldFormatLine = false;
          }
        }
        if (shouldFormatLine)
        {
          TextFormatting::lineToLines(curLine, MAP_W - 3, formatted);
          for (const auto& line : formatted)
          {
            lines_.push_back(line);
          }
        }
        else
        {
          curLine.erase(curLine.begin());
          lines_.push_back(curLine);
        }
      }
    }
  }

  file.close();
}

void drawManualInterface()
{
  const string decorationLine(MAP_W, '-');

  const int X_LABEL = 3;

  Render::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  Render::drawText(" Displaying manual ", Panel::screen, Pos(X_LABEL, 0), clrGray);

  Render::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1), clrGray);

  Render::drawText(infoScrCmdInfo, Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrGray);
}

} //namespace

void init()
{
  readFile();
}

void run()
{
  const int LINE_JUMP           = 3;
  const int NR_LINES_TOT        = lines_.size();
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

  int topNr = 0;
  int btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

  while (true)
  {
    Render::clearScreen();
    drawManualInterface();
    int yPos = 1;
    for (int i = topNr; i <= btmNr; ++i)
    {
      Render::drawText(lines_[i], Panel::screen, Pos(0, yPos++),
                       clrWhite);
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
}

} //Manual
