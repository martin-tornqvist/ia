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
	PlayerBonus(Abilities_t ability, string groupTitle, string title, string descriptionGeneral,
	            Engine* engine, int startSkill, int bon1 = 0, string descrBon1 = "",
	            int bon2 = 0, string descrBon2 = "", int bon3 = 0, string descrBon3 = "");

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

	const vector<string>& getDescriptionGeneral() const {
		return descriptionGeneral_;
	}

	const vector< vector<string> >& getDescriptionRanks() const {
		return descriptionRanks_;
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

	int getAbilityBonusAtRank(const int RANK) const {
		return abilityBonusAtRanks_.at(RANK);
	}

protected:
	friend class PlayerBonusHandler;
	int rank_;
	bool picked_;
	Abilities_t ability_;
	string titleGroup_, title_;
	vector< vector<string> > descriptionRanks_;
	vector<string> descriptionGeneral_;
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

	int getBonusAbilityModifierAtRank(const Abilities_t ability, const int RANK) {
		int modifier = 0;
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			if(bonuses_.at(i).getAbility() == ability) {
				modifier += bonuses_.at(i).getAbilityBonusAtRank(RANK);
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

	void increaseBonusForAbility(const Abilities_t ability) {
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			if(bonuses_.at(i).getAbility() == ability) {
				bonuses_.at(i).incrRank();
			}
		}
	}

	int getBonusRankForAbility(const Abilities_t ability) {
		for(unsigned int i = 0; i < bonuses_.size(); i++) {
			if(bonuses_.at(i).getAbility() == ability) {
				return bonuses_.at(i).rank_;
			}
		}
		return -1;
	}

	bool isBonusPickableAt(const unsigned int ELEMENT) {
		return getBonusAt(ELEMENT).isPickable();
	}

	int getBonusRankLimitAt(const unsigned int ELEMENT) {
		return getBonusAt(ELEMENT).getRankLimit();
	}

	const vector<string>& getBonusDescriptionGeneralAt(const unsigned int ELEMENT) {
		return getBonusAt(ELEMENT).getDescriptionGeneral();
	}

	const vector< vector<string> >& getBonusDescriptionRanksAt(const unsigned int ELEMENT) {
		return getBonusAt(ELEMENT).getDescriptionRanks();
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
