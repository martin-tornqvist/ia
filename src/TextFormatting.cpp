#include "TextFormatting.h"

using namespace std;

namespace {

//Reads and removes the first word of the string.
//It reads until ' ' or end of string. The ' '-char is also removed.
void readAndRemoveNextWord(string& line, string& nextWordToMk) {
  nextWordToMk = "";

  //Build a word until parameter string is empty,
  //or a a space character is found.
  for(int i = 0; i < int(line.size()); ++i) {
    const char CURRENT_CHARACTER = line.at(0);

    line.erase(line.begin());

    if(CURRENT_CHARACTER == ' ') {
      return;
    } else {
      nextWordToMk += CURRENT_CHARACTER;
    }

    i--;
  }
}

bool isWordFit(const string& curString, const string& wordToFit,
               const int MAX_W) {
  return int(curString.size() + wordToFit.size() + 1) <= MAX_W;
}

} //namespace


namespace TextFormatting {

void lineToLines(string line, const int MAX_W, vector<string>& linesRef) {
  linesRef.resize(0);

  if(line.empty()) return;

  int curRow = 0;

  string curWord;

  readAndRemoveNextWord(line, curWord);
  if(linesRef.empty()) {
    linesRef.resize(1);
    linesRef.front() = "";
  }

  while(!curWord.empty()) {
    if(!isWordFit(linesRef.at(curRow), curWord, MAX_W)) {
      //Current word did not fit on current line, make a new line
      curRow++;
      linesRef.resize(curRow + 1);
      linesRef.at(curRow) = "";
    }

    //If this is not the first word on the current line,
    //add a space before the word
    if(!linesRef.at(curRow).empty()) {linesRef.at(curRow) += " ";}

    linesRef.at(curRow) += curWord;

    readAndRemoveNextWord(line, curWord);
  }
}

void getSpaceSeparatedList(const string& line, vector<string>& linesRef) {
  string curLine = "";
  for(char c : line) {
    if(c == ' ') {
      linesRef.push_back(curLine);
      curLine = "";
    } else {
      curLine += c;
    }
  }
}

void replaceAll(const std::string& line, const std::string& from,
                const std::string& to, std::string& resultRef) {
  if(from.empty()) {return;}

  resultRef = line;

  size_t startPos = 0;
  while((startPos = resultRef.find(from, startPos)) != std::string::npos) {
    resultRef.replace(startPos, from.length(), to);
    //In case 'to' contains 'from', like replacing 'x' with 'yx'
    startPos += to.length();
  }
}

} //TextFormatting

