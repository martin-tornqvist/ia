#include "saving.hpp"

#include <fstream>
#include <iostream>

#include "init.hpp"
#include "msg_log.hpp"
#include "io.hpp"
#include "actor_player.hpp"
#include "game.hpp"
#include "item_scroll.hpp"
#include "item_potion.hpp"
#include "item_rod.hpp"
#include "player_bon.hpp"
#include "map.hpp"
#include "map_travel.hpp"
#include "inventory.hpp"
#include "game_time.hpp"
#include "player_spells.hpp"
#include "item_amulet.hpp"
#include "postmortem.hpp"
#include "insanity.hpp"
#include "map_templates.hpp"

namespace saving
{

namespace
{

#ifndef NDEBUG

// Only used to verify that the put/get methods are not called at the wrong time
enum class State
{
    saving,
    loading,
    stopped
};

State state_;

#endif // NDEBUG

std::vector<std::string> lines_;

void save_modules()
{
    TRACE_FUNC_BEGIN;

    ASSERT(lines_.empty());

    put_str(map::player->name_a());

    game::save();
    scroll_handling::save();
    potion_handling::save();
    rod_handling::save();
    item_data::save();
    amulet_handling::save();
    map::player->inv().save();
    map::player->save();
    insanity::save();
    player_bon::save();
    map_travel::save();
    map::save();
    actor_data::save();
    game_time::save();
    player_spells::save();

    TRACE_FUNC_END;
}

void load_modules()
{
    TRACE_FUNC_BEGIN;

    ASSERT(!lines_.empty());

    const std::string player_name = get_str();

    ASSERT(!player_name.empty());

    map::player->data().name_a      = player_name;
    map::player->data().name_the    = player_name;

    game::load();
    scroll_handling::load();
    potion_handling::load();
    rod_handling::load();
    item_data::load();
    amulet_handling::load();
    map::player->inv().load();
    map::player->load();
    insanity::load();
    player_bon::load();
    map_travel::load();
    map::load();
    actor_data::load();
    game_time::load();
    player_spells::load();

    TRACE_FUNC_END;
}

void write_file()
{
    std::ofstream file;

    file.open("data/save", std::ios::trunc);

    if (file.is_open())
    {
        for (size_t i = 0; i < lines_.size(); ++i)
        {
            file << lines_[i];

            if (i != lines_.size() - 1)
            {
                file << std::endl;
            }
        }

        file.close();
    }
}

void read_file()
{
    std::ifstream file("data/save");

    if (file.is_open())
    {
        std::string current_line = "";

        while (getline(file, current_line))
        {
            lines_.push_back(current_line);
        }

        file.close();
    }
    else //Could not open save file
    {
        ASSERT(false && "Failed to open save file");
    }
}

} // namespace

void init()
{
    lines_.clear();

#ifndef NDEBUG
    state_ = State::stopped;
#endif // NDEBUG
}

void save_game()
{
#ifndef NDEBUG
    ASSERT(state_ == State::stopped);
    ASSERT(lines_.empty());

    state_ = State::saving;
#endif // NDEBUG

    // Tell all modules to append to the save lines (via this modules store
    // functions)
    save_modules();

#ifndef NDEBUG
    state_ = State::stopped;
#endif // NDEBUG

    // Write the save lines to the save file
    write_file();

    lines_.clear();
}

void load_game()
{
#ifndef NDEBUG
    ASSERT(state_ == State::stopped);
    ASSERT(lines_.empty());

    state_ = State::loading;
#endif // NDEBUG

    // Read the save file to the save lines
    read_file();

    ASSERT(!lines_.empty());

    // Tell all modules to set up their state from the save lines (via the
    // read functions of this module)
    load_modules();

#ifndef NDEBUG
    state_ = State::stopped;
#endif // NDEBUG

    // Save file corruption check
    ASSERT(lines_.empty());

    // Loading finished, write an empty save file to prevent reloading the game
    write_file();
}

bool is_save_available()
{
    std::ifstream file("data/save");

    if (file.good())
    {
        const bool is_empty = file.peek() == std::ifstream::traits_type::eof();
        file.close();
        return !is_empty;
    }
    else // Failed to open file
    {
        file.close();
        return false;
    }
}

void put_str(const std::string str)
{
#ifndef NDEBUG
    ASSERT(state_ == State::saving);
#endif // NDEBUG

    lines_.push_back(str);
}

void put_int(const int v)
{
    put_str(to_str(v));
}

void put_bool(const bool v)
{
    const std::string str = v ? "T" : "F";

    put_str(str);
}

std::string get_str()
{
#ifndef NDEBUG
    ASSERT(state_ == State::loading);
#endif // NDEBUG

    // Save file corruption check
    ASSERT(!lines_.empty());

    const std::string str = lines_.front();

    lines_.erase(begin(lines_));

    return str;
}

int get_int()
{
    return to_int(get_str());
}

bool get_bool()
{
    return get_str() == "T";
}

} // save
