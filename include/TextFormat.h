#ifndef TEXT_FORMATTING_H
#define TEXT_FORMATTING_H

#include <vector>
#include <string>

namespace TextFormat
{

void lineToLines(std::string line, const int MAX_W, std::vector<std::string>& linesRef);

void getSpaceSeparatedList(const std::string& line, std::vector<std::string>& linesRef);

void replaceAll(const std::string& line, const std::string& from, const std::string& to,
                std::string& resultRef);

void firstToLower(std::string& str);
void firstToUpper(std::string& str);
void allToUpper  (std::string& str);

} //TextFormat

#endif
