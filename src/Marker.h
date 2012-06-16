#ifndef MARKER_H
#define MARKER_H

#include <iostream>
#include <vector>

#include "SDL/SDL.h"

#include "ConstTypes.h"

using namespace std;

enum MarkerTask_t {markerTask_aim,
                   markerTask_look,
                   markerTask_throw,
                   markerTask_throwLitExplosive
                  };


class Engine;
class Actor;

class Marker
{
public:
	Marker(Engine* engine) : eng(engine) {}

	void place(const MarkerTask_t markerTask);

	int getX() const {return m_xPos;}
	int getY() const {return m_yPos;}

private:
	coord m_lastKnownPlayerPos;

	coord getClosestPos(const coord c, const vector<coord>& positions) const;
	void setCoordToClosestEnemyIfVisible();
	bool setCoordToTargetIfVisible();

	void drawMarker(const MarkerTask_t markerTask) const;
	void readKeys(const MarkerTask_t markerTask);
	void move(const int xDif, const int yDif, const MarkerTask_t markerTask);
	void cancel();
	void done();

	bool        m_done;

	int         m_xPos;
	int         m_yPos;

	SDL_Event   m_event;
	Uint8*      m_keystates;

	Engine*     eng;
};


#endif
