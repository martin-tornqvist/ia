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

P column_and_row_tile(const TileId tile)
{
    switch (tile)
    {
    case TileId::player_firearm:                return P(0, 0);
    case TileId::player_melee:                  return P(1, 0);
    case TileId::zombie_unarmed:                return P(2, 0);
    case TileId::zombie_armed:                  return P(3, 0);
    case TileId::zombie_bloated:                return P(4, 0);
    case TileId::cultist_firearm:               return P(5, 0);
    case TileId::cultist_dagger:                return P(6, 0);
    case TileId::witch_or_warlock:              return P(7, 0);
    case TileId::ghoul:                         return P(8, 0);
    case TileId::mummy:                         return P(9, 0);
    case TileId::deep_one:                      return P(10, 0);
    case TileId::shadow:                        return P(11, 0);
    case TileId::leng_elder:                    return P(12, 0);
    case TileId::fiend:                         return P(13, 0);
    case TileId::ape:                           return P(14, 0);
    case TileId::croc_head_mummy:               return P(15, 0);
    case TileId::the_high_priest:               return P(16, 0);
    case TileId::floating_skull:                return P(17, 0);
    case TileId::crawling_hand:                 return P(18, 0);
    case TileId::crawling_intestines:           return P(19, 0);
    case TileId::raven:                         return P(20, 0);
    case TileId::armor:                         return P(0, 1);
    case TileId::potion:                        return P(2, 1);
    case TileId::ammo:                          return P(4, 1);
    case TileId::scroll:                        return P(8, 1);
    case TileId::elder_sign:                    return P(10, 1);
    case TileId::chest_closed:                  return P(11, 1);
    case TileId::chest_open:                    return P(12, 1);
    case TileId::amulet:                        return P(14, 1);
    case TileId::ring:                          return P(15, 1);
    case TileId::lantern:                       return P(16, 1);
    case TileId::medical_bag:                   return P(17, 1);
    case TileId::gas_mask:                      return P(18, 1);
    case TileId::void_traveler:                 return P(19, 1);
    case TileId::snake:                         return P(20, 1);
    case TileId::rat:                           return P(1, 2);
    case TileId::spider:                        return P(2, 2);
    case TileId::wolf:                          return P(3, 2);
    case TileId::phantasm:                      return P(4, 2);
    case TileId::rat_thing:                     return P(5, 2);
    case TileId::hound:                         return P(6, 2);
    case TileId::bat:                           return P(7, 2);
    case TileId::byakhee:                       return P(8, 2);
    case TileId::mass_of_worms:                 return P(9, 2);
    case TileId::ooze:                          return P(10, 2);
    case TileId::gas_spore:                     return P(11, 2);
    case TileId::polyp:                         return P(7, 5);
    case TileId::fungi:                         return P(8, 5);
    case TileId::vortex:                        return P(5, 5);
    case TileId::ghost:                         return P(12, 2);
    case TileId::wraith:                        return P(13, 2);
    case TileId::mantis:                        return P(14, 2);
    case TileId::locust:                        return P(15, 2);
    case TileId::bog_tcher:                     return P(16, 2);
    case TileId::mi_go_armor:                   return P(17, 2);
    case TileId::horn:                          return P(18, 2);
    case TileId::clockwork:                     return P(19, 2);
    case TileId::crystal:                       return P(20, 2);
    case TileId::pistol:                        return P(1, 3);
    case TileId::tommy_gun:                     return P(3, 3);
    case TileId::shotgun:                       return P(4, 3);
    case TileId::dynamite:                      return P(5, 3);
    case TileId::dynamite_lit:                  return P(6, 3);
    case TileId::molotov:                       return P(7, 3);
    case TileId::incinerator:                   return P(8, 3);
    case TileId::mi_go_gun:                     return P(9, 3);
    case TileId::flare:                         return P(10, 3);
    case TileId::flare_gun:                     return P(11, 3);
    case TileId::flare_lit:                     return P(12, 3);
    case TileId::orb:                           return P(13, 3);
    case TileId::zombie_dust:                   return P(14, 3);
    case TileId::dagger:                        return P(0, 4);
    case TileId::crowbar:                       return P(3, 4);
    case TileId::rod:                           return P(6, 4);
    case TileId::axe:                           return P(9, 4);
    case TileId::club:                          return P(10, 4);
    case TileId::hammer:                        return P(11, 4);
    case TileId::machete:                       return P(12, 4);
    case TileId::pitchfork:                     return P(13, 4);
    case TileId::sledge_hammer:                 return P(14, 4);
    case TileId::rock:                          return P(15, 4);
    case TileId::iron_spike:                    return P(16, 4);
    case TileId::pharaoh_staff:                 return P(17, 4);
    case TileId::lockpick:                      return P(17, 4);
    case TileId::hunting_horror:                return P(4, 5);
    case TileId::giant_spider:                  return P(10, 5);
    case TileId::chthonian:                     return P(0, 5);
    case TileId::mind_eater:                    return P(3, 5);
    case TileId::mi_go:                         return P(12, 5);
    case TileId::floor:                         return P(1, 6);
    case TileId::aim_marker_head:               return P(2, 6);
    case TileId::aim_marker_line:               return P(3, 6);
    case TileId::wall_front:                    return P(3, 7);
    case TileId::square_checkered_sparse:       return P(5, 6);
    case TileId::rubble_high:                   return P(7, 6);
    case TileId::rubble_low:                    return P(8, 6);
    case TileId::pylon:                         return P(19, 5);
    case TileId::stairs_down:                   return P(9, 6);
    case TileId::brazier:                       return P(11, 6);
    case TileId::altar:                         return P(12, 6);
    case TileId::web:                           return P(13, 6);
    case TileId::door_closed:                   return P(14, 6);
    case TileId::door_open:                     return P(15, 6);
    case TileId::door_broken:                   return P(16, 6);
    case TileId::fountain:                      return P(18, 6);
    case TileId::gate_closed:                   return P(19, 6);
    case TileId::gate_open:                     return P(20, 6);
    case TileId::tree:                          return P(1, 7);
    case TileId::bush:                          return P(2, 7);
    case TileId::church_bench:                  return P(4, 7);
    case TileId::grave_stone:                   return P(5, 7);
    case TileId::tomb_open:                     return P(13, 5);
    case TileId::tomb_closed:                   return P(14, 5);
    case TileId::water1:                        return P(7, 7);
    case TileId::water2:                        return P(8, 7);
    case TileId::cocoon_open:                   return P(4, 6);
    case TileId::cocoon_closed:                 return P(10, 7);
    case TileId::blast1:                        return P(11, 7);
    case TileId::blast2:                        return P(12, 7);
    case TileId::corpse:                        return P(13, 7);
    case TileId::corpse2:                       return P(14, 7);
    case TileId::projectile_std_front_slash:    return P(15, 7);
    case TileId::projectile_std_back_slash:     return P(16, 7);
    case TileId::projectile_std_dash:           return P(17, 7);
    case TileId::projectile_std_vertical_bar:   return P(18, 7);
    case TileId::vines:                         return P(19, 7);
    case TileId::grating:                       return P(20, 7);
    case TileId::gore1:                         return P(0, 8);
    case TileId::gore2:                         return P(1, 8);
    case TileId::gore3:                         return P(2, 8);
    case TileId::gore4:                         return P(3, 8);
    case TileId::gore5:                         return P(4, 8);
    case TileId::gore6:                         return P(5, 8);
    case TileId::gore7:                         return P(6, 8);
    case TileId::gore8:                         return P(7, 8);
    case TileId::smoke:                         return P(9, 8);
    case TileId::wall_front_alt1:               return P(1, 9);
    case TileId::wall_front_alt2:               return P(0, 9);
    case TileId::trapez:                        return P(11, 8);
    case TileId::pit:                           return P(12, 8);
    case TileId::lever_right:                   return P(13, 8);
    case TileId::lever_left:                    return P(14, 8);
    case TileId::device1:                       return P(15, 8);
    case TileId::device2:                       return P(16, 8);
    case TileId::chains:                        return P(19, 8);
    case TileId::cabinet_closed:                return P(3, 9);
    case TileId::cabinet_open:                  return P(4, 9);
    case TileId::pillar_broken:                 return P(5, 9);
    case TileId::pillar:                        return P(6, 9);
    case TileId::pillar_carved:                 return P(7, 9);
    case TileId::barrel:                        return P(8, 9);
    case TileId::sarcophagus:                   return P(9, 9);
    case TileId::cave_wall_front:               return P(10, 9);
    case TileId::cave_wall_top:                 return P(11, 9);
    case TileId::wall_top:                      return P(15, 9);
    case TileId::square_checkered:              return P(15, 9);
    case TileId::egypt_wall_front:              return P(16, 9);
    case TileId::egypt_wall_top:                return P(17, 9);
    case TileId::monolith:                      return P(18, 9);
    case TileId::bookshelf_full:                return P(19, 9);
    case TileId::bookshelf_empty:               return P(20, 9);
    case TileId::popup_top_l:                   return P(0, 10);
    case TileId::popup_top_r:                   return P(1, 10);
    case TileId::popup_btm_l:                   return P(2, 10);
    case TileId::popup_btm_r:                   return P(3, 10);
    case TileId::popup_hor:                     return P(4, 10);
    case TileId::popup_ver:                     return P(5, 10);
    case TileId::hangbridge_ver:                return P(6, 10);
    case TileId::hangbridge_hor:                return P(7, 10);
    case TileId::scorched_ground:               return P(8, 10);
    case TileId::popup_hor_down:                return P(9, 10);
    case TileId::popup_hor_up:                  return P(10, 10);
    case TileId::popup_ver_r:                   return P(11, 10);
    case TileId::popup_ver_l:                   return P(12, 10);
    case TileId::stalagmite:                    return P(13, 10);
    case TileId::trap_general:                  return P(14, 10);
    case TileId::heart:                         return P(0, 11);
    case TileId::brain:                         return P(1, 11);
    case TileId::weight:                        return P(2, 11);
    case TileId::spirit:                        return P(3, 11);
    case TileId::stopwatch:                     return P(4, 11);

    case TileId::empty:
    default:                                    return P(18, 10);
    }

} // column_and_row_glyph

} // namespace

P glyph_pos(const char glyph)
{
     return column_and_row_glyph(glyph);
}

P tile_pos(const TileId tile)
{
     return column_and_row_tile(tile);
}

} // art
