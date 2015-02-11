#include "TextFormat.h"

#include "Init.h"

#include <algorithm>

using namespace std;

namespace
{

//Reads and removes the first word of the string.
void readAndRemoveWord(string& line, string& wordRef)
{
    wordRef = "";

    for (auto it = begin(line); it != end(line); /* No increment */)
    {
        const char CUR_CHAR = *it;

        line.erase(it);

        if (CUR_CHAR == ' ')
        {
            break;
        }

        wordRef += CUR_CHAR;
    }
}

bool isWordFit(const string& curString, const string& wordToFit, const size_t MAX_W)
{
    return (curString.size() + wordToFit.size() + 1) <= MAX_W;
}

} //namespace


namespace TextFormat
{

void lineToLines(string line, const int MAX_W, vector<string>& out)
{
    out.clear();

    if (line.empty())
    {
        return;
    }

    string curWord = "";

    readAndRemoveWord(line, curWord);

    if (line.empty())
    {
        out = {curWord};
        return;
    }

    out.resize(1);
    out[0] = "";

    size_t curRowIdx = 0;

    while (!curWord.empty())
    {
        if (!isWordFit(out[curRowIdx], curWord, MAX_W))
        {
            //Current word did not fit on current line, make a new line
            ++curRowIdx;
            out.resize(curRowIdx + 1);
            out[curRowIdx] = "";
        }

        //If this is not the first word on the current line, add a space before the word
        if (!out[curRowIdx].empty())
        {
            out[curRowIdx] += " ";
        }

        out[curRowIdx] += curWord;

        readAndRemoveWord(line, curWord);
    }
}

void getSpaceSeparatedList(const string& line, vector<string>& out)
{
    string curLine = "";
    for (char c : line)
    {
        if (c == ' ')
        {
            out.push_back(curLine);
            curLine = "";
        }
        else
        {
            curLine += c;
        }
    }
}

void replaceAll(const string& line, const string& from, const string& to, string& out)
{
    if (from.empty())
    {
        return;
    }

    out = line;

    size_t startPos = 0;
    while ((startPos = out.find(from, startPos)) != string::npos)
    {
        out.replace(startPos, from.length(), to);
        //In case 'to' contains 'from', like replacing 'x' with 'yx'
        startPos += to.length();
    }
}

void firstToLower(std::string& str)
{
    if (!str.empty())
    {
        str[0] = tolower(str[0]);
    }
}

void firstToUpper(string& str)
{
    if (!str.empty())
    {
        str[0] = toupper(str[0]);
    }
}

void allToUpper(string& str)
{
    transform(begin(str), end(str), begin(str), ::toupper);
}

} //TextFormat

