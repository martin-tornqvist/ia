#ifndef TEXT_FORMATTING_H
#define TEXT_FORMATTING_H

/*---------------------------------------------------------
Purpose: This class came about from the need to break rows
correctly when typing out large ammounts of text. It was
a lot of unnecessary work to format the lines manually.
The class can take a string and a max width (nr of
characters), and return a vector of lines with nice line
breaks.
-----------------------------------------------------------*/

class Engine;

#include <vector>
#include <string>

using namespace std;

class TextFormatting {
public:
  TextFormatting() {}
  ~TextFormatting() {}

  vector<string> lineToLines(string line, const int MAX_WIDTH) const;

  vector<string> getSpaceSeparatedList(string line) const;

private:
  string readNextWord(string& line) const;
  bool isWordFit(string& currentString, const string& newWord, const unsigned int MAX_WIDTH) const;
};

#endif
