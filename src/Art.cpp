#include "Art.h"

#include "Engine.h"


coord Art::getColumnAndRow(const char glyph) {
  switch(glyph)
  {
  default:  return coord(-1, -1);  break;
  case ' ': return coord(0, 0);  break;
  case '!': return coord(1, 0); break;
  case '"': return coord(2, 0); break;
  case '#': return coord(3, 0); break;
  case '%': return coord(4, 0); break;
  case '&': return coord(5, 0); break;
  case  39: return coord(6, 0); break;
  case '(': return coord(7, 0); break;
  case ')': return coord(8, 0); break;
  case '*': return coord(9, 0); break;
  case '+': return coord(10, 0); break;
  case ',': return coord(11, 0); break;
  case '-': return coord(12, 0); break;
  case '.': return coord(13, 0); break;
  case '/': return coord(14, 0); break;
  case '0': return coord(15, 0); break;
  case '1': return coord(0, 1); break;
  case '2': return coord(1, 1); break;
  case '3': return coord(2, 1); break;
  case '4': return coord(3, 1); break;
  case '5': return coord(4, 1); break;
  case '6': return coord(5, 1); break;
  case '7': return coord(6, 1); break;
  case '8': return coord(7, 1); break;
  case '9': return coord(8, 1); break;
  case ':': return coord(9, 1); break;
  case ';': return coord(10, 1); break;
  case '<': return coord(11, 1); break;
  case '=': return coord(12, 1); break;
  case '>': return coord(13, 1); break;
  case '?': return coord(14, 1); break;
  case '@': return coord(15, 1); break;
  case 'A': return coord(0, 2); break;
  case 'B': return coord(1, 2); break;
  case 'C': return coord(2, 2); break;
  case 'D': return coord(3, 2); break;
  case 'E': return coord(4, 2); break;
  case 'F': return coord(5, 2); break;
  case 'G': return coord(6, 2); break;
  case 'H': return coord(7, 2); break;
  case 'I': return coord(8, 2); break;
  case 'J': return coord(9, 2); break;
  case 'K': return coord(10, 2); break;
  case 'L': return coord(11, 2); break;
  case 'M': return coord(12, 2); break;
  case 'N': return coord(13, 2); break;
  case 'O': return coord(14, 2); break;
  case 'P': return coord(15, 2); break;
  case 'Q': return coord(0, 3); break;
  case 'R': return coord(1, 3); break;
  case 'S': return coord(2, 3); break;
  case 'T': return coord(3, 3); break;
  case 'U': return coord(4, 3); break;
  case 'V': return coord(5, 3); break;
  case 'W': return coord(6, 3); break;
  case 'X': return coord(7, 3); break;
  case 'Y': return coord(8, 3); break;
  case 'Z': return coord(9, 3); break;
  case '[': return coord(10, 3); break;
  case  92: return coord(11, 3);  break;
  case ']': return coord(12, 3);  break;
  case '^': return coord(13, 3);  break;
  case '_': return coord(14, 3);  break;
  case '`': return coord(15, 3);  break;
  case 'a': return coord(0, 4);  break;
  case 'b': return coord(1, 4);  break;
  case 'c': return coord(2, 4);  break;
  case 'd': return coord(3, 4);  break;
  case 'e': return coord(4, 4);  break;
  case 'f': return coord(5, 4);  break;
  case 'g': return coord(6, 4);  break;
  case 'h': return coord(7, 4);  break;
  case 'i': return coord(8, 4);  break;
  case 'j': return coord(9, 4);  break;
  case 'k': return coord(10, 4);  break;
  case 'l': return coord(11, 4);  break;
  case 'm': return coord(12, 4);  break;
  case 'n': return coord(13, 4);  break;
  case 'o': return coord(14, 4);  break;
  case 'p': return coord(15, 4);  break;
  case 'q': return coord(0, 5);  break;
  case 'r': return coord(1, 5);  break;
  case 's': return coord(2, 5);  break;
  case 't': return coord(3, 5);  break;
  case 'u': return coord(4, 5);  break;
  case 'v': return coord(5, 5);  break;
  case 'w': return coord(6, 5);  break;
  case 'x': return coord(7, 5);  break;
  case 'y': return coord(8, 5);  break;
  case 'z': return coord(9, 5);  break;
  case '{': return coord(10, 5);  break;
  case '|': return coord(11, 5);  break;
  case '}': return coord(12, 5);  break;
  case '~': return coord(13, 5);  break;
  case   1: return coord(14, 5);  break;
  case   2: return coord(0, 6);  break;
  case   3: return coord(1, 6);  break;
  case   4: return coord(2, 6);  break;
  case   5: return coord(3, 6); break;
  case   6: return coord(4, 6); break;
  case   7: return coord(5, 6); break;
  case   8: return coord(6, 6); break;
  case   9: return coord(7, 6); break;
  case  10: return coord(8, 6); break;
  }
}



