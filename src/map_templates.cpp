#include "map_templates.hpp"

#include "rl_utils.hpp"
#include "saving.hpp"

#include <fstream>
#include <unordered_map>

namespace map_templates
{

namespace
{

Array2<char> level_templates_[(size_t)LevelTemplId::END];

std::vector<RoomTempl> room_templates_;

std::vector<RoomTemplStatus> room_templ_status_;

// Space and tab
const std::string whitespace_chars = " \t";

const char comment_symbol = '\'';


// Checks if this line has non-whitespace characters, and is not commented out
bool line_has_content(const std::string& line)
{
    const size_t first_non_space =
        line.find_first_not_of(whitespace_chars);

    return
        (first_non_space != std::string::npos) &&
        (line[first_non_space] != comment_symbol);
}

void trim_trailing_whitespace(std::string& line)
{
    const size_t end_pos =
        line.find_last_not_of(whitespace_chars);

    line = line.substr(0, end_pos + 1);
}

size_t max_length(const std::vector<std::string>& lines)
{
    size_t max_len = 0;

    std::for_each(begin(lines),
                  end(lines),
                  [&max_len](const std::string& buffer_line)
    {
        if (buffer_line.size() > max_len)
        {
            max_len = buffer_line.size();
        }
    });

    return max_len;
}

void load_level_templates()
{
    TRACE_FUNC_BEGIN;

    std::ifstream ifs("res/data/map/levels.txt");

    ASSERT(!ifs.fail());
    ASSERT(ifs.is_open());

    std::unordered_map<std::string, LevelTemplId> name_to_level_id;

    name_to_level_id["Intro forest"] = LevelTemplId::intro_forest;
    name_to_level_id["Egypt"] = LevelTemplId::egypt;
    name_to_level_id["Leng"] = LevelTemplId::leng;
    name_to_level_id["Rat cave"] = LevelTemplId::rat_cave;
    name_to_level_id["Boss level"] = LevelTemplId::boss_level;
    name_to_level_id["Trapezohedron level"] = LevelTemplId::trapez_level;

    const char name_symbol = '$';

    LevelTemplId current_id = LevelTemplId::END;

    std::vector<std::string> template_buffer;

    std::vector<std::string> lines_read;

    for (std::string line; std::getline(ifs, line); /* No increment */)
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
            if (!line_has_content(line))
            {
                try_finalize = true;
            }
        }

        if (!try_finalize)
        {
            trim_trailing_whitespace(line);

            // Is this line a template name?
            if (line[0] == name_symbol)
            {
                const size_t name_pos = 2;

                const std::string name_str =
                    line.substr(name_pos, line.size() - name_pos);

                auto id_it = name_to_level_id.find(name_str);

                if (id_it == name_to_level_id.end())
                {
                    TRACE << "Unrecognized level template name: "
                          << name_str << std::endl;

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
        if (try_finalize &&
            current_id != LevelTemplId::END &&
            !template_buffer.empty())
        {
            ASSERT(template_buffer.size() == map_h);

            // Not all lines in a template needs to be the same length, so we
            // to find the length of the longest line
            const size_t max_len = max_length(template_buffer);

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

    TRACE_FUNC_END;

} // load_level_templates


//
// Add all combinations of rotating and flipping the template
//
void mk_room_templ_variants(RoomTempl& templ)
{
    auto& symbols = templ.symbols;

    //
    // "Up"
    //
    room_templates_.push_back(templ);

    symbols.flip_hor();

    room_templates_.push_back(templ);

    //
    // "Right"
    //
    symbols.rotate_cw();

    room_templates_.push_back(templ);

    symbols.flip_ver();

    room_templates_.push_back(templ);

    //
    // "Down"
    //
    symbols.rotate_cw();

    room_templates_.push_back(templ);

    symbols.flip_hor();

    room_templates_.push_back(templ);

    //
    // "Left"
    //
    symbols.rotate_cw();

    room_templates_.push_back(templ);

    symbols.flip_ver();

    room_templates_.push_back(templ);

} // mk_room_templ_variants


void load_room_templates()
{
    TRACE_FUNC_BEGIN;

    room_templates_.clear();

    room_templ_status_.clear();

    std::unordered_map<std::string, RoomType> name_to_room_type;

    name_to_room_type["plain"] = RoomType::plain;
    name_to_room_type["human"] = RoomType::human;
    name_to_room_type["ritual"] = RoomType::ritual;
    name_to_room_type["jail"] = RoomType::jail;
    name_to_room_type["spider"] = RoomType::spider;
    name_to_room_type["snake_pit"] = RoomType::snake_pit;
    name_to_room_type["crypt"] = RoomType::crypt;
    name_to_room_type["monster"] = RoomType::monster;
    name_to_room_type["flooded"] = RoomType::flooded;
    name_to_room_type["muddy"] = RoomType::muddy;
    name_to_room_type["cave"] = RoomType::cave;
    name_to_room_type["chasm"] = RoomType::chasm;
    name_to_room_type["forest"] = RoomType::forest;

    std::ifstream ifs("res/data/map/rooms.txt");

    ASSERT(!ifs.fail());
    ASSERT(ifs.is_open());

    std::vector<std::string> template_buffer;

    std::vector<std::string> lines_read;

    for (std::string line; std::getline(ifs, line); /* No increment */)
    {
        lines_read.push_back(line);
    }

    const char type_symbol = '$';

    RoomTempl templ;

    size_t current_base_templ_idx = 0;

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
            if (!line_has_content(line))
            {
                try_finalize = true;
            }
        }

        if (!try_finalize)
        {
            trim_trailing_whitespace(line);

            // Is this line a room type?
            if (line[0] == type_symbol)
            {
                const size_t type_pos = 2;

                const std::string type_str =
                    line.substr(type_pos, line.size() - type_pos);

                auto type_it = name_to_room_type.find(type_str);

                if (type_it == name_to_room_type.end())
                {
                    TRACE << "Unrecognized room template type: "
                          << type_str << std::endl;

                    ASSERT(false);
                }

                templ.type = type_it->second;
            }
            else // Not a name line
            {
                template_buffer.push_back(line);
            }
        }

        // Is this the last line? Then we should try finalizing the template
        if (line_idx == (lines_read.size() - 1))
        {
            try_finalize = true;
        }

        // Is the current template done?
        if (try_finalize &&
            !template_buffer.empty())
        {
            // Not all lines in a template needs to be the same length, so we
            // have to find the length of the longest line
            const size_t max_len = max_length(template_buffer);

            templ.symbols.resize(max_len, template_buffer.size());

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
                    templ.symbols(line_idx, buffer_idx) = buffer_line[line_idx];
                }
            }

            templ.base_templ_idx = current_base_templ_idx;

            mk_room_templ_variants(templ);

            template_buffer.clear();

            ++current_base_templ_idx;
        }

    } // for

