#include "art.hpp"

#include "rl_utils.hpp"

namespace art
{

namespace
{

P column_and_row_glyph(const char glyph)
{
    switch (glyph)
    {
    default:  return P(-1, -1);
    case ' ': return P(0, 0);
    case '!': return P(1, 0);
    case '"': return P(2, 0);
    case '#': return P(3, 0);
    case '%': return P(4, 0);
    case '&': return P(5, 0);
    case  39: return P(6, 0);
    case '(': return P(7, 0);
    case ')': return P(8, 0);
    case '*': return P(9, 0);
    case '+': return P(10, 0);
    case ',': return P(11, 0);
    case '-': return P(12, 0);
    case '.': return P(13, 0);
    case '/': return P(14, 0);
    case '0': return P(15, 0);
    case '1': return P(0, 1);
    case '2': return P(1, 1);
    case '3': return P(2, 1);
    case '4': return P(3, 1);
    case '5': return P(4, 1);
    case '6': return P(5, 1);
    case '7': return P(6, 1);
    case '8': return P(7, 1);
    case '9': return P(8, 1);
    case ':': return P(9, 1);
    case ';': return P(10, 1);
    case '<': return P(11, 1);
    case '=': return P(12, 1);
    case '>': return P(13, 1);
    case '?': return P(14, 1);
    case '@': return P(15, 1);
    case 'A': return P(0, 2);
    case 'B': return P(1, 2);
    case 'C': return P(2, 2);
    case 'D': return P(3, 2);
    case 'E': return P(4, 2);
    case 'F': return P(5, 2);
    case 'G': return P(6, 2);
    case 'H': return P(7, 2);
    case 'I': return P(8, 2);
    case 'J': return P(9, 2);
    case 'K': return P(10, 2);
    case 'L': return P(11, 2);
    case 'M': return P(12, 2);
    case 'N': return P(13, 2);
    case 'O': return P(14, 2);
    case 'P': return P(15, 2);
    case 'Q': return P(0, 3);
    case 'R': return P(1, 3);
    case 'S': return P(2, 3);
    case 'T': return P(3, 3);
    case 'U': return P(4, 3);
    case 'V': return P(5, 3);
    case 'W': return P(6, 3);
    case 'X': return P(7, 3);
    case 'Y': return P(8, 3);
    case 'Z': return P(9, 3);
    case '[': return P(10, 3);
    case  92: return P(11, 3);
    case ']': return P(12, 3);
    case '^': return P(13, 3);
    case '_': return P(14, 3);
    case '`': return P(15, 3);
    case 'a': return P(0, 4);
    case 'b': return P(1, 4);
    case 'c': return P(2, 4);
    case 'd': return P(3, 4);
    case 'e': return P(4, 4);
    case 'f': return P(5, 4);
    case 'g': return P(6, 4);
    case 'h': return P(7, 4);
    case 'i': return P(8, 4);
    case 'j': return P(9, 4);
    case 'k': return P(10, 4);
    case 'l': return P(11, 4);
    case 'm': return P(12, 4);
    case 'n': return P(13, 4);
    case 'o': return P(14, 4);
    case 'p': return P(15, 4);
    case 'q': return P(0, 5);
    case 'r': return P(1, 5);
    case 's': return P(2, 5);
    case 't': return P(3, 5);
    case 'u': return P(4, 5);
    case 'v': return P(5, 5);
    case 'w': return P(6, 5);
    case 'x': return P(7, 5);
    case 'y': return P(8, 5);
    case 'z': return P(9, 5);
    case '{': return P(10, 5);
    case '|': return P(11, 5);
    case '}': return P(12, 5);
    case '~': return P(13, 5);
    case   1: return P(14, 5);
    case   2: return P(0, 6);
    case   3: return P(1, 6);
    case   4: return P(2, 6);
    case   5: return P(3, 6);
    case   6: return P(4, 6);
    case   7: return P(5, 6);
    case   8: return P(6, 6);
    case   9: return P(7, 6);
    case  10: return P(8, 6);
    }
}

P column_and_row_tile(const Tile_id tile)
{
    switch (tile)
    {
    case Tile_id::player_firearm:               return P(0, 0);
    case Tile_id::player_melee:                 return P(1, 0);
    case Tile_id::zombie_unarmed:               return P(2, 0);
    case Tile_id::zombie_armed:                 return P(3, 0);
    case Tile_id::zombie_bloated:               return P(4, 0);
    case Tile_id::cultist_firearm:              return P(5, 0);
    case Tile_id::cultist_dagger:               return P(6, 0);
    case Tile_id::witch_or_warlock:             return P(7, 0);
    case Tile_id::ghoul:                        return P(8, 0);
    case Tile_id::mummy:                        return P(9, 0);
    case Tile_id::deep_one:                     return P(10, 0);
    case Tile_id::shadow:                       return P(11, 0);
    case Tile_id::leng_elder:                   return P(12, 0);
    case Tile_id::fiend:                        return P(13, 0);
    case Tile_id::ape:                          return P(14, 0);
    case Tile_id::croc_head_mummy:              return P(15, 0);
    case Tile_id::the_high_priest:              return P(16, 0);
    case Tile_id::floating_skull:                return P(17, 0);
    case Tile_id::crawling_hand:                return P(18, 0);
    case Tile_id::crawling_intestines:          return P(19, 0);
    case Tile_id::raven:                        return P(20, 0);
    case Tile_id::armor:                        return P(0, 1);
    case Tile_id::potion:                       return P(2, 1);
    case Tile_id::ammo:                         return P(4, 1);
    case Tile_id::scroll:                       return P(8, 1);
    case Tile_id::elder_sign:                   return P(10, 1);
    case Tile_id::chest_closed:                 return P(11, 1);
    case Tile_id::chest_open:                   return P(12, 1);
    case Tile_id::amulet:                       return P(14, 1);
    case Tile_id::ring:                         return P(15, 1);
    case Tile_id::lantern:                      return P(16, 1);
    case Tile_id::medical_bag:                  return P(17, 1);
    case Tile_id::gas_mask:                     return P(18, 1);
    case Tile_id::snake:                        return P(20, 1);
    case Tile_id::rat:                          return P(1, 2);
    case Tile_id::spider:                       return P(2, 2);
    case Tile_id::wolf:                         return P(3, 2);
    case Tile_id::phantasm:                     return P(4, 2);
    case Tile_id::rat_thing:                    return P(5, 2);
    case Tile_id::hound:                        return P(6, 2);
    case Tile_id::bat:                          return P(7, 2);
    case Tile_id::byakhee:                      return P(8, 2);
    case Tile_id::mass_of_worms:                return P(9, 2);
    case Tile_id::ooze:                         return P(10, 2);
    case Tile_id::gas_spore:                    return P(11, 2);
    case Tile_id::polyp:                        return P(7, 5);
    case Tile_id::fungi:                        return P(8, 5);
    case Tile_id::vortex:                       return P(5, 5);
    case Tile_id::ghost:                        return P(12, 2);
    case Tile_id::wraith:                       return P(13, 2);
    case Tile_id::mantis:                       return P(14, 2);
    case Tile_id::locust:                       return P(15, 2);
    case Tile_id::mi_go_armor:                  return P(17, 2);
    case Tile_id::pistol:                       return P(1, 3);
    case Tile_id::tommy_gun:                    return P(3, 3);
    case Tile_id::shotgun:                      return P(4, 3);
    case Tile_id::dynamite:                     return P(5, 3);
    case Tile_id::dynamite_lit:                 return P(6, 3);
    case Tile_id::molotov:                      return P(7, 3);
    case Tile_id::incinerator:                  return P(8, 3);
    case Tile_id::mi_go_gun:                    return P(9, 3);
    case Tile_id::flare:                        return P(10, 3);
    case Tile_id::flare_gun:                    return P(11, 3);
    case Tile_id::flare_lit:                    return P(12, 3);
    case Tile_id::dagger:                       return P(0, 4);
    case Tile_id::crowbar:                      return P(3, 4);
    case Tile_id::rod:                          return P(6, 4);
    case Tile_id::axe:                          return P(9, 4);
    case Tile_id::club:                         return P(10, 4);
    case Tile_id::hammer:                       return P(11, 4);
    case Tile_id::machete:                      return P(12, 4);
    case Tile_id::pitchfork:                    return P(13, 4);
    case Tile_id::sledge_hammer:                return P(14, 4);
    case Tile_id::rock:                         return P(15, 4);
    case Tile_id::iron_spike:                   return P(16, 4);
    case Tile_id::pharaoh_staff:                return P(17, 4);
    case Tile_id::lockpick:                     return P(17, 4);
    case Tile_id::hunting_horror:               return P(4, 5);
    case Tile_id::giant_spider:                 return P(10, 5);
    case Tile_id::chthonian:                    return P(0, 5);
    case Tile_id::mi_go:                        return P(12, 5);
    case Tile_id::floor:                        return P(1, 6);
    case Tile_id::aim_marker_head:              return P(2, 6);
    case Tile_id::aim_marker_trail:             return P(3, 6);
    case Tile_id::wall_top:                     return P(5, 6);
    case Tile_id::wall_front:                   return P(3, 7);
    case Tile_id::square_checkered:             return P(5, 6);
    case Tile_id::rubble_high:                  return P(7, 6);
    case Tile_id::rubble_low:                   return P(8, 6);
    case Tile_id::stairs_down:                  return P(9, 6);
    case Tile_id::brazier:                      return P(11, 6);
    case Tile_id::altar:                        return P(12, 6);
    case Tile_id::web:                          return P(13, 6);
    case Tile_id::door_closed:                  return P(14, 6);
    case Tile_id::door_open:                    return P(15, 6);
    case Tile_id::door_broken:                  return P(16, 6);
    case Tile_id::fountain:                     return P(18, 6);
    case Tile_id::tree:                         return P(1, 7);
    case Tile_id::bush:                         return P(2, 7);
    case Tile_id::church_bench:                 return P(4, 7);
    case Tile_id::grave_stone:                  return P(5, 7);
    case Tile_id::tomb_open:                    return P(13, 5);
    case Tile_id::tomb_closed:                  return P(14, 5);
    case Tile_id::water1:                       return P(7, 7);
    case Tile_id::water2:                       return P(8, 7);
    case Tile_id::cocoon_open:                  return P(4, 6);
    case Tile_id::cocoon_closed:                return P(10, 7);
    case Tile_id::blast1:                       return P(11, 7);
    case Tile_id::blast2:                       return P(12, 7);
    case Tile_id::corpse:                       return P(13, 7);
    case Tile_id::corpse2:                      return P(14, 7);
    case Tile_id::projectile_std_front_slash:   return P(15, 7);
    case Tile_id::projectile_std_back_slash:    return P(16, 7);
    case Tile_id::projectile_std_dash:          return P(17, 7);
    case Tile_id::projectile_std_vertical_bar:  return P(18, 7);
    case Tile_id::gore1:                        return P(0, 8);
    case Tile_id::gore2:                        return P(1, 8);
    case Tile_id::gore3:                        return P(2, 8);
    case Tile_id::gore4:                        return P(3, 8);
    case Tile_id::gore5:                        return P(4, 8);
    case Tile_id::gore6:                        return P(5, 8);
    case Tile_id::gore7:                        return P(6, 8);
    case Tile_id::gore8:                        return P(7, 8);
    case Tile_id::smoke:                        return P(9, 8);
    case Tile_id::wall_front_alt1:              return P(1, 9);
    case Tile_id::wall_front_alt2:              return P(0, 9);
    case Tile_id::trapez:                return P(11, 8);
    case Tile_id::pit:                          return P(12, 8);
    case Tile_id::lever_right:                  return P(13, 8);
    case Tile_id::lever_left:                   return P(14, 8);
    case Tile_id::device1:                      return P(15, 8);
    case Tile_id::device2:                      return P(16, 8);
    case Tile_id::cabinet_closed:               return P(3, 9);
    case Tile_id::cabinet_open:                 return P(4, 9);
    case Tile_id::pillar_broken:                return P(5, 9);
    case Tile_id::pillar:                       return P(6, 9);
    case Tile_id::pillar_carved:                return P(7, 9);
    case Tile_id::barrel:                       return P(8, 9);
    case Tile_id::sarcophagus:                  return P(9, 9);
    case Tile_id::cave_wall_front:              return P(10, 9);
    case Tile_id::cave_wall_top:                return P(11, 9);
    case Tile_id::egypt_wall_front:             return P(16, 9);
    case Tile_id::egypt_wall_top:               return P(17, 9);
    case Tile_id::monolith:                     return P(18, 9);
    case Tile_id::popup_top_l:                  return P(0, 10);
    case Tile_id::popup_top_r:                  return P(1, 10);
    case Tile_id::popup_btm_l:                  return P(2, 10);
    case Tile_id::popup_btm_r:                  return P(3, 10);
    case Tile_id::popup_hor:                    return P(4, 10);
    case Tile_id::popup_ver:                    return P(5, 10);
    case Tile_id::hangbridge_ver:               return P(6, 10);
    case Tile_id::hangbridge_hor:               return P(7, 10);
    case Tile_id::scorched_ground:              return P(8, 10);
    case Tile_id::popup_hor_down:               return P(9, 10);
    case Tile_id::popup_hor_up:                 return P(10, 10);
    case Tile_id::popup_ver_r:                  return P(11, 10);
    case Tile_id::popup_ver_l:                  return P(12, 10);
    case Tile_id::stalagmite:                   return P(13, 10);
    case Tile_id::trap_general:                 return P(14, 10);
    case Tile_id::heart:                        return P(0, 11);
    case Tile_id::brain:                        return P(1, 11);
    case Tile_id::weight:                       return P(2, 11);
    case Tile_id::spirit:                       return P(3, 11);
    case Tile_id::stopwatch:                    return P(4, 11);

    case Tile_id::empty:
    default:                                    return P(18, 10);
    }
}

}

P glyph_pos(const char glyph)
{
    return column_and_row_glyph(glyph);
}

P tile_pos(const Tile_id tile)
{
    return column_and_row_tile(tile);
}

} //Art
