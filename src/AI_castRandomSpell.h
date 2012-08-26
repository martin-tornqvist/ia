#ifndef AI_CAST_RANDOM_SPELL_H
#define AI_CAST_RANDOM_SPELL_H

class AI_castRandomSpellIfAware {
public:
	static bool action(Monster* monsterActing, Engine* engine) {
		if(monsterActing->deadState == actorDeadState_alive) {
			if(monsterActing->playerAwarenessCounter > 0) {
				if(monsterActing->spellCoolDownCurrent == 0) {

					vector<Spell*>* spellsKnown = &(monsterActing->spellsKnown);

					const unsigned int NR_OF_SPELLS = spellsKnown->size();

					if(NR_OF_SPELLS > 0) {

						vector<unsigned int> indexes;
						for(unsigned int i = 0; i < NR_OF_SPELLS; i++) {
							indexes.push_back(i);
						}

						while(indexes.empty() == false) {
							//ii is a random index of the vector of indexes
							const unsigned int ii = engine->dice(1, indexes.size()) - 1;
							//i is an index of the spell list
							const unsigned int i = indexes.at(ii);

							if(spellsKnown->at(i)->isGoodForMonsterNow(monsterActing, engine)) {
								spellsKnown->at(i)->monsterCast(monsterActing, engine);
								return true;
							} else {
								indexes.erase(indexes.begin() + ii);
							}
						}
					}
				}
			}
		}
		return false;
	}
};

#endif
