#ifndef AUDIO_H
#define AUDIO_H

#include <string>

class Engine;

//#include "fmod.h"
//#include "fmod.hpp"

using namespace std;

enum Audio_t
{
	audio_none,
	audio_pistol_fire,
	audio_pistol_reload,
	audio_shotgunPump_fire,
	audio_shotgun_load_shell,
	audio_tommygun_fire,
	audio_explosion,
	audio_molotovExplosion,
	//audio_shotgunDouble,
	//audio_tommyGun,
	end_of_audio_t
};

class Audio
{
public:
	Audio(Engine* engine) : eng(engine) {
		init();
	}

	~Audio() {
		cleanup();
	}

	void updateSystem();

void playSound(const Audio_t audio);

private:
	//FMOD::System *system;
	//FMOD::Channel* channel;

	//FMOD::Sound* soundList[end_of_audio_t - 1];

	void createSound(const Audio_t audio, const string file);

	void init();
	void cleanup();

	Engine* eng;
};

#endif