#include "save_handling.hpp"

#include <fstream>
#include <iostream>

#include "init.hpp"
#include "log.hpp"
#include "render.hpp"
#include "actor_player.hpp"
#include "dungeon_master.hpp"
#include "item_scroll.hpp"
#include "item_potion.hpp"
#include "player_bon.hpp"
#include "map.hpp"
#include "map_travel.hpp"
#include "inventory.hpp"
#include "game_time.hpp"
#include "player_spells_handling.hpp"
#include "item_jewelry.hpp"

using namespace std;

namespace save_handling
{

namespace
{

void collect_lines_from_game(vector<string>& lines)
{
    lines.clear();
    lines.push_back(Map::player->get_name_a());

    Dungeon_master::store_to_save_lines(lines);
    Scroll_handling::store_to_save_lines(lines);
    Potion_handling::store_to_save_lines(lines);
    Item_data::store_to_save_lines(lines);
    Jewelry_handling::store_to_save_lines(lines);
    Map::player->get_inv().store_to_save_lines(lines);
    Map::player->store_to_save_lines(lines);
    Player_bon::store_to_save_lines(lines);
    Map_travel::store_to_save_lines(lines);
    Map::store_to_save_lines(lines);
    Actor_data::store_to_save_lines(lines);
    Game_time::store_to_save_lines(lines);
    Player_spells_handling::store_to_save_lines(lines);
}

void setup_game_from_lines(vector<string>& lines)
{
    TRACE_FUNC_BEGIN;
    const string& player_name = lines.front();
    Map::player->get_data().name_a = player_name;
    Map::player->get_data().name_the = player_name;
    lines.erase(begin(lines));

    Dungeon_master::setup_from_save_lines(lines);
    Scroll_handling::setup_from_save_lines(lines);
    Potion_handling::setup_from_save_lines(lines);
    Item_data::setup_from_save_lines(lines);
    Jewelry_handling::setup_from_save_lines(lines);
    Map::player->get_inv().setup_from_save_lines(lines);
    Map::player->setup_from_save_lines(lines);
    Player_bon::setup_from_save_lines(lines);
    Map_travel::setup_from_save_lines(lines);
    Map::setup_from_save_lines(lines);
    Actor_data::setup_from_save_lines(lines);
    Game_time::setup_from_save_lines(lines);
    Player_spells_handling::setup_from_save_lines(lines);
    TRACE_FUNC_END;
}

void write_file(const vector<string>& lines)
{
    ofstream file;
    file.open("data/save", ios::trunc);

    if (file.is_open())
    {
        for (size_t i = 0; i < lines.size(); ++i)
        {
            file << lines[i];
            if (i != lines.size() - 1) {file << endl;}
        }
        file.close();
    }
}

void read_file(vector<string>& lines)
{
    lines.clear();

    string cur_line;
    ifstream file("data/save");
    if (file.is_open())
    {
        while (getline(file, cur_line)) {lines.push_back(cur_line);}
        file.close();

        vector<string> empty_lines;
        empty_lines.clear();
        write_file(empty_lines);
    }
    else
    {
        assert(false && "Failed to open save file");
    }
}

} //namespace

void save()
{
    vector<string> lines;
    collect_lines_from_game(lines);
    write_file(lines);
}

void load()
{
    vector<string> lines;
    read_file(lines);
    setup_game_from_lines(lines);
}

bool is_save_available()
{
    ifstream file("data/save");
    if (file.good())
    {
        const bool IS_EMPTY = file.peek() == std::ifstream::traits_type::eof();
        file.close();
        return !IS_EMPTY;
    }
    else
    {
        file.close();
        return false;
    }
}

} //Save_handling
