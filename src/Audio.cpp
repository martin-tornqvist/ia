#include "Audio.h"

#include "Engine.h"

void Audio::updateSystem()
{
	//system->update();
}

void Audio::init()
{
	/*
	FMOD::System_Create(&system);
	system->init(32, FMOD_INIT_NORMAL, 0);
	channel = NULL;

	createSound(audio_shotgunPump_fire, "shotgunPump_fire.wav");
	createSound(audio_shotgun_load_shell, "shotgun_load_shell.wav");
	createSound(audio_pistol_fire, "pistol_fire.wav");
	createSound(audio_pistol_reload, "pistol_reload.wav");
	createSound(audio_explosion, "explosion.wav");
	createSound(audio_molotovExplosion, "molotovExplosion.wav");
	createSound(audio_tommygun_fire, "tommygun_fire.wav");
	*/
}

void Audio::createSound(const Audio_t audio, const string file)
{
	(void)audio;
	(void)file;
	/*
	system->createSound(("sound/" + file).data(), FMOD_HARDWARE, 0, &soundList[audio]);
	soundList[audio]->setMode(FMOD_LOOP_OFF);
	*/
}

void Audio::cleanup()
{
	/*
	for(unsigned int i = 1; i < end_of_audio_t; i++) {
		soundList[i]->release();
	}

	system->close();
	system->release();
	*/
}


void Audio::playSound(const Audio_t audio)
{
	(void)audio;
	//system->playSound(FMOD_CHANNEL_FREE, soundList[audio], false, &channel);
}