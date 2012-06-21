#ifndef DUNGEON_MASTER_H
#define DUNGEON_MASTER_H

#include <iostream>
#include <math.h>

#include "ConstDungeonSettings.h"
#include "AbilityValues.h"
#include "Colors.h"
#include "Converters.h"

class PlayerAllocStats;

class Engine;

class DungeonMaster
{
public:
	DungeonMaster(Engine* engine) : eng(engine) {
		init();
	}
	~DungeonMaster() {
	}

	int getXp() const {
		return playerExp;
	}

	int getLevel() const {
		return playerLvl;
	}

	int getXpToNextLvl() const {
		return expTable[playerLvl];
	}

	int getXpToNextLvlAtLvl(int lvl) const {
		return expTable[lvl];
	}

	void monsterKilled(Actor* monster);

	void playerGainsExp(int exp);

	void playerGainsXpPercent(const int PERCENT, const int PLUS_XP) {
		const int XP_GAINED = PLUS_XP + static_cast<int>((static_cast<float>(PERCENT)/100) * static_cast<float>(playerExp));
		playerGainsExp(XP_GAINED);
	}

	void playerLoseXpPercent(const int PERCENT) {
		playerExp = static_cast<int>((static_cast<float>(100-PERCENT)/100) * static_cast<float>(playerExp));
	}

	void addSaveLines(vector<string>& lines) const;
	void setParametersFromSaveLines(vector<string>& lines);

	void winGame();

private:
	void init();

	int playerExp;
	int playerLvl;

	void initExpTable();

	int expTable[PLAYER_MAX_LEVEL + 1];

	Engine* const eng;
};

#endif
