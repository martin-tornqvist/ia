#ifndef PLAYER_H
#define PLAYER_H

#include "Actor.h"

#include "ConstTypes.h"

//const int HUNGER_LEVEL_FULL             = 1400;
//const int HUNGER_LEVEL_FIRST_WARNING    = 400;
//const int HUNGER_LEVEL_FINAL_WARNING    = 200;

enum InsanityPhobias_t
{
	insanityPhobia_rat,
	insanityPhobia_spider,
	insanityPhobia_dog,
	insanityPhobia_undead,
	insanityPhobia_openPlace,
	insanityPhobia_closedPlace,
	insanityPhobia_deepPlaces,
	endOfInsanityPhobias
};

enum InsanityCompulsions_t
{
	insanityCompulsion_sadism,
	insanityCompulsion_masochism,
	endOfInsanityCompulsions
};

class Monster;

class Player: public Actor {
public:
	Player();
	~Player() {}

	void FOVupdate();

	void moveDirection(const coord dir) {
		moveDirection(dir.x, dir.y);
	}
	void moveDirection(const int X_DIR, int Y_DIR);

	void actorSpecific_init() {}

	void actorSpecific_spawnStartItems();

	void act();

	void registerHeardSound(const Sound& sound);

	void explosiveThrown();

	int firstAidTurnsLeft;
	int waitTurnsLeft;

	int insanityLong, insanityShort, insanityShortTemp;

	int arcaneKnowledge;

	void shock(const ShockValues_t shockValue, const int MODIFIER);
	void incrInsanityLong();

	int dynamiteFuseTurns;
	int molotovFuseTurns;
	int flareFuseTurns;

	void addSaveLines(vector<string>& lines) const;
	void setParametersFromSaveLines(vector<string>& lines);

	bool insanityPhobias[endOfInsanityPhobias];
	bool insanityCompulsions[endOfInsanityCompulsions];

	const Actor* target;

	void autoMelee();

	void kick();

	void updateColor();

private:
	//void decreaseHungerByNumber(const int satiate);
	//int hungerLevel;

	void attemptIdentifyItems();
	void testPhobias();
	void actorSpecific_hit(const int DMG);
	void FOVhack();
	void queryInterruptActions();
	friend class MessageLog;
	bool isStandingInOpenPlace() const;
};


#endif
