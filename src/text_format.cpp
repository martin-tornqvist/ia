#include "text_format.hpp"

#include "init.hpp"

#include <algorithm>

using namespace std;

namespace
{

//Reads and removes the first word of the string.
void read_and_remove_word(string& line, string& word_ref)
{
    word_ref = "";

    for (auto it = begin(line); it != end(line); /* No increment */)
    {
        const char CUR_CHAR = *it;

        line.erase(it);

        if (CUR_CHAR == ' ')
        {
            break;
        }

        word_ref += CUR_CHAR;
    }
}

bool is_word_fit(const string& cur_string, const string& word_to_fit, const size_t MAX_W)
{
    return (cur_string.size() + word_to_fit.size() + 1) <= MAX_W;
}

} //namespace


namespace text_format
{

void line_to_lines(string line, const int MAX_W, vector<string>& out)
{
    out.clear();

    if (line.empty())
    {
        return;
    }

    string cur_word = "";

    read_and_remove_word(line, cur_word);

    if (line.empty())
    {
        out = {cur_word};
        return;
    }

    out.resize(1);
    out[0] = "";

    size_t cur_row_idx = 0;

    while (!cur_word.empty())
    {
        if (!is_word_fit(out[cur_row_idx], cur_word, MAX_W))
        {
            //Current word did not fit on current line, make a new line
            ++cur_row_idx;
            out.resize(cur_row_idx + 1);
            out[cur_row_idx] = "";
        }

        //If this is not the first word on the current line, add a space before the word
        if (!out[cur_row_idx].empty())
        {
            out[cur_row_idx] += " ";
        }

        out[cur_row_idx] += cur_word;

        read_and_remove_word(line, cur_word);
    }
}

void get_space_separated_list(const string& line, vector<string>& out)
{
    string cur_line = "";

    for (char c : line)
    {
        if (c == ' ')
        {
            out.push_back(cur_line);
            cur_line = "";
        }
        else
        {
            cur_line += c;
        }
    }
}

void replace_all(const string& line, const string& from, const string& to, string& out)
{
    if (from.empty())
    {
        return;
    }

    out = line;

    size_t start_pos = 0;

    while ((start_pos = out.find(from, start_pos)) != string::npos)
    {
        out.replace(start_pos, from.length(), to);
        //In case 'to' contains 'from', like replacing 'x' with 'yx'
        start_pos += to.length();
    }
}

void first_to_lower(std::string& str)
{
    if (!str.empty())
    {
        str[0] = tolower(str[0]);
    }
}

void first_to_upper(string& str)
{
    if (!str.empty())
    {
        str[0] = toupper(str[0]);
    }
}

void all_to_upper(string& str)
{
    transform(begin(str), end(str), begin(str), ::toupper);
}

} //Text_format