coord Art::getColumnAndRowTile(const Tile_t tile) {
  switch(tile)
  {
  case tile_playerFirearm: return coord(0, 0); break;
  case tile_playerMelee: return coord(1, 0); break;
  case tile_zombieUnarmed: return coord(2, 0); break;
  case tile_zombieArmed: return coord(3, 0); break;
  case tile_zombieBloated: return coord(4, 0); break;
  case tile_cultistFirearm: return coord(5, 0); break;
  case tile_cultistDagger: return coord(6, 0); break;
  case tile_witchOrWarlock: return coord(7, 0); break;
  case tile_ghoul: return coord(8, 0); break;
  case tile_mummy: return coord(9, 0); break;
  case tile_deepOne: return coord(10, 0); break;
  case tile_armor: return coord(0, 1); break;
  case tile_potion: return coord(2, 1); break;
  case tile_ammo: return coord(4, 1); break;
  case tile_scroll: return coord(8, 1); break;
  case tile_elderSign: return coord(10, 1); break;
  case tile_rat: return coord(1, 2); break;
  case tile_spider: return coord(2, 2); break;
  case tile_wolf: return coord(3, 2); break;
  case tile_phantasm: return coord(4, 2); break;
  case tile_ratThing: return coord(5, 2); break;
  case tile_hound: return coord(6, 2); break;
  case tile_bat: return coord(7, 2); break;
  case tile_byakhee: return coord(8, 2); break;
  case tile_massOfWorms: return coord(9, 2); break;
  case tile_ooze: return coord(10, 2); break;
  case tile_fireVampire: return coord(11, 2); break;
  case tile_ghost: return coord(12, 2); break;
  case tile_wraith: return coord(13, 2); break;
  case tile_pistol: return coord(1, 3); break;
  case tile_tommyGun: return coord(3, 3); break;
  case tile_shotgun: return coord(4, 3); break;
  case tile_dynamite: return coord(5, 3); break;
  case tile_dynamiteLit: return coord(6, 3); break;
  case tile_molotov: return coord(7, 3); break;
  case tile_incinerator: return coord(8, 3); break;
  case tile_teslaCannon: return coord(9, 3); break;
  case tile_flare: return coord(10, 3); break;
  case tile_flareGun: return coord(11, 3); break;
  case tile_flareLit: return coord(12, 3); break;
  case tile_dagger: return coord(0, 4); break;
  case tile_axe: return coord(9, 4); break;
  case tile_club: return coord(10, 4); break;
  case tile_hammer: return coord(11, 4); break;
  case tile_machete: return coord(12, 4); break;
  case tile_pitchfork: return coord(13, 4); break;
  case tile_sledgeHammer: return coord(14, 4); break;
  case tile_rock: return coord(15, 4); break;
  case tile_ironSpike: return coord(16, 4); break;
  case tile_huntingHorror: return coord(4, 5); break;
  case tile_spiderLeng: return coord(10, 5); break;
  case tile_migo: return coord(12, 5); break;
  case tile_floor: return coord(1, 6); break;
  case tile_aimMarkerHead: return coord(2, 6); break;
  case tile_aimMarkerTrail: return coord(3, 6); break;
  case tile_wall: return coord(5, 6); break;
  case tile_wallFront: return coord(3, 7); break;
  case tile_squareCheckered: return coord(5, 6); break;
  case tile_rubbleHigh: return coord(7, 6); break;
  case tile_rubbleLow: return coord(8, 6); break;
  case tile_stairsDown: return coord(9, 6); break;
  case tile_brazier: return coord(11, 6); break;
  case tile_altar: return coord(12, 6); break;
  case tile_spiderWeb: return coord(13, 6); break;
  case tile_doorClosed: return coord(14, 6); break;
  case tile_doorOpen: return coord(15, 6); break;
  case tile_doorBroken: return coord(16, 6); break;
  case tile_tree: return coord(1, 7); break;
  case tile_bush: return coord(2, 7); break;
  case tile_churchBench: return coord(4, 7); break;
  case tile_graveStone: return coord(5, 7); break;
  case tile_tomb: return coord(14, 5); break;
  case tile_water1: return coord(7, 7); break;
  case tile_water2: return coord(8, 7); break;
  case tile_trapGeneral: return coord(9, 7); break;
  case tile_cocoon: return coord(10, 7); break;
  case tile_blastAnimation1: return coord(11, 7); break;
  case tile_blastAnimation2: return coord(12, 7); break;
  case tile_corpse: return coord(13, 7); break;
  case tile_projectileStandardFrontSlash: return coord(15, 7); break;
  case tile_projectileStandardBackSlash: return coord(16, 7); break;
  case tile_projectileStandardDash: return coord(17, 7); break;
  case tile_projectileStandardVerticalBar: return coord(18, 7); break;
  case tile_gore1: return coord(0, 8); break;
  case tile_gore2: return coord(1, 8); break;
  case tile_gore3: return coord(2, 8); break;
  case tile_gore4: return coord(3, 8); break;
  case tile_gore5: return coord(4, 8); break;
  case tile_gore6: return coord(5, 8); break;
  case tile_gore7: return coord(6, 8); break;
  case tile_gore8: return coord(7, 8); break;
  case tile_smoke: return coord(9, 8); break;
  case tile_trapezohedron: return coord(11, 8); break;
  case tile_pit: return coord(12, 8); break;
  case tile_lever_left: return coord(13, 8); break;
  case tile_lever_right: return coord(14, 8); break;
  case tile_empty: return coord(18, 8); break;
  default: return coord(18, 8); break;
  }
}


coord Art::getGlyphCoords(const char glyph, Engine* engine) {
  coord c = getColumnAndRow(glyph);

  c.x *= engine->config->CELL_W;
  c.y *= engine->config->CELL_H;

  return c;
}

coord Art::getTileCoords(const Tile_t tile, Engine* engine) {
  coord c = getColumnAndRowTile(tile);

  c.x *= engine->config->CELL_W;
  c.y *= engine->config->CELL_H;

  return c;
}
