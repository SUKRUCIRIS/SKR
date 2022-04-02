#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	void SKR_AudioInit(void);

	void SKR_AudioQuit(void);

	unsigned int SKR_PlayAudio(const char* filename, float volume, unsigned char loop, unsigned char fadein, double fadeintime);

	void SKR_SetVolume(unsigned int handle, float volume);

	float SKR_GetVolume(unsigned int handle);

	void SKR_FadeOutAudio(unsigned int handle, double fadeouttime);

	void SKR_SetGlobalVolume(float volume);

	float SKR_GetGlobalVolume(void);

	void SKR_SetLoop(unsigned int handle, unsigned char loop);

#ifdef __cplusplus
}
#endif