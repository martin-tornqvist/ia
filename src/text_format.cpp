#include "text_format.hpp"

#include "init.hpp"

#include <algorithm>

namespace
{

// Reads and removes the first word of the string.
std::string read_and_remove_word(std::string& line)
{
        std::string str = "";

        for (auto it = begin(line); it != end(line); /* No increment */)
        {
                const char current_char = *it;

                line.erase(it);

                if (current_char == ' ')
                {
                        break;
                }

                str += current_char;
        }

        return str;
}

bool is_word_fit(const std::string& current_string,
                 const std::string& word_to_fit,
                 const size_t max_w)
{
        return (current_string.size() + word_to_fit.size() + 1) <= max_w;
}

} // namespace


namespace text_format
{

std::vector<std::string> split(std::string line, const int max_w)
{
        std::vector<std::string> result;

        if (line.empty())
        {
                return result;
        }

        std::string current_word = read_and_remove_word(line);

        if (line.empty())
        {
                result = {current_word};

                return result;
        }

        result.resize(1);

        result[0] = "";

        size_t current_row_idx = 0;

        while (!current_word.empty())
        {
                if (!is_word_fit(result[current_row_idx], current_word, max_w))
                {
                        // Current word did not fit on current line, make a new
                        // line
                        ++current_row_idx;

                        result.resize(current_row_idx + 1);

                        result[current_row_idx] = "";
                }

                // If this is not the first word on the current line, add a
                // space before the word
                if (!result[current_row_idx].empty())
                {
                        result[current_row_idx] += " ";
                }

                result[current_row_idx] += current_word;

                current_word = read_and_remove_word(line);
        }

        return result;
}

std::vector<std::string> space_separated_list(const std::string& line)
{
        std::vector<std::string> result;

        std::string current_line = "";

        for (char c : line)
        {
                if (c == ' ')
                {
                        result.push_back(current_line);

                        current_line = "";
                }
                else
                {
                        current_line += c;
                }
        }

        return result;
}

std::string replace_all(const std::string& line,
                        const std::string& from,
                        const std::string& to)
{
        std::string result;

        if (from.empty())
        {
                return result;
        }

        result = line;

        size_t start_pos = 0;

        while ((start_pos = result.find(from, start_pos)) != std::string::npos)
        {
                result.replace(start_pos, from.length(), to);

                //In case 'to' contains 'from', like replacing 'x' with 'yx'
                start_pos += to.length();
        }

        return result;
}

std::string pad_before_to(const std::string& str,
                          const size_t tot_w,
                          const char c)
{
        std::string result = str;

        if (tot_w > str.size())
        {
                result.insert(0, tot_w - result.size(), c);
        }

        return result;
}

std::string pad_after_to(const std::string& str,
                         const size_t tot_w,
                         const char c)
{
        std::string result = str;

        if (tot_w > result.size())
        {
                result.insert(result.size(), tot_w, c);
        }

        return result;
}

std::string first_to_lower(const std::string& str)
{
        std::string result = str;

        if (!result.empty())
        {
                result[0] = tolower(result[0]);
        }

        return result;
}

std::string first_to_upper(const std::string& str)
{
        std::string result = str;

        if (!result.empty())
        {
                result[0] = toupper(result[0]);
        }

        return result;
}

std::string all_to_upper(const std::string& str)
{
        std::string result = str;

        transform(begin(result), end(result), begin(result), ::toupper);

        return result;
}

} // text_format
