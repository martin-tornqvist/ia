#ifndef TEXT_FORMATTING_H
#define TEXT_FORMATTING_H

#include <vector>
#include <string>

namespace TextFormat
{

//Reads a line of space separated words, and splits them into several lines with the
//given maximum width. If any single word in the "line" parameter is longer than the
//maximum width, we do not bother to split that word (the entire word is simply added
//to the output vector, breaking the maximum width).
void lineToLines(std::string line, const int MAX_W, std::vector<std::string>& out);

void getSpaceSeparatedList(const std::string& line, std::vector<std::string>& out);

void replaceAll(const std::string& line, const std::string& from, const std::string& to,
                std::string& out);

void firstToLower(std::string& str);
void firstToUpper(std::string& str);
void allToUpper  (std::string& str);

} //TextFormat

#endif
