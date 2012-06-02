#ifndef PHRASES_H
#define PHRASES_H

#include <iostream>
#include <vector>
#include <string>

enum PhraseSets_t {phraseSet_silent, phraseSet_cultist, phraseSet_zombie};

class Engine;

class BasicUtils;
class Actor;

using namespace std;

class Phrases
{
public:
	Phrases(Engine* engine) : eng(engine) {makeList();}

	string* getRandomAggroPhrase(Actor* const actorSpeaking);

	string* getAggroPhraseFromPhraseSet(const PhraseSets_t phraseSet);

private:
	void makeList();

	struct PhraseSet
	{
		vector<string> m_aggroPhrases;
	};

	PhraseSet m_phraseSets[5];

	Engine* eng;
};


#endif
