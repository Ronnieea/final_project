#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>

int load_sound(const char *filename, ALuint *buffer);
int init_openal();
void cleanup_openal();
void play_sound(int index);
void stop_sound(int index);

#endif