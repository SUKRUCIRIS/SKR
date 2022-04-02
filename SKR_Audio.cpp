#include "SKR_Audio.h"
#include "SoLoud\soloud.h"
#include "SoLoud\soloud_wav.h"

SoLoud::Soloud core;
SoLoud::Wav sound;

void SKR_AudioInit(void) {
	core.init();
}

void SKR_AudioQuit(void) {
	core.deinit();
}

unsigned int SKR_PlayAudio(const char* filename, float volume, unsigned char loop, unsigned char fadein, double fadeintime) {
	sound.load(filename);
	if (loop) {
		sound.setLooping(1);
	}
	else {
		sound.setLooping(0);
	}
	sound.setVolume(volume);
	unsigned int handle;
	if (!fadein) {
		handle = core.play(sound, volume);
	}
	else {
		handle = core.play(sound, 0, 0, 0);
		core.fadeVolume(handle, volume, fadeintime);
	}
	return handle;
}

void SKR_SetVolume(unsigned int handle, float volume) {
	core.setVolume(handle, volume);
	return;
}

float SKR_GetVolume(unsigned int handle) {
	return core.getVolume(handle);
}

void SKR_FadeOutAudio(unsigned int handle, double fadeouttime) {
	core.fadeVolume(handle, 0, fadeouttime);
	return;
}

void SKR_SetGlobalVolume(float volume) {
	core.setGlobalVolume(volume);
	return;
}

float SKR_GetGlobalVolume(void) {
	return core.getGlobalVolume();
}

void SKR_SetLoop(unsigned int handle, unsigned char loop) {
	if (loop) {
		core.setLooping(handle, 1);
	}
	else {
		core.setLooping(handle, 0);
	}
	return;
}