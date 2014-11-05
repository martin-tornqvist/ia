#include "Art.h"

#include "Utils.h"

namespace Art
{

namespace
{

Pos getColumnAndRowGlyph(const char glyph)
{
  switch(glyph)
  {
    default:  return Pos(-1, -1);  break;
    case ' ': return Pos(0, 0);  break;
    case '!': return Pos(1, 0); break;
    case '"': return Pos(2, 0); break;
    case '#': return Pos(3, 0); break;
    case '%': return Pos(4, 0); break;
    case '&': return Pos(5, 0); break;
    case  39: return Pos(6, 0); break;
    case '(': return Pos(7, 0); break;
    case ')': return Pos(8, 0); break;
    case '*': return Pos(9, 0); break;
    case '+': return Pos(10, 0); break;
    case ',': return Pos(11, 0); break;
    case '-': return Pos(12, 0); break;
    case '.': return Pos(13, 0); break;
    case '/': return Pos(14, 0); break;
    case '0': return Pos(15, 0); break;
    case '1': return Pos(0, 1); break;
    case '2': return Pos(1, 1); break;
    case '3': return Pos(2, 1); break;
    case '4': return Pos(3, 1); break;
    case '5': return Pos(4, 1); break;
    case '6': return Pos(5, 1); break;
    case '7': return Pos(6, 1); break;
    case '8': return Pos(7, 1); break;
    case '9': return Pos(8, 1); break;
    case ':': return Pos(9, 1); break;
    case ';': return Pos(10, 1); break;
    case '<': return Pos(11, 1); break;
    case '=': return Pos(12, 1); break;
    case '>': return Pos(13, 1); break;
    case '?': return Pos(14, 1); break;
    case '@': return Pos(15, 1); break;
    case 'A': return Pos(0, 2); break;
    case 'B': return Pos(1, 2); break;
    case 'C': return Pos(2, 2); break;
    case 'D': return Pos(3, 2); break;
    case 'E': return Pos(4, 2); break;
    case 'F': return Pos(5, 2); break;
    case 'G': return Pos(6, 2); break;
    case 'H': return Pos(7, 2); break;
    case 'I': return Pos(8, 2); break;
    case 'J': return Pos(9, 2); break;
    case 'K': return Pos(10, 2); break;
    case 'L': return Pos(11, 2); break;
    case 'M': return Pos(12, 2); break;
    case 'N': return Pos(13, 2); break;
    case 'O': return Pos(14, 2); break;
    case 'P': return Pos(15, 2); break;
    case 'Q': return Pos(0, 3); break;
    case 'R': return Pos(1, 3); break;
    case 'S': return Pos(2, 3); break;
    case 'T': return Pos(3, 3); break;
    case 'U': return Pos(4, 3); break;
    case 'V': return Pos(5, 3); break;
    case 'W': return Pos(6, 3); break;
    case 'X': return Pos(7, 3); break;
    case 'Y': return Pos(8, 3); break;
    case 'Z': return Pos(9, 3); break;
    case '[': return Pos(10, 3); break;
    case  92: return Pos(11, 3);  break;
    case ']': return Pos(12, 3);  break;
    case '^': return Pos(13, 3);  break;
    case '_': return Pos(14, 3);  break;
    case '`': return Pos(15, 3);  break;
    case 'a': return Pos(0, 4);  break;
    case 'b': return Pos(1, 4);  break;
    case 'c': return Pos(2, 4);  break;
    case 'd': return Pos(3, 4);  break;
    case 'e': return Pos(4, 4);  break;
    case 'f': return Pos(5, 4);  break;
    case 'g': return Pos(6, 4);  break;
    case 'h': return Pos(7, 4);  break;
    case 'i': return Pos(8, 4);  break;
    case 'j': return Pos(9, 4);  break;
    case 'k': return Pos(10, 4);  break;
    case 'l': return Pos(11, 4);  break;
    case 'm': return Pos(12, 4);  break;
    case 'n': return Pos(13, 4);  break;
    case 'o': return Pos(14, 4);  break;
    case 'p': return Pos(15, 4);  break;
    case 'q': return Pos(0, 5);  break;
    case 'r': return Pos(1, 5);  break;
    case 's': return Pos(2, 5);  break;
    case 't': return Pos(3, 5);  break;
    case 'u': return Pos(4, 5);  break;
    case 'v': return Pos(5, 5);  break;
    case 'w': return Pos(6, 5);  break;
    case 'x': return Pos(7, 5);  break;
    case 'y': return Pos(8, 5);  break;
    case 'z': return Pos(9, 5);  break;
    case '{': return Pos(10, 5);  break;
    case '|': return Pos(11, 5);  break;
    case '}': return Pos(12, 5);  break;
    case '~': return Pos(13, 5);  break;
    case   1: return Pos(14, 5);  break;
    case   2: return Pos(0, 6);  break;
    case   3: return Pos(1, 6);  break;
    case   4: return Pos(2, 6);  break;
    case   5: return Pos(3, 6); break;
    case   6: return Pos(4, 6); break;
    case   7: return Pos(5, 6); break;
    case   8: return Pos(6, 6); break;
    case   9: return Pos(7, 6); break;
    case  10: return Pos(8, 6); break;
  }
}

Pos getColumnAndRowTile(const TileId tile)
{
  switch(tile)
  {
    case TileId::playerFirearm: return Pos(0, 0); break;
    case TileId::playerMelee: return Pos(1, 0); break;
    case TileId::zombieUnarmed: return Pos(2, 0); break;
    case TileId::zombieArmed: return Pos(3, 0); break;
    case TileId::zombieBloated: return Pos(4, 0); break;
    case TileId::cultistFirearm: return Pos(5, 0); break;
    case TileId::cultistDagger: return Pos(6, 0); break;
    case TileId::witchOrWarlock: return Pos(7, 0); break;
    case TileId::ghoul: return Pos(8, 0); break;
    case TileId::mummy: return Pos(9, 0); break;
    case TileId::deepOne: return Pos(10, 0); break;
    case TileId::shadow: return Pos(11, 0); break;
    case TileId::lengElder: return Pos(12, 0); break;
    case TileId::armor: return Pos(0, 1); break;
    case TileId::potion: return Pos(2, 1); break;
    case TileId::ammo: return Pos(4, 1); break;
    case TileId::scroll: return Pos(8, 1); break;
    case TileId::elderSign: return Pos(10, 1); break;
    case TileId::chestClosed: return Pos(11, 1); break;
    case TileId::chestOpen: return Pos(12, 1); break;
    case TileId::electricLantern: return Pos(16, 1); break;
    case TileId::medicalBag: return Pos(17, 1); break;
    case TileId::mask: return Pos(18, 1); break;
    case TileId::rat: return Pos(1, 2); break;
    case TileId::spider: return Pos(2, 2); break;
    case TileId::wolf: return Pos(3, 2); break;
    case TileId::phantasm: return Pos(4, 2); break;
    case TileId::ratThing: return Pos(5, 2); break;
    case TileId::hound: return Pos(6, 2); break;
    case TileId::bat: return Pos(7, 2); break;
    case TileId::byakhee: return Pos(8, 2); break;
    case TileId::massOfWorms: return Pos(9, 2); break;
    case TileId::ooze: return Pos(10, 2); break;
    case TileId::polyp: return Pos(7, 5); break;
    case TileId::vortex: return Pos(5, 5); break;
    case TileId::ghost: return Pos(12, 2); break;
    case TileId::wraith: return Pos(13, 2); break;
    case TileId::mantis: return Pos(14, 2); break;
    case TileId::locust: return Pos(15, 2); break;
    case TileId::pistol: return Pos(1, 3); break;
    case TileId::tommyGun: return Pos(3, 3); break;
    case TileId::shotgun: return Pos(4, 3); break;
    case TileId::dynamite: return Pos(5, 3); break;
    case TileId::dynamiteLit: return Pos(6, 3); break;
    case TileId::molotov: return Pos(7, 3); break;
    case TileId::incinerator: return Pos(8, 3); break;
    case TileId::teslaCannon: return Pos(9, 3); break;
    case TileId::flare: return Pos(10, 3); break;
    case TileId::flareGun: return Pos(11, 3); break;
    case TileId::flareLit: return Pos(12, 3); break;
    case TileId::dagger: return Pos(0, 4); break;
    case TileId::crowbar: return Pos(3, 4); break;
    case TileId::axe: return Pos(9, 4); break;
    case TileId::club: return Pos(10, 4); break;
    case TileId::hammer: return Pos(11, 4); break;
    case TileId::machete: return Pos(12, 4); break;
    case TileId::pitchfork: return Pos(13, 4); break;
    case TileId::sledgeHammer: return Pos(14, 4); break;
    case TileId::rock: return Pos(15, 4); break;
    case TileId::ironSpike: return Pos(16, 4); break;
    case TileId::pharaohStaff: return Pos(17, 4); break;
    case TileId::lockpick: return Pos(17, 4); break;
    case TileId::huntingHorror: return Pos(4, 5); break;
    case TileId::spiderLeng: return Pos(10, 5); break;
    case TileId::chthonian: return Pos(0, 5); break;
    case TileId::migo: return Pos(12, 5); break;
    case TileId::floor: return Pos(1, 6); break;
    case TileId::aimMarkerHead: return Pos(2, 6); break;
    case TileId::aimMarkerTrail: return Pos(3, 6); break;
    case TileId::wallTop: return Pos(5, 6); break;
    case TileId::wallFront: return Pos(3, 7); break;
    case TileId::squareCheckered: return Pos(5, 6); break;
    case TileId::rubbleHigh: return Pos(7, 6); break;
    case TileId::rubbleLow: return Pos(8, 6); break;
    case TileId::stairsDown: return Pos(9, 6); break;
    case TileId::brazier: return Pos(11, 6); break;
    case TileId::altar: return Pos(12, 6); break;
    case TileId::web: return Pos(13, 6); break;
    case TileId::doorClosed: return Pos(14, 6); break;
    case TileId::doorOpen: return Pos(15, 6); break;
    case TileId::doorBroken: return Pos(16, 6); break;
    case TileId::fountain: return Pos(18, 6); break;
    case TileId::tree: return Pos(1, 7); break;
    case TileId::bush: return Pos(2, 7); break;
    case TileId::churchBench: return Pos(4, 7); break;
    case TileId::graveStone: return Pos(5, 7); break;
    case TileId::tomb: return Pos(14, 5); break;
    case TileId::water1: return Pos(7, 7); break;
    case TileId::water2: return Pos(8, 7); break;
    case TileId::trapGeneral: return Pos(17, 5); break;
    case TileId::cocoon: return Pos(10, 7); break;
    case TileId::blast1: return Pos(11, 7); break;
    case TileId::blast2: return Pos(12, 7); break;
    case TileId::corpse: return Pos(13, 7); break;
    case TileId::corpse2: return Pos(14, 7); break;
    case TileId::projectileStdFrontSlash: return Pos(15, 7); break;
    case TileId::projectileStdBackSlash: return Pos(16, 7); break;
    case TileId::projectileStdDash: return Pos(17, 7); break;
    case TileId::projectileStdVerticalBar: return Pos(18, 7); break;
    case TileId::gore1: return Pos(0, 8); break;
    case TileId::gore2: return Pos(1, 8); break;
    case TileId::gore3: return Pos(2, 8); break;
    case TileId::gore4: return Pos(3, 8); break;
    case TileId::gore5: return Pos(4, 8); break;
    case TileId::gore6: return Pos(5, 8); break;
    case TileId::gore7: return Pos(6, 8); break;
    case TileId::gore8: return Pos(7, 8); break;
    case TileId::smoke: return Pos(9, 8); break;
    case TileId::wallFrontAlt1: return Pos(1, 9); break;
    case TileId::wallFrontAlt2: return Pos(0, 9); break;
    case TileId::trapezohedron: return Pos(11, 8); break;
    case TileId::pit: return Pos(12, 8); break;
    case TileId::leverRight: return Pos(13, 8); break;
    case TileId::leverLeft: return Pos(14, 8); break;
    case TileId::device1: return Pos(15, 8); break;
    case TileId::device2: return Pos(16, 8); break;
    case TileId::cabinetClosd: return Pos(3, 9); break;
    case TileId::cabinetOpen: return Pos(4, 9); break;
    case TileId::pillarBroken: return Pos(5, 9); break;
    case TileId::pillar: return Pos(6, 9); break;
    case TileId::pillarCarved: return Pos(7, 9); break;
    case TileId::barrel: return Pos(8, 9); break;
    case TileId::sarcophagus: return Pos(9, 9); break;
    case TileId::caveWallFront: return Pos(10, 9); break;
    case TileId::caveWallTop: return Pos(11, 9); break;
    case TileId::egyptWallFront: return Pos(16, 9); break;
    case TileId::egyptWallTop: return Pos(17, 9); break;
    case TileId::popupTopL: return Pos(0, 10); break;
    case TileId::popupTopR: return Pos(1, 10); break;
    case TileId::popupBtmL: return Pos(2, 10); break;
    case TileId::popupBtmR: return Pos(3, 10); break;
    case TileId::popupHor: return Pos(4, 10); break;
    case TileId::popupVer: return Pos(5, 10); break;
    case TileId::popupHorDown: return Pos(9, 10); break;
    case TileId::popupHorUp: return Pos(10, 10); break;
    case TileId::popupVerR: return Pos(11, 10); break;
    case TileId::popupVerL: return Pos(12, 10); break;
    case TileId::hangbridgeVer: return Pos(6, 10); break;
    case TileId::hangbridgeHor: return Pos(7, 10); break;
    case TileId::scorchedGround: return Pos(8, 10); break;
    case TileId::empty:
    default: return Pos(18, 10); break;
  }
}

}

Pos getGlyphPos(const char glyph)
{
  return getColumnAndRowGlyph(glyph);
}

Pos getTilePos(const TileId tile)
{
  return getColumnAndRowTile(tile);
}

} //Art
