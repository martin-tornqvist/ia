#ifndef PLAYER_BONUSES_H
#define PLAYER_BONUSES_H

#include <string>
#include <vector>

#include "AbilityValues.h"
#include "Converters.h"

#include <math.h>

using namespace std;

class Engine;

class PlayerBonusHandler;
class PlayerBonusGroup;

class PlayerBonus {
public:
	PlayerBonus(Abilities_t ability, string groupTitle, string title, string description, Engine* engine, int startSkill, int bon1 = 0, int bon2 = 0, int bon3 = 0,
	            int bon4 = 0, int bon5 = 0);

	PlayerBonus() {
	}

	~PlayerBonus() {
	}

	int getRank() const {
		return rank_;
	}

	int getRankLimit() const {
		return abilityBonusAtRanks_.size() - 1;
	}

	void incrRank() {
		rank_ = min(getRankLimit(), rank_ + 1);
	}

	const string& getTitleGroup() const {
		return titleGroup_;
	}

	const string& getTitle() const {
		return title_;
	}

	const vector<string>& getDescription() const {
		return description_;
	}

	bool isPickable() const {
		return rank_ < getRankLimit() && !picked_;
	}

	Abilities_t getAbility() const {
		return ability_;
	}

	int getAbilityBonus() const {
		return abilityBonusAtRanks_.at(rank_);
	}

protected:
	friend class PlayerBonusHandler;
	int rank_;
	bool picked_;
	Abilities_t ability_;
	string titleGroup_;
	string title_;
	vector<string> description_;
	vector<int> abilityBonusAtRanks_;
};

class PlayerBonusHandler {
public:
	PlayerBonusHandler(Engine* eng);

	void addSaveLines(vector<string>& lines) {
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			lines.push_back(intToString(bonuses_.at(i).getRank()));
		}
	}

	void setParametersFromSaveLines(vector<string>& lines) {
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			bonuses_.at(i).rank_ = stringToInt(lines.front());
			lines.erase(lines.begin());
		}
	}

	void setAllToUnpicked() {
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			bonuses_.at(i).picked_ = false;
		}
	}

	unsigned int getNrOfBonuses() const {
		return bonuses_.size();
	}

	int getBonusAbilityModifier(const Abilities_t ability) {
		int modifier = 0;
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			if(bonuses_.at(i).getAbility() == ability) {
				modifier += bonuses_.at(i).getAbilityBonus();
			}
		}
		return modifier;
	}

	string getBonusGroupTitleAt(const unsigned int ELEMENT) {
		const PlayerBonus& bonus = getBonusAt(ELEMENT);
		return bonus.getTitleGroup();
	}

	string getBonusTitleAt(const unsigned int ELEMENT) {
		const PlayerBonus& bonus = getBonusAt(ELEMENT);
		return bonus.getTitle();
	}

	int getBonusRankAt(const unsigned int ELEMENT) {
		const PlayerBonus& bonus = getBonusAt(ELEMENT);
		return bonus.getRank();
	}

	void increaseBonusAt(const unsigned int ELEMENT) {
		getBonusAt(ELEMENT).incrRank();
		getBonusAt(ELEMENT).picked_ = true;
	}

	void increaseBonus(const Abilities_t ability) {
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			if(bonuses_.at(i).getAbility() == ability) {
				bonuses_.at(i).incrRank();
			}
		}
	}

	bool isBonusPickableAt(const unsigned int ELEMENT) {
		return getBonusAt(ELEMENT).isPickable();
	}

	int getBonusRankLimitAt(const unsigned int ELEMENT) {
		return getBonusAt(ELEMENT).getRankLimit();
	}

	const vector<string>& getBonusDescriptionAt(const unsigned int ELEMENT) {
		return getBonusAt(ELEMENT).getDescription();
	}

	bool canAnyBonusBePicked() {
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			if(isBonusPickableAt(i)) {
				return true;
			}
		}
		return false;
	}

	void setAllBonusesToMax() {
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			bonuses_.at(i).rank_ = bonuses_.at(i).getRankLimit();
		}
	}

private:
	bool isPicked_;

	PlayerBonus& getBonusAt(const unsigned int ELEMENT) {
		return bonuses_.at(ELEMENT);
	}

	vector<PlayerBonus> bonuses_;
};

#endif
