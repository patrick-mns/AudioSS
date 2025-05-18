#define DR_WAV_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include "dr_wav.h"
#include "wav16.h"

WavAudio16* load_wav16(const char* filename) {

    drwav wav;
    if (!drwav_init_file(&wav, filename, NULL)) {
        fprintf(stderr, "Failed to load WAV file: %s\n", filename);
        return NULL;
    }

    if (wav.bitsPerSample != 16) {
        fprintf(stderr, "Error: Only 16-bit PCM WAV files are supported. This file has %d bits per sample.\n", wav.bitsPerSample);
        drwav_uninit(&wav);
        return NULL;
    }

    WavAudio16* audio = malloc(sizeof(WavAudio16));
    if (!audio) {
        fprintf(stderr, "Memory allocation failed\n");
        drwav_uninit(&wav);
        return NULL;
    }

    audio->sample_rate = wav.sampleRate;
    audio->num_channels = wav.channels;
    audio->total_samples = wav.totalPCMFrameCount;

    audio->samples = malloc(wav.totalPCMFrameCount * wav.channels * sizeof(int16_t));
    if (!audio->samples) {
        fprintf(stderr, "Memory allocation failed for samples\n");
        free(audio);
        drwav_uninit(&wav);
        return NULL;
    }

    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, audio->samples);

    drwav_uninit(&wav);
    return audio;
}

void free_wav16(WavAudio16* audio) {
    if (audio) {
        free(audio->samples);
        free(audio);
    }
}
