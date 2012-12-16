#ifndef COLORS_H
#define COLORS_H

#include <SFML/Graphics/Color.hpp>

//STANDARD TERMINAL COLORS (somewhat modified...)
//===========================================================
const sf::Color clrBlack(0, 0, 0);
const sf::Color clrGray(128, 128, 128);
const sf::Color clrWhite(192, 192, 192);
const sf::Color clrWhiteHigh(255, 255, 255);

const sf::Color clrRed(128, 0, 0);
const sf::Color clrRedLight(255, 0, 0); //(220, 0, 0);

const sf::Color clrGreen(0, 128, 0);
const sf::Color clrGreenLight(0, 255, 0);

const sf::Color clrYellow(255, 255, 0);
const sf::Color clrBlue(0, 0, 139);
const sf::Color clrBlueLight(92, 92, 255);

const sf::Color clrMagenta(139, 0, 139);
const sf::Color clrMagentaLight(255, 0, 255);

const sf::Color clrCyan(0, 128, 128);
const sf::Color clrCyanLight(0, 255, 255);

const sf::Color clrBrown(153, 102, 61);
//===========================================================

const sf::Color	clrBrownDark(96, 64, 32);

const sf::Color clrMessageGood = clrGreenLight;
const sf::Color clrMessageBad = clrRedLight;


#endif
