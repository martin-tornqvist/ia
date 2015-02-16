#include "Manual.h"

#include "Init.h"

#include <fstream>
#include <vector>

#include "Input.h"
#include "TextFormat.h"
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

    if (!file.is_open())
    {
        return;
    }

    vector<string> formatted;

    while (getline(file, curLine))
    {
        if (curLine.empty())
        {
            lines_.push_back(curLine);
        }
        else //Current line not empty
        {
            //Do not format lines that start with two spaces
            bool shouldFormatLine = true;

            if (curLine.size() >= 2)
            {
                if (curLine[0] == ' ' && curLine[1] == ' ')
                {
                    shouldFormatLine = false;
                }
            }

            if (shouldFormatLine)
            {
                TextFormat::lineToLines(curLine, MAP_W, formatted);

                TRACE << "curLine: " << curLine << endl;

                for (const auto& line : formatted)
                {
                    lines_.push_back(line);
                }
            }
            else //Do not format line
            {
                lines_.push_back(curLine);
            }
        }
    }

    file.close();
}

void drawManualInterface()
{
    const string decorationLine(MAP_W, '-');

    const int   X_LABEL = 3;
    const auto  panel   = Panel::screen;

    Render::drawText(decorationLine, panel, Pos(0, 0), clrGray);
    Render::drawText(" Browsing the Tome of Wisdom ", panel, Pos(X_LABEL, 0), clrGray);
    Render::drawText(decorationLine, panel, Pos(0, SCREEN_H - 1), clrGray);
    Render::drawText(infoScrCmdInfo, panel, Pos(X_LABEL, SCREEN_H - 1), clrGray);
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
