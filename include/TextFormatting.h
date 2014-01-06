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

  void lineToLines(string line, const int MAX_W,
                   vector<string>& linesRef) const;

  void getSpaceSeparatedList(string line, vector<string>& linesRef) const;

private:
  void readAndRemoveNextWord(string& line, string& nextWordToMake) const;

  bool isWordFit(string& currentString, const string& wordToFit,
                 const unsigned int MAX_W) const;
};

#endif
