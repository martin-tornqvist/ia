#include "art.hpp"

#include "utils.hpp"

namespace art
{

namespace
{

Pos column_and_row_glyph(const char glyph)
{
    switch (glyph)
    {
    default:  return Pos(-1, -1);
    case ' ': return Pos(0, 0);
    case '!': return Pos(1, 0);
    case '"': return Pos(2, 0);
    case '#': return Pos(3, 0);
    case '%': return Pos(4, 0);
    case '&': return Pos(5, 0);
    case  39: return Pos(6, 0);
    case '(': return Pos(7, 0);
    case ')': return Pos(8, 0);
    case '*': return Pos(9, 0);
    case '+': return Pos(10, 0);
    case ',': return Pos(11, 0);
    case '-': return Pos(12, 0);
    case '.': return Pos(13, 0);
    case '/': return Pos(14, 0);
    case '0': return Pos(15, 0);
    case '1': return Pos(0, 1);
    case '2': return Pos(1, 1);
    case '3': return Pos(2, 1);
    case '4': return Pos(3, 1);
    case '5': return Pos(4, 1);
    case '6': return Pos(5, 1);
    case '7': return Pos(6, 1);
    case '8': return Pos(7, 1);
    case '9': return Pos(8, 1);
    case ':': return Pos(9, 1);
    case ';': return Pos(10, 1);
    case '<': return Pos(11, 1);
    case '=': return Pos(12, 1);
    case '>': return Pos(13, 1);
    case '?': return Pos(14, 1);
    case '@': return Pos(15, 1);
    case 'A': return Pos(0, 2);
    case 'B': return Pos(1, 2);
    case 'C': return Pos(2, 2);
    case 'D': return Pos(3, 2);
    case 'E': return Pos(4, 2);
    case 'F': return Pos(5, 2);
    case 'G': return Pos(6, 2);
    case 'H': return Pos(7, 2);
    case 'I': return Pos(8, 2);
    case 'J': return Pos(9, 2);
    case 'K': return Pos(10, 2);
    case 'L': return Pos(11, 2);
    case 'M': return Pos(12, 2);
    case 'N': return Pos(13, 2);
    case 'O': return Pos(14, 2);
    case 'P': return Pos(15, 2);
    case 'Q': return Pos(0, 3);
    case 'R': return Pos(1, 3);
    case 'S': return Pos(2, 3);
    case 'T': return Pos(3, 3);
    case 'U': return Pos(4, 3);
    case 'V': return Pos(5, 3);
    case 'W': return Pos(6, 3);
    case 'X': return Pos(7, 3);
    case 'Y': return Pos(8, 3);
    case 'Z': return Pos(9, 3);
    case '[': return Pos(10, 3);
    case  92: return Pos(11, 3);
    case ']': return Pos(12, 3);
    case '^': return Pos(13, 3);
    case '_': return Pos(14, 3);
    case '`': return Pos(15, 3);
    case 'a': return Pos(0, 4);
    case 'b': return Pos(1, 4);
    case 'c': return Pos(2, 4);
    case 'd': return Pos(3, 4);
    case 'e': return Pos(4, 4);
    case 'f': return Pos(5, 4);
    case 'g': return Pos(6, 4);
    case 'h': return Pos(7, 4);
    case 'i': return Pos(8, 4);
    case 'j': return Pos(9, 4);
    case 'k': return Pos(10, 4);
    case 'l': return Pos(11, 4);
    case 'm': return Pos(12, 4);
    case 'n': return Pos(13, 4);
    case 'o': return Pos(14, 4);
    case 'p': return Pos(15, 4);
    case 'q': return Pos(0, 5);
    case 'r': return Pos(1, 5);
    case 's': return Pos(2, 5);
    case 't': return Pos(3, 5);
    case 'u': return Pos(4, 5);
    case 'v': return Pos(5, 5);
    case 'w': return Pos(6, 5);
    case 'x': return Pos(7, 5);
    case 'y': return Pos(8, 5);
    case 'z': return Pos(9, 5);
    case '{': return Pos(10, 5);
    case '|': return Pos(11, 5);
    case '}': return Pos(12, 5);
    case '~': return Pos(13, 5);
    case   1: return Pos(14, 5);
    case   2: return Pos(0, 6);
    case   3: return Pos(1, 6);
    case   4: return Pos(2, 6);
    case   5: return Pos(3, 6);
    case   6: return Pos(4, 6);
    case   7: return Pos(5, 6);
    case   8: return Pos(6, 6);
    case   9: return Pos(7, 6);
    case  10: return Pos(8, 6);
    }
}

Pos column_and_row_tile(const Tile_id tile)
{
    switch (tile)
    {
    case Tile_id::player_firearm:               return Pos(0, 0);
    case Tile_id::player_melee:                 return Pos(1, 0);
    case Tile_id::zombie_unarmed:               return Pos(2, 0);
    case Tile_id::zombie_armed:                 return Pos(3, 0);
    case Tile_id::zombie_bloated:               return Pos(4, 0);
    case Tile_id::cultist_firearm:              return Pos(5, 0);
    case Tile_id::cultist_dagger:               return Pos(6, 0);
    case Tile_id::witch_or_warlock:             return Pos(7, 0);
    case Tile_id::ghoul:                        return Pos(8, 0);
    case Tile_id::mummy:                        return Pos(9, 0);
    case Tile_id::deep_one:                     return Pos(10, 0);
    case Tile_id::shadow:                       return Pos(11, 0);
    case Tile_id::leng_elder:                   return Pos(12, 0);
    case Tile_id::fiend:                        return Pos(13, 0);
    case Tile_id::ape:                          return Pos(14, 0);
    case Tile_id::croc_head_mummy:              return Pos(15, 0);
    case Tile_id::the_high_priest:              return Pos(16, 0);
    case Tile_id::floating_head:                return Pos(17, 0);
    case Tile_id::crawling_hand:                return Pos(18, 0);
    case Tile_id::crawling_intestines:          return Pos(19, 0);
    case Tile_id::raven:                        return Pos(20, 0);
    case Tile_id::armor:                        return Pos(0, 1);
    case Tile_id::potion:                       return Pos(2, 1);
    case Tile_id::ammo:                         return Pos(4, 1);
    case Tile_id::scroll:                       return Pos(8, 1);
    case Tile_id::elder_sign:                   return Pos(10, 1);
    case Tile_id::chest_closed:                 return Pos(11, 1);
    case Tile_id::chest_open:                   return Pos(12, 1);
    case Tile_id::amulet:                       return Pos(14, 1);
    case Tile_id::ring:                         return Pos(15, 1);
    case Tile_id::lantern:                      return Pos(16, 1);
    case Tile_id::medical_bag:                  return Pos(17, 1);
    case Tile_id::mask:                         return Pos(18, 1);
    case Tile_id::rat:                          return Pos(1, 2);
    case Tile_id::spider:                       return Pos(2, 2);
    case Tile_id::wolf:                         return Pos(3, 2);
    case Tile_id::phantasm:                     return Pos(4, 2);
    case Tile_id::rat_thing:                    return Pos(5, 2);
    case Tile_id::hound:                        return Pos(6, 2);
    case Tile_id::bat:                          return Pos(7, 2);
    case Tile_id::byakhee:                      return Pos(8, 2);
    case Tile_id::mass_of_worms:                return Pos(9, 2);
    case Tile_id::ooze:                         return Pos(10, 2);
    case Tile_id::gas_spore:                    return Pos(11, 2);
    case Tile_id::polyp:                        return Pos(7, 5);
    case Tile_id::fungi:                        return Pos(8, 5);
    case Tile_id::vortex:                       return Pos(5, 5);
    case Tile_id::ghost:                        return Pos(12, 2);
    case Tile_id::wraith:                       return Pos(13, 2);
    case Tile_id::mantis:                       return Pos(14, 2);
    case Tile_id::locust:                       return Pos(15, 2);
    case Tile_id::mi_go_armor:                  return Pos(17, 2);
    case Tile_id::pistol:                       return Pos(1, 3);
    case Tile_id::tommy_gun:                    return Pos(3, 3);
    case Tile_id::shotgun:                      return Pos(4, 3);
    case Tile_id::dynamite:                     return Pos(5, 3);
    case Tile_id::dynamite_lit:                 return Pos(6, 3);
    case Tile_id::molotov:                      return Pos(7, 3);
    case Tile_id::incinerator:                  return Pos(8, 3);
    case Tile_id::mi_go_gun:                    return Pos(9, 3);
    case Tile_id::flare:                        return Pos(10, 3);
    case Tile_id::flare_gun:                    return Pos(11, 3);
    case Tile_id::flare_lit:                    return Pos(12, 3);
    case Tile_id::dagger:                       return Pos(0, 4);
    case Tile_id::crowbar:                      return Pos(3, 4);
    case Tile_id::rod:                          return Pos(6, 4);
    case Tile_id::axe:                          return Pos(9, 4);
    case Tile_id::club:                         return Pos(10, 4);
    case Tile_id::hammer:                       return Pos(11, 4);
    case Tile_id::machete:                      return Pos(12, 4);
    case Tile_id::pitchfork:                    return Pos(13, 4);
    case Tile_id::sledge_hammer:                return Pos(14, 4);
    case Tile_id::rock:                         return Pos(15, 4);
    case Tile_id::iron_spike:                   return Pos(16, 4);
    case Tile_id::pharaoh_staff:                return Pos(17, 4);
    case Tile_id::lockpick:                     return Pos(17, 4);
    case Tile_id::hunting_horror:               return Pos(4, 5);
    case Tile_id::spider_leng:                  return Pos(10, 5);
    case Tile_id::chthonian:                    return Pos(0, 5);
    case Tile_id::mi_go:                        return Pos(12, 5);
    case Tile_id::floor:                        return Pos(1, 6);
    case Tile_id::aim_marker_head:              return Pos(2, 6);
    case Tile_id::aim_marker_trail:             return Pos(3, 6);
    case Tile_id::wall_top:                     return Pos(5, 6);
    case Tile_id::wall_front:                   return Pos(3, 7);
    case Tile_id::square_checkered:             return Pos(5, 6);
    case Tile_id::rubble_high:                  return Pos(7, 6);
    case Tile_id::rubble_low:                   return Pos(8, 6);
    case Tile_id::stairs_down:                  return Pos(9, 6);
    case Tile_id::brazier:                      return Pos(11, 6);
    case Tile_id::altar:                        return Pos(12, 6);
    case Tile_id::web:                          return Pos(13, 6);
    case Tile_id::door_closed:                  return Pos(14, 6);
    case Tile_id::door_open:                    return Pos(15, 6);
    case Tile_id::door_broken:                  return Pos(16, 6);
    case Tile_id::fountain:                     return Pos(18, 6);
    case Tile_id::tree:                         return Pos(1, 7);
    case Tile_id::bush:                         return Pos(2, 7);
    case Tile_id::church_bench:                 return Pos(4, 7);
    case Tile_id::grave_stone:                  return Pos(5, 7);
    case Tile_id::tomb_open:                    return Pos(13, 5);
    case Tile_id::tomb_closed:                  return Pos(14, 5);
    case Tile_id::water1:                       return Pos(7, 7);
    case Tile_id::water2:                       return Pos(8, 7);
    case Tile_id::cocoon_open:                  return Pos(4, 6);
    case Tile_id::cocoon_closed:                return Pos(10, 7);
    case Tile_id::blast1:                       return Pos(11, 7);
    case Tile_id::blast2:                       return Pos(12, 7);
    case Tile_id::corpse:                       return Pos(13, 7);
    case Tile_id::corpse2:                      return Pos(14, 7);
    case Tile_id::projectile_std_front_slash:   return Pos(15, 7);
    case Tile_id::projectile_std_back_slash:    return Pos(16, 7);
    case Tile_id::projectile_std_dash:          return Pos(17, 7);
    case Tile_id::projectile_std_vertical_bar:  return Pos(18, 7);
    case Tile_id::gore1:                        return Pos(0, 8);
    case Tile_id::gore2:                        return Pos(1, 8);
    case Tile_id::gore3:                        return Pos(2, 8);
    case Tile_id::gore4:                        return Pos(3, 8);
    case Tile_id::gore5:                        return Pos(4, 8);
    case Tile_id::gore6:                        return Pos(5, 8);
    case Tile_id::gore7:                        return Pos(6, 8);
    case Tile_id::gore8:                        return Pos(7, 8);
    case Tile_id::smoke:                        return Pos(9, 8);
    case Tile_id::wall_front_alt1:              return Pos(1, 9);
    case Tile_id::wall_front_alt2:              return Pos(0, 9);
    case Tile_id::trapezohedron:                return Pos(11, 8);
    case Tile_id::pit:                          return Pos(12, 8);
    case Tile_id::lever_right:                  return Pos(13, 8);
    case Tile_id::lever_left:                   return Pos(14, 8);
    case Tile_id::device1:                      return Pos(15, 8);
    case Tile_id::device2:                      return Pos(16, 8);
    case Tile_id::cabinet_closed:               return Pos(3, 9);
    case Tile_id::cabinet_open:                 return Pos(4, 9);
    case Tile_id::pillar_broken:                return Pos(5, 9);
    case Tile_id::pillar:                       return Pos(6, 9);
    case Tile_id::pillar_carved:                return Pos(7, 9);
    case Tile_id::barrel:                       return Pos(8, 9);
    case Tile_id::sarcophagus:                  return Pos(9, 9);
    case Tile_id::cave_wall_front:              return Pos(10, 9);
    case Tile_id::cave_wall_top:                return Pos(11, 9);
    case Tile_id::egypt_wall_front:             return Pos(16, 9);
    case Tile_id::egypt_wall_top:               return Pos(17, 9);
    case Tile_id::monolith:                     return Pos(18, 9);
    case Tile_id::popup_top_l:                  return Pos(0, 10);
    case Tile_id::popup_top_r:                  return Pos(1, 10);
    case Tile_id::popup_btm_l:                  return Pos(2, 10);
    case Tile_id::popup_btm_r:                  return Pos(3, 10);
    case Tile_id::popup_hor:                    return Pos(4, 10);
    case Tile_id::popup_ver:                    return Pos(5, 10);
    case Tile_id::hangbridge_ver:               return Pos(6, 10);
    case Tile_id::hangbridge_hor:               return Pos(7, 10);
    case Tile_id::scorched_ground:              return Pos(8, 10);
    case Tile_id::popup_hor_down:               return Pos(9, 10);
    case Tile_id::popup_hor_up:                 return Pos(10, 10);
    case Tile_id::popup_ver_r:                  return Pos(11, 10);
    case Tile_id::popup_ver_l:                  return Pos(12, 10);
    case Tile_id::stalagmite:                   return Pos(13, 10);
    case Tile_id::trap_general:                 return Pos(14, 10);
    case Tile_id::heart:                        return Pos(0, 11);
    case Tile_id::brain:                        return Pos(1, 11);
    case Tile_id::weight:                       return Pos(2, 11);
    case Tile_id::spirit:                       return Pos(3, 11);
    case Tile_id::stopwatch:                    return Pos(4, 11);

    case Tile_id::empty:
    default:                                    return Pos(18, 10);
    }
}

}

Pos glyph_pos(const char glyph)
{
    return column_and_row_glyph(glyph);
}

Pos tile_pos(const Tile_id tile)
{
    return column_and_row_tile(tile);
}

} //Art
