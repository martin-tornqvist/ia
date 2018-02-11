#include "gfx.hpp"

#include "rl_utils.hpp"

namespace gfx
{

P character_pos(const char character)
{
    switch (character)
    {
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
    case 1: return P(14, 5);
    case 2: return P(0, 6);
    case 3: return P(1, 6);
    case 4: return P(2, 6);
    case 5: return P(3, 6);
    case 6: return P(4, 6);
    case 7: return P(5, 6);
    case 8: return P(6, 6);
    case 9: return P(7, 6);
    case 10: return P(8, 6);

    default: return P(-1, -1);
    }
}

} // gfx
