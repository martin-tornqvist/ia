#include "TextFormatting.h"

#include "Engine.h"

vector<string> TextFormatting::lineToLines(string line, const int MAX_WIDTH) const {
  vector<string> lines;
  lines.resize(0);

  int curRow = 0;

  string curWord = readNextWord(line);

  while(curWord != "") {
    if(lines.empty()) {
      lines.resize(1);
      lines.front() = "";
    }

    if(isWordFit(lines.at(curRow), curWord, MAX_WIDTH) == false) {
      curRow++;
      lines.resize(curRow + 1);
      lines.at(curRow) = "";
    }

    lines.at(curRow) += curWord + " ";

    curWord = readNextWord(line);
  }

  return lines;
}

//Reads and removes the first word of the string. It reads until ' '-char or end of string.
//The ' '-char is also removed.
string TextFormatting::readNextWord(string& line) const {
  string returnWord = "";

  //Build a word until parameter string is empty, or a a space character is found.
  for(unsigned int i = 0; i < line.size(); i++) {
    const char CURRENT_CHARACTER = line.at(0);

    line.erase(line.begin());

    if(CURRENT_CHARACTER == ' ') {
      return returnWord;
    } else {
      returnWord += CURRENT_CHARACTER;
    }

    i--;
  }

  return returnWord;
}

bool TextFormatting::isWordFit(string& currentString, const string& newWord, const unsigned int MAX_WIDTH) const {
  return currentString.size() + newWord.size() + 1 <= MAX_WIDTH;
}

vector<string> TextFormatting::getSpaceSeparatedList(string line) const {
  string curLine = "";
  vector<string> ret;
  for(unsigned int i = 0; i < line.size(); i++) {
    char c = line.at(i);
    if(c == ' ') {
      ret.push_back(curLine);
      curLine = "";
    } else {
      curLine += c;
    }
  }
  return ret;
}



