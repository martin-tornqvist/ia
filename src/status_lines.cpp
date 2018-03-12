#include "status_lines.hpp"

#include "game_time.hpp"
#include "colors.hpp"
#include "io.hpp"
#include "panel.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "game.hpp"
#include "mapgen.hpp"
#include "player_bon.hpp"
#include "inventory.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "text_format.hpp"
#include "item.hpp"
#include "item_device.hpp"
#include "property_handler.hpp"

namespace status_lines
{

void draw()
{
        const int x_wielded_default = 43;

        const size_t min_nr_steps_to_nxt_label = 3;

        const Panel panel = Panel::status_lines;

        io::cover_panel(panel);

        Player& player = *map::player;

        // Hit points
        P p(0, 0);

        std::string str = "HP:";
        io::draw_text(str, panel, p, colors::dark_gray());

        p.x += str.size();

        str =
                std::to_string(player.hp()) +
                "/" +
                std::to_string(player.hp_max(true));

        io::draw_text(str, panel, p, colors::light_red());

        // Spirit
        p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

        str = "SP:";

        io::draw_text(str, panel, p, colors::dark_gray());

        p.x += str.size();

        str =
                std::to_string(player.spi()) +
                "/" +
                std::to_string(player.spi_max());

        io::draw_text(str, panel, p, colors::magenta());

        // Insanity
        p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

        str = "Ins:";

        io::draw_text(str, panel, p, colors::dark_gray());

        p.x += str.size();

        const int shock = std::min(999, player.shock_tot());

        const int ins = player.ins();

        const Color shock_color =
                shock < 50  ? colors::green() :
                shock < 75  ? colors::yellow() :
                shock < 100 ? colors::magenta() : colors::light_red();

        str = std::to_string(shock) + "%";

        io::draw_text(str, panel, p, shock_color);

        p.x += str.size();

        str = "/";

        io::draw_text(str, panel, p, colors::dark_gray());

        p.x += str.size();

        str = std::to_string(ins) + "%";

        io::draw_text(str, panel, p, colors::magenta());

        // Experience
        p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

        str = "Exp:";

        io::draw_text(str, panel, p, colors::dark_gray());

        p.x += str.size();

        str = std::to_string(game::clvl());

        if (game::clvl() < player_max_clvl)
        {
                // Not at maximum character level
                str += "(" + std::to_string(game::xp_pct()) + "%)";
        }

        io::draw_text(str, panel, p, colors::white());

        // Wielded weapon
        p.x = std::max(x_wielded_default,
                       (int)(p.x + str.size() + 1));

        int x_wielded = p.x;

        const Item* wpn = player.inv().item_in_slot(SlotId::wpn);

        if (!wpn)
        {
                wpn = &player.unarmed_wpn();
        }

        const Color item_color = wpn->color();

        io::draw_symbol(
                wpn->tile(),
                wpn->character(),
                panel,
                p,
                item_color);

        p.x += 2;

        const auto& data = wpn->data();

        // If mainly a throwing weapon, use melee info - otherwise use context
        const ItemRefAttInf att_inf =
                (data.main_att_mode == AttMode::thrown) ?
                ItemRefAttInf::melee :
                ItemRefAttInf::wpn_main_att_mode;

        str = wpn->name(
                ItemRefType::plain,
                ItemRefInf::yes,
                att_inf);

        str = text_format::first_to_upper(str);

        io::draw_text(str, panel, p, colors::white());

        // ---------------------------------------------------------------------
        // Second row
        // ---------------------------------------------------------------------
        ++p.y;
        p.x = 0;

        // Dungeon level
        str = "Dlvl:";

        io::draw_text(str, panel, p, colors::dark_gray());

        p.x += str.size();

        str = map::dlvl > 0 ? std::to_string(map::dlvl) : "-";

        io::draw_text(str, panel, p, colors::white());

        // Encumbrance
        p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

        str = "W:";

        io::draw_text(str, panel, p, colors::dark_gray());

        p.x += str.size();

        const int enc = player.enc_percent();

        str = std::to_string(enc) + "%";

        const Color enc_color =
                (enc < 100) ? colors::white() :
                (enc < enc_immobile_lvl) ? colors::yellow() :
                colors::light_red();

        io::draw_text(str, panel, p, enc_color);

        // Armor
        p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

        Color armor_color = colors::gray().fraction(1.5);

        // Use flack jacket as default tile/character
        const auto& flak_jacket_data =
                item_data::data[(size_t)ItemId::armor_flak_jacket];

        TileId armor_tile = flak_jacket_data.tile;

        char armor_character = flak_jacket_data.character;

        // If wearing body item, override the armor symbol with the item's
        const Item* const body_item = player.inv().item_in_slot(SlotId::body);

        if (body_item)
        {
                armor_color = body_item->color();

                armor_tile = body_item->tile();

                armor_character = body_item->character();
        }

        io::draw_symbol(
                armor_tile,
                armor_character,
                panel,
                p,
                armor_color);

        ++p.x;

        str = ":";

        io::draw_text(str, panel, p, colors::dark_gray());

        p.x += str.size();

        str = std::to_string(player.armor_points());

        io::draw_text(str, panel, p, colors::white());

        // Lantern
        const Item* const lantern_item =
                player.inv().item_in_backpack(ItemId::lantern);

        if (lantern_item)
        {
                p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

                const DeviceLantern* const lantern =
                        static_cast<const DeviceLantern*>(lantern_item);

                io::draw_symbol(
                        lantern_item->tile(),
                        lantern_item->character(),
                        panel,
                        p,
                        colors::yellow());

                ++p.x;

                str = ":";

                io::draw_text(str, panel, p, colors::dark_gray());

                p.x += str.size();

                const Color color =
                        lantern->is_activated_ ?
                        colors::yellow() :
                        colors::white();

                str = lantern->is_activated_ ? "On" : "Off";

                str += "(" + std::to_string(lantern->nr_turns_left_) + ")";

                io::draw_text(str, panel, p, color);
        }

        // Medical bag
        const Item* const medical_item =
                player.inv().item_in_backpack(ItemId::medical_bag);

        if (medical_item)
        {
                p.x += std::max(str.size() + 1, min_nr_steps_to_nxt_label);

                const Color color = medical_item->color();

                io::draw_symbol(
                        medical_item->tile(),
                        medical_item->character(),
                        panel,
                        p,
                        color);

                ++p.x;

                str = ":";

                io::draw_text(str, panel, p, colors::dark_gray());

                p.x += str.size();

                const MedicalBag* const medical_bag =
                        static_cast<const MedicalBag*>(medical_item);

                const int nr_suppl = medical_bag->nr_supplies_;

                str = std::to_string(nr_suppl);

                io::draw_text(str, panel, p, colors::white());
        }

        // Thrown item, or active explosive
        p.x = x_wielded;

        auto* const thr_item =
                player.active_explosive_ ?
                player.active_explosive_ :
                player.thrown_item_;

        if (thr_item)
        {
                const Color item_color = thr_item->color();

                io::draw_symbol(
                        thr_item->tile(),
                        thr_item->character(),
                        panel,
                        p,
                        item_color);

                p.x += 2;

                auto att_inf = ItemRefAttInf::thrown;

                Color text_color = colors::white();

                if (player.active_explosive_)
                {
                        att_inf = ItemRefAttInf::none;

                        text_color = colors::yellow();
                }

                // Non-stackable thrown items should be printed the same way as
                // wielded items, i.e. "Hammer", and not "A Hammer"
                const auto item_ref_type =
                        thr_item->data().is_stackable ?
                        ItemRefType::plural :
                        ItemRefType::plain;

                str = thr_item->name(
                        item_ref_type,
                        ItemRefInf::yes,
                        att_inf);

                str = text_format::first_to_upper(str);

                io::draw_text(
                        str,
                        panel,
                        p,
                        text_color);
        }

        // ---------------------------------------------------------------------
        // Third row
        // ---------------------------------------------------------------------
        ++p.y;

        p.x = 0;

        const auto props_line = player.properties().text_line();

        for (const ColoredString& current_prop_label : props_line)
        {
                io::draw_text(current_prop_label.str,
                              panel,
                              p,
                              current_prop_label.color);

                p.x += current_prop_label.str.size() + 1;
        }

        // Turn number
        const int turn_nr = game_time::turn_nr();

        const std::string turn_nr_str = std::to_string(turn_nr);

        // "T:" + current turn number
        const int total_turn_info_w = turn_nr_str.size() + 2;

        p.x = panels::get_x1(panel) - total_turn_info_w + 1;

        io::draw_text("T", panel, p, colors::dark_gray(), colors::black());

        ++p.x;

        io::draw_text(":", panel, p, colors::dark_gray());

        ++p.x;

        io::draw_text(turn_nr_str, panel, p, colors::white());
}

} // status_lines
