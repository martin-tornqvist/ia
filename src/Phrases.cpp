#include "Phrases.h"

#include "Engine.h"

void Phrases::makeList()
{
    vector<string> newPhrases;

    //ZOMBIE AGGRO
    newPhrases.resize(0);
    newPhrases.push_back("More brains!!");
    newPhrases.push_back("Braaains...");
    newPhrases.push_back("Uuuhn...");
    newPhrases.push_back("Uuh...");
    m_phraseSets[phraseSet_zombie].m_aggroPhrases.swap(newPhrases);

    //CULTIST AGGRO
    newPhrases.resize(0);
    newPhrases.push_back("Apigami!");
    newPhrases.push_back("Bhuudesco invisuu!");
    newPhrases.push_back("Bhuuesco marana!");
    newPhrases.push_back("Crudux cruo!");
    newPhrases.push_back("Cruento paashaeximus!");
    newPhrases.push_back("Cruento pestis shatruex!");
    newPhrases.push_back("Cruo crunatus durbe!");
    newPhrases.push_back("Cruo lokemundux!");
    newPhrases.push_back("Cruo-stragaraNa!");
    newPhrases.push_back("Gero shay cruo!");
    newPhrases.push_back("In marana domus-bhaava crunatus!");
    newPhrases.push_back("Caecux infirmux!");
    newPhrases.push_back("Malax sayti!");
    newPhrases.push_back("Marana pallex!");
    newPhrases.push_back("Marana malax!");
    newPhrases.push_back("Pallex ti!");
    newPhrases.push_back("Peroshay bibox malax!");
    newPhrases.push_back("Pestis Cruento!");
    newPhrases.push_back("Pestis cruento vilomaxus pretiacruento!");
    newPhrases.push_back("Pretaanluxis cruonit!");
    newPhrases.push_back("Pretiacruento!");
    newPhrases.push_back("StragarNaya!");
    newPhrases.push_back("Vorox esco marana!");
    newPhrases.push_back("Vilomaxus!");
    newPhrases.push_back("Prostragaranar malachtose!");
    m_phraseSets[phraseSet_cultist].m_aggroPhrases.swap(newPhrases);
}

string* Phrases::getAggroPhraseFromPhraseSet(const PhraseSets_t phraseSet)
{
    //Lookup character types available phrases
    int availablePhrases = m_phraseSets[phraseSet].m_aggroPhrases.size();

    if (availablePhrases != 0) {
        //Randomize a phrase from available ones
        int phraseNr = eng->dice(1, availablePhrases) - 1;

        //Get the phrase
        return &(m_phraseSets[phraseSet].m_aggroPhrases[phraseNr]);
    }
	return NULL;
}

string* Phrases::getRandomAggroPhrase(Actor* const actorSpeaking)
{
    PhraseSets_t phraseSet = actorSpeaking->getInstanceDefinition()->phraseSet;
	return getAggroPhraseFromPhraseSet(phraseSet);
}

