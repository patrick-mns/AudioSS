/*
 * Audio§ - Audio Silence Segmentation
 * ------------------------------------
 * Open-source tool for detecting silence in .wav audio files.
 *
 * Copyright (c) 2025 Patrick <patrick-mns@hotmail.com>
 *
 * This file is part of the Audio§ project.
 * See the LICENSE file for licensing information.
 */

/**
 * @file audioss.c
 * @brief Implements silence detection logic for Audio§.
 *
 * Contains the core algorithm for detecting silent regions in PCM WAV files.
 * 
 * @author Patrick
 * @date 2025
 * @version 0.1 preview
 */

#include "audioss.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define MAX_EVENTS 256

static void record_silence_event(SilenceThreadData* data, size_t silence_start, size_t silence_end) {
    WavAudio16* audio = data->audio;
    int min_samples = (int)ceil(data->min_duration * audio->sample_rate);

    size_t dur = silence_end - silence_start;
    if ((int)dur < min_samples) return;

    double silence_start_sec = (double)silence_start / audio->sample_rate;
    double silence_end_sec = (double)silence_end / audio->sample_rate;
    double silence_duration = silence_end_sec - silence_start_sec;

    if (data->verbose) {
        printf("[Audio§ silencedetect @ %p] [channel: %i] silence_start: %f | silence_end: %f | silence_duration: %f\n",
               (void*)data, data->channel, silence_start_sec, silence_end_sec, silence_duration);
    }

    pthread_mutex_lock(data->mutex);
    if (data->res->count >= data->res->max_events) {
        size_t new_max = data->res->max_events * 2;
        if (new_max <= data->res->max_events) {
            pthread_mutex_unlock(data->mutex);
            return; 
        }
        SilenceEvent* tmp = realloc(data->res->events, sizeof(SilenceEvent) * new_max);
        if (!tmp) {
            pthread_mutex_unlock(data->mutex);
            return; 
        }
        data->res->events = tmp;
        data->res->max_events = new_max;
    }
    data->res->events[data->res->count++] = (SilenceEvent){
        .silence_start = silence_start_sec,
        .silence_end = silence_end_sec,
        .silence_duration = silence_duration,
        .channel = data->channel
    };
    pthread_mutex_unlock(data->mutex);
}

void* detect_silence_channel(void* arg) {
    SilenceThreadData* data = (SilenceThreadData*)arg;
    WavAudio16* audio = data->audio;
    int threshold = data->threshold;

    size_t frames = audio->total_samples / audio->num_channels;
    int in_silence = 0;
    size_t silence_start = 0;

    for (size_t i = 0; i < frames; i++) {
        int16_t sample = audio->samples[i * audio->num_channels + data->channel];
        if (abs(sample) <= threshold) {
            if (!in_silence) {
                in_silence = 1;
                silence_start = i;
            }
        } else if (in_silence) {
            record_silence_event(data, silence_start, i);
            in_silence = 0;
        }
    }
    if (in_silence) {
        record_silence_event(data, silence_start, frames);
    }

    return NULL;
}

SilenceResult detect_silence(const WavAudio16* audio, int threshold, float min_duration, int verbose) {

    SilenceResult res = { .events = NULL, .count = 0, .max_events = MAX_EVENTS };
    if (!audio || threshold < 0 || min_duration <= 0.0f) return res;

    res.events = malloc(sizeof(SilenceEvent) * res.max_events);

    pthread_t* threads = malloc(audio->num_channels * sizeof(pthread_t));
    SilenceThreadData* thread_data = malloc(audio->num_channels * sizeof(SilenceThreadData));

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    for (int ch = 0; ch < audio->num_channels; ch++) {
        thread_data[ch] = (SilenceThreadData){
            .audio = (WavAudio16*)audio,
            .threshold = threshold,
            .min_duration = min_duration,
            .res = &res,
            .channel = ch,
            .verbose = verbose,
            .mutex = &mutex
        };
        pthread_create(&threads[ch], NULL, detect_silence_channel, &thread_data[ch]);
    }

    for (int ch = 0; ch < audio->num_channels; ch++) {
        pthread_join(threads[ch], NULL);
    }

    pthread_mutex_destroy(&mutex);
    free(threads);
    free(thread_data);
    return res;
}

void free_silence_result(SilenceResult* result) {
    if (!result) return;
    free(result->events);
    result->events = NULL;
    result->count = 0;
} 