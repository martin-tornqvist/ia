#ifndef KNOCKBACK_H
#define KNOCKBACK_H

#include "constTypes.h"

class Engine;
class Actor;
struct AttackData;

class KnockBack {
public:
	KnockBack(Engine* engine) : eng(engine) {}

	void attemptKnockBack(Actor* const defender, const coord& attackedFromPos, const bool IS_SPIKE_GUN);

private:
	Engine* eng;

};

#endif
