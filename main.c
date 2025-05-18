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
 * @file main.c
 * @brief Entry point for the Audio§ application.
 *
 * Handles command-line argument parsing, loads the input WAV file,
 * and calls the silence detection engine.
 *
 * @author Patrick
 * @date 2025
 * @version 0.1 preview
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h> 
#include "audioss.h"
#include "external/wav16.h"

void export_to_json(const SilenceResult* result, const char *output_file) {
    json_object *jarray = json_object_new_array();

    for (size_t i = 0; i < result->count; i++) {
        json_object *jobj = json_object_new_object();
        json_object_object_add(jobj, "channel", json_object_new_int(result->events[i].channel));
        json_object_object_add(jobj, "silence_start", json_object_new_double(result->events[i].silence_start));
        json_object_object_add(jobj, "silence_end", json_object_new_double(result->events[i].silence_end));
        json_object_object_add(jobj, "silence_duration", json_object_new_double(result->events[i].silence_duration));
        json_object_array_add(jarray, jobj);
    }

    FILE *file = fopen(output_file, "w");
    if (file) {
        fprintf(file, "%s\n", json_object_to_json_string_ext(jarray, JSON_C_TO_STRING_PRETTY));
        fclose(file);
    } else {
        fprintf(stderr, "Error opening file for writing.\n");
    }

    json_object_put(jarray);  
}

void export_to_txt(const SilenceResult* result, const char *output_file) {
    FILE *file = fopen(output_file, "w");
    if (file) {
        for (size_t i = 0; i < result->count; i++) {
            fprintf(file, "%d %.3f %.3f %.3f\n", 
                    result->events[i].channel, result->events[i].silence_start, result->events[i].silence_end, result->events[i].silence_duration);
        }
        fclose(file);
    } else {
        fprintf(stderr, "Error opening file for writing.\n");
    }
}

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <wav_file> [options]\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -threshold <int>          Silence threshold (default: 500)\n");
    fprintf(stderr, "  -min_duration <float>     Minimum silence duration in seconds (default: 1.0)\n");
    fprintf(stderr, "  -output_format <txt|json> Output format (default: txt)\n");
    fprintf(stderr, "  -output_file <name>       Output file (default: output.txt)\n");
    fprintf(stderr, "  -verbose <0|1>            Verbosity (default: 1)\n");
    fprintf(stderr, "  -h, --help                Show this help message\n");
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *filename = NULL;

    int threshold = 500;
    float min_duration = 1.0f;
    int verbose = 1;
    const char *output_format = NULL;
    const char *output_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-threshold") == 0 && i + 1 < argc) {
            threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-min_duration") == 0 && i + 1 < argc) {
            min_duration = atof(argv[++i]);
        } else if (strcmp(argv[i], "-output_format") == 0 && i + 1 < argc) {
            output_format = argv[++i];
        } else if (strcmp(argv[i], "-output_file") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-verbose") == 0 && i + 1 < argc) {
            verbose = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-' && filename == NULL) {
            filename = argv[i];
        } else {
            fprintf(stderr, "Unknown argument or missing value: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "Missing input WAV file.\n");
        print_usage(argv[0]);
        return 1;
    }

    WavAudio16 *audio = load_wav16(filename);
    if (!audio) {
        fprintf(stderr, "Error loading WAV file.\n");
        return 1;
    }

    if (verbose) {
        printf("File loaded: %s\n", filename);
        printf("Sample rate: %u Hz\n", audio->sample_rate);
        printf("Number of channels: %u\n", audio->num_channels);
        printf("Total number of samples: %zu\n", audio->total_samples);
        printf("Detection parameters:\n");
        printf("  - Threshold: %d\n", threshold);
        printf("  - Minimum duration: %.3f seconds\n", min_duration);
        printf("  - Output format: %s\n", output_format);
        printf("  - Output file: %s\n", output_file);
    }

    SilenceResult silence_result = detect_silence(audio, threshold, min_duration, verbose);
    
    if (output_file != NULL && strcmp(output_file, "") != 0) {
        if (strcmp(output_format, "json") == 0) {
            export_to_json(&silence_result, output_file);
        } else {
            export_to_txt(&silence_result, output_file);
        }
    }

    free_silence_result(&silence_result);
    free_wav16(audio);

    return 0;
}
