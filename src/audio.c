#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "audio.h"

ALuint buffers[6];
ALuint sources[6];

typedef struct audio
{
    char path[256];
} Audio;

int load_sound(const char *filename, ALuint *buffer)
{
    SF_INFO sf_info;
    SNDFILE *sndfile = sf_open(filename, SFM_READ, &sf_info);
    if (!sndfile)
    {
        fprintf(stderr, "Could not open audio file %s\n", filename);
        return 0;
    }

    ALenum format;
    if (sf_info.channels == 1)
    {
        format = AL_FORMAT_MONO16;
    }
    else if (sf_info.channels == 2)
    {
        format = AL_FORMAT_STEREO16;
    }
    else
    {
        sf_close(sndfile);
        fprintf(stderr, "Unsupported channel count: %d\n", sf_info.channels);
        return 0;
    }

    short *membuf = (short *)malloc(sf_info.frames * sf_info.channels * sizeof(short));
    sf_readf_short(sndfile, membuf, sf_info.frames);
    sf_close(sndfile);

    alGenBuffers(1, buffer);
    alBufferData(*buffer, format, membuf, sf_info.frames * sf_info.channels * sizeof(short), sf_info.samplerate);

    free(membuf);
    return 1;
}

int init_openal()
{
    ALCdevice *device = alcOpenDevice(NULL);
    if (!device)
    {
        fprintf(stderr, "Could not open OpenAL device\n");
        return 0;
    }

    ALCcontext *context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(context))
    {
        fprintf(stderr, "Could not set OpenAL context\n");
        return 0;
    }

    alGenBuffers(5, buffers);

    Audio audio_path[6];
    strcpy(audio_path[0].path, "example_game/assets/market_audio.wav");
    strcpy(audio_path[1].path, "example_game/assets/forest_audio.wav");
    strcpy(audio_path[2].path, "example_game/assets/cave_audio.wav");
    strcpy(audio_path[3].path, "example_game/assets/witch_speaking_audio.wav");
    strcpy(audio_path[4].path, "example_game/assets/castle_audio.wav");
    strcpy(audio_path[5].path, "src/assets/start_audio.wav");

    char current_path[256];
    if (getcwd(current_path, sizeof(current_path)) != NULL)
    {
        for (int i = 0; i < 6; i++)
        {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_path, audio_path[i].path);
            if (!load_sound(full_path, &buffers[i]))
            {
                return 0;
            }
        }
    }

    // if (!load_sound("example_game/assets/market_audio.wav", &buffers[0]) ||
    //     !load_sound("example_game/assets/forest_audio.wav", &buffers[1]) ||
    //     !load_sound("example_game/assets/cave_audio.wav", &buffers[2]) ||
    //     !load_sound("example_game/assets/witch_speaking_audio.wav", &buffers[3]) ||
    //     !load_sound("example_game/assets/castle_audio.wav", &buffers[4]) ||
    //     !load_sound("src/assets/start_audio.wav", &buffers[5]))
    // {
    //     return 0;
    // }

    alGenSources(6, sources);

    for (int i = 0; i < 6; i++)
    {
        alSourcei(sources[i], AL_BUFFER, buffers[i]);
        alSourcef(sources[i], AL_PITCH, 1);
        alSourcef(sources[i], AL_GAIN, 1);
        alSource3f(sources[i], AL_POSITION, 0, 0, 0);
        alSource3f(sources[i], AL_VELOCITY, 0, 0, 0);
        alSourcei(sources[i], AL_LOOPING, AL_TRUE); // Enable looping
    }

    return 1;
}

void cleanup_openal()
{
    alDeleteSources(6, sources);
    alDeleteBuffers(6, buffers);
    ALCcontext *context = alcGetCurrentContext();
    ALCdevice *device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void play_sound(int index)
{
    if (index < 0 || index >= 6)
    {
        printf("Invalid sound index: %d\n", index);
        return;
    }
    alSourcePlay(sources[index]);
}

void stop_sound(int index)
{
    if (index < 0 || index >= 6)
    {
        printf("Invalid sound index: %d\n", index);
        return;
    }
    alSourceStop(sources[index]);
}