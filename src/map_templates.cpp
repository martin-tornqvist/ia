#include "map_templates.hpp"

#include <fstream>
#include <map>

namespace map_templates
{

namespace
{

Array2<char> level_templates_[(size_t)LevelTemplId::END];

std::map<std::string, LevelTemplId> name_2_level_id_;


void load_level_templates()
{
    std::ifstream levels_ifs("data/map/levels.txt");

    ASSERT(!levels_ifs.fail());
    ASSERT(levels_ifs.is_open());

    const char comment_symbol   = '\'';
    const char name_symbol      = '$';

    // Space and tab
    const std::string whitespace_chars = " \t";

    LevelTemplId current_id = LevelTemplId::END;

    std::vector<std::string> template_buffer;

    std::vector<std::string> lines_read;

    for (std::string line; std::getline(levels_ifs, line); )
    {
        lines_read.push_back(line);
    }

    for (size_t line_idx = 0; line_idx < lines_read.size(); ++line_idx)
    {
        std::string& line = lines_read[line_idx];

        bool try_finalize = false;

        if (line.empty())
        {
            try_finalize = true;
        }

        if (!try_finalize)
        {
            // Check if this is a whitespace-only line or a comment line
            const size_t first_non_space =
                line.find_first_not_of(whitespace_chars);

            if (first_non_space == std::string::npos ||
                line[first_non_space] == comment_symbol)
            {
                try_finalize = true;
            }
        }

        if (!try_finalize)
        {
            // Trim trailing whitespace
            const size_t end_pos =
                line.find_last_not_of(whitespace_chars);

            line = line.substr(0, end_pos + 1);
        }

        if (!try_finalize)
        {
            // Is this line a template name?
            if (line[0] == name_symbol)
            {
                const size_t name_pos = 2;

                const std::string name =
                    line.substr(name_pos, line.size() - name_pos);

                auto id_it = name_2_level_id_.find(name);

                if (id_it == name_2_level_id_.end())
                {
                    TRACE << "Unrecognized level template name: "
                          << name << std::endl;

                    ASSERT(false);
                }

                current_id = id_it->second;
            }
            else // Not a name line
            {
                // This must be a template line
                template_buffer.push_back(line);
            }
        }

        // Is this the last line? Then we should try finalizing the template
        if (line_idx == (lines_read.size() - 1))
        {
            try_finalize = true;
        }

        // Is the current template done?
        if (try_finalize                    &&
            current_id != LevelTemplId::END &&
            !template_buffer.empty())
        {
            ASSERT(template_buffer.size() == map_h);

            // Not all lines in a template needs to be the same length, so we
            // to find the length of the longest line
            size_t max_len = 0;

            std::for_each(begin(template_buffer),
                          end(template_buffer),
                          [&max_len](std::string& buffer_line)
            {
                if (buffer_line.size() > max_len)
                {
                    max_len = buffer_line.size();
                }
            });

            ASSERT(max_len == map_w);

            auto& templ = level_templates_[(size_t)current_id];

            templ.resize(max_len, template_buffer.size());

            for (size_t buffer_idx = 0;
                 buffer_idx < template_buffer.size();
                 ++buffer_idx)
            {
                std::string& buffer_line = template_buffer[buffer_idx];

                // Pad the buffer line with space characters
                buffer_line.append(max_len - buffer_line.size(), ' ');

                // Fill the template araray with the buffer characters
                for (size_t line_idx = 0;
                     line_idx < max_len;
                     ++line_idx)
                {
                    templ(line_idx, buffer_idx) = buffer_line[line_idx];
                }
            }

            current_id = LevelTemplId::END;

            template_buffer.clear();
        }

    } // for

} // load_level_templates

} // namespace

void init()
{
    name_2_level_id_.clear();

    name_2_level_id_["Intro forest"] =
        LevelTemplId::intro_forest;

    name_2_level_id_["Egypt"] =
        LevelTemplId::egypt;

    name_2_level_id_["Leng"] =
        LevelTemplId::leng;

    name_2_level_id_["Rat cave"] =
        LevelTemplId::rat_cave;

    name_2_level_id_["Boss level"] =
        LevelTemplId::boss_level;

    name_2_level_id_["Trapezohedron level"] =
        LevelTemplId::trapez_level;

    load_level_templates();
}

const Array2<char>& level_templ(LevelTemplId id)
{
    ASSERT(id != LevelTemplId::END);

    return level_templates_[(size_t)id];
}

} // map_templates
