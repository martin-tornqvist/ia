#ifndef TEXT_FORMATTING_H
#define TEXT_FORMATTING_H

#include <vector>
#include <string>

namespace text_format
{

//Reads a line of space separated words, and splits them into several lines with the
//given maximum width. If any single word in the "line" parameter is longer than the
//maximum width, we do not bother to split that word (the entire word is simply added
//to the output vector, breaking the maximum width).
void line_to_lines(std::string line, const int MAX_W, std::vector<std::string>& out);

void get_space_separated_list(const std::string& line, std::vector<std::string>& out);

void replace_all(const std::string& line, const std::string& from, const std::string& to,
                 std::string& out);

void first_to_lower(std::string& str);
void first_to_upper(std::string& str);
void all_to_upper(std::string& str);

} //Text_format

#endif
