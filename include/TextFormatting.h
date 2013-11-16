#ifndef TEXT_FORMATTING_H
#define TEXT_FORMATTING_H

class Engine;

#include <vector>
#include <string>

using namespace std;

class TextFormatting {
public:
  TextFormatting() {}
  ~TextFormatting() {}

  void lineToLines(string line, const int MAX_WIDTH,
                   vector<string>& linesToMake) const;

  void getSpaceSeparatedList(string line, vector<string>& linesToMake) const;

private:
  void readAndRemoveNextWord(string& line, string& nextWordToMake) const;

  bool isWordFit(string& currentString, const string& wordToFit,
                 const unsigned int MAX_WIDTH) const;
};

#endif
