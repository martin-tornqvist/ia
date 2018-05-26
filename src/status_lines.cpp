#include "status_lines.hpp"

#include "actor_player.hpp"
#include "colors.hpp"
#include "game.hpp"
#include "game_time.hpp"
#include "inventory.hpp"
#include "io.hpp"
#include "item.hpp"
#include "item_device.hpp"
#include "map.hpp"
#include "panel.hpp"
#include "player_bon.hpp"
#include "property_handler.hpp"
#include "text_format.hpp"

namespace status_lines
{

void draw()
{
        const Panel panel = Panel::player_stats;

        io::cover_panel(panel, colors::extra_dark_gray());

        io::draw_box(panels::get_area(panel));

        const bool draw_text_bg = false;

        const Color& label_color = colors::dark_sepia();

        const int x0 = 1;
        const int x1 = panels::get_w(panel) - 2;

        Player& player = *map::player;

        int y = 1;

        // Player name
        io::draw_text(
                player.name_the(),
                panel,
                P(x0, y),
                colors::light_sepia(),
                draw_text_bg);

        y += 1;

        // Player class
        io::draw_text(
                player_bon::bg_title(player_bon::bg()),
                panel,
                P(x0, y),
                colors::light_sepia(),
                draw_text_bg);

        y += 1;

        // Character level and experience
        {
                io::draw_text(
                        "Level",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                const std::string xp_str =
                        std::to_string(game::clvl()) +
                        " (" +
                        std::to_string(game::xp_pct()) +
                        "%)";

                io::draw_text_right(
                        xp_str,
                        panel,
                        P(x1, y),
                        colors::white(),
                        draw_text_bg);
        }

        y += 1;

        // Dungeon level
        {
                io::draw_text(
                        "Depth",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                std::string dlvl_str = std::to_string(map::dlvl);

                io::draw_text_right(
                        dlvl_str,
                        panel,
                        P(x1, y),
                        colors::white(),
                        draw_text_bg);
        }

        y += 1;

        // Hit points
        {
                io::draw_text(
                        "Health",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                const std::string str =
                        std::to_string(player.hp()) +
                        "/" +
                        std::to_string(player.hp_max(true));

                io::draw_text_right(
                        str,
                        panel,
                        P(x1, y),
                        colors::light_red(),
                        draw_text_bg);
        }

        y += 1;

        // Spirit
        {
                io::draw_text(
                        "Spirit",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                const std::string str =
                        std::to_string(player.spi()) +
                        "/" +
                        std::to_string(player.spi_max());

                io::draw_text_right(
                        str,
                        panel,
                        P(x1, y),
                        colors::light_blue(),
                        draw_text_bg);
        }

        y += 1;

        // Shock
        {
                io::draw_text(
                        "Shock",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                const int shock = std::min(999, player.shock_tot());

                const std::string shock_str = std::to_string(shock) + "%";

                // const Color shock_color =
                //         shock < 50  ? colors::white() :
                //         shock < 75  ? colors::yellow() :
                //         shock < 100 ? colors::magenta() :
                //         colors::light_red();

                io::draw_text_right(
                        shock_str,
                        panel,
                        P(x1, y),
                        colors::magenta() /* shock_color */,
                        draw_text_bg);
        }

        y += 1;

        // Insanity
        {
                io::draw_text(
                        "Insanity",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                const std::string ins_str = std::to_string(player.ins()) + "%";

                io::draw_text_right(
                        ins_str,
                        panel,
                        P(x1, y),
                        colors::magenta(),
                        draw_text_bg);
        }

        y += 2;

        // Lantern
        {
                io::draw_text(
                        "Lantern",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                const Item* const item =
                        player.inv().item_in_backpack(ItemId::lantern);

                Color color = colors::white();

                std::string lantern_str = "None";

                if (item)
                {
                        const DeviceLantern* const lantern =
                                static_cast<const DeviceLantern*>(item);

                        if (lantern->is_activated_)
                        {

                                color = colors::yellow();
                        }

                        lantern_str = std::to_string(lantern->nr_turns_left_);
                }

                io::draw_text_right(
                        lantern_str,
                        panel,
                        P(x1, y),
                        color,
                        draw_text_bg);
        }

        y += 1;

        // Medical supplies
        {
                io::draw_text(
                        "Med. Suppl.",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                std::string suppl_str = "-";

                const Item* const item =
                        player.inv().item_in_backpack(ItemId::medical_bag);

                if (item)
                {
                        const MedicalBag* const medical_bag =
                                static_cast<const MedicalBag*>(item);

                        suppl_str = std::to_string(medical_bag->nr_supplies_);
                }

                io::draw_text_right(
                        suppl_str,
                        panel,
                        P(x1, y),
                        colors::white(),
                        draw_text_bg);
        }

        y += 1;

        // Armor
        {
                io::draw_text(
                        "Armor",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                const std::string armor_str =
                        std::to_string(player.armor_points());

                io::draw_text_right(
                        armor_str,
                        panel,
                        P(x1, y),
                        colors::white(),
                        draw_text_bg);
        }

        y += 1;

        // Encumbrance
        {
                io::draw_text(
                        "Weight",
                        panel,
                        P(x0, y),
                        label_color,
                        draw_text_bg);

                const int enc = player.enc_percent();

                const std::string enc_str = std::to_string(enc) + "%";

                const Color enc_color =
                        (enc < 100) ? colors::white() :
                        (enc < enc_immobile_lvl) ? colors::yellow() :
                        colors::light_red();

                io::draw_text_right(
                        enc_str,
                        panel,
                        P(x1, y),
                        enc_color,
                        draw_text_bg);
        }

        y += 2;

        // Properties
        {
                const auto property_names =
                        player.properties().property_names_short();

                for (const auto& name : property_names)
                {
                        if (y >= panels::get_y1(panel))
                        {
                                break;
                        }

                        io::draw_text(
                                name.str,
                                panel,
                                P(x0, y),
                                name.color,
                                draw_text_bg);

                        y += 1;
                }
        }

        // Turn number
        // const int turn_nr = game_time::turn_nr();

        // const std::string turn_nr_str = std::to_string(turn_nr);

        // // "T:" + current turn number
        // const int total_turn_info_w = turn_nr_str.size() + 2;

        // p.x = panels::get_x1(panel) - total_turn_info_w + 1;

        // io::draw_text("T", panel, p, colors::dark_gray(), colors::black());

        // ++p.x;

        // io::draw_text(":", panel, p, colors::dark_gray());

        // ++p.x;

        // io::draw_text(turn_nr_str, panel, p, colors::white());
}

} // status_lines
