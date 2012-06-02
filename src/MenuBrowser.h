#ifndef MENU_BROWSER_H
#define MENU_BROWSER_H


/*
 * This class is responsible for handling and storing menu positions,
 * and for changing the position when receiving a command to do so.
 *
 * Menu-using applications creates an instance of this class to keep
 * track of which menu-position is selected, and to use it to communicate
 * with the keyboard input handling class for menu's.
 *
 */

#include "ConstTypes.h"
#include "SDL/SDL.h"

class MenuBrowser {
public:
	MenuBrowser(int nrItemsFirst, int nrItemsSecond) :
		upLeft(coord(0, 0)), upRight(nrItemsSecond == 0 ? coord(0, 0) : coord(1, 0)), btmLeft(coord(0, nrItemsFirst - 1)), btmRight(
				nrItemsSecond == 0 ? coord(0, nrItemsFirst - 1) : coord(1, nrItemsSecond - 1)), pos(upLeft), NR_ITEMS_FIRST(
				nrItemsFirst), NR_ITEMS_SECOND(nrItemsSecond) {
	}

	MenuBrowser& operator=(const MenuBrowser& other) {
		(void)other;
		return *this;
	}

	void navigate(const Directions_t direction);
	void navigate(const char KEY);

	Uint16 enter() const {
		return static_cast<Uint16> (SDLK_a + pos.y + pos.x * NR_ITEMS_FIRST);
	}

	int getNrOfItemsInFirstList() const {
		return NR_ITEMS_FIRST;
	}
	int getNrOfItemsInSecondList() const {
		return NR_ITEMS_SECOND;
	}
	coord getPos() const {
		return pos;
	}

	bool isPosAtKey(const char KEY) const;

private:
	const coord upLeft;
	const coord upRight;
	const coord btmLeft;
	const coord btmRight;
	coord pos;
	const int NR_ITEMS_FIRST;
	const int NR_ITEMS_SECOND;
};

#endif
