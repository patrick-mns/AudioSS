#ifndef WAV16_H
#define WAV16_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t sample_rate;
    uint16_t num_channels;
    uint16_t bits_per_sample;
    int16_t *samples;
    size_t total_samples;
} WavAudio16;

WavAudio16* load_wav16(const char* filename);
void free_wav16(WavAudio16* audio);

#endif // WAV16_H