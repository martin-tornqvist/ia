#ifndef TEXT_FORMATTING_HPP
#define TEXT_FORMATTING_HPP

#include <vector>
#include <string>

namespace text_format
{

// Reads a line of space separated words, and splits them into several lines
// with the given maximum width. If any single word in the "line" parameter is
// longer than the maximum width, we do not bother to split that word (the
// entire word is simply added to the output vector, breaking the maximum width).
std::vector<std::string> split(std::string line, const int max_w);

std::vector<std::string> space_separated_list(const std::string& line);

std::string replace_all(const std::string& line,
                        const std::string& from,
                        const std::string& to);

std::string pad_before_to(const std::string& str,
                          const size_t tot_w,
                          const char c = ' ');

std::string pad_after_to(const std::string& str,
                         const size_t tot_w,
                         const char c = ' ');

std::string first_to_lower(const std::string& str);
std::string first_to_upper(const std::string& str);
std::string all_to_upper(const std::string& str);

} // text_format

#endif // TEXT_FORMATTING_HPP