    room_templ_status_.resize(current_base_templ_idx, RoomTemplStatus::unused);

    TRACE << "Number of room templates loaded from template file: "
          << current_base_templ_idx
          << std::endl
          << "Total variants: " << room_templates_.size()
          << std::endl;

    TRACE_FUNC_END;
}

} // namespace

void init()
{
    TRACE_FUNC_BEGIN;

    load_level_templates();

    load_room_templates();

    TRACE_FUNC_END;
}

void save()
{
    TRACE_FUNC_BEGIN;

    for (auto status : room_templ_status_)
    {
        saving::put_int((int)status);
    }

    TRACE_FUNC_END;
}

void load()
{
    TRACE_FUNC_BEGIN;

    for (size_t i = 0; i < room_templ_status_.size(); ++i)
    {
        room_templ_status_[i] = (RoomTemplStatus)saving::get_int();
    }

    TRACE_FUNC_END;
}

const Array2<char>& level_templ(LevelTemplId id)
{
    ASSERT(id != LevelTemplId::END);

    return level_templates_[(size_t)id];
}

RoomTempl* random_room_templ(const P& max_dims)
{
    ASSERT(!room_templates_.empty());

    std::vector<RoomTempl*> bucket;

    for (auto& templ : room_templates_)
    {
        const auto status = room_templ_status_[templ.base_templ_idx];

        if (status != RoomTemplStatus::unused)
        {
            continue;
        }

        const P templ_dims(templ.symbols.dims());

        if (templ_dims.x <= max_dims.x &&
            templ_dims.y <= max_dims.y)
        {
            bucket.push_back(&templ);
        }
    }

    TRACE << "Attempting to find valid room template, for max dimensions: "
          << max_dims.x << "x" << max_dims.y
          << std::endl
          << "Number of candidates found: " << bucket.size()
          << std::endl;

    if (bucket.empty())
    {
        return nullptr;
    }

    const size_t idx = rnd::range(0, bucket.size() - 1);

    return bucket[idx];
}

void clear_base_room_templates_used()
{
    std::fill(begin(room_templ_status_),
              end(room_templ_status_),
              RoomTemplStatus::unused);
}

void on_base_room_template_placed(const RoomTempl& templ)
{
    ASSERT(templ.base_templ_idx < room_templ_status_.size());

    ASSERT(room_templ_status_[templ.base_templ_idx] == RoomTemplStatus::unused);

    room_templ_status_[templ.base_templ_idx] = RoomTemplStatus::placed;
}

void on_map_discarded()
{
    // "Placed" -> "unused"
    for (size_t i = 0; i < room_templ_status_.size(); ++i)
    {
        auto& status = room_templ_status_[i];

        if (status == RoomTemplStatus::placed)
        {
            status = RoomTemplStatus::unused;
        }
    }
}

void on_map_ok()
{
    // "Placed" -> "used"
    for (size_t i = 0; i < room_templ_status_.size(); ++i)
    {
        auto& status = room_templ_status_[i];

        if (status == RoomTemplStatus::placed)
        {
            status = RoomTemplStatus::used;
        }
    }
}

} // map_templates
