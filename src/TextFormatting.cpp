#include "TextFormatting.h"

#include "Engine.h"

void TextFormatting::lineToLines(string line, const int MAX_WIDTH,
                                 vector<string>& linesRef) const {
  linesRef.resize(0);

  int curRow = 0;

  string curWord;

  readAndRemoveNextWord(line, curWord);

  while(curWord.empty() == false) {
    if(linesRef.empty()) {
      linesRef.resize(1);
      linesRef.front() = "";
    }

    if(isWordFit(linesRef.at(curRow), curWord, MAX_WIDTH) == false) {
      //Current word did not fit on current line, make a new line
      curRow++;
      linesRef.resize(curRow + 1);
      linesRef.at(curRow) = "";
    }

    //If this is not the first word on the current line,
    //add a space before the word
    if(linesRef.at(curRow).empty() == false) {
      linesRef.at(curRow) += " ";
    }

    linesRef.at(curRow) += curWord;

    readAndRemoveNextWord(line, curWord);
  }
}

//Reads and removes the first word of the string.
//It reads until ' ' or end of string. The ' '-char is also removed.
void TextFormatting::readAndRemoveNextWord(
  string& line, string& nextWordToMake) const {
  nextWordToMake = "";

  //Build a word until parameter string is empty,
  //or a a space character is found.
  for(int i = 0; i < int(line.size()); i++) {
    const char CURRENT_CHARACTER = line.at(0);

    line.erase(line.begin());

    if(CURRENT_CHARACTER == ' ') {
      return;
    } else {
      nextWordToMake += CURRENT_CHARACTER;
    }

    i--;
  }
}

bool TextFormatting::isWordFit(string& currentString, const string& wordToFit,
                               const unsigned int MAX_WIDTH) const {

  return currentString.size() + wordToFit.size() + 1 <= MAX_WIDTH;
}

void TextFormatting::getSpaceSeparatedList(
  string line, vector<string>& linesRef) const {

  string curLine = "";

  for(unsigned int i = 0; i < line.size(); i++) {
    char character = line.at(i);
    if(character == ' ') {
      linesRef.push_back(curLine);
      curLine = "";
    } else {
      curLine += character;
    }
  }
}



