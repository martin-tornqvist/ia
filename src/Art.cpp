#include "Art.h"

#include "Utils.h"

namespace Art
{

namespace
{

Pos getColumnAndRowGlyph(const char glyph)
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

Pos getColumnAndRowTile(const TileId tile)
{
    switch (tile)
    {
    case TileId::playerFirearm:               return Pos(0, 0);
    case TileId::playerMelee:                 return Pos(1, 0);
    case TileId::zombieUnarmed:               return Pos(2, 0);
    case TileId::zombieArmed:                 return Pos(3, 0);
    case TileId::zombieBloated:               return Pos(4, 0);
    case TileId::cultistFirearm:              return Pos(5, 0);
    case TileId::cultistDagger:               return Pos(6, 0);
    case TileId::witchOrWarlock:              return Pos(7, 0);
    case TileId::ghoul:                       return Pos(8, 0);
    case TileId::mummy:                       return Pos(9, 0);
    case TileId::deepOne:                     return Pos(10, 0);
    case TileId::shadow:                      return Pos(11, 0);
    case TileId::lengElder:                   return Pos(12, 0);
    case TileId::theDarkOne:                  return Pos(13, 0);
    case TileId::armor:                       return Pos(0, 1);
    case TileId::potion:                      return Pos(2, 1);
    case TileId::ammo:                        return Pos(4, 1);
    case TileId::scroll:                      return Pos(8, 1);
    case TileId::elderSign:                   return Pos(10, 1);
    case TileId::chestClosed:                 return Pos(11, 1);
    case TileId::chestOpen:                   return Pos(12, 1);
    case TileId::amulet:                      return Pos(14, 1);
    case TileId::ring:                        return Pos(15, 1);
    case TileId::electricLantern:             return Pos(16, 1);
    case TileId::medicalBag:                  return Pos(17, 1);
    case TileId::mask:                        return Pos(18, 1);
    case TileId::rat:                         return Pos(1, 2);
    case TileId::spider:                      return Pos(2, 2);
    case TileId::wolf:                        return Pos(3, 2);
    case TileId::phantasm:                    return Pos(4, 2);
    case TileId::ratThing:                    return Pos(5, 2);
    case TileId::hound:                       return Pos(6, 2);
    case TileId::bat:                         return Pos(7, 2);
    case TileId::byakhee:                     return Pos(8, 2);
    case TileId::massOfWorms:                 return Pos(9, 2);
    case TileId::ooze:                        return Pos(10, 2);
    case TileId::gasSpore:                    return Pos(11, 2);
    case TileId::polyp:                       return Pos(7, 5);
    case TileId::fungi:                       return Pos(8, 5);
    case TileId::vortex:                      return Pos(5, 5);
    case TileId::ghost:                       return Pos(12, 2);
    case TileId::wraith:                      return Pos(13, 2);
    case TileId::mantis:                      return Pos(14, 2);
    case TileId::locust:                      return Pos(15, 2);
    case TileId::migoArmor:                   return Pos(17, 2);
    case TileId::pistol:                      return Pos(1, 3);
    case TileId::tommyGun:                    return Pos(3, 3);
    case TileId::shotgun:                     return Pos(4, 3);
    case TileId::dynamite:                    return Pos(5, 3);
    case TileId::dynamiteLit:                 return Pos(6, 3);
    case TileId::molotov:                     return Pos(7, 3);
    case TileId::incinerator:                 return Pos(8, 3);
    case TileId::migoGun:                     return Pos(9, 3);
    case TileId::flare:                       return Pos(10, 3);
    case TileId::flareGun:                    return Pos(11, 3);
    case TileId::flareLit:                    return Pos(12, 3);
    case TileId::dagger:                      return Pos(0, 4);
    case TileId::crowbar:                     return Pos(3, 4);
    case TileId::axe:                         return Pos(9, 4);
    case TileId::club:                        return Pos(10, 4);
    case TileId::hammer:                      return Pos(11, 4);
    case TileId::machete:                     return Pos(12, 4);
    case TileId::pitchfork:                   return Pos(13, 4);
    case TileId::sledgeHammer:                return Pos(14, 4);
    case TileId::rock:                        return Pos(15, 4);
    case TileId::ironSpike:                   return Pos(16, 4);
    case TileId::pharaohStaff:                return Pos(17, 4);
    case TileId::lockpick:                    return Pos(17, 4);
    case TileId::huntingHorror:               return Pos(4, 5);
    case TileId::spiderLeng:                  return Pos(10, 5);
    case TileId::chthonian:                   return Pos(0, 5);
    case TileId::migo:                        return Pos(12, 5);
    case TileId::floor:                       return Pos(1, 6);
    case TileId::aimMarkerHead:               return Pos(2, 6);
    case TileId::aimMarkerTrail:              return Pos(3, 6);
    case TileId::wallTop:                     return Pos(5, 6);
    case TileId::wallFront:                   return Pos(3, 7);
    case TileId::squareCheckered:             return Pos(5, 6);
    case TileId::rubbleHigh:                  return Pos(7, 6);
    case TileId::rubbleLow:                   return Pos(8, 6);
    case TileId::stairsDown:                  return Pos(9, 6);
    case TileId::brazier:                     return Pos(11, 6);
    case TileId::altar:                       return Pos(12, 6);
    case TileId::web:                         return Pos(13, 6);
    case TileId::doorClosed:                  return Pos(14, 6);
    case TileId::doorOpen:                    return Pos(15, 6);
    case TileId::doorBroken:                  return Pos(16, 6);
    case TileId::fountain:                    return Pos(18, 6);
    case TileId::tree:                        return Pos(1, 7);
    case TileId::bush:                        return Pos(2, 7);
    case TileId::churchBench:                 return Pos(4, 7);
    case TileId::graveStone:                  return Pos(5, 7);
    case TileId::tombOpen:                    return Pos(13, 5);
    case TileId::tombClosed:                  return Pos(14, 5);
    case TileId::water1:                      return Pos(7, 7);
    case TileId::water2:                      return Pos(8, 7);
    case TileId::trapGeneral:                 return Pos(17, 5);
    case TileId::cocoonOpen:                  return Pos(4, 6);
    case TileId::cocoonClosed:                return Pos(10, 7);
    case TileId::blast1:                      return Pos(11, 7);
    case TileId::blast2:                      return Pos(12, 7);
    case TileId::corpse:                      return Pos(13, 7);
    case TileId::corpse2:                     return Pos(14, 7);
    case TileId::projectileStdFrontSlash:     return Pos(15, 7);
    case TileId::projectileStdBackSlash:      return Pos(16, 7);
    case TileId::projectileStdDash:           return Pos(17, 7);
    case TileId::projectileStdVerticalBar:    return Pos(18, 7);
    case TileId::gore1:                       return Pos(0, 8);
    case TileId::gore2:                       return Pos(1, 8);
    case TileId::gore3:                       return Pos(2, 8);
    case TileId::gore4:                       return Pos(3, 8);
    case TileId::gore5:                       return Pos(4, 8);
    case TileId::gore6:                       return Pos(5, 8);
    case TileId::gore7:                       return Pos(6, 8);
    case TileId::gore8:                       return Pos(7, 8);
    case TileId::smoke:                       return Pos(9, 8);
    case TileId::wallFrontAlt1:               return Pos(1, 9);
    case TileId::wallFrontAlt2:               return Pos(0, 9);
    case TileId::trapezohedron:               return Pos(11, 8);
    case TileId::pit:                         return Pos(12, 8);
    case TileId::leverRight:                  return Pos(13, 8);
    case TileId::leverLeft:                   return Pos(14, 8);
    case TileId::device1:                     return Pos(15, 8);
    case TileId::device2:                     return Pos(16, 8);
    case TileId::cabinetClosed:               return Pos(3, 9);
    case TileId::cabinetOpen:                 return Pos(4, 9);
    case TileId::pillarBroken:                return Pos(5, 9);
    case TileId::pillar:                      return Pos(6, 9);
    case TileId::pillarCarved:                return Pos(7, 9);
    case TileId::barrel:                      return Pos(8, 9);
    case TileId::sarcophagus:                 return Pos(9, 9);
    case TileId::caveWallFront:               return Pos(10, 9);
    case TileId::caveWallTop:                 return Pos(11, 9);
    case TileId::egyptWallFront:              return Pos(16, 9);
    case TileId::egyptWallTop:                return Pos(17, 9);
    case TileId::monolith:                    return Pos(18, 9);
    case TileId::popupTopL:                   return Pos(0, 10);
    case TileId::popupTopR:                   return Pos(1, 10);
    case TileId::popupBtmL:                   return Pos(2, 10);
    case TileId::popupBtmR:                   return Pos(3, 10);
    case TileId::popupHor:                    return Pos(4, 10);
    case TileId::popupVer:                    return Pos(5, 10);
    case TileId::hangbridgeVer:               return Pos(6, 10);
    case TileId::hangbridgeHor:               return Pos(7, 10);
    case TileId::scorchedGround:              return Pos(8, 10);
    case TileId::popupHorDown:                return Pos(9, 10);
    case TileId::popupHorUp:                  return Pos(10, 10);
    case TileId::popupVerR:                   return Pos(11, 10);
    case TileId::popupVerL:                   return Pos(12, 10);
    case TileId::stalagmite:                  return Pos(13, 10);
    case TileId::empty:
    default:                                  return Pos(18, 10);
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
