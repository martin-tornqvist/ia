#ifndef AI_CAST_RANDOM_SPELL_H
#define AI_CAST_RANDOM_SPELL_H

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
                engine->dice.getInRange(0, spellCandidates.size() - 1);

              Spell* const spell = spellCandidates.at(ELEMENT);

              if(spell->isGoodForMonsterToCastNow(monsterActing, engine)) {
                spell->cast(monsterActing, engine);
                return true;
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
