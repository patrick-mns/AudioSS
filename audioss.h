#ifndef AUDIOSS_H
#define AUDIOSS_H

#include <stdint.h>
#include <stdlib.h>
#include "external/wav16.h"

typedef struct {
    double silence_start;
    double silence_end;
    double silence_duration;
    int channel;
} SilenceEvent;

typedef struct {
    SilenceEvent* events; 
    size_t count; 
    size_t max_events;
} SilenceResult;

typedef struct {
    WavAudio16* audio;
    int threshold;
    float min_duration;
    SilenceResult* res;
    int channel;
    int verbose;
    pthread_mutex_t* mutex;
} SilenceThreadData;

SilenceResult detect_silence(const WavAudio16* audio, int threshold, float min_duration, int verbose);
void free_silence_result(SilenceResult* result);

#endif // AUDIOSS_H
    