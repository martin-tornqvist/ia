#ifndef AI_CAST_RANDOM_SPELL_H
#define AI_CAST_RANDOM_SPELL_H

#include "Log.h"

class AI_castRandomSpellIfAware {
public:
  static bool action(Monster* monsterActing, Engine* engine) {
    if(monsterActing->deadState == actorDeadState_alive) {
      if(monsterActing->playerAwarenessCounter > 0) {
        if(monsterActing->spellCoolDownCurrent == 0) {

          if(monsterActing->spellsKnown.empty() == false) {
            vector<Spell*> spellCandidates = monsterActing->spellsKnown;

            while(spellCandidates.empty() == false) {
              const unsigned int ELEMENT =
                engine->dice.range(0, spellCandidates.size() - 1);

              Spell* const spell = spellCandidates.at(ELEMENT);

              if(spell->isGoodForMonsterToCastNow(monsterActing, engine)) {
                const int CUR_SPI = monsterActing->getSpi();
                const int SPELL_MAX_SPI =
                  spell->getMaxSpiCost(false, monsterActing, engine);

                // Cast spell if max spirit cost is lower than current spirit,
                if(SPELL_MAX_SPI < CUR_SPI) {
                  spell->cast(monsterActing, true, engine);
                  return true;
                }

                const int CUR_HP  = monsterActing->getHp();
                const int MAX_HP  = monsterActing->getHpMax(true);

                // Cast spell with a certain chance if HP is low.
                if(CUR_HP < MAX_HP / 3 && engine->dice.percentile() <= 5) {
                  if(engine->player->checkIfSeeActor(*monsterActing, NULL)) {
                    engine->log->addMessage(
                      monsterActing->getNameThe() + " looks desperate.");
                  }
                  spell->cast(monsterActing, true, engine);
                  return true;
                }
                return false;
              } else {
                spellCandidates.erase(spellCandidates.begin() + ELEMENT);
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
