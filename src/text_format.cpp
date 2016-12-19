#include "text_format.hpp"

#include "init.hpp"

#include <algorithm>

namespace
{

// Reads and removes the first word of the string.
void read_and_remove_word(std::string& line, std::string& word_ref)
{
    word_ref = "";

    for (auto it = begin(line); it != end(line); /* No increment */)
    {
        const char current_char = *it;

        line.erase(it);

        if (current_char == ' ')
        {
            break;
        }

        word_ref += current_char;
    }
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

void split(std::string line,
           const int max_w,
           std::vector<std::string>& out)
{
    out.clear();

    if (line.empty())
    {
        return;
    }

    std::string current_word = "";

    read_and_remove_word(line, current_word);

    if (line.empty())
    {
        out = {current_word};
        return;
    }

    out.resize(1);
    out[0] = "";

    size_t current_row_idx = 0;

    while (!current_word.empty())
    {
        if (!is_word_fit(out[current_row_idx], current_word, max_w))
        {
            // Current word did not fit on current line, make a new line
            ++current_row_idx;
            out.resize(current_row_idx + 1);
            out[current_row_idx] = "";
        }

        // If this is not the first word on the current line, add a space before
        // the word
        if (!out[current_row_idx].empty())
        {
            out[current_row_idx] += " ";
        }

        out[current_row_idx] += current_word;

        read_and_remove_word(line, current_word);
    }
}

void space_separated_list(const std::string& line,
                          std::vector<std::string>& out)
{
    std::string current_line = "";

    for (char c : line)
    {
        if (c == ' ')
        {
            out.push_back(current_line);
            current_line = "";
        }
        else
        {
            current_line += c;
        }
    }
}

void replace_all(const std::string& line,
                 const std::string& from,
                 const std::string& to,
                 std::string& out)
{
    if (from.empty())
    {
        return;
    }

    out = line;

    size_t start_pos = 0;

    while ((start_pos = out.find(from, start_pos)) != std::string::npos)
    {
        out.replace(start_pos, from.length(), to);
        //In case 'to' contains 'from', like replacing 'x' with 'yx'
        start_pos += to.length();
    }
}

void pad_before_to(std::string& str,
                   const size_t tot_w,
                   const char c)
{
    if (tot_w > str.size())
    {
        str.insert(0, tot_w - str.size(), c);
    }
}

void pad_after_to(std::string& str,
                  const size_t tot_w,
                  const char c)
{
    if (tot_w > str.size())
    {
        str.insert(str.size(), tot_w, c);
    }
}

void first_to_lower(std::string& str)
{
    if (!str.empty())
    {
        str[0] = tolower(str[0]);
    }
}

void first_to_upper(std::string& str)
{
    if (!str.empty())
    {
        str[0] = toupper(str[0]);
    }
}

void all_to_upper(std::string& str)
{
    transform(begin(str), end(str), begin(str), ::toupper);
}

} // text_format
